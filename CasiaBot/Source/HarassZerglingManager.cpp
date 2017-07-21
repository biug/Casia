#include "HarassZerglingManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

HarassZerglingManager::HarassZerglingManager()
{

}

void HarassZerglingManager::executeMicro(const BWAPI::Unitset & targets)
{
	assignTargetsOld(targets);
}

void HarassZerglingManager::setPattern(bool newPattern)
{
	_isAttackPattern = newPattern;
}

void HarassZerglingManager::assignTargetsOld(const BWAPI::Unitset & targets)
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
	BWAPI::Position ourBaseLocation = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	// for each zerglingUnit
	for (auto & zerglingUnit : zerglingUnits)
	{
		bool flee = false;
		if (!_isAttackPattern && zerglingUnit->getDistance(ourBaseLocation) > 500)
		{
			Micro::SmartMove(zerglingUnit, ourBaseLocation);
			flee = true;
		}

		if (zerglingUnit->getDistance(ourBaseLocation) > 500)
		{
			if (zerglingUnit->getHitPoints() < BWAPI::UnitTypes::Zerg_Zergling.maxHitPoints() * 0.5)
			{
				for (auto & target : targets)
				{
					if (target->getType().groundWeapon() != BWAPI::WeaponTypes::None && !target->getType().isWorker())
					{
						Micro::SmartMove(zerglingUnit, ourBaseLocation);
						flee = true;
						break;
					}
				}
			}
		}

		if (!flee)
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

std::pair<BWAPI::Unit, BWAPI::Unit> HarassZerglingManager::findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets)
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
BWAPI::Unit HarassZerglingManager::getTarget(BWAPI::Unit zerglingUnit, const BWAPI::Unitset & targets)
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
int HarassZerglingManager::getAttackPriority(BWAPI::Unit zerglingUnit, BWAPI::Unit target)
{
	BWAPI::UnitType rangedType = zerglingUnit->getType();
	BWAPI::UnitType targetType = target->getType();

	bool isThreat = targetType.isFlyer() ? false : targetType.groundWeapon() != BWAPI::WeaponTypes::None;

	if (targetType == BWAPI::UnitTypes::Terran_Missile_Turret)
	{
		return 18;
	}

	int priority = 0;
	BWAPI::UnitType type(targetType);
	double hpRatio = (type.maxHitPoints() > 0) ? target->getHitPoints() / type.maxHitPoints() : 1.0;
	//low hp
	priority = (int)((1 - hpRatio) * 10);

	if (zerglingUnit->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())) < 500 || zerglingUnit->getHitPoints() > BWAPI::UnitTypes::Zerg_Zergling.maxHitPoints() * 0.5)
	{
		//Medic
		if (targetType == BWAPI::UnitTypes::Terran_Medic)
		{
			return priority + 15;
		}
		//Science Vessel, Shuttle
		else if (targetType == BWAPI::UnitTypes::Terran_Science_Vessel ||
			targetType == BWAPI::UnitTypes::Protoss_Shuttle)
		{
			return priority + 14;
		}
		//Tank, Reaver, High Templar, Bunker
		else if (targetType == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode ||
			targetType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode ||
			targetType == BWAPI::UnitTypes::Protoss_Reaver ||
			targetType == BWAPI::UnitTypes::Protoss_High_Templar ||
			targetType == BWAPI::UnitTypes::Terran_Bunker
			)
		{
			return priority + 13;
		}
		// next priority is worker
		else if (targetType.isWorker())
		{
			return priority + 9;
		}
		//can attack us
		else if (isThreat)
		{
			return priority + 11;
		}
		// next is special buildings
		else if (targetType == BWAPI::UnitTypes::Zerg_Spawning_Pool ||
			targetType == BWAPI::UnitTypes::Protoss_Pylon)
		{
			return 7;
		}
		// next is buildings that cost
		else if (targetType.gasPrice() > 0 || targetType.mineralPrice() > 0)
		{
			return targetType.gasPrice() / 50 + targetType.mineralPrice() / 100;
		}
		// then everything else
		else
		{
			return 1;
		}
	}
	else
	{
		if (targetType.isWorker())
		{
			return priority + 9;
		}
		// next is special buildings
		else if (targetType == BWAPI::UnitTypes::Zerg_Spawning_Pool ||
			targetType == BWAPI::UnitTypes::Protoss_Pylon)
		{
			return 7;
		}
		// next is buildings that cost
		else if (targetType.gasPrice() > 0 || targetType.mineralPrice() > 0)
		{
			return targetType.gasPrice() / 50 + targetType.mineralPrice() / 100;
		}
		// then everything else
		else
		{
			return 1;
		}
	}

}

BWAPI::Unit HarassZerglingManager::closestzerglingUnit(BWAPI::Unit target, const BWAPI::Unitset & zerglingUnitsToAssign)
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

bool HarassZerglingManager::zerglingUnitShouldRetreat(BWAPI::Unit zerglingUnit, const BWAPI::Unitset & targets)
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
void HarassZerglingManager::assignTargetsNew(const BWAPI::Unitset & targets)
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