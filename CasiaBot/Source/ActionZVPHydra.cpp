#include "ActionZVPHydra.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVPHydra::ActionZVPHydra()
{
}

void ActionZVPHydra::init()
{
	if (!isInitialized)
	{
		lastFrameCount = BWAPI::Broodwar->getFrameCount();
		lastFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		lastFrameGasAmount = BWAPI::Broodwar->self()->gas();
		isInitialized = true;
	}
}

bool ActionZVPHydra::canDeployAction()
{
	if (true)
		//waiting for more circumstances
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ActionZVPHydra::tick()
{
	if (true)
		//waiting for more circumstances
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ActionZVPHydra::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	// 当前帧数（累计）
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();
	int gas = BWAPI::Broodwar->self()->gas();
	int minerals = BWAPI::Broodwar->self()->minerals();
	being_rushed = InformationManager::Instance().isEncounterRush();

	if (currentFrameCount % 200 == 0)
	{

		int diffMineralAmount = minerals - lastFrameMineralAmount;
		int diffGasAmount = gas - lastFrameGasAmount;
		mineralNetIncrease.pop_front();
		mineralNetIncrease.push_back(diffMineralAmount);
		gasNetIncrease.pop_front();
		gasNetIncrease.push_back(diffGasAmount);
		lastFrameCount = currentFrameCount;
		lastFrameMineralAmount = minerals;
		lastFrameGasAmount = gas;

	}
	bool mineralDequePositive = IsDequeAllPositive(mineralNetIncrease);
	bool gasDequePositive = IsDequeNoneNegative(gasNetIncrease);

	bool isSunkenColonyExist = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue > 0;
	bool isCreepColonyExist = creep_colony_count + creep_colony_being_built + creep_colony_in_queue > 0;
	int sunken_colony_total = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue;
	int creep_colony_total = creep_colony_count + creep_colony_being_built + creep_colony_in_queue;
	bool isHatcheryExist = hatchery_being_built + hatchery_count + hatchery_in_queue > 0;
	bool isHiveExist = hive_being_built + hive_count + hive_in_queue > 0;
	bool isQueenNestExist = queens_nest_being_built + queens_nest_count + queens_nest_in_queue > 0;
	bool isLairExist = lair_being_built + lair_count + lair_in_queue > 0;
	bool isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	bool isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;
	bool isHydraliskDenExist = hydralisk_den_being_built + hydralisk_den_count + hydralisk_den_in_queue > 0;
	//TODO
	larva_lacking = false;


	// 每200帧检查队列一次
	if ((currentFrameCount + 100) % 200 == 0)
	{
		if (!isSpawningPoolExist && hydralisk_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isSpawningPoolExist && hydralisk_den_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isHatcheryExist && lair_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isLairExist && hive_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isHydraliskDenExist && hydralisk_in_queue > 0)
		{
			queue.clear();
		}
		else if (!(lurker_count + lurker_in_queue < hydralisk_completed))
		{
			queue.clear();
		}
		else if (!isCreepColonyExist && sunken_colony_in_queue > 0)
		{
			queue.clear();
		}
		if (hydralisk_count + hydralisk_in_queue > zerglingLimit)
		{
			queue.clear();
		}
		// 农民过少时
		if (drone_count < 5 && drone_in_queue < 1)
		{
			queue.clear();
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
	}

	if (!isSpawningPoolExist)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	if (!isExtractorExist && drone_count >= 11 && spawning_pool_count > 0)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
	}

	if (isSpawningPoolExist && !isHydraliskDenExist && gas > 50)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den), true);
	}

	//being rushed
	if (being_rushed && !able_defend)
	{
		BWAPI::Broodwar->drawTextScreen(200, 240, "We are rushed, with able defend %d", able_defend);
		able_defend = sunken_colony_total > 4 || army_supply > 20;
		if (!isCreepColonyExist && !isSunkenColonyExist)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
			if (drone_count + drone_in_queue < 9)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
			}
		}
		else
		{
			BWAPI::Broodwar->drawTextScreen(200, 260, "Here, for defend building %d, %d", 
				creep_colony_in_queue + creep_colony_being_built, creep_colony_count);
			if (sunken_colony_total < 4)
			{
				if (creep_colony_in_queue + creep_colony_being_built + creep_colony_count + sunken_colony_count< 5 
					&& creep_colony_in_queue < 2)
				{
					BWAPI::Broodwar->drawTextScreen(200, 280, "Are we building Creep Colony?");
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), creep_colony_in_queue < 1);
					if (drone_count + drone_in_queue < 12)
					{
						tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
					}
				}
			}
		}
		if (0 < creep_colony_completed)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), sunken_colony_total < 3);
		}

		if (larva_count > 2) {
			if (sunken_colony_total > 2 &&
				drone_count + drone_in_queue < 12)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, DRONE!");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
			}
			else if (minerals > 300 && gas > 50 && hydralisk_den_count > 0 && hydralisk_in_queue < 2) 
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, Hydra!");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk), true);
			}
			else if (minerals > 200 && zergling_count + zergling_in_queue < 10)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, Zergling!");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Zergling), true);
			}
		}

	}
	//not rushed
	else {
		BWAPI::Broodwar->drawTextScreen(200, 250, "We are here, with able_defend %d", able_defend);
		able_defend = sunken_colony_total > 4 || army_supply > 20;
		int extractorUpperBound = std::min(base_completed - 1, 3);
		if (!isExtractorExist)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		}

		if (enemy_army_supply > army_supply * 1.1) {
			BWAPI::Broodwar->drawTextScreen(200, 290, "too few army");
			if (!isHydraliskDenExist) {
				if (isExtractorExist && gas > 50)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den));
				}
				if (zergling_in_queue < 3 && zergling_count < 20)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Zergling));
				}
			}
			else if (isHydraliskDenExist)
			{
				BWAPI::Broodwar->drawTextScreen(250, 290, "more hydralisks");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
				if (lurker_aspect_count > 0 && gas > 100 && lurker_count * 5 < hydralisk_count
					&& enemy_air_army_supply < 10)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lurker));
				}
				else if (lurker_aspect_count == 0 && hydralisk_count > 15 && 
					enemy_air_army_supply < 8 && lurker_aspect_count > 0)
				{
					BWAPI::Broodwar->drawTextScreen(250, 290, "Are we looking for lurkers?");
					tryAddInQueue(queue, MetaType(BWAPI::TechTypes::Lurker_Aspect));
				}
			}

			if (gas > 300 && minerals > 500)
			{
				BWAPI::Broodwar->drawTextScreen(200, 300, "update tech");
				if (!muscular_argument_completing)
				{
					if (hydralisk_den_completed > 0 && hydralisk_count + hydralisk_in_queue > 7)
					{
						tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
					}
				}
				else if (!grooved_spines_completing) {
					if (hydralisk_den_completed > 0 && hydralisk_count + hydralisk_in_queue > 12)
					{
						tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
					}
				}
			}
		}
		//build more bases
		else if (larva_lacking && mineralDequePositive)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main"));
		}
		else if (opponent_has_expanded ||
			drone_count + drone_in_queue >= InformationManager::Instance().getSelfBases().size() * 12 + extractor_count * 3
			&& InformationManager::Instance().getSelfBases().size() < currentFrameCount / 6000
			)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
		}
		//train more workers
		else if (drone_count + drone_in_queue < InformationManager::Instance().getSelfBases().size() * 12 + extractor_count * 3
			&& drone_in_queue < 2 &&
			(larva_count) / InformationManager::Instance().getSelfBases().size() > 1)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
		//train more army & upgrade tech
		else
		{
			if (!muscular_argument_completing)
			{
				if (hydralisk_den_completed > 0 && gas >= 150 && hydralisk_count + hydralisk_in_queue > 7)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
				}
			}
			else if (!grooved_spines_completing) {
				if (hydralisk_den_completed > 0 && gas >= 150 && hydralisk_count + hydralisk_in_queue > 12)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
				}
			}
			else if (!isHiveExist)	// 若蜂巢不存在
			{
				if (isQueenNestExist)	// 若皇后巢存在
				{
					if (currentFrameCount > 10800 && army_supply > 40)
					{
						tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hive));
					}
				}
				else	// 若皇后巢不存在
				{
					if (isLairExist)	// 若兽穴存在
					{
						if (currentFrameCount > 9000 && army_supply > 20)
						{
							tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
						}
					}
					else if (currentFrameCount > 4800 && hydralisk_count > 15)	// 若兽穴不存在
					{
						tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lair));
					}
				}
			}

			if (hydralisk_in_queue < 3)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
			}

			if (hydralisk_den_completed > 0 && (lair_completed > 0 || hive_completed > 0) &&
				lurker_aspect_count == 0 && gas >= 125 && hydralisk_count + hydralisk_in_queue > 15)
			{
				tryAddInQueue(queue, MetaType(BWAPI::TechTypes::Lurker_Aspect));
			}

			//补气矿
			int extractorUpperBound = std::min(base_completed - 1, 3);
			if (isExtractorExist && extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound && gas < 300)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor));
			}
		}
	}
}

