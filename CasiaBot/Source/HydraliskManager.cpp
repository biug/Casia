#include "HydraliskManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

HydraliskManager::HydraliskManager() 
{ 
}

void HydraliskManager::executeMicro(const BWAPI::Unitset & targets) 
{
	assignTargetsOld(targets);
}


void HydraliskManager::assignTargetsOld(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & hydraliskUnits = getUnits();

	// figure out targets
	BWAPI::Unitset hydraliskUnitTargets;
    std::copy_if(targets.begin(), targets.end(), std::inserter(hydraliskUnitTargets, hydraliskUnitTargets.end()),
     [](BWAPI::Unit u){ return u->isVisible(); });

    for (auto & hydraliskUnit : hydraliskUnits)
	{
		// if the order is to attack or defend
		if (order.getType() == SquadOrderTypes::Attack || order.getType() == SquadOrderTypes::Defend) 
        {
            BWAPI::Position ourBasePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
			// if there are targets
			if (!hydraliskUnitTargets.empty() && hydraliskUnit->getDistance(ourBasePosition) < 500)
			{
				// find the best target for this zealot
				BWAPI::Unit target = getTarget(hydraliskUnit, hydraliskUnitTargets);
                
                if (target && Config::Debug::DrawUnitTargetInfo) 
	            {
		            BWAPI::Broodwar->drawLineMap(hydraliskUnit->getPosition(), hydraliskUnit->getTargetPosition(), BWAPI::Colors::Purple);
	            }

				// attack it
                if (Config::Micro::KiteWithRangedUnits)
                {
                    Micro::SmartKiteTarget(hydraliskUnit, target);
                }
                else
                {
                    Micro::SmartAttackUnit(hydraliskUnit, target);
                }
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (hydraliskUnit->getDistance(ourBasePosition) > 500)
				{
					// move to it
					Micro::SmartAttackMove(hydraliskUnit, ourBasePosition);
				}
			}
		}
	}
}

std::pair<BWAPI::Unit, BWAPI::Unit> HydraliskManager::findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets)
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
BWAPI::Unit HydraliskManager::getTarget(BWAPI::Unit hydraliskUnit, const BWAPI::Unitset & targets)
{
	int bestPriorityDistance = 1000000;
    int bestPriority = 0;
    
    double bestLTD = 0;

    int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

    for (const auto & target : targets)
    {
        double distance         = hydraliskUnit->getDistance(target);
        double LTD              = UnitUtil::CalculateLTD(target, hydraliskUnit);
        int priority            = getAttackPriority(hydraliskUnit, target);
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
int HydraliskManager::getAttackPriority(BWAPI::Unit hydraliskUnit, BWAPI::Unit target) 
{
	BWAPI::UnitType rangedType = hydraliskUnit->getType();
	BWAPI::UnitType targetType = target->getType();

    
    if (hydraliskUnit->getType() == BWAPI::UnitTypes::Zerg_Scourge)
    {
        if (target->getType() == BWAPI::UnitTypes::Protoss_Carrier)
        {
            
            return 100;
        }

        if (target->getType() == BWAPI::UnitTypes::Protoss_Corsair)
        {
            return 90;
        }
    }


    if (target->getType() == BWAPI::UnitTypes::Zerg_Larva || target->getType() == BWAPI::UnitTypes::Zerg_Egg)
    {
        return 0;
    }

    if (hydraliskUnit->isFlying() && target->getType() == BWAPI::UnitTypes::Protoss_Carrier)
    {
        return 101;
    }

    // if the target is building something near our base something is fishy
    BWAPI::Position ourBasePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
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
    //Archon
    else if (targetType == BWAPI::UnitTypes::Protoss_Archon)
    {
        return priority + 12;
    }
    // next priority is worker
    else if (targetType.isWorker())
    {
        return priority + 9;
    }
    //can attack us
    else if (targetType.groundWeapon() != BWAPI::WeaponTypes::None)
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

BWAPI::Unit HydraliskManager::closestrangedUnit(BWAPI::Unit target, std::set<BWAPI::Unit> & hydraliskUnitsToAssign)
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
void HydraliskManager::assignTargetsNew(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & hydraliskUnits = getUnits();

	// figure out targets
	BWAPI::Unitset hydraliskUnitTargets;
    std::copy_if(targets.begin(), targets.end(), std::inserter(hydraliskUnitTargets, hydraliskUnitTargets.end()), [](BWAPI::Unit u){ return u->isVisible(); });

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