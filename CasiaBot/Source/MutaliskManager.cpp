#include "MutaliskManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

MutaliskManager::MutaliskManager() 
{ 
}

void MutaliskManager::executeMicro(const BWAPI::Unitset & targets) 
{
	assignTargetsOld(targets);
}


void MutaliskManager::assignTargetsOld(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & mutaliskUnits = getUnits();

	// figure out targets
	BWAPI::Unitset mutaliskUnitTargets;
    std::copy_if(targets.begin(), targets.end(), std::inserter(mutaliskUnitTargets, mutaliskUnitTargets.end()),
     [](BWAPI::Unit u){ return u->isVisible(); });

    for (auto & mutaliskUnit : mutaliskUnits)
	{
		// if the order is to attack or defend
		if (order.getType() == SquadOrderTypes::Attack || order.getType() == SquadOrderTypes::Defend) 
        {
			// if there are targets
			if (!mutaliskUnitTargets.empty())
			{
				// find the best target for this zealot
				BWAPI::Unit target = getTarget(mutaliskUnit, mutaliskUnitTargets);
                
                if (target && Config::Debug::DrawUnitTargetInfo) 
	            {
		            BWAPI::Broodwar->drawLineMap(mutaliskUnit->getPosition(), mutaliskUnit->getTargetPosition(), BWAPI::Colors::Purple);
	            }

				// attack it
                Micro::MutaDanceTarget(mutaliskUnit, target);
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				BWAPI::Position ourBasePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
				if (mutaliskUnit->getDistance(ourBasePosition) > 1000)
				{
					// move to it
					Micro::SmartAttackMove(mutaliskUnit, ourBasePosition);
				}
			}
		}
	}
}

std::pair<BWAPI::Unit, BWAPI::Unit> MutaliskManager::findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets)
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
BWAPI::Unit MutaliskManager::getTarget(BWAPI::Unit mutaliskUnit, const BWAPI::Unitset & targets)
{
	int bestPriorityDistance = 1000000;
    int bestPriority = 0;
    
    double bestLTD = 0;

    int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

    for (const auto & target : targets)
    {
        double distance         = mutaliskUnit->getDistance(target);
        double LTD              = UnitUtil::CalculateLTD(target, mutaliskUnit);
        int priority            = getAttackPriority(mutaliskUnit, target);
        bool targetIsThreat     = LTD > 0;
        
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
int MutaliskManager::getAttackPriority(BWAPI::Unit mutaliskUnit, BWAPI::Unit target) 
{
	BWAPI::UnitType rangedType = mutaliskUnit->getType();
	BWAPI::UnitType targetType = target->getType();

    if (targetType == BWAPI::UnitTypes::Zerg_Larva || targetType == BWAPI::UnitTypes::Zerg_Egg)
    {
        return 0;
    }

    if (targetType == BWAPI::UnitTypes::Protoss_Carrier)
    {
        return 101;
    }

    // if the target is building something near our base something is fishy
    BWAPI::Position ourBasePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
    if (targetType.isWorker() && (target->isConstructing() || target->isRepairing()) && target->getDistance(ourBasePosition) < 1200)
    {
        return 80;
    }
	//可以攻击空中单位
    if (targetType.canAttack() && targetType.airWeapon().isValid())
    {
		if (targetType.isFlyer()) return 95 + targetType.airWeapon().damageAmount();
        else return 90;
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
    else if (targetType.groundWeapon() != BWAPI::WeaponTypes::None)
    {
        return priority + 10;
    }
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

BWAPI::Unit MutaliskManager::closestrangedUnit(BWAPI::Unit target, std::set<BWAPI::Unit> & mutaliskUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit closest = nullptr;

	for (auto & mutaliskUnit : mutaliskUnitsToAssign)
	{
		double distance = mutaliskUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = mutaliskUnit;
		}
	}
	
	return closest;
}


// still has bug in it somewhere, use Old version
void MutaliskManager::assignTargetsNew(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & mutaliskUnits = getUnits();

	// figure out targets
	BWAPI::Unitset mutaliskUnitTargets;
    std::copy_if(targets.begin(), targets.end(), std::inserter(mutaliskUnitTargets, mutaliskUnitTargets.end()), [](BWAPI::Unit u){ return u->isVisible(); });

    BWAPI::Unitset mutaliskUnitsToAssign(mutaliskUnits);
    std::map<BWAPI::Unit, int> attackersAssigned;

    for (auto & unit : mutaliskUnitTargets)
    {
        attackersAssigned[unit] = 0;
    }

    // keep assigning targets while we have attackers and targets remaining
    while (!mutaliskUnitsToAssign.empty() && !mutaliskUnitTargets.empty())
    {
        auto attackerAssignment = findClosestUnitPair(mutaliskUnitsToAssign, mutaliskUnitTargets);
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
			Micro::MutaDanceTarget(attacker, target);
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
            mutaliskUnitTargets.erase(target);
        }
        // if it's a building and there's more than 10 things assigned to it already, don't assign more
        else if (target->getType().isBuilding() && (assigned > 10))
        {
            mutaliskUnitTargets.erase(target);
        }

        mutaliskUnitsToAssign.erase(attacker);
    }

    // if there's no targets left, attack move to the order destination
    if (mutaliskUnitTargets.empty())
    {
        for (auto & unit : mutaliskUnitsToAssign)    
        {
			if (unit->getDistance(order.getPosition()) > 100)
			{
				// move to it
				Micro::SmartAttackMove(unit, order.getPosition());
			}
        }
    }
}