void ActionZVPHydra::getBuildOrderListNew(CasiaBot::ProductionQueue & queue)
{
	// 当前帧数（累计）
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();
	int gas = BWAPI::Broodwar->self()->gas();
	int minerals = BWAPI::Broodwar->self()->minerals();
	being_rushed = InformationManager::Instance().isEncounterRush();

	if (currentFrameCount % 200 == 0)
	{

		int diffMineralAmount = minerals - lastFrameMineralAmount;
		int diffGasAmount = gas - lastFrameGasAmount;
		mineralNetIncrease.pop_front();
		mineralNetIncrease.push_back(diffMineralAmount);
		gasNetIncrease.pop_front();
		gasNetIncrease.push_back(diffGasAmount);
		lastFrameCount = currentFrameCount;
		lastFrameMineralAmount = minerals;
		lastFrameGasAmount = gas;
	}
	bool mineralDequePositive = IsDequeAllPositive(mineralNetIncrease);
	bool gasDequePositive = IsDequeNoneNegative(gasNetIncrease);

	bool isSunkenColonyExist = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue > 0;
	bool isCreepColonyExist = creep_colony_count + creep_colony_being_built + creep_colony_in_queue > 0;
	int sunken_colony_total = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue;
	int creep_colony_total = creep_colony_count + creep_colony_being_built + creep_colony_in_queue;
	bool isHatcheryExist = hatchery_being_built + hatchery_count + hatchery_in_queue > 0;
	bool isHiveExist = hive_being_built + hive_count + hive_in_queue > 0;
	bool isQueenNestExist = queens_nest_being_built + queens_nest_count + queens_nest_in_queue > 0;
	bool isLairExist = lair_being_built + lair_count + lair_in_queue > 0;
	bool isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	bool isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;
	bool isHydraliskDenExist = hydralisk_den_being_built + hydralisk_den_count + hydralisk_den_in_queue > 0;

	// 每200帧检查队列一次
	if ((currentFrameCount + 100) % 200 == 0)
	{
		if (!isSpawningPoolExist && hydralisk_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isSpawningPoolExist && hydralisk_den_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isHatcheryExist && lair_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isLairExist && hive_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isHydraliskDenExist && hydralisk_in_queue > 0)
		{
			queue.clear();
		}
		else if (!(lurker_count + lurker_in_queue < hydralisk_completed))
		{
			queue.clear();
		}
		else if (!isCreepColonyExist && sunken_colony_in_queue > 0)
		{
			queue.clear();
		}
		if (hydralisk_count + hydralisk_in_queue > zerglingLimit)
		{
			queue.clear();
		}
		// 农民过少时
		if (drone_count < 5 && drone_in_queue < 1)
		{
			queue.clear();
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
	}

	//是否需要防御建筑
	if (!isCreepColonyExist && !isSunkenColonyExist)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
	}
	else
	{
		if (!isCreepColonyExist && isSunkenColonyExist)
		{
			if (sunken_colony_total < (being_rushed ? 7 : 3))
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), being_rushed);
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), being_rushed);
			}
		}
	}
	if (sunken_colony_in_queue + sunken_colony_being_built < creep_colony_completed)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), true);
	}

	//if (creep_colony_count + creep_colony_being_built + creep_colony_in_queue +
	//	sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < 2)
	//{
	//	tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
	//	//tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
	//}
	//else if (being_rushed && creep_colony_completed + sunken_colony_count >= 2 &&
	//	creep_colony_count + creep_colony_being_built + creep_colony_in_queue +
	//	sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < 5)
	//{
	//	tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
	//	//tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
	//}
	//else if (being_rushed && creep_colony_completed + sunken_colony_count >= 5 &&
	//	creep_colony_count + creep_colony_being_built + creep_colony_in_queue +
	//	sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < 7)
	//{
	//	tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
	//	//tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
	//}
	//if (sunken_colony_in_queue < creep_colony_completed)
	//{
	//	tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), true);
	//	tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
	//	//tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Zergling), true);
	//}

	// 判断前提建筑是否存在
	if (!isSpawningPoolExist)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	if (!isExtractorExist && drone_count >= 10 && spawning_pool_count > 0)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
	}

	if (!isHydraliskDenExist && gas >= 50)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den));
	}

	// 判断是否需要增加母巢
	//attention
	if (currentFrameCount % 200 == 0 && base_count + base_in_queue + base_being_built <= 4 && currentFrameCount > 10)
	{
		if (base_count + base_in_queue + base_being_built <= 1)
		{
			if ((lair_count > 0 || hive_count > 0) && hydralisk_count >= 4)
			{
				ProductionItem item = MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main");
				tryAddInQueue(queue, item);
			}
		}
		else if (base_count + base_in_queue + base_being_built <= 2)
		{
			if ((lair_completed > 0 || hive_completed > 0) && hydralisk_count >= 8 && minerals > 400)
			{
				if (real_base_count == base_count + base_in_queue + base_being_built)
				{
					ProductionItem item = MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main");
					tryAddInQueue(queue, item);
				}
				else
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
				}
			}
		}
		else if (base_count + base_in_queue + base_being_built <= 3)
		{
			if (mineralDequePositive && minerals > 500)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
			}
		}
		else
		{
			if (isHiveExist && mineralDequePositive && gasDequePositive)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
			}
		}
	}

	bool notEnoughDrone = false;
	if (real_base_count == 1)
	{
		if (drone_count + drone_in_queue < 7 + 3 * (extractor_count > 0))
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
		else if (hydralisk_count >= 6 && drone_count + drone_in_queue < 15)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
		notEnoughDrone = drone_count + drone_in_queue < 14 + 3 * (extractor_count > 0);
	}
	else
	{
		if (drone_count + drone_in_queue < real_base_count * 10 + extractor_count * 3)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
		notEnoughDrone = drone_count + drone_in_queue < 14 * real_base_count + extractor_count * 3;
	}

	// 判断需要建造多少部队
	double need_hydra_count = 0;
	if (isSpawningPoolExist)
	{
		//首先根据敌方单位数量判断
		need_hydra_count = std::max(need_hydra_count, enemy_zealot_count * 1.3 + enemy_dragoon_count * 1.2 - hydralisk_count - hydralisk_in_queue);
		if (need_hydra_count < 2)
		{
			//保证数量
			if (hydralisk_count + hydralisk_in_queue < 20)
				need_hydra_count = 2;
			//在资源富余的情况下继续生产
			if (mineralDequePositive && isExtractorExist && gasDequePositive  && hydralisk_in_queue < 6)
				need_hydra_count = 2;
		}
		else if (need_hydra_count > 2)
		{
			need_hydra_count = 2;
		}
		if (hydralisk_count + hydralisk_in_queue > 20)
		{
			need_hydra_count = 0;
		}
		if (hydralisk_count + hydralisk_in_queue < 12)
		{
			need_hydra_count = 4;
		}

		//优先补农民
		if (notEnoughDrone && hydralisk_count + hydralisk_in_queue >= 15)
		{
			need_hydra_count = 0;
		}
	}
	double need_lurker_count = 0;
	if (isHydraliskDenExist && (isLairExist || isHiveExist))
	{
		//首先根据敌方单位数量判断
		need_lurker_count = std::max(need_lurker_count, need_hydra_count / 5);
		if (need_lurker_count < 1) {
			//保证数量
			if (lurker_count + lurker_in_queue < 6)
				need_lurker_count = 1;
			//在资源富余的情况下继续生产
			if (isHiveExist && mineralDequePositive && isExtractorExist && gasDequePositive && lurker_in_queue < 2)
				need_lurker_count = 1;
		}
		//优先补农民
		if (notEnoughDrone && lurker_count + lurker_in_queue >= 3)
		{
			need_lurker_count = 0;
		}
	}

	// 穿插建造Zergling和Lurker
	do
	{
		if (need_hydra_count > 0)
		{
			// 2个Zergling
			if (spawning_pool_count > 0)
			{
				if (currentFrameCount < 7200 && hydralisk_count + hydralisk_in_queue < 12)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk), true);
				}
				else
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
				}
			}
			need_hydra_count -= 2;
		}
		if (need_lurker_count > 0)
		{
			if (hydralisk_den_completed > 0
				&& hydralisk_count + hydralisk_in_queue <= 5)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
			}
			if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
				&& lurker_in_queue < hydralisk_completed)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lurker));
			}
			need_lurker_count--;
		}
		if (need_hydra_count <= 0 && need_lurker_count <= 0)
		{
			break;
		}

	} while (true);

	if (!isHiveExist)	// 若蜂巢不存在
	{
		if (isQueenNestExist)	// 若皇后巢存在
		{
			if (currentFrameCount > 10800)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hive));
			}
		}
		else	// 若皇后巢不存在
		{
			if (isLairExist)	// 若兽穴存在
			{
				if (currentFrameCount > 9000)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
				}
			}
			else if (currentFrameCount > 4800)	// 若兽穴不存在
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lair));
			}
		}
	}

	if (!muscular_argument_completing) 
	{
		if (hydralisk_den_completed > 0 && gas >= 150 && hydralisk_count + hydralisk_in_queue > 7)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
		}
	}
	else if (!grooved_spines_completing) {
		if (hydralisk_den_completed > 0 && gas >= 150 && hydralisk_count + hydralisk_in_queue > 12)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
		}
	}

	if (hydralisk_den_completed > 0 && (lair_completed > 0 || hive_completed > 0) &&
		lurker_aspect_count == 0 && gas >= 125 && hydralisk_count + hydralisk_in_queue > 15)
	{
		tryAddInQueue(queue, MetaType(BWAPI::TechTypes::Lurker_Aspect));
	}

	//补气矿
	int extractorUpperBound = std::min(base_completed, 3);
	if (isExtractorExist && extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound && gas < 600)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}
}

