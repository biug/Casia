#include "ActionZVPZealot.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVPZealot::ActionZVPZealot()
{
}

void ActionZVPZealot::init()
{
	if (!isInitialized)
	{
		lastFrameCount = BWAPI::Broodwar->getFrameCount();
		lastFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		lastFrameGasAmount = BWAPI::Broodwar->self()->gas();
		isInitialized = true;
	}
}

bool ActionZVPZealot::canDeployAction()
{
	if (enemyDragoonOverZealotRate <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ActionZVPZealot::tick()
{
	if (enemyDragoonOverZealotRate > 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ActionZVPZealot::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	// 当前帧数（累计）
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();
	int freeMineral =
		BWAPI::Broodwar->self()->minerals()
		- BuildingManager::Instance().getReservedMinerals();
	int freeGas =
		BWAPI::Broodwar->self()->gas()
		- BuildingManager::Instance().getReservedGas();
	being_rushed = InformationManager::Instance().isEncounterRush();

	bool isSunkenColonyExist = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue > 0;
	bool isCreepColonyExist = creep_colony_count + creep_colony_being_built + creep_colony_in_queue > 0;
	int sunken_colony_total = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue;
	int creep_colony_total = creep_colony_count + creep_colony_being_built + creep_colony_in_queue;

	// 每10秒检查队列一次
	if ((currentFrameCount + 150) % 150 == 0)
	{
		if (spawning_pool_completed == 0 && zergling_in_queue > 0)
		{
			queue.clear();
		}
		else if (hatchery_completed == 0 && lair_in_queue > 0)
		{
			queue.clear();
		}
		else if (lair_completed == 0 && hive_in_queue > 0)
		{
			queue.clear();
		}
		else if (hydralisk_den_completed == 0 && hydralisk_in_queue > 0)
		{
			queue.clear();
		}
		else if (lurker_in_queue > hydralisk_completed)
		{
			queue.clear();
		}
		else if (creep_colony_completed == 0 && sunken_colony_in_queue > 0)
		{
			queue.clear();
		}
		else if (spire_completed == 0 && mutalisk_in_queue > 0)
		{
			queue.clear();
		}
		if (zergling_count + zergling_in_queue > zerglingLimit)
		{
			queue.clear();
		}
		// 农民过少时
		if (drone_count < 3)
		{
			queue.clear();
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
	}

	// 防rush放地堡
	int creep_need = std::min(5, std::max(2, (int)(enemy_zealot_count / 1.6)));
	if (spawning_pool_count > 0	// 存在血池
		&& creep_colony_being_built + creep_colony_in_queue == 0	// 队列中没有地堡
		&& creep_colony_count + creep_colony_being_built + creep_colony_in_queue +
		sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < (being_rushed ? creep_need : 0))
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
	}

	if (creep_colony_completed > 0 && spawning_pool_completed > 0 &&
		sunken_colony_being_built + sunken_colony_in_queue < creep_colony_completed)	//如果有creep就变地堡
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), true);
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);	// 补地堡也要补农民
	}

	// 判断前提建筑是否存在
	if (spawning_pool_count + spawning_pool_in_queue + spawning_pool_being_built == 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	if (extractor_count + extractor_in_queue + extractor_being_built == 0
		&& drone_count >= 10 && spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

	// 判断是否二本
	if (lair_count + lair_being_built + lair_in_queue == 0	// 没有多造
		&& spawning_pool_completed > 0 && freeGas > 50		// 资源和前置条件
		&& (!being_rushed || sunken_colony_count >= 2)		// 被rush要地堡优先
		&& zergling_count >= 8
		&& currentFrameCount >= 5400)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair), true);	// 二本优先
	}

	if (hydralisk_den_count + hydralisk_den_in_queue + hydralisk_den_being_built == 0
		&& freeGas >= 50 && lair_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den, "50%Lair"));
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// 若飞龙塔不存在，并且出了lurker
	if (spire_count + spire_in_queue + spire_being_built == 0
		&& (lair_completed > 0 || hive_completed > 0)
		&& lurker_completed > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spire));
	}

	// 开分矿
	if (freeMineral > 400
		&& base_in_queue + base_being_built == 0
		&& base_count < 5
		&& lurker_count > 0
		&& !being_rushed)
	{
		BWAPI::Broodwar->printf("add a hatchery");
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
	}

	// 三本
	if (lair_completed > 0)
	{
		if (queens_nest_count + queens_nest_being_built + queens_nest_in_queue == 0
			&& spire_completed > 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
		}
		if (queens_nest_completed > 0)
		{
			if (hive_count + hive_being_built + hive_in_queue == 0)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
			}
		}
	}

	// lurker科技
	if (hydralisk_den_completed > 0 && lair_completed > 0 && lurker_aspect_count == 0)
	{
		queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect), true);
	}
	// 小狗科技
	if (spawning_pool_completed > 0 && metabolic_boost_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (hive_completed > 0 && adrenal_glands_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}
	// 飞龙科技
	if (spire_completed > 0 && mutalisk_count > 0 && flyer_carapace_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace));
	}
	// 刺蛇科技
	if (hydralisk_den_completed > 0 && hydralisk_completed > 0 && muscular_arguments_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
	}
	if (hydralisk_den_completed > 0 && hydralisk_completed > 0 && grooved_spines_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
	}

	// 计算需要的农民数量
	int numTotalPatch = 0;
	int minNumWorkers = 0;
	for (const auto & base : InformationManager::Instance().getSelfBases())
	{
		int numWorkers = WorkerManager::Instance().getMineralPatches(base).size();
		numTotalPatch += numWorkers;
		minNumWorkers += numWorkers + 3;
	}
	// 如果农民不足，需要加入优先队列
	bool notEnoughDrone = (drone_count + drone_in_queue) < minNumWorkers;
	if (drone_count + drone_in_queue < numTotalPatch * Config::Macro::WorkersPerMineralPatch)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), notEnoughDrone);
	}

	// 分矿的气矿
	int numMineralBases = WorkerManager::Instance().getMineralBases().size();
	int extractorNeed = std::min(numMineralBases, 3);
	if (!notEnoughDrone
		&& extractor_count + extractor_being_built + extractor_in_queue < extractorNeed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		// 补一个农民
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// 判断需要建造多少部队
	// 小狗
	int zerglingMax = being_rushed || currentFrameCount < 6400 ? 16 : 48;
	int zerglingNeed = (zerglingMax - zergling_count - zergling_in_queue) / 2;
	if (zerglingNeed > 0 && zergling_in_queue < 6)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
	}

	// 刺蛇
	if (hydralisk_den_completed > 0
		&& hydralisk_in_queue < 3
		&& hydralisk_count + hydralisk_in_queue < enemy_air_army_supply + 3)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
	}
	// lurker加入优先队列
	if (hydralisk_completed > 0
		&& BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
		&& lurker_in_queue == 0)
	{
		int hydra_reserved = enemy_air_army_supply - hydralisk_count;
		if (hydra_reserved <= 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker), true);
		}
	}
	// 飞龙
	if (spire_completed > 0 && lurker_completed > 0
		&& mutalisk_count + mutalisk_in_queue < 12)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
	}
}

void ActionZVPZealot::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyDragoonOverZealotRate = enemy_zealot_count == 0 ? 10 : (double)enemy_dragoon_count / (double)enemy_zealot_count;
	if (enemy_dragoon_count == 0) enemyDragoonOverZealotRate = 0;
	droneLimit = real_base_count == 1 ? 16 : real_base_count * 10;

	zerglingLimit = lurker_completed * 3;
	if (zerglingLimit < 20)
	{
		zerglingLimit = 20;
	}
	else if (zerglingLimit > 45)
	{
		zerglingLimit = 45;
	}

	enemyAirForceCount = enemy_scout_count + enemy_corsair_count + enemy_carrier_count + enemy_arbiter_count;
}

void ActionZVPZealot::tryAddInQueue(ProductionQueue & queue, const ProductionItem & item, bool priority)
{
}
