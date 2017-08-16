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

void ActionZVPHydra::updateInformation(CasiaBot::ProductionQueue & queue)
{
	// 当前帧数（累计）
	currentFrameCount = BWAPI::Broodwar->getFrameCount();
	gas = BWAPI::Broodwar->self()->gas();
	minerals = BWAPI::Broodwar->self()->minerals();
	being_rushed = InformationManager::Instance().isEncounterRush();

	if (currentFrameCount % 72 == 0)
	{
		int diffMineralAmount = minerals - lastFrameMineralAmount;
		int diffGasAmount = gas - lastFrameGasAmount;
		mineralNetIncrease.pop_front();
		mineralNetIncrease.push_back(diffMineralAmount);
		gasNetIncrease.pop_front();
		gasNetIncrease.push_back(diffGasAmount);
		larvaQueue.pop_front();
		larvaQueue.push_back(larva_count - base_completed);
		lastFrameCount = currentFrameCount;
		lastFrameMineralAmount = minerals;
		lastFrameGasAmount = gas;
	}
	mineralDequePositive = IsDequeAllPositive(mineralNetIncrease);
	gasDequePositive = IsDequeNoneNegative(gasNetIncrease);
	larva_lacking = IsDequeAllNegative(larvaQueue);

	if (currentFrameCount % 200 == 10) 
	{
		patches = 0;
		for (const auto & b : InformationManager::Instance().getSelfBases()) {
			patches += WorkerManager::Instance().getMineralPatches(b).size();
		}
	}
	
	BWAPI::Broodwar->drawTextScreen(200, 280, "Patches : %d", patches);

	isSunkenColonyExist = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue > 0;
	isCreepColonyExist = creep_colony_count + creep_colony_being_built + creep_colony_in_queue > 0;
	sunken_colony_total = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue;
	creep_colony_total = creep_colony_count + creep_colony_being_built + creep_colony_in_queue;
	isHatcheryExist = hatchery_being_built + hatchery_count + hatchery_in_queue > 0;
	isHiveExist = hive_being_built + hive_count + hive_in_queue > 0;
	isQueenNestExist = queens_nest_being_built + queens_nest_count + queens_nest_in_queue > 0;
	isLairExist = lair_being_built + lair_count + lair_in_queue > 0;
	isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;
	isHydraliskDenExist = hydralisk_den_being_built + hydralisk_den_count + hydralisk_den_in_queue > 0;

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

}

void ActionZVPHydra::economy(CasiaBot::ProductionQueue & queue, int mode)
{
	switch (mode)
	{
	case 1:
	{
		
	}

	case 2:
	{

	}
	default:
		break;
	}
}

void ActionZVPHydra::army(CasiaBot::ProductionQueue & queue, int mode)
{
	switch (mode)
	{
	case 1:
		//being rushed
	{

		//train some force
		if (larva_count > 1) {
			if (sunken_colony_total > 2 &&
				drone_count + drone_in_queue < 12)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, DRONE!");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), larva_count > 2);
			}
			else if (minerals > 300 && gas > 50 && isHydraliskDenExist && hydralisk_in_queue < 2)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, Hydra!");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk), larva_count > 2 && gas > 100);
			}
			else if (minerals > 200 && zergling_count + zergling_in_queue < 7)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, Zergling!");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Zergling), larva_count > 2);
			}
		}
	}
	case 2:
	{
		if (!isHydraliskDenExist) {
			if (isExtractorExist && gas > 50)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den), true);
			}
			if (zergling_in_queue < 3 && zergling_count < 17)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Zergling));
			}
		}
		else if (isHydraliskDenExist)
		{
			BWAPI::Broodwar->drawTextScreen(300, 300, "more hydralisks");
			if (hydralisk_in_queue < 3)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
			}

			if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect) &&
				gas > 100 && lurker_count * 5 < hydralisk_count
				&& enemy_air_army_supply < 10)
			{
				BWAPI::Broodwar->drawTextScreen(350, 300, "want lurkers");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lurker));
			}
			else if (lurker_aspect_count == 0 && hydralisk_count > 15 &&
				enemy_air_army_supply < 8 && lurker_aspect_count > 0
				&& lair_completed)
			{
				BWAPI::Broodwar->drawTextScreen(250, 290, "Are we looking for lurkers?");
				tryAddInQueue(queue, MetaType(BWAPI::TechTypes::Lurker_Aspect));
			}
		}
	}

	case 3:
	{
		if (hydralisk_in_queue < 3)
		{
			BWAPI::Broodwar->drawTextScreen(200, 310, "why not more hydras");
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
		}

		if (hydralisk_den_completed > 0 && (lair_completed > 0 || hive_completed > 0) &&
			lurker_aspect_count == 0 && gas >= 125 && hydralisk_count + hydralisk_in_queue > 15)
		{
			tryAddInQueue(queue, MetaType(BWAPI::TechTypes::Lurker_Aspect));
		}

		if (hydralisk_den_completed > 0 &&
			BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
			&& lurker_count * 5 < hydralisk_count
			&& enemy_air_army_supply < 10)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lurker));
		}

	}
	default:
		break;
	}
}

