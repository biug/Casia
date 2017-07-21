#include "LurkerManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

LurkerManager::LurkerManager()
{
}

void LurkerManager::executeMicro(const BWAPI::Unitset & targets)
{
	const BWAPI::Unitset & lurkers = getUnits();

	// figure out targets
	BWAPI::Unitset lurkerTargets;
	std::copy_if(targets.begin(), targets.end(), std::inserter(lurkerTargets, lurkerTargets.end()),
		[](BWAPI::Unit u){ return u->isVisible() && !u->isFlying(); });

	int lurkerRange = BWAPI::UnitTypes::Zerg_Lurker.groundWeapon().maxRange() - 32;

	// for each lurker
	for (auto & lurker : lurkers)
	{

		bool lurkerNearChokepoint = false;
		for (auto & choke : BWTA::getChokepoints())
		{
			if (choke->getCenter().getDistance(lurker->getPosition()) < 64)
			{
				lurkerNearChokepoint = true;
				break;
			}
		}

		// if the order is to attack or defend
		if (order.getType() == SquadOrderTypes::Attack || order.getType() == SquadOrderTypes::Defend)
		{
			// if there are targets
			if (!lurkerTargets.empty())
			{
				bool flee = false;
				for (auto & target : targets)
				{
					BWAPI::UnitType targetType = target->getType();
					if(targetType == BWAPI::UnitTypes::Terran_Missile_Turret ||
						targetType == BWAPI::UnitTypes::Terran_Science_Vessel ||
						targetType == BWAPI::UnitTypes::Protoss_Photon_Cannon ||
						targetType == BWAPI::UnitTypes::Protoss_Observer ||
						targetType == BWAPI::UnitTypes::Zerg_Spore_Colony ||
						targetType == BWAPI::UnitTypes::Zerg_Overlord)
					{
						if (lurker->getDistance(target) >= target->getType().sightRange() - 32)
						{
							continue;
						}
						if (lurker->canUnburrow()){
							lurker->unburrow();
						}
						BWAPI::Position fleeVector = Micro::GetKiteVector(target, lurker);
						BWAPI::Position moveToPosition(lurker->getPosition() + fleeVector);
						Micro::SmartMove(lurker, moveToPosition);
						flee = true;
						break;
					}
				}
				if (!flee)
				{
					// find the best target for this lurker
					BWAPI::Unit target = getTarget(lurker, lurkerTargets);

					if (target && Config::Debug::DrawUnitTargetInfo)
					{
						BWAPI::Broodwar->drawLineMap(lurker->getPosition(), lurker->getTargetPosition(), BWAPI::Colors::Purple);
					}

					// if we are within attack range, burrow
					if (lurker->getDistance(target) < lurkerRange && lurker->canBurrow() && !lurkerNearChokepoint)
					{
						lurker->burrow();
					}
					// otherwise unburrow and move in
					else if ((!target || lurker->getDistance(target) > lurkerRange + 32) && lurker->canUnburrow())
					{
						lurker->unburrow();
					}

					// if we're in siege mode just attack the target
					if (lurker->isBurrowed())
					{
						Micro::SmartAttackUnit(lurker, target);
					}
					// if we're not in siege mode kite the target
					else
					{
						// move to it
						Micro::SmartMove(lurker, target->getPosition());
					}
				}
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (lurker->getDistance(order.getPosition()) > 160)
				{
					if (lurker->canUnburrow())
					{
						lurker->unburrow();
					}
					else
					{
						// move to it
						Micro::SmartMove(lurker, order.getPosition());
					}
				}
			}
		}
	}
}