void ActionZVPHydra::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyDragoonOverZealotRate = enemy_zealot_count == 0 ? 10 : (double)enemy_dragoon_count / (double)enemy_zealot_count;
	if (enemy_dragoon_count == 0) enemyDragoonOverZealotRate = 0;
	droneLimit = real_base_count == 1 ? 16 : real_base_count * 12 + extractor_count * 3;
}

void ActionZVPHydra::tryAddInQueue(ProductionQueue & queue, const ProductionItem & item, bool priority)
{
	const MetaType & unit = item._unit;
	auto unitType = unit.getUnitType();
	auto upgradeType = unit.getUpgradeType();
	auto techType = unit.getTechType();

	if (unitType == BWAPI::UnitTypes::Zerg_Drone)
	{
		if (drone_count + drone_in_queue < droneLimit)
		{
			queue.add(item, priority);
			drone_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Zergling)
	{
		if (spawning_pool_count > 0 && hydralisk_count + hydralisk_in_queue < zerglingLimit)
		{
			queue.add(item, priority);
			hydralisk_in_queue += 2;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Hydralisk)
	{
		if (hydralisk_den_count > 0 && hydralisk_count + hydralisk_in_queue < hydraliskLimit)
		{
			queue.add(item, priority);
			hydralisk_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Lurker)
	{
		if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
			&& lurker_in_queue < hydralisk_completed
			&& lurker_count + lurker_in_queue < lurkerLimit)
		{
			queue.add(item, priority);
			lurker_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Creep_Colony)
	{
		if (creep_colony_count + creep_colony_being_built + creep_colony_in_queue < creepColonyLimit)
		{
			queue.add(item, priority);
			creep_colony_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Sunken_Colony)
	{
		if (sunken_colony_in_queue < creep_colony_completed
			&& sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < sunkenColonyLimit)
		{
			queue.add(item, priority);
			sunken_colony_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Spawning_Pool)
	{
		if (spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue < spawningPoolLimit)
		{
			queue.add(item, priority);
			spawning_pool_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Extractor)
	{
		if (extractor_being_built + extractor_count + extractor_in_queue < extractorLimit)
		{
			queue.add(item, priority);
			extractor_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Hydralisk_Den)
	{
		if (hydralisk_den_being_built + hydralisk_den_count + hydralisk_den_in_queue < hydraliskDenLimit)
		{
			queue.add(item, priority);
			hydralisk_den_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Hatchery)
	{
		if (hatchery_being_built + hatchery_count + hatchery_in_queue < hatcheryLimit)
		{
			queue.add(item, priority);
			hatchery_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Lair)
	{
		if (lair_being_built + lair_count + lair_in_queue < lairLimit
			&& hive_being_built + hive_count + hive_in_queue < hiveLimit)
		{
			queue.add(item, priority);
			lair_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Hive)
	{
		if (hive_being_built + hive_count + hive_in_queue < hiveLimit)
		{
			queue.add(item, priority);
			hive_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Queens_Nest)
	{
		if (queens_nest_being_built + queens_nest_count + queens_nest_in_queue < queenNestLimit)
		{
			queue.add(item, priority);
			queens_nest_in_queue++;
		}
	}
	else if (upgradeType == BWAPI::UpgradeTypes::Muscular_Augments) {
		if (hydralisk_den_completed > 0 && !muscular_argument_completing)
		{
			queue.add(item, priority);
			muscular_argument_completing = true;
		}
	}
	else if (upgradeType == BWAPI::UpgradeTypes::Grooved_Spines) {
		if (hydralisk_den_completed > 0 && !grooved_spines_completing)
		{
			queue.add(item, priority);
			grooved_spines_completing = true;
		}
	}
	else if (techType == BWAPI::TechTypes::Lurker_Aspect)
	{
		if (hydralisk_den_completed > 0 && (lair_completed > 0 || hive_completed > 0) && lurker_aspect_count < lurkerAspectLimit)
		{
			queue.add(item, priority);
			lurker_aspect_count++;
		}
	}
	else
	{
		queue.add(item, priority);
	}
}
