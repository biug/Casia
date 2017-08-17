#include "ActionZVZZerglingLurker.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVZZerglingLurker::ActionZVZZerglingLurker()
{
}

void ActionZVZZerglingLurker::init()
{
}

bool ActionZVZZerglingLurker::canDeployAction()
{
	return _status.enemy_mutalisk_count == 0 && _status.enemy_hydralisk_count > 0;
}

bool ActionZVZZerglingLurker::tick()
{
	return _status.enemy_hydralisk_count == 0;
}

void ActionZVZZerglingLurker::getBuildOrderList(CasiaBot::ProductionQueue & queue)
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
	int creep_need = std::max(2, _status.enemy_zergling_count / 12);
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
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));	// 补地堡也要补农民
	}

	// 判断血池是否存在
	if (_status.spawning_pool_total == 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	// 判断第一个气矿是否存在
	if (_status.extractor_total == 0
		&& _status.drone_count >= 10 && _status.spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

	// 判断是否二本
	if (_status.lair_total == 0	// 没有多造
		&& _status.spawning_pool_completed > 0		// 资源和前置条件
		&& (!being_rushed || _status.sunken_colony_count >= 2)		// 被rush要地堡优先
		&& _status.zergling_count >= 8
		&& currentFrameCount >= 4800)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair), true);	// 二本优先
	}

	// 飞龙塔
	if (_status.lair_completed > 0
		&& _status.spire_total == 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spire));
	}

	// 开分矿
	if (freeMineral > 350
		&& _status.base_in_queue + _status.base_being_built == 0
		&& _status.base_count < 5)
	{
		std::string cond = _status.base_total == 1 ? "Main" : "";
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
	}

	// 三本
	if (_status.lair_completed > 0)
	{
		if (_status.queens_nest_total == 0 && _status.lurker_completed > 2)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
		}
		if (_status.queens_nest_completed > 0)
		{
			if (_status.hive_total == 0)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
			}
		}
	}

	// 小狗科技
	if (_status.spawning_pool_completed > 0 && _status.metabolic_boost_total == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (_status.hive_completed > 0 && _status.adrenal_glands_total == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}

	// 计算需要的农民数量
	int numTotalPatch = 0;
	for (const auto & base : InformationManager::Instance().getSelfBases())
	{
		numTotalPatch += WorkerManager::Instance().getMineralPatches(base).size();
	}
	// 如果农民不足，需要加入优先队列
	bool notEnoughDrone = (_status.drone_total) < numTotalPatch;
	if (_status.drone_total < numTotalPatch * Config::Macro::WorkersPerMineralPatch)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), notEnoughDrone);
	}

	// 分矿的气矿
	int numMineralBases = WorkerManager::Instance().getMineralBases().size();
	int extractorNeed = std::min(numMineralBases, 3);
	if (!notEnoughDrone && _status.extractor_total < extractorNeed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		// 补一个农民
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// 判断需要建造多少部队
	// 小狗
	int zerglingMax = std::max(12, _status.enemy_zergling_count + 2);
	int zerglingNeed = (zerglingMax - _status.zergling_total) / 2;
	if (zerglingNeed > 0 && _status.zergling_in_queue < 4)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
	}

	// 飞龙
	if (_status.spire_completed > 0 && _status.mutalisk_in_queue < 2)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), _status.enemy_mutalisk_count - _status.mutalisk_count > 0);
	}

	int air_require = _status.enemy_mutalisk_count - _status.mutalisk_count - _status.scourge_count;
	if (air_require > 0 && _status.scourge_in_queue < 2)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Scourge), true);
	}
}