void ActionZVPHydra::staticDefend(CasiaBot::ProductionQueue & queue)
{
	if (!isCreepColonyExist && !isSunkenColonyExist)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
		while (drone_count + drone_in_queue < 9)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
	}
	else
	{
		//BWAPI::Broodwar->drawTextScreen(200, 260, "Here, for defend building %d, %d",
		//	creep_colony_in_queue + creep_colony_being_built, creep_colony_count);
		if (sunken_colony_total < 2 + being_rushed)
		{
			if (creep_colony_in_queue + creep_colony_being_built + creep_colony_count + sunken_colony_total< 3
				&& creep_colony_in_queue < 2)
			{
				//BWAPI::Broodwar->drawTextScreen(200, 280, "Are we building Creep Colony?");
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), 
					creep_colony_being_built < 1 && being_rushed);
				if (drone_count + drone_in_queue < 12)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
				}
			}
		}
	}
	if (0 < creep_colony_completed)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony),
			sunken_colony_count + sunken_colony_being_built < 3 && being_rushed);
	}
}

void ActionZVPHydra::tech(CasiaBot::ProductionQueue & queue, int mode)
{
	switch (mode)
	{
	case 1:
		//being rushed
	{
		if (gas > 200 && minerals > 400 || (larva_count < 2 && gas > 180 && minerals > 220))
		{
			BWAPI::Broodwar->drawTextScreen(200, 300, "update tech");
			if (!muscular_argument_completing)
			{
				BWAPI::Broodwar->drawTextScreen(300, 300, "no speed");
				if (hydralisk_den_completed > 0 && hydralisk_count + hydralisk_in_queue > 7)
				{
					BWAPI::Broodwar->drawTextScreen(400, 300, "update speed");
					tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
				}
			}
			else if (!grooved_spines_completing) {
				BWAPI::Broodwar->drawTextScreen(300, 300, "no range");
				if (hydralisk_den_completed > 0 && hydralisk_count + hydralisk_in_queue > 12)
				{
					BWAPI::Broodwar->drawTextScreen(400, 300, "update range");
					tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
				}
			}
		}
	}

	case 2: 
	{
		if (gas > 150 && minerals > 200 )
		{
			BWAPI::Broodwar->drawTextScreen(200, 300, "update tech");
			if (!muscular_argument_completing)
			{
				BWAPI::Broodwar->drawTextScreen(300, 300, "no speed");
				if (hydralisk_den_completed > 0 && hydralisk_count + hydralisk_in_queue > 7
					&& gas > 150 && minerals > 150)
				{
					BWAPI::Broodwar->drawTextScreen(400, 300, "update speed");
					tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
				}
			}
			else if (!grooved_spines_completing) {
				BWAPI::Broodwar->drawTextScreen(300, 300, "no range");
				if (hydralisk_den_completed > 0 && hydralisk_count + hydralisk_in_queue > 12)
				{
					BWAPI::Broodwar->drawTextScreen(400, 300, "update range");
					tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
				}
			}
		}
		//base tech
		if (!isHiveExist &&
			(mineralDequePositive && gasDequePositive ||
			(minerals > 500 && gas > 300)))	// 若蜂巢不存在
		{
			if (isQueenNestExist)	// 若皇后巢存在
			{
				BWAPI::Broodwar->drawTextScreen(200, 340, "for the queen");
				if (currentFrameCount > 10800 && army_supply > 40)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hive));
				}
			}
			else	// 若皇后巢不存在
			{
				BWAPI::Broodwar->drawTextScreen(200, 340, "Long live the king");
				if (isLairExist)	// 若兽穴存在
				{
					if (currentFrameCount > 9000 && army_supply > 20)
					{
						tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
					}
				}
				else if (currentFrameCount > 4800 && hydralisk_count > 12)	// 若兽穴不存在
				{
					BWAPI::Broodwar->drawTextScreen(200, 320, "lion lair");
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lair));
				}
			}
		}
	}
		

	case 3:
		// normal
	{
		if (!muscular_argument_completing)
		{
			if (hydralisk_den_completed > 0 && minerals > 150 &&
				gas >= 150 && hydralisk_count + hydralisk_in_queue > 7)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
			}
		}
		else if (!grooved_spines_completing) {
			if (hydralisk_den_completed > 0 && gas >= 150 &&
				hydralisk_count + hydralisk_in_queue > 12)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
			}
		}

		//base tech
		if (!isHiveExist &&
			(mineralDequePositive && gasDequePositive ||
			(minerals > 400 && gas > 200)))	// 若蜂巢不存在
		{
			if (isQueenNestExist)	// 若皇后巢存在
			{
				BWAPI::Broodwar->drawTextScreen(200, 340, "for the queen");
				if (currentFrameCount > 10800 && army_supply > 40)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hive));
				}
			}
			else	// 若皇后巢不存在
			{
				BWAPI::Broodwar->drawTextScreen(200, 340, "Long live the king");
				if (isLairExist)	// 若兽穴存在
				{
					if (currentFrameCount > 9000 && army_supply > 20)
					{
						tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
					}
				}
				else if (currentFrameCount > 4800 && hydralisk_count > 12)	// 若兽穴不存在
				{
					BWAPI::Broodwar->drawTextScreen(200, 320, "lion lair");
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lair));
				}
			}
		}



		//TODO
		//gongfang

	}
	default:
		break;
	}
	
	
}

void ActionZVPHydra::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	updateInformation(queue);

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
		able_defend = sunken_colony_count >= 3 || army_supply > 15;

		staticDefend(queue);
		army(queue, 1);
		tech(queue, 1);
	}
	//not rushed
	else {
		BWAPI::Broodwar->drawTextScreen(200, 260, "larvaQ, %d, %d, %d ,%d, %d",
			larvaQueue.at(0), larvaQueue.at(1), larvaQueue.at(2), larvaQueue.at(3), 
			larva_lacking ? 1 : 0);
		BWAPI::Broodwar->drawTextScreen(200, 250, "We are here, with able_defend %d", able_defend);
		able_defend = sunken_colony_count >= 3 || army_supply > 15;

		if (!isExtractorExist && spawning_pool_count > 0)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
		}

		//we need some zerglings for the very beginning game
		if (currentFrameCount < 5040 && zergling_count < 7 &&
			zergling_in_queue < 1 && spawning_pool_completed)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Zergling), zergling_count < 5);
		}
		
		if (drone_count > 11)
		{
			staticDefend(queue);
		}
		//more bases
		if (larva_lacking && minerals > 400 && gas > 100 &&
			base_count - real_base_count < 2 &&
			hatchery_in_queue < 1)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main"));
		}

		//need more army force
		else if (enemy_army_supply > army_supply * 1.3 && larva_count > 0 ) {

			BWAPI::Broodwar->drawTextScreen(200, 290, "too few army, enemy %d, self %d",
				enemy_army_supply, army_supply);
			
			army(queue, 2);
			tech(queue, 2);
		}
		
		//train more workers
		else
		{
			if (drone_count + drone_in_queue < patches * 1.8 + extractor_count * 3
				&& drone_in_queue < 2 && larva_count > 0)
			{
				BWAPI::Broodwar->drawTextScreen(200, 330, "more drones! %d",
					(larva_count) / real_base_count > 1);
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
			}

			//search for more treasures
			
			else if (opponent_has_expanded || real_base_count < enemy_nexus_count ||
				drone_count + drone_in_queue >= real_base_count * 1.7 + extractor_count * 3 &&
				real_base_count + hatchery_in_queue < int(double(currentFrameCount) / 7200 + 0.3) &&
				hatchery_in_queue < 2 && minerals > 350
				)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
			}
			
			//train more army & upgrade tech
			//else
			{
				BWAPI::Broodwar->drawTextScreen(200, 320, "tech is first power %d", 
					int(double(currentFrameCount) / 7200 + 0.3));
				tech(queue, 3);
				army(queue, 3);
			}
		}

		//补气矿
		int extractorUpperBound = std::min(base_completed, 3);
		if (isExtractorExist &&
			extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound &&
			extractor_in_queue < 1 &&
			((gas < 100 && minerals > 450) ||
			currentFrameCount / extractor_count < 9600))
		{
			BWAPI::Broodwar->drawTextScreen(200, 330, "GAS!GAS!GAS!");
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		}
	}
}

void ActionZVPHydra::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyDragoonOverZealotRate = enemy_zealot_count == 0 ? 10 : (double)enemy_dragoon_count / (double)enemy_zealot_count;
	if (enemy_dragoon_count == 0) enemyDragoonOverZealotRate = 0;
	//droneLimit = real_base_count == 1 ? 16 : real_base_count * 12 + extractor_count * 3;
}

void ActionZVPHydra::tryAddInQueue(ProductionQueue & queue, const ProductionItem & item, bool priority)
{
	const MetaType & unit = item._unit;
	auto unitType = unit.getUnitType();
	auto upgradeType = unit.getUpgradeType();
	auto techType = unit.getTechType();

	if (unitType == BWAPI::UnitTypes::Zerg_Drone)
	{
		//if (drone_count + drone_in_queue < droneLimit)
		{
			queue.add(item, priority);
			drone_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Zergling)
	{
		if (spawning_pool_count > 0)// && hydralisk_count + hydralisk_in_queue < zerglingLimit)
		{
			queue.add(item, priority);
			hydralisk_in_queue += 2;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Hydralisk)
	{
		if (hydralisk_den_count > 0)
		{
			queue.add(item, priority);
			hydralisk_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Lurker)
	{
		if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
			&& lurker_in_queue < hydralisk_completed)
			//&& lurker_count + lurker_in_queue < lurkerLimit)
		{
			queue.add(item, priority);
			lurker_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Creep_Colony)
	{
		//if (creep_colony_count + creep_colony_being_built + creep_colony_in_queue < creepColonyLimit)
		{
			queue.add(item, priority);
			creep_colony_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Sunken_Colony)
	{
		if (sunken_colony_in_queue < creep_colony_completed)
			//&& sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < sunkenColonyLimit)
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
		//if (extractor_being_built + extractor_count + extractor_in_queue < extractorLimit)
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
		//if (hatchery_being_built + hatchery_count + hatchery_in_queue < hatcheryLimit)
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
		if (hydralisk_den_completed > 0 && 
			(lair_completed > 0 || hive_completed > 0) && 
			lurker_aspect_count < lurkerAspectLimit)
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
