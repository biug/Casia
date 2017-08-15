#include "ZerglingManager.h"
#include "UnitUtil.h"
#include "BuildingPlacer.h"

using namespace CasiaBot;

ZerglingManager::ZerglingManager()
{

}

void ZerglingManager::execute(const SquadOrder & inputOrder)
{
	if (!InformationManager::Instance().isEncounterRush())
	{
		MicroManager::execute(inputOrder);
	}
	// if being rush, zergling guard
	else
	{
		//BWAPI::Broodwar->printf("being rushed");
		// find defense base
		BWAPI::Unit base = BuildingPlacer::Instance().getDefenseBase();
		if (!base) return;
		auto baseP = base->getPosition();
		// find a sunken
		const auto & sunkens = InformationManager::Instance().getUnitset(BWAPI::UnitTypes::Zerg_Sunken_Colony);
		const auto & creeps = InformationManager::Instance().getUnitset(BWAPI::UnitTypes::Zerg_Creep_Colony);
		BWAPI::Unitset creepsunken;
		creepsunken.insert(sunkens.begin(), sunkens.end());
		creepsunken.insert(creeps.begin(), creeps.end());
		auto center = baseP;
		if (!creepsunken.empty()) center = creepsunken.getPosition();
		// 集结在地堡和基地的中间
		auto groupP = (center + baseP) / 2;

		BWAPI::Unitset nearbyEnemies;
		const BWAPI::Unitset & meleeUnits = getUnits();
		auto meleeCenter = meleeUnits.getPosition();
		MapGrid::Instance().GetUnits(nearbyEnemies, center, 160, false, true);
		if (creepsunken.empty())
		{
			MapGrid::Instance().GetUnits(nearbyEnemies, meleeCenter, 160, false, true);
		}
		if (nearbyEnemies.empty() || meleeCenter.getDistance(groupP) > 480)
		{
			for (auto & meleeUnit : meleeUnits)
			{
				Micro::SmartMove(meleeUnit, BWAPI::Position(groupP));
			}
		}
		else
		{
			for (auto & meleeUnit : meleeUnits)
			{
				Micro::SmartAttackMove(meleeUnit, nearbyEnemies.getPosition());
			}
		}
		return;
	}
}

void ZerglingManager::executeMicro(const BWAPI::Unitset & targets)
{
	assignTargetsOld(targets);
}

void ZerglingManager::assignTargetsOld(const BWAPI::Unitset & targets)
{
	const BWAPI::Unitset & zerglingUnits = getUnits();

	// figure out targets
	BWAPI::Unitset zerglingUnitTargets;
	for (auto & target : targets)
	{
		// conditions for targeting
		if (!(target->getType().isFlyer()) &&
			!(target->isLifted()) &&
			!(target->getType() == BWAPI::UnitTypes::Zerg_Larva) &&
			!(target->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
			target->isVisible())
		{
			zerglingUnitTargets.insert(target);
		}
	}

	// for each zerglingUnit
	for (auto & zerglingUnit : zerglingUnits)
	{
		// if the order is to attack or defend
		if (order.getType() == SquadOrderTypes::Attack || order.getType() == SquadOrderTypes::Defend)
		{
			// run away if we meet the retreat critereon
			if (zerglingUnitShouldRetreat(zerglingUnit, targets))
			{
				BWAPI::Position fleeTo(BWAPI::Broodwar->self()->getStartLocation());

				Micro::SmartMove(zerglingUnit, fleeTo);
			}
			// if there are targets
			else if (!zerglingUnitTargets.empty())
			{
				// find the best target for this zerglingUnit
				BWAPI::Unit target = getTarget(zerglingUnit, zerglingUnitTargets);

				// attack it
				Micro::SmartAttackUnit(zerglingUnit, target);
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (zerglingUnit->getDistance(order.getPosition()) > 100)
				{
					// move to it
					Micro::SmartMove(zerglingUnit, order.getPosition());
				}
			}
		}

		if (Config::Debug::DrawUnitTargetInfo)
		{
			BWAPI::Broodwar->drawLineMap(zerglingUnit->getPosition().x, zerglingUnit->getPosition().y,
				zerglingUnit->getTargetPosition().x, zerglingUnit->getTargetPosition().y, Config::Debug::ColorLineTarget);
		}
	}
}

std::pair<BWAPI::Unit, BWAPI::Unit> ZerglingManager::findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets)
{
	std::pair<BWAPI::Unit, BWAPI::Unit> closestPair(nullptr, nullptr);
	double closestDistance = std::numeric_limits<double>::max();

	for (auto & attacker : attackers)
	{
		BWAPI::Unit target = getTarget(attacker, targets);
		double dist = attacker->getDistance(attacker);

		if (!closestPair.first || (dist < closestDistance))
		{
			closestPair.first = attacker;
			closestPair.second = target;
			closestDistance = dist;
		}
	}

	return closestPair;
}

// get a target for the zerglingUnit to attack
BWAPI::Unit ZerglingManager::getTarget(BWAPI::Unit zerglingUnit, const BWAPI::Unitset & targets)
{
	int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

	// for each target possiblity
	for (auto & unit : targets)
	{
		int priority = getAttackPriority(zerglingUnit, unit);
		int distance = zerglingUnit->getDistance(unit);

		// if it's a higher priority, or it's closer, set it
		if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
		{
			closestDist = distance;
			highPriority = priority;
			closestTarget = unit;
		}
	}

	return closestTarget;
}

