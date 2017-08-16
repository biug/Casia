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
}

bool ActionZVPZealot::canDeployAction()
{
	return true;
}

bool ActionZVPZealot::tick()
{
	return false;
}

void ActionZVPZealot::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();
	int freeMineral =
		BWAPI::Broodwar->self()->minerals()
		- BuildingManager::Instance().getReservedMinerals();
	int freeGas =
		BWAPI::Broodwar->self()->gas()
		- BuildingManager::Instance().getReservedGas();
	bool being_rushed = _status.being_rushed;

	// 防rush放地堡
	int creep_need = std::min(4, std::max(2, (int)(_status.enemy_zealot_count / 1.6)));
	if (_status.spawning_pool_count > 0	// 存在血池
		&& _status.creep_colony_being_built + _status.creep_colony_in_queue == 0	// 队列中没有地堡
		&& _status.creep_colony_total + _status.sunken_colony_total < (being_rushed ? creep_need : 0))
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
	}

	if (_status.creep_colony_completed > 0 && _status.spawning_pool_completed > 0 &&
		_status.sunken_colony_being_built + _status.sunken_colony_in_queue < _status.creep_colony_completed)
		//如果有creep就变地堡
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), true);
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);	// 补地堡也要补农民
	}

	// 判断前提建筑是否存在
	if (_status.spawning_pool_total == 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	// 判断第一个气矿是否存在
	if (_status.extractor_total == 0
		&& _status.drone_count >= 10
		&& _status.spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

	// 判断是否二本
	if (_status.lair_total == 0	// 没有多造
		&& _status.spawning_pool_completed > 0 && freeGas > 50		// 资源和前置条件
		&& (!being_rushed || _status.sunken_colony_count >= 2)		// 被rush要地堡优先
		&& _status.zergling_count >= 8
		&& currentFrameCount >= 5400)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair), true);	// 二本优先
	}

	// 判断刺蛇洞是否存在
	if (_status.hydralisk_den_total == 0
		&& freeGas >= 50 && _status.lair_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den, "50%Lair"));
	}

	// 若飞龙塔不存在，并且出了lurker
	if (_status.spire_total == 0
		&& (_status.lair_completed > 0 || _status.hive_completed > 0)
		&& _status.lurker_completed > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spire));
	}

	// 开分矿
	if (freeMineral > 400
		&& _status.base_in_queue + _status.base_being_built == 0
		&& _status.base_count < 5
		&& _status.lurker_count > 0
		&& !being_rushed)
	{
		BWAPI::Broodwar->printf("add a hatchery");
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
	}

	// 三本
	if (_status.lair_completed > 0)
	{
		if (_status.queens_nest_total == 0 && _status.spire_completed > 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
		}
		if (_status.queens_nest_completed > 0 && _status.hive_total == 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
		}
	}

	// lurker科技
	if (_status.hydralisk_den_completed > 0
		&& _status.lair_completed > 0
		&& _status.lurker_aspect_count == 0)
	{
		queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect), true);
	}
	// 小狗科技
	if (_status.spawning_pool_completed > 0 && _status.metabolic_boost_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (_status.hive_completed > 0 && _status.adrenal_glands_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}
	// 飞龙科技
	if (_status.spire_completed > 0
		&& _status.mutalisk_count > 0
		&& _status.flyer_carapace_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace));
	}
	// 刺蛇科技
	if (_status.hydralisk_den_completed > 0
		&& _status.hydralisk_completed > 0
		&& _status.muscular_arguments_count == 0
		&& _status.hydralisk_count > 3)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
	}
	if (_status.hydralisk_den_completed > 0
		&& _status.hydralisk_completed > 0
		&& _status.grooved_spines_count == 0
		&& _status.muscular_arguments_count > 0)
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
	bool notEnoughDrone = _status.drone_total < minNumWorkers;
	if (_status.drone_total < numTotalPatch * Config::Macro::WorkersPerMineralPatch)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), notEnoughDrone);
	}

	// 分矿的气矿
	int numMineralBases = WorkerManager::Instance().getMineralBases().size();
	int extractorNeed = std::min(numMineralBases, 3);
	if (!notEnoughDrone
		&& _status.extractor_total < extractorNeed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		// 补一个农民
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// 判断需要建造多少部队
	// 小狗
	int zerglingMax = being_rushed || currentFrameCount < 6400 ? 16 : 48;
	int zerglingNeed = (zerglingMax - _status.zergling_total) / 2;
	if (zerglingNeed > 0 && _status.zergling_in_queue < 6)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
	}

	// 刺蛇
	if (_status.hydralisk_den_completed > 0
		&& _status.hydralisk_in_queue < 3
		&& _status.hydralisk_total < _status.enemy_air_army_supply + 3)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
	}
	// lurker加入优先队列
	if (_status.hydralisk_completed > 0
		&& BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
		&& _status.lurker_in_queue == 0)
	{
		int hydra_reserved = _status.enemy_air_army_supply - _status.hydralisk_count;
		if (hydra_reserved <= 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker), true);
		}
	}
	// 飞龙
	if (_status.spire_completed > 0 && _status.lurker_completed > 0
		&& _status.mutalisk_total < 12)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk));
	}
}
