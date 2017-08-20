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
	const auto & sbases = InformationManager::Instance().getSelfBases();
	auto sbaseP = sbases.empty()
		? BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())
		: sbases.front()->getPosition();
	const auto & ebases = InformationManager::Instance().getEnemyBaseInfos();
	
	size_t numOverlord = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Overlord, BWAPI::Broodwar->self());
    size_t current(0);
	BWAPI::Position baseLocation[6];
	size_t baseCount(0);
	size_t curBase(0);
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery ||
			unit->getType() == BWAPI::UnitTypes::Zerg_Lair ||
			unit->getType() == BWAPI::UnitTypes::Zerg_Hive)
		{
			baseLocation[baseCount] = unit->getPosition();
			baseCount++;
			if (baseCount == 6)
			{
				break;
			}
		}
	}
	numOverlord -= baseCount;

    for (auto & overlordUnit : overlordUnits)
    {
        BWAPI::Unitset nearbyEnemies;

        MapGrid::Instance().GetUnits(nearbyEnemies, overlordUnit->getPosition(), BWAPI::UnitTypes::Zerg_Overlord.sightRange() + 150, false, true);

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
        {//	BWTA::getStartLocations();
			//第一只血多的出去探路 顺时针
			if (overlordUnit->getHitPoints() > BWAPI::UnitTypes::Zerg_Overlord.maxHitPoints() * 0.5 && current == 0)
			{
				BWAPI::Position aimPosition;
				int aimPlace = -1;
				if (ebases.empty())
				{
					//顺时针探路
					auto baseTPs = BWEM::Map::Instance().StartingLocations();
					auto cmp = [](const BWAPI::TilePosition baseTP1, const BWAPI::TilePosition baseTP2) {
						BWAPI::Position center(BWAPI::Broodwar->mapWidth() * 16, BWAPI::Broodwar->mapHeight() * 16);
						BWAPI::Position baseP1(baseTP1), baseP2(baseTP2);
						int dx1 = baseP1.x - center.x, dy1 = baseP1.y - center.y;
						int dx2 = baseP2.x - center.x, dy2 = baseP2.y - center.y;
						double theta1 = std::atan2(dy1, dx1);
						double theta2 = std::atan2(dy2, dx2);
						//升序列->顺时针
						return theta1 < theta2;
					};

					std::sort(baseTPs.begin(), baseTPs.end(), cmp);
					auto iter = std::find(baseTPs.begin(), baseTPs.end(), BWAPI::Broodwar->self()->getStartLocation());
					std::reverse(baseTPs.begin(), iter);
					std::reverse(iter++, baseTPs.end());
					std::reverse(baseTPs.begin(), baseTPs.end());
					for (const auto baseTP : baseTPs)
					{
						// if we haven't explored it yet
						if (!BWAPI::Broodwar->isExplored(baseTP))
						{
							aimPosition = BWAPI::Position(baseTP);
							break;
						}
					}
				}
				//探到基地，去角落呆着
				else
				{
					auto ebaseP = ebases.front().lastPosition;
					aimPosition = ebaseP;
					int maxWidth = 32 * BWAPI::Broodwar->mapWidth();
					int maxHeight = 32 * BWAPI::Broodwar->mapHeight();
					if (aimPosition.x > maxWidth / 2)
					{
						aimPosition.x = maxWidth - 16;
					}
					else
					{
						aimPosition.x = 16;
					}
					if (aimPosition.y > maxHeight / 2)
					{
						aimPosition.y = maxHeight - 16;
					}
					else
					{
						aimPosition.y = 16;
					}
				}
				//BWAPI::Position fleeVec(aimPosition - ourBaseLocation);
				//double fleeAngle = atan2(fleeVec.y, fleeVec.x);
				//fleeVec = BWAPI::Position(static_cast<int>(384 * cos(fleeAngle)), static_cast<int>(384 * sin(fleeAngle)));
				//BWAPI::Position movePosition(static_cast<int>(640 * sin(fleeAngle)), static_cast<int>(-640 * cos(fleeAngle)));
				//aimPosition = BWAPI::Position(aimPosition - fleeVec);
				Micro::SmartMove(overlordUnit, aimPosition);
				current++;
			}
			//基地每个一只
			else if (curBase < baseCount)
			{
				if (overlordUnit->getDistance(baseLocation[curBase]) < 100)
				{
					Micro::SmartMove(overlordUnit, baseLocation[curBase]);
				}
				curBase++;
			}
			//剩下全呆家里
			else if (overlordUnit->getDistance(sbaseP) < 100)
			{
				Micro::SmartMove(overlordUnit, sbaseP);
			}
        }
    }
}