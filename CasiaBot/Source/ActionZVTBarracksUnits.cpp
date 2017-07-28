#include "ActionZVTBarracksUnits.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVTBarracksUnits::ActionZVTBarracksUnits()
{
}

void ActionZVTBarracksUnits::init()
{
	if (!isInitialized)
	{
		lastFrameCount = BWAPI::Broodwar->getFrameCount();
		lastFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		lastFrameGasAmount = BWAPI::Broodwar->self()->gas();
		isInitialized = true;
	}
}

bool ActionZVTBarracksUnits::canDeployAction()
{
	if (enemyTerranMechanizationRate < 0.8 || enemyTerranFactoryUnitsAmount <= 8)
	{
		return true;
	}
	else
		return false;
}

bool ActionZVTBarracksUnits::tick()
{
	if (enemyTerranMechanizationRate >= 0.8 || enemyTerranFactoryUnitsAmount > 8)
	{
		return true;
	}
	else
		return false;
}

void ActionZVTBarracksUnits::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	being_rushed = InformationManager::Instance().isEncounterRush();
	// 当前帧数（累计）
	int gas = BWAPI::Broodwar->self()->gas();
	int minerals = BWAPI::Broodwar->self()->minerals();
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();

	//if (base_count + base_in_queue + base_being_built <= 1)
	//{
	//	if ((spawning_pool_count > 0 && drone_count >= 12) || minerals > 320)
	//	{
	//		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery), true);
	//	}
	//}

	if (BWAPI::Broodwar->self()->minerals() > 500 && base_in_queue + base_being_built < 1
		&& base_count + base_in_queue + base_being_built < 5)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
	}

	// 判断前提建筑是否存在
	bool isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	if (!isSpawningPoolExist)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	if (spawning_pool_count > 0 &&
		creep_colony_count + creep_colony_being_built + creep_colony_in_queue +
		sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < (being_rushed ? 2 : 0))
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), being_rushed);
	}

	if (creep_colony_completed > 0 && spawning_pool_completed > 0 &&
		sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < creep_colony_completed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony));
	}

	bool isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;
	if (!isExtractorExist && drone_count >= 10 && spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

	bool isHydraliskDenExist = hydralisk_den_being_built + hydralisk_den_count + hydralisk_den_in_queue > 0;
	if (!isHydraliskDenExist && gas >= 50 && lair_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den), true);
	}

	if (lair_count + lair_being_built + lair_in_queue == 0
		&& spawning_pool_completed > 0 && gas >= 100
		&& currentFrameCount > 3600)
	{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair), true);
	}
	if (lair_completed > 0)
	{
		if (queens_nest_count + queens_nest_being_built + queens_nest_in_queue == 0)
		{
			if (currentFrameCount > 15400)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
			}
		}
		if (queens_nest_completed > 0)
		{
			if (hive_count + hive_being_built + hive_in_queue == 0)
			{
				if (currentFrameCount > 17280)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
				}
			}
		}
	}

	if (spawning_pool_completed > 0 && metabolic_boost_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (hydralisk_den_completed > 0 && lair_completed > 0 && lurker_aspect_count == 0 && gas >= 125)
	{
		queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect), true);
	}
	if (hive_completed > 0 && adrenal_glands_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}

	bool notEnoughDrone = false;
	if (base_count == 1)
	{
		if (drone_count + drone_in_queue < 15)
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), drone_count + drone_in_queue < 12);
		notEnoughDrone = (drone_count + drone_in_queue) < 10;
	}
	else
	{
		if (drone_count + drone_in_queue < base_count * 10)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
		notEnoughDrone = ((drone_count + drone_in_queue) < 6 * base_count);
	}

	// 判断需要建造多少部队
	int need_zergling_count = zergling_count + zergling_in_queue < 18 ? 2 : 0;

	int need_lurker_count = (int)(enemyTerranBarrackUnitsAmount * 0.75) - lurker_count - lurker_in_queue;
	if (need_lurker_count <= 0 && lurker_count + lurker_in_queue < 5)
	{
		need_lurker_count = 1;
	}

	if (notEnoughDrone)
	{
		need_lurker_count = lurker_count + lurker_in_queue < 3 ? 1 : 0;
		need_zergling_count = zergling_count + zergling_in_queue < 6 ? 2 : 0;
	}

	if (being_rushed && hydralisk_completed == 0)
	{
		need_zergling_count = 0;
	}

	// 穿插建造Zergling和Lurker
	do
	{
		if (need_zergling_count > 0)
		{
			// 2个Zergling
			if (spawning_pool_count > 0)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
			}
			need_zergling_count -= 2;
		}
		if (need_lurker_count > 0)
		{
			if (hydralisk_den_completed > 0
				&& (hydralisk_count + hydralisk_in_queue) <= 5)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk), enemyTerranBarrackUnitsAmount > 15);
			}
			if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
				&& lurker_count + lurker_in_queue < hydralisk_completed)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker), enemyTerranBarrackUnitsAmount > 15);
			}
			need_lurker_count--;
		}
		if (need_zergling_count <= 0 && need_lurker_count <= 0)
		{
			break;
		}

	} while (0);

	int extractorUpperBound = std::min(base_completed, 3);
	if (isExtractorExist
		&& extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

}

void ActionZVTBarracksUnits::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyTerranBarrackUnitsAmount = enemy_marine_count + enemy_firebat_count + enemy_ghost_count + enemy_medic_count;
	enemyTerranFactoryUnitsAmount = enemy_vulture_count * 2 + enemy_tank_count * 2 + enemy_goliath_count * 2;
	enemyTerranMechanizationRate = enemyTerranBarrackUnitsAmount == 0 ? 10 : (double)enemyTerranFactoryUnitsAmount / (double)enemyTerranBarrackUnitsAmount;
	if (enemyTerranFactoryUnitsAmount == 0) enemyTerranMechanizationRate = 0;
}

void ActionZVTBarracksUnits::tryAddInQueue(ProductionQueue & queue, const ProductionItem & item, bool priority)
{

}