// get a target for the zealot to attack
BWAPI::Unit LurkerManager::getTarget(BWAPI::Unit lurker, const BWAPI::Unitset & targets)
{
	int bestPriorityDistance = 1000000;
	int bestPriority = 0;

	double bestLTD = 0;

	BWAPI::Unit bestTargetThreatInRange = nullptr;
	double bestTargetThreatInRangeLTD = 0;

	int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

	int lurkerRange = BWAPI::UnitTypes::Zerg_Lurker.groundWeapon().maxRange() - 32;
	BWAPI::Unitset targetsInSiegeRange;
	for (auto & target : targets)
	{
		if (target->getDistance(lurker) < lurkerRange && UnitUtil::CanAttack(lurker, target))
		{
			targetsInSiegeRange.insert(target);
		}
	}

	const BWAPI::Unitset & newTargets = targetsInSiegeRange.empty() ? targets : targetsInSiegeRange;

	// check first for units that are in range of our attack that can cause damage
	// choose the highest priority one from them at the lowest health
	for (const auto & target : newTargets)
	{
		if (!UnitUtil::CanAttack(lurker, target))
		{
			continue;
		}

		double distance = lurker->getDistance(target);
		double LTD = UnitUtil::CalculateLTD(target, lurker);
		int priority = getAttackPriority(lurker, target);
		bool targetIsThreat = LTD > 0;
		BWAPI::Broodwar->drawTextMap(target->getPosition(), "%d", priority);

		if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
		{
			closestDist = distance;
			highPriority = priority;
			closestTarget = target;
		}
	}

	if (bestTargetThreatInRange)
	{
		return bestTargetThreatInRange;
	}

	return closestTarget;
}

// get the attack priority of a type in relation to a zergling
int LurkerManager::getAttackPriority(BWAPI::Unit rangedUnit, BWAPI::Unit target)
{
	BWAPI::UnitType rangedType = rangedUnit->getType();
	BWAPI::UnitType targetType = target->getType();

	bool isThreat = targetType.isFlyer() ? false : targetType.groundWeapon() != BWAPI::WeaponTypes::None;

	if (target->getType() == BWAPI::UnitTypes::Zerg_Larva || target->getType() == BWAPI::UnitTypes::Zerg_Egg)
	{
		return 0;
	}
	// if the target is building something near our base something is fishy
	BWAPI::Position ourBasePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	if (target->getType() == BWAPI::UnitTypes::Terran_Marine || target->getType() == BWAPI::UnitTypes::Terran_Medic
		|| target->getType() == BWAPI::UnitTypes::Terran_Firebat)
	{
		return 300;
	}
	if (target->getType().isWorker() && (target->isConstructing() || target->isRepairing()) && target->getDistance(ourBasePosition) < 1200)
	{
		return 100;
	}
	if (target->getType().isBuilding() && (target->isCompleted() || target->isBeingConstructed()) && target->getDistance(ourBasePosition) < 1200)
	{
		return 90;
	}

	int priority = 0;
	BWAPI::UnitType type(targetType);
	double hpRatio = (type.maxHitPoints() > 0) ? target->getHitPoints() / type.maxHitPoints() : 1.0; 
	//low hp
	priority = (int)((1 - hpRatio) * 10);

    //Medic
    if (targetType == BWAPI::UnitTypes::Terran_Medic ||
    	targetType == BWAPI::UnitTypes::Terran_SCV)
    {
        return priority + 15;
    }
	//Tank, Reaver, High Templar, Bunker
	else if (targetType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode ||
			targetType == BWAPI::UnitTypes::Protoss_Reaver ||
			targetType == BWAPI::UnitTypes::Protoss_High_Templar ||
			targetType == BWAPI::UnitTypes::Terran_Bunker)
	{
		return priority + 13;
	}
	//Archon
	else if(targetType == BWAPI::UnitTypes::Protoss_Archon)
	{
		return priority + 12;
	}
	// next priority is worker
	else if (targetType.isWorker())
	{
		return 9;
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

BWAPI::Unit LurkerManager::closestrangedUnit(BWAPI::Unit target, std::set<BWAPI::Unit> & rangedUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit closest = nullptr;

	for (auto & rangedUnit : rangedUnitsToAssign)
	{
		double distance = rangedUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = rangedUnit;
		}
	}

	return closest;
}