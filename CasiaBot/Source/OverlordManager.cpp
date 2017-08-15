#include "OverlordManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

OverlordManager::OverlordManager() 
{
	initializeFlag = false;
	startBase[0] = BWAPI::TilePositions::None;
	startBase[2] = BWAPI::TilePositions::None;
	startBase[1] = BWAPI::TilePositions::None;
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
	const auto & sbases = InformationManager::Instance().getSelfBases();
	auto sbaseTP = sbases.empty()
		? BWAPI::Broodwar->self()->getStartLocation()
		: sbases.front()->getTilePosition();
	auto sbaseP = BWAPI::Position(sbaseTP);
	//0����ģ�1��Զ�ģ�2ʣ���Ǹ�
	int minDistance = 0;
	int maxDistance = 0;
	int nowDistance;
	for (const auto & base : BWEM::Map::Instance().StartingLocations()) {
		nowDistance = abs(base.x - sbaseTP.x) + abs(base.y - sbaseTP.y);
		if ((!startBase[0].isValid() || minDistance > nowDistance) && nowDistance != 0){
			startBase[0] = base;
			minDistance = nowDistance;
		}
		if ((!startBase[1].isValid() || maxDistance < nowDistance) && nowDistance != 0){
			startBase[1] = base;
			maxDistance = nowDistance;
			needDetect[1] = true;
		}
	}
	if (startBase[1] == startBase[0])
	{
		startBase[1] = BWAPI::TilePositions::None;
		needDetect[1] = false;
	}
	for (const auto & base : BWEM::Map::Instance().StartingLocations()) {
		if (startBase[0] != base && startBase[1] != base && sbaseTP != base){
			startBase[2] = base;
			needDetect[2] = true;
		}
	}
	//��ȡ��ʼλ�ý���
	initializeFlag = true;
	int halfWidth = 16 * BWAPI::Broodwar->mapWidth();
	int halfHeight = 16 * BWAPI::Broodwar->mapHeight();
	//�Ե�ͼ���Ľ�������ϵ�����ͼ������y�����෴
	double angle[4];
	auto start0 = BWAPI::Position(startBase[0]);
	angle[0] = atan2(halfHeight - start0.y, start0.x - halfWidth);
	angle[3] = atan2(halfHeight - sbaseP.y, sbaseP.x - halfWidth);
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
	auto startCompare = BWAPI::Position(startBase[compare]);
	angle[compare] = atan2(halfHeight - startCompare.y, startCompare.x - halfWidth);
	if ((angle[3] > angle[0] && angle[3] > angle[compare] && angle[0] < angle[compare])//�������Ͻ�
		|| (angle[3] < angle[0] && angle[3] > angle[compare] && angle[0] < angle[compare])//�����Ҳ�
		|| (angle[3] < angle[0] && angle[3] < angle[compare] && angle[0] < angle[compare]))//�������½�
	{
		//������˳ʱ�뷽��
		std::swap(startBase[0], startBase[compare]);
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
	const auto & sbases = InformationManager::Instance().getSelfBases();
	auto sbaseP = sbases.empty()
		? BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())
		: sbases.front()->getPosition();
	const auto & ebases = InformationManager::Instance().getEnemyBaseInfos();
	//��ʼ����ʼ���ص�����
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
			//��һֻѪ��ĳ�ȥ̽· ˳ʱ��
			if (overlordUnit->getHitPoints() > BWAPI::UnitTypes::Zerg_Overlord.maxHitPoints() * 0.5 && current == 0)
			{
				BWAPI::Position aimPosition;
				int aimPlace = -1;
				//˳ʱ��̽·
				if (ebases.empty())
				{
					////����С��ʼ�ķ���
					//if (current == 0)
					//{
					if (needDetect[0])
					{
						aimPosition = BWAPI::Position(startBase[0]);
						aimPlace = 0;
					}
					else if (needDetect[1])
					{
						aimPosition = BWAPI::Position(startBase[1]);
						aimPlace = 1;
					}
					else if (needDetect[2])
					{
						aimPosition = BWAPI::Position(startBase[2]);
						aimPlace = 2;
					}
					//}
					////��������
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
				//̽�����أ�ȥ�������
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

				if (overlordUnit->getDistance(aimPosition) < 32 && aimPlace != -1)
				{
					needDetect[aimPlace] = false;
				}
				current++;
			}
			//����ÿ��һֻ
			else if (curBase < baseCount)
			{
				if (overlordUnit->getDistance(baseLocation[curBase]) < 100)
				{
					Micro::SmartMove(overlordUnit, baseLocation[curBase]);
				}
				curBase++;
			}
			//ʣ��ȫ������
			else if (overlordUnit->getDistance(sbaseP) < 100)
			{
				Micro::SmartMove(overlordUnit, sbaseP);
			}
        }
    }
}