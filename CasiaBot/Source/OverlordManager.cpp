#include "OverlordManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

OverlordManager::OverlordManager() 
{ 
}
void OverlordManager::executeMicro(const BWAPI::Unitset & targets)
{
}
void OverlordManager::executeMove(const SquadOrder & inputOrder) 
{
    order = inputOrder;

    const BWAPI::Unitset & overlordUnits = getUnits();
    //different position need to do
    BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	if (enemyBaseLocation == nullptr) {
		for (auto &base : BWTA::getStartLocations()) {
			if (base != BWTA::getStartLocation(BWAPI::Broodwar->self())) {
				enemyBaseLocation = base;
				break;
			}
		}
	}
    BWAPI::Position ourBaseLocation = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	

	size_t numOverlord = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Overlord, BWAPI::Broodwar->self());
    size_t current(0);
	BWAPI::Position baseLocation[3];
	size_t baseCount(0);

	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery ||
			unit->getType() == BWAPI::UnitTypes::Zerg_Lair ||
			unit->getType() == BWAPI::UnitTypes::Zerg_Hive)
		{
			baseLocation[baseCount] = unit->getPosition();
			baseCount++;
			if (baseCount == 3)
			{
				break;
			}
		}
	}

	numOverlord -= baseCount;

    for (auto & overlordUnit : overlordUnits)
    {
        BWAPI::Unitset nearbyEnemies;

        MapGrid::Instance().GetUnits(nearbyEnemies, overlordUnit->getPosition(), BWAPI::UnitTypes::Zerg_Overlord.sightRange() + 64, false, true);

        bool isFlee = false;
        if (!nearbyEnemies.empty())
        {
            for (auto & nearbyEnemy : nearbyEnemies)
            {
                BWAPI::UnitType targetType = nearbyEnemy->getType();
                if(targetType.airWeapon() != BWAPI::WeaponTypes::None || 
                    targetType == BWAPI::UnitTypes::Terran_Bunker){
                    BWAPI::Position fleeVector = Micro::GetKiteVector(nearbyEnemy, overlordUnit);
                    BWAPI::Position moveToPosition(overlordUnit->getPosition() + fleeVector);
                    Micro::SmartMove(overlordUnit, moveToPosition);
                    isFlee = true;
                    break;
                }
            }
        }
        // sent to move around it's area
        if (!isFlee)
        {
			if (current < baseCount)
			{
				Micro::SmartMove(overlordUnit, baseLocation[current]);
			}
            else if (overlordUnit->getHitPoints() < BWAPI::UnitTypes::Zerg_Overlord.maxHitPoints() * 0.5)
            {
                BWAPI::Position fleeTo(BWAPI::Broodwar->self()->getStartLocation());
				Micro::SmartMove(overlordUnit, fleeTo);
                current--;
            }
            else
            {
				BWAPI::Position fleeVec(enemyBaseLocation->getPosition() - ourBaseLocation);
                double fleeAngle = atan2(fleeVec.y, fleeVec.x);

                fleeVec = BWAPI::Position(static_cast<int>(384 * cos(fleeAngle)), static_cast<int>(384 * sin(fleeAngle)));

                BWAPI::Position movePosition(static_cast<int>(640 * sin(fleeAngle)), static_cast<int>(-640 * cos(fleeAngle)));

                BWAPI::Position aimPosition(fleeVec);

                aimPosition = BWAPI::Position((ourBaseLocation * (current - baseCount) + enemyBaseLocation->getPosition() * (numOverlord - current + baseCount)) / numOverlord + movePosition * flag - fleeVec);
                Micro::SmartMove(overlordUnit, aimPosition);
                
                if (overlordUnit->getDistance(aimPosition) < 32)
                {
                    flag *= -1;
                }
            }

        }
        current++;
    }
}