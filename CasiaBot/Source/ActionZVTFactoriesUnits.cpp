#include "ActionZVTFactoriesUnits.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVTFactoriesUnits::ActionZVTFactoriesUnits()
{
}

void ActionZVTFactoriesUnits::init()
{
	if (!isInitialized)
	{
		lastFrameCount = BWAPI::Broodwar->getFrameCount();
		lastFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		lastFrameGasAmount = BWAPI::Broodwar->self()->gas();
		isInitialized = true;
	}
}

bool ActionZVTFactoriesUnits::canDeployAction()
{
	if (enemyTerranMechanizationRate >= 0.8 || enemyTerranFactoryUnitsAmount > 8)
	{
		return true;
	}
	else
		return false;
}

bool ActionZVTFactoriesUnits::tick()
{
	if (enemyTerranMechanizationRate < 0.8 || enemyTerranFactoryUnitsAmount <= 8)
	{
		return true;
	}
	else
		return false;
}

void ActionZVTFactoriesUnits::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	// 当前帧数（累计）
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();

	if (base_count + base_in_queue + base_being_built <= 1)
	{
		if (spawning_pool_completed > 0 && drone_count > 11)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery), true);
		}
	}

	if (BWAPI::Broodwar->self()->minerals() > 500)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
	}

	// 判断前提建筑是否存在
	bool isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	if (!isSpawningPoolExist)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	bool isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;
	if (!isExtractorExist && drone_count >= 12 && spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

	if (lair_count + lair_being_built + lair_in_queue == 0)
	{
		if (currentFrameCount > 4320)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair));
		}
	}
	if (lair_completed > 0)
	{
		if (spire_count + spire_being_built + spire_in_queue == 0)
		{
			if (currentFrameCount > 6120)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spire), true);
			}
		}
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
	if (hive_completed > 0 && adrenal_glands_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}

	int currentFlyerCarapaceLevel = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);
	bool isFlyerCarapaceUpgrading = BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);
	if (!isFlyerCarapaceUpgrading
		&& currentFlyerCarapaceLevel < 1
		&& queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace) == 0
		&& spire_complete > 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace));
	}

	bool notEnoughDrone = false;
	if (base_count == 1)
	{
		if (drone_count + drone_in_queue < 15)
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
		notEnoughDrone = (drone_count + drone_in_queue) < 12;
	}
	else
	{
		if (drone_count + drone_in_queue < base_count * 10)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
		notEnoughDrone = drone_count + drone_in_queue < 7 * base_count;
	}

	if (enemy_army_supply < escalationMark)
	{
		// 判断需要建造多少部队
		int need_mutalisk_count = (int)((enemy_goliath_count + enemy_marine_count) * 1.2 + 2) - mutalisk_count - mutalisk_in_queue;
		if (need_mutalisk_count <= 0)
		{
			need_mutalisk_count = 1;
		}
		int need_defiler_count = 1;
		int need_zergling_count = (int)(enemyTerranBarrackUnitsAmount * 1.5) - zergling_count - zergling_in_queue;
		if (need_zergling_count <= 0 && zergling_count + zergling_in_queue < 12)
		{
			need_zergling_count = 2;
		}
		if (notEnoughDrone)
		{
			need_mutalisk_count = mutalisk_count + mutalisk_in_queue < 6 ? 1 : 0;
			need_defiler_count = 0;
			need_zergling_count = zergling_count + zergling_in_queue < 8 ? 2 : 0;
		}

		// 穿插建造Mutalisk、Defiler、Zergling
		do
		{
			if (need_zergling_count > 0)
			{
				// 2个Zergling
				if (spawning_pool_completed > 0)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
				}
				need_zergling_count -= 2;
			}
			if (need_mutalisk_count > 0)
			{
				if (spire_completed > 0)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk));
				}
				need_mutalisk_count--;
			}
			if (need_defiler_count > 0)
			{
				if (defiler_mound_count > 0)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Defiler));
				}
				need_defiler_count--;
			}
			if (need_mutalisk_count <= 0 && need_defiler_count <= 0 && need_zergling_count <= 0)
			{
				break;
			}

		} while (0);

		int extractorUpperBound = std::min(base_completed, 3);
		if (isExtractorExist && extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		}
	}
	else
	{
		bool isUltraliskCavernExist = ultralisk_cavern_being_built + ultralisk_cavern_count + ultralisk_cavern_in_queue > 0;
		if (!isUltraliskCavernExist)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern));
		}

		// 判断需要建造多少部队
		int need_mutalisk_count = (int)(enemy_goliath_count * 1.2 + 2) - mutalisk_count - mutalisk_in_queue;
		int need_defiler_count = 1;
		int need_zergling_count = (int)(enemyTerranBarrackUnitsAmount * 1.5) - zergling_count - zergling_in_queue;

		if (need_mutalisk_count <= 0 && mutalisk_count + mutalisk_in_queue < 6)
		{
			need_mutalisk_count = 1;
		}

		if (need_zergling_count <= 0 && zergling_count + zergling_in_queue < 12)
		{
			need_zergling_count = 2;
		}
		if (notEnoughDrone)
		{
			need_mutalisk_count = mutalisk_count + mutalisk_in_queue < 6 ? 1 : 0;
			need_defiler_count = 0;
			need_zergling_count = zergling_count + zergling_in_queue < 12 ? 2 : 0;
		}

		// 穿插建造Mutalisk、Defiler、Zergling、Ultralisk
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
			if (need_mutalisk_count > 0)
			{
				if (spire_count > 0)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk));
				}
				need_mutalisk_count--;
			}
			if (need_defiler_count > 0)
			{
				if (defiler_mound_count > 0)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Defiler));
				}
				need_defiler_count--;
			}
			if (ultralisk_cavern_count > 0)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Ultralisk));
			}
			if (need_mutalisk_count <= 0 && need_defiler_count <= 0 && need_zergling_count <= 0)
			{
				break;
			}

		} while (true);

		int extractorUpperBound = std::min(base_count + base_being_built, 3);
		if (isExtractorExist && extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		}
	}
}

void ActionZVTFactoriesUnits::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyTerranBarrackUnitsAmount = enemy_marine_count + enemy_firebat_count + enemy_ghost_count + enemy_medic_count;
	enemyTerranFactoryUnitsAmount = enemy_vulture_count * 2 + enemy_tank_count * 2 + enemy_goliath_count * 2;
	enemyTerranMechanizationRate = enemyTerranBarrackUnitsAmount == 0 ? 10 : (double)enemyTerranFactoryUnitsAmount / (double)enemyTerranBarrackUnitsAmount;
	if (enemyTerranFactoryUnitsAmount == 0) enemyTerranMechanizationRate = 0;
}

void ActionZVTFactoriesUnits::tryAddInQueue(ProductionQueue & queue, const ProductionItem & item, bool priority)
{
}
