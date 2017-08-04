#include "ZerglingManager.h"
#include "UnitUtil.h"

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
		// find a sunken
		const auto & sunkens = InformationManager::Instance().getUnitset(BWAPI::UnitTypes::Zerg_Sunken_Colony);
		if (sunkens.empty()) return;
		auto center = sunkens.getPosition();
		// find sunken region
		BWTA::BaseLocation * ebase = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
		if (!center.isValid() || ebase == nullptr) return;
		auto path = MapPath::Instance().getPath({ BWAPI::Position(center), ebase->getPosition() });
		if (path.empty())
		{
			MapPath::Instance().insert({ BWAPI::Position(center), ebase->getPosition() });
			return;
		}
		for (int i = 2; i < path.size(); ++i)
		{
			int dist = std::pow(path[i].x - center.x, 2) + std::pow(path[i].y - center.y, 2);
			if (dist >= 9)
			{
				auto targetP = BWAPI::Position(path[i]);
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(center), targetP, BWAPI::Colors::Green);
				BWAPI::Broodwar->drawCircleMap(targetP, 4, BWAPI::Colors::Red, true);

				BWAPI::Unitset nearbyEnemies;
				const BWAPI::Unitset & meleeUnits = getUnits();
				MapGrid::Instance().GetUnits(nearbyEnemies, targetP, 128, false, true);
				if (nearbyEnemies.empty())
				{
					for (auto & meleeUnit : meleeUnits)
					{
						Micro::SmartMove(meleeUnit, targetP);
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
	}
}

void ZerglingManager::executeMicro(const BWAPI::Unitset & targets)
{
	//assignTargetsOld(targets);
	const BWAPI::Unitset & meleeUnits = getUnits();
	BWAPI::Unitset meleeUnitTargets;
	for (auto & target : targets)
	{
		// conditions for targeting
		if (!(target->getType().isFlyer()) &&
			!(target->isLifted()) &&
			!(target->getType() == BWAPI::UnitTypes::Zerg_Larva) &&
			!(target->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
			target->isVisible())
		{
			meleeUnitTargets.insert(target);
		}
	}
	if (meleeUnitTargets.empty())
	{
		for (auto & meleeUnit : meleeUnits)
		{
			if (meleeUnit->getDistance(order.getPosition()) > 100)
			{
				// move to it
				Micro::SmartMove(meleeUnit, order.getPosition());
			}
		}

		return;
	}


	std::vector<int> hpRemainings;

	for (auto & target : meleeUnitTargets)
	{
		hpRemainings.push_back(target->getHitPoints());
	}



	for (auto & meleeUnit : meleeUnits)
	{
		// if the order is to attack or defend
		if (order.getType() == SquadOrderTypes::Attack || order.getType() == SquadOrderTypes::Defend)
		{

			int currentCooldown = meleeUnit->isStartingAttack() ? meleeUnit->getType().groundWeapon().damageCooldown() : meleeUnit->getGroundWeaponCooldown();
			bool coolDown = currentCooldown == 0 ? true : false;
			// run away if we meet the retreat critereon
			if (zerglingUnitShouldRetreat(meleeUnit, targets))
			{
				BWAPI::Position fleeTo(BWAPI::Broodwar->self()->getStartLocation());

				Micro::SmartMove(meleeUnit, fleeTo);
			}
			// if there are targets
			else if (!meleeUnitTargets.empty())
			{
				int highPriority = -1;
				double closestDist = std::numeric_limits<double>::infinity();
				BWAPI::Unit closestTarget = nullptr, attackTarget = nullptr, moveTarget = nullptr;
				double maxDpsHPValue = -1;
				//int range = meleeUnit->getType().groundWeapon().maxRange();
				int idx = 0;
				int choose_idx = 0;
				bool is_attack = false;
				for (auto & unit : meleeUnitTargets)
				{
					if (hpRemainings[idx] <= 0 || !UnitUtil::CanAttack(meleeUnit, unit))
					{
						idx++;
						continue;
					}
					//unit->get
					int range = UnitUtil::GetAttackRange(meleeUnit, unit);
					int priority = getAttackPriority(meleeUnit, unit);
					int distance = meleeUnit->getDistance(unit);
					int attack_distance = (meleeUnit->getPosition().x - unit->getPosition().x)*(meleeUnit->getPosition().x - unit->getPosition().x) +
						(meleeUnit->getPosition().y - unit->getPosition().y)*(meleeUnit->getPosition().y - unit->getPosition().y);

					float damage = BWAPI::UnitTypes::Protoss_Zealot ? 2 * UnitUtil::GetWeapon(unit, meleeUnit).damageAmount()
						: UnitUtil::GetWeapon(unit, meleeUnit).damageAmount();
					//unit->getType() == BWAPI::UnitTypes::Protoss_Zealot ? 2 * unit->getType().groundWeapon().damageAmount() :
					//unit->getType().groundWeapon().damageAmount();


					float dpsHPValue = (float)std::max(0.001f, (float)damage / ((float)unit->getType().groundWeapon().damageCooldown() + 1.0f));
					dpsHPValue = dpsHPValue / (float)unit->getHitPoints();

					// if it's a higher priority, or it's closer, set it
					if (!closestTarget || (priority > highPriority))
					{
						closestDist = distance;
						highPriority = priority;
						closestTarget = unit;
						maxDpsHPValue = -1;

						if (coolDown && attack_distance <= (range)*(range))
						{
							attackTarget = unit;
							choose_idx = idx;
						}


					}
					else if (priority == highPriority && coolDown && attack_distance <= (range)*(range))
					{
						is_attack = true;
						if (dpsHPValue >= maxDpsHPValue)
						{
							closestDist = distance;
							highPriority = priority;
							closestTarget = unit;
							maxDpsHPValue = dpsHPValue;
							attackTarget = unit;
							choose_idx = idx;
						}


					}
					else if (priority == highPriority && attack_distance >(range)*(range) && closestDist>distance)
					{
						closestDist = distance;
						highPriority = priority;
						closestTarget = unit;
						maxDpsHPValue = dpsHPValue;
						moveTarget = unit;

					}
					idx++;
				}
				if (attackTarget)
				{
					Micro::SmartAttackUnit(meleeUnit, attackTarget);
					hpRemainings[choose_idx] -= UnitUtil::GetWeapon(meleeUnit, attackTarget).damageAmount() - meleeUnit->getShields();

					//GetWeapon(BWAPI::Unit attacker, BWAPI::Unit target);

				}
				else
					Micro::SmartAttackUnit(meleeUnit, closestTarget);

				// find the best target for this meleeUnit
				//BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);

				// attack it
				//Micro::SmartAttackUnit(meleeUnit, target);
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (meleeUnit->getDistance(order.getPosition()) > 100)
				{
					// move to it
					Micro::SmartMove(meleeUnit, order.getPosition());
				}
			}
		}

		if (Config::Debug::DrawUnitTargetInfo)
		{
			BWAPI::Broodwar->drawLineMap(meleeUnit->getPosition().x, meleeUnit->getPosition().y,
				meleeUnit->getTargetPosition().x, meleeUnit->getTargetPosition().y, Config::Debug::ColorLineTarget);
		}
	}


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
		type == BWAPI::UnitTypes::Protoss_Reaver ||
		(type.isWorker() && unitNearChokepoint(unit)))
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
