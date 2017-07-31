#include "OverlordManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

OverlordManager::OverlordManager() 
{
	initializeFlag = false;
	startBase[0] = nullptr;
	startBase[2] = nullptr;
	startBase[1] = nullptr;
	needDetect[0] = true;
	needDetect[1] = false;
	needDetect[2] = false;
}
void OverlordManager::executeMicro(const BWAPI::Unitset & targets)
{
}
void OverlordManager::initEnemyBase()
{
	if (initializeFlag)
	{
		return;
	}
	BWAPI::Position ourBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition();
	//0最近的，1最远的，2剩下那个
	int minDistance = 0;
	int maxDistance = 0;
	int nowDistance;
	for (auto & base : BWTA::getStartLocations()) {
		nowDistance = abs(base->getPosition().x - ourBaseLocation.x) + abs(base->getPosition().y - ourBaseLocation.y);
		if ((startBase[0] == nullptr || minDistance > nowDistance) && nowDistance != 0){
			startBase[0] = base;
			minDistance = nowDistance;
		}
		if ((startBase[1] == nullptr || maxDistance < nowDistance) && nowDistance != 0){
			startBase[1] = base;
			maxDistance = nowDistance;
			needDetect[1] = true;
		}
	}
	if (startBase[1] == startBase[0])
	{
		startBase[1] = nullptr;
		needDetect[1] = false;
	}
	for (auto & base : BWTA::getStartLocations()) {
		if (startBase[0] != base && startBase[1] != base && ourBaseLocation != base->getPosition()){
			startBase[2] = base;
			needDetect[2] = true;
		}
	}
	initializeFlag = true;
	int halfWidth = 16 * BWAPI::Broodwar->mapWidth();
	int halfHeight = 16 * BWAPI::Broodwar->mapHeight();
	//以地图中心建立坐标系，与地图数据中y方向相反
	double angle[4];
	angle[0] = atan2(halfHeight - startBase[0]->getPosition().y, startBase[0]->getPosition().x - halfWidth);
	angle[3] = atan2(halfHeight - ourBaseLocation.y, ourBaseLocation.x - halfWidth);
	int compare = 0;
	if (needDetect[2])
	{
		compare = 2;
	}
	else if (needDetect[1]){
		compare = 1;
	}
	if (compare == 0)
	{
		return;
	}
	angle[compare] = atan2(halfHeight - startBase[compare]->getPosition().y, startBase[compare]->getPosition().x - halfWidth);
	if ((angle[3] > angle[0] && angle[3] > angle[compare] && angle[0] < angle[compare])//基地左上角
		|| (angle[3] < angle[0] && angle[3] > angle[compare] && angle[0] < angle[compare])//基地右侧
		|| (angle[3] < angle[0] && angle[3] < angle[compare] && angle[0] < angle[compare]))//基地左下角
	{
		//交换成顺时针方向
		BWTA::BaseLocation *mid;
		mid = startBase[0];
		startBase[0] = startBase[compare];
		startBase[compare] = mid;
	}
	//CAB_ASSERT_SIMPLE("1 %d %d", startBase[0]->getPosition().x, startBase[0]->getPosition().y);
	//CAB_ASSERT_SIMPLE("2 %d %d", startBase[1]->getPosition().x, startBase[1]->getPosition().y);
	//CAB_ASSERT_SIMPLE("3 %d %d", startBase[2]->getPosition().x, startBase[2]->getPosition().y);
	//CAB_ASSERT_SIMPLE("4 %d %d", ourBaseLocation.x, ourBaseLocation.y);
	//CAB_ASSERT_SIMPLE("Map %d %d", BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight());
}
void OverlordManager::executeMove(const SquadOrder & inputOrder) 
{
    order = inputOrder;

    const BWAPI::Unitset & overlordUnits = getUnits();
    //different position need to do
	BWAPI::Position ourBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition();
    BWTA::BaseLocation *enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	//初始化开始基地
	initEnemyBase();
	
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
				//顺时针探路
				if (enemyBaseLocation == nullptr)
				{
					////从最小开始的方向
					//if (current == 0)
					//{
					if (needDetect[0])
					{
						aimPosition = startBase[0]->getPosition();
						aimPlace = 0;
					}
					else if (needDetect[1])
					{
						aimPosition = startBase[1]->getPosition();
						aimPlace = 1;
					}
					else if (needDetect[2])
					{
						aimPosition = startBase[2]->getPosition();
						aimPlace = 2;
					}
					//}
					////换个方向
					//else
					//{
					//	if (needDetect[2] && startBase[2] != nullptr)
					//	{
					//		aimPosition = startBase[1]->getPosition();
					//		aimPlace = 2;
					//	}
					//	else if (needDetect[1] && startBase[1] != nullptr)
					//	{
					//		aimPosition = startBase[1]->getPosition();
					//		aimPlace = 1;
					//	}
					//}
				}
				//探到基地，去角落呆着
				else
				{
					aimPosition = enemyBaseLocation->getPosition();
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

				if (overlordUnit->getDistance(aimPosition) < 32 && aimPlace != -1)
				{
					needDetect[aimPlace] = false;
				}
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
			else if (overlordUnit->getDistance(ourBaseLocation) < 100)
			{
				Micro::SmartMove(overlordUnit, ourBaseLocation);
			}
        }
    }
}