// get the attack priority of a type in relation to a zergling
int ZerglingManager::getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	if (attacker->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar
		&& unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
		&& (BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) == 0))
	{
		return 13;
	}

	if (attacker->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar && unit->getType().isWorker())
	{
		return 12;
	}

	// highest priority is something that can attack us or aid in combat
	if (type == BWAPI::UnitTypes::Terran_Bunker)
	{
		return 11;
	}
	else if (type == BWAPI::UnitTypes::Terran_Medic ||
		(type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker()) ||
		type == BWAPI::UnitTypes::Terran_Bunker ||
		type == BWAPI::UnitTypes::Protoss_High_Templar ||
		type == BWAPI::UnitTypes::Protoss_Reaver)
	{
		return 10;
	}
	// next priority is worker
	else if (type.isWorker())
	{
		return 9;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Zerg_Spawning_Pool)
	{
		return 5;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon)
	{
		return 5;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 4;
	}
	else if (type.mineralPrice() > 0)
	{
		return 3;
	}
	// then everything else
	else
	{
		return 1;
	}
}

BWAPI::Unit ZerglingManager::closestzerglingUnit(BWAPI::Unit target, const BWAPI::Unitset & zerglingUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit closest = nullptr;

	for (auto & zerglingUnit : zerglingUnitsToAssign)
	{
		double distance = zerglingUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = zerglingUnit;
		}
	}

	return closest;
}

bool ZerglingManager::zerglingUnitShouldRetreat(BWAPI::Unit zerglingUnit, const BWAPI::Unitset & targets)
{

	// we don't want to retreat the zergling unit if its shields or hit points are above the threshold set in the config file
	// set those values to zero if you never want the unit to retreat from combat individually
	if (zerglingUnit->getShields() > Config::Micro::RetreatMeleeUnitShields || zerglingUnit->getHitPoints() > Config::Micro::RetreatMeleeUnitHP)
	{
		return false;
	}

	// if there is a ranged enemy unit within attack range of this zergling unit then we shouldn't bother retreating since it could fire and kill it anyway
	for (auto & unit : targets)
	{
		int groundWeaponRange = unit->getType().groundWeapon().maxRange();
		if (groundWeaponRange >= 64 && unit->getDistance(zerglingUnit) < groundWeaponRange)
		{
			return false;
		}
	}

	return true;
}


// still has bug in it somewhere, use Old version
void ZerglingManager::assignTargetsNew(const BWAPI::Unitset & targets)
{
	const BWAPI::Unitset & zerglingUnits = getUnits();

	// figure out targets
	BWAPI::Unitset zerglingUnitTargets;
	for (auto & target : targets)
	{
		// conditions for targeting
		if (!(target->getType().isFlyer()) &&
			!(target->isLifted()) &&
			!(target->getType() == BWAPI::UnitTypes::Zerg_Larva) &&
			!(target->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
			target->isVisible())
		{
			zerglingUnitTargets.insert(target);
		}
	}

	BWAPI::Unitset zerglingUnitsToAssign(zerglingUnits);
	std::map<BWAPI::Unit, int> attackersAssigned;

	for (auto & unit : zerglingUnitTargets)
	{
		attackersAssigned[unit] = 0;
	}

	int smallThreshold = 3;
	int bigThreshold = 12;

	// keep assigning targets while we have attackers and targets remaining
	while (!zerglingUnitsToAssign.empty() && !zerglingUnitTargets.empty())
	{
		auto attackerAssignment = findClosestUnitPair(zerglingUnitsToAssign, zerglingUnitTargets);
		BWAPI::Unit & attacker = attackerAssignment.first;
		BWAPI::Unit & target = attackerAssignment.second;

		CAB_ASSERT_WARNING(attacker, "We should have chosen an attacker!");

		if (!attacker)
		{
			break;
		}

		if (!target)
		{
			Micro::SmartMove(attacker, order.getPosition());
			continue;
		}

		Micro::SmartAttackUnit(attacker, target);

		// update the number of units assigned to attack the target we found
		int & assigned = attackersAssigned[attackerAssignment.second];
		assigned++;

		// if it's a small / fast unit and there's more than 2 things attacking it already, don't assign more
		if ((target->getType().isWorker() || target->getType() == BWAPI::UnitTypes::Zerg_Zergling) && (assigned >= smallThreshold))
		{
			zerglingUnitTargets.erase(target);
		}
		// if it's a building and there's more than 10 things assigned to it already, don't assign more
		else if (assigned > bigThreshold)
		{
			zerglingUnitTargets.erase(target);
		}

		zerglingUnitsToAssign.erase(attacker);
	}

	// if there's no targets left, attack move to the order destination
	if (zerglingUnitTargets.empty())
	{
		for (auto & unit : zerglingUnitsToAssign)
		{
			if (unit->getDistance(order.getPosition()) > 100)
			{
				// move to it
				Micro::SmartMove(unit, order.getPosition());
				BWAPI::Broodwar->drawLineMap(unit->getPosition(), order.getPosition(), BWAPI::Colors::Yellow);
			}
		}
	}
}
