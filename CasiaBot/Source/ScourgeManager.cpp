#include "ScourgeManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

ScourgeManager::ScourgeManager()
{
}

void ScourgeManager::execute(const SquadOrder & inputOrder)
{
	BWAPI::Unitset flyers;
	auto & enemies = InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy());
	for (const auto & enemy : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (enemy->getType().isFlyer())
		{
			flyers.insert(enemy);
		}
	}
	// 要么在家里，要么攻击
	auto newPos = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	for (const auto & flyer : flyers)
	{
		if (flyer->canAttack())
		{
			newPos = flyer->getPosition();
			break;
		}
	}
	auto newOrder = SquadOrder(inputOrder.getType(), newPos, inputOrder.getRadius(), inputOrder.getStatus());
	MicroManager::execute(newOrder);
}

void ScourgeManager::executeMicro(const BWAPI::Unitset & targets)
{
	assignTargetsOld(targets);
}


void ScourgeManager::assignTargetsOld(const BWAPI::Unitset & targets)
{
	const BWAPI::Unitset & MutaliskUnits = getUnits();

	// figure out targets
	BWAPI::Unitset MutaliskUnitTargets;
	std::copy_if(targets.begin(), targets.end(), std::inserter(MutaliskUnitTargets, MutaliskUnitTargets.end()),
		[](BWAPI::Unit u) {
		return u->isVisible() && u->isFlying() && !u->getType().isBuilding();
	});

	for (auto & MutaliskUnit : MutaliskUnits)
	{
		// if the order is to attack or defend
		if (order.getType() == SquadOrderTypes::Attack || order.getType() == SquadOrderTypes::Defend)
		{
			// if there are targets 
			if (!MutaliskUnitTargets.empty())
			{
				// find the best target for this zealot
				BWAPI::Unit target = getTarget(MutaliskUnit, MutaliskUnitTargets);

				if (target && Config::Debug::DrawUnitTargetInfo)
				{
					BWAPI::Broodwar->drawLineMap(MutaliskUnit->getPosition(), MutaliskUnit->getTargetPosition(), BWAPI::Colors::Purple);
				}

				Micro::SmartAttackMove(MutaliskUnit, target->getPosition());

			}
			// if there are no targets
			else
			{
				// move to base
				Micro::SmartAttackMove(MutaliskUnit, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
			}
		}
	}
}

std::pair<BWAPI::Unit, BWAPI::Unit> ScourgeManager::findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets)
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

// get a target for the zealot to attack
BWAPI::Unit ScourgeManager::getTarget(BWAPI::Unit hydraliskUnit, const BWAPI::Unitset & targets)
{
	int bestPriorityDistance = 1000000;
	int bestPriority = 0;

	double bestLTD = 0;

	int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

	for (const auto & target : targets)
	{
		double distance = hydraliskUnit->getDistance(target);
		double LTD = UnitUtil::CalculateLTD(target, hydraliskUnit);
		int priority = getAttackPriority(hydraliskUnit, target);
		bool targetIsThreat = LTD > 0;

		if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
		{
			closestDist = distance;
			highPriority = priority;
			closestTarget = target;
		}
	}

	return closestTarget;
}

// get the attack priority of a type in relation to a zergling
int ScourgeManager::getAttackPriority(BWAPI::Unit hydraliskUnit, BWAPI::Unit target)
{
	BWAPI::UnitType rangedType = hydraliskUnit->getType();
	BWAPI::UnitType targetType = target->getType();

	if (targetType.isFlyer())
	{
		if (targetType.canAttack() && targetType.airWeapon().isValid())
			return targetType.airWeapon().damageAmount();
		else return 1;
	}

	return 0;
}

BWAPI::Unit ScourgeManager::closestrangedUnit(BWAPI::Unit target, std::set<BWAPI::Unit> & hydraliskUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit closest = nullptr;

	for (auto & hydraliskUnit : hydraliskUnitsToAssign)
	{
		double distance = hydraliskUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = hydraliskUnit;
		}
	}

	return closest;
}


// still has bug in it somewhere, use Old version
void ScourgeManager::assignTargetsNew(const BWAPI::Unitset & targets)
{
	const BWAPI::Unitset & hydraliskUnits = getUnits();

	// figure out targets
	BWAPI::Unitset hydraliskUnitTargets;
	std::copy_if(targets.begin(), targets.end(), std::inserter(hydraliskUnitTargets, hydraliskUnitTargets.end()), [](BWAPI::Unit u) { return u->isVisible(); });

	BWAPI::Unitset hydraliskUnitsToAssign(hydraliskUnits);
	std::map<BWAPI::Unit, int> attackersAssigned;

	for (auto & unit : hydraliskUnitTargets)
	{
		attackersAssigned[unit] = 0;
	}

	// keep assigning targets while we have attackers and targets remaining
	while (!hydraliskUnitsToAssign.empty() && !hydraliskUnitTargets.empty())
	{
		auto attackerAssignment = findClosestUnitPair(hydraliskUnitsToAssign, hydraliskUnitTargets);
		BWAPI::Unit & attacker = attackerAssignment.first;
		BWAPI::Unit & target = attackerAssignment.second;

		CAB_ASSERT_WARNING(attacker, "We should have chosen an attacker!");

		if (!attacker)
		{
			break;
		}

		if (!target)
		{
			Micro::SmartAttackMove(attacker, order.getPosition());
			continue;
		}

		if (Config::Micro::KiteWithRangedUnits)
		{
			if (attacker->getType() == BWAPI::UnitTypes::Zerg_Mutalisk)
			{
				Micro::MutaDanceTarget(attacker, target);
			}
			else
			{
				Micro::SmartKiteTarget(attacker, target);
			}
		}
		else
		{
			Micro::SmartAttackUnit(attacker, target);
		}

		// update the number of units assigned to attack the target we found
		int & assigned = attackersAssigned[attackerAssignment.second];
		assigned++;

		// if it's a small / fast unit and there's more than 2 things attacking it already, don't assign more
		if ((target->getType().isWorker() || target->getType() == BWAPI::UnitTypes::Zerg_Zergling) && (assigned > 2))
		{
			hydraliskUnitTargets.erase(target);
		}
		// if it's a building and there's more than 10 things assigned to it already, don't assign more
		else if (target->getType().isBuilding() && (assigned > 10))
		{
			hydraliskUnitTargets.erase(target);
		}

		hydraliskUnitsToAssign.erase(attacker);
	}

	// if there's no targets left, attack move to the order destination
	if (hydraliskUnitTargets.empty())
	{
		for (auto & unit : hydraliskUnitsToAssign)
		{
			if (unit->getDistance(order.getPosition()) > 100)
			{
				// move to it
				Micro::SmartAttackMove(unit, order.getPosition());
			}
		}
	}
}