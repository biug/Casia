#include "ActionZVPDragoon.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVPDragoon::ActionZVPDragoon()
{
}

void ActionZVPDragoon::init()
{
	if (!isInitialized)
	{
		lastFrameCount = BWAPI::Broodwar->getFrameCount();
		lastFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		lastFrameGasAmount = BWAPI::Broodwar->self()->gas();
		isInitialized = true;
	}
}

bool ActionZVPDragoon::canDeployAction()
{
	if (enemyDragoonOverZealotRate > 1 || enemy_dragoon_count > 5)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ActionZVPDragoon::tick()
{
	if (enemyDragoonOverZealotRate <= 1 && enemy_dragoon_count <= 5)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ActionZVPDragoon::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	// 当前帧数（累计）
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();

	if (currentFrameCount % 200 == 0)
	{
		int currentFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		int currentFrameGasAmount = BWAPI::Broodwar->self()->gas();
		int diffMineralAmount = currentFrameMineralAmount - lastFrameMineralAmount;
		int diffGasAmount = currentFrameGasAmount - lastFrameGasAmount;

		mineralNetIncrease.pop_front();
		mineralNetIncrease.push_back(diffMineralAmount);
		gasNetIncrease.pop_front();
		gasNetIncrease.push_back(diffGasAmount);
		lastFrameCount = currentFrameCount;
		lastFrameMineralAmount = currentFrameMineralAmount;
		lastFrameGasAmount = currentFrameGasAmount;
	}
	bool mineralDequePositive = IsDequeAllPositive(mineralNetIncrease);
	bool gasDequePositive = IsDequeNoneNegative(gasNetIncrease);

	//是否需要防御建筑


	// 判断前提建筑是否存在
	bool isHiveExist = hive_being_built + hive_count + hive_in_queue > 0;
	bool isQueenNestExist = queens_nest_being_built + queens_nest_count + queens_nest_in_queue > 0;
	bool isLairExist = lair_being_built + lair_count + lair_in_queue > 0;
	bool isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	if (!isSpawningPoolExist)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	bool isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;
	if (!isExtractorExist && drone_count >= 7 && spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
	}

	
	

	// 判断是否需要增加母巢
	if (currentFrameCount % 200 == 0 && base_count + base_in_queue + base_being_built <= 4 && currentFrameCount > 10)
	{
		if (base_count + base_in_queue + base_being_built <= 2)
		{
			if (mineralDequePositive)
			{
				//queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery), true);
			}
		}
		else
		{
			if (mineralDequePositive && gasDequePositive)
			{
				//queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery), true);
			}
		}
	}

	bool notEnoughDrone = false;
	if (base_count == 1)
	{
		if (drone_count + drone_in_queue < 15)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
		notEnoughDrone = drone_count + drone_in_queue < 12;
	}
	else
	{
		if (drone_count + drone_in_queue < base_count * 10)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
		notEnoughDrone = drone_count + drone_in_queue < 8 * base_count;
	}

	// 判断需要建造多少部队
	int need_zergling_count = enemy_zealot_count * 8 + enemy_dragoon_count * 7 - zergling_count - zergling_in_queue;
	if (need_zergling_count <= 0 && zergling_count + zergling_in_queue < 12)
	{
		need_zergling_count = 2;
	}
	if (notEnoughDrone)
	{
		need_zergling_count = zergling_count + zergling_in_queue < 12 ? 2 : 0;
	}

	if (spawning_pool_count > 0)
	{
		for (int i = 0; i < need_zergling_count; i += 2)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
		}
	}

	if (!isHiveExist)	// 若蜂巢不存在
	{
		if (isQueenNestExist)	// 若皇后巢存在
		{
			if (currentFrameCount > 10800)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
			}
		}
		else	// 若皇后巢不存在
		{
			if (isLairExist)	// 若兽穴存在
			{
				if (currentFrameCount > 9000)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
				}
			}
			else if (currentFrameCount > 4800)	// 若兽穴不存在
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair));
			}
		}
	}

	if (spawning_pool_count > 0 && queue.upgradeCount(BWAPI::UpgradeTypes::Metabolic_Boost) == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (hive_count > 0 && queue.upgradeCount(BWAPI::UpgradeTypes::Adrenal_Glands) == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}

	//补气矿
	int extractorUpperBound = std::min(base_count + base_being_built, 2);
	if (extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}
}

void ActionZVPDragoon::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyDragoonOverZealotRate = enemy_zealot_count == 0 ? 10 : (double)enemy_dragoon_count / (double)enemy_zealot_count;
}