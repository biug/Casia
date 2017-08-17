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
		return false;
	}
	else
	{
		return true;
	}
}

void ActionZVPHydra::updateInformation(CasiaBot::ProductionQueue & queue)
{
	// µ±Ç°Ö¡Êý£¨ÀÛ¼Æ£©
	currentFrameCount = BWAPI::Broodwar->getFrameCount();
	gas = BWAPI::Broodwar->self()->gas();
	minerals = BWAPI::Broodwar->self()->minerals();
	_status.being_rushed = InformationManager::Instance().isEncounterRush();
	
	if (currentFrameCount % 72 == 0)
	{
		_status.larvaQueue.pop_front();
		_status.larvaQueue.push_back(_status.larva_count - _status.base_completed);
	}
	larva_lacking = IsDequeAllNegative(_status.larvaQueue);

	if (currentFrameCount % 200 == 10) 
	{
		patches = 0;
		for (const auto & b : InformationManager::Instance().getSelfBases()) {
			patches += WorkerManager::Instance().getMineralPatches(b).size();
		}
	}

	isSunkenColonyExist = _status.sunken_colony_count + 
		_status.sunken_colony_being_built + 
		_status.sunken_colony_in_queue > 0;
	isCreepColonyExist = _status.creep_colony_count + 
		_status.creep_colony_being_built + 
		_status.creep_colony_in_queue > 0;
	sunken_colony_total = _status.sunken_colony_count + 
		_status.sunken_colony_being_built + 
		_status.sunken_colony_in_queue;
	creep_colony_total = _status.creep_colony_count + 
		_status.creep_colony_being_built + 
		_status.creep_colony_in_queue;
	isHatcheryExist = _status.hatchery_being_built + 
		_status.hatchery_count + 
		_status.hatchery_in_queue > 0;
	isHiveExist = _status.hive_being_built + 
		_status.hive_count + 
		_status.hive_in_queue > 0;
	isQueenNestExist = _status.queens_nest_being_built + 
		_status.queens_nest_count + 
		_status.queens_nest_in_queue > 0;
	isLairExist = _status.lair_being_built + 
		_status.lair_count + 
		_status.lair_in_queue > 0;
	isSpawningPoolExist = _status.spawning_pool_being_built + 
		_status.spawning_pool_count + 
		_status.spawning_pool_in_queue > 0;
	isExtractorExist = _status.extractor_being_built + 
		_status.extractor_count + 
		_status.extractor_in_queue > 0;
	isHydraliskDenExist = _status.hydralisk_den_being_built + 
		_status.hydralisk_den_count + 
		_status.hydralisk_den_in_queue > 0;

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
		if (_status.larva_count > 1) {
			if (sunken_colony_total > 2 &&
				_status.drone_count + _status.drone_in_queue < 12)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, DRONE!");
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), 
					_status.larva_count > 2);
			}
			else if (minerals > 300 && gas > 50 && isHydraliskDenExist && 
				_status.hydralisk_in_queue < 2)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, Hydra!");
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk), 
					_status.larva_count > 2 && gas > 100);
			}
			else if (minerals > 200 && 
				_status.zergling_count + _status.zergling_in_queue < 7)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, Zergling!");
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling), 
					_status.larva_count > 2);
			}
		}
	}
	case 2:
	{
		if (!isHydraliskDenExist) {
			if (isExtractorExist && gas > 50)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den), true);
			}
			if (_status.zergling_in_queue < 3 
				&& _status.zergling_count < 17)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
			}
		}
		else if (isHydraliskDenExist)
		{
			BWAPI::Broodwar->drawTextScreen(300, 300, "more hydralisks");
			if (_status.hydralisk_in_queue < 3)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
			}

			if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect) &&
				gas > 100 && _status.lurker_count * 5 < _status.hydralisk_count
				&& _status.enemy_air_army_supply < 10)
			{
				BWAPI::Broodwar->drawTextScreen(350, 300, "want lurkers");
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker));
			}
			else if (_status.lurker_aspect_count == 0 && 
				_status.hydralisk_count > 15 &&
				_status.enemy_air_army_supply < 8 && 
				_status.lurker_aspect_count > 0 &&
				_status.lair_completed)
			{
				BWAPI::Broodwar->drawTextScreen(250, 290, "Are we looking for lurkers?");
				queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect));
			}
		}
	}

	case 3:
	{
		if (_status.hydralisk_in_queue < 3)
		{
			BWAPI::Broodwar->drawTextScreen(200, 310, "why not more hydras");
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
		}

		if (_status.hydralisk_den_completed > 0 && 
			(_status.lair_completed > 0 || _status.hive_completed > 0) &&
			_status.lurker_aspect_count == 0 && 
			gas >= 125 && 
			_status.hydralisk_count + _status.hydralisk_in_queue > 15)
		{
			queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect));
		}

		if (_status.hydralisk_den_completed > 0 &&
			BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
			&& _status.lurker_count * 5 < _status.hydralisk_count
			&& _status.enemy_air_army_supply < 10)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker));
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
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
		while (_status.drone_count + _status.drone_in_queue < 9)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
	}
	else
	{
		//BWAPI::Broodwar->drawTextScreen(200, 260, "Here, for defend building %d, %d",
		//	creep_colony_in_queue + creep_colony_being_built, creep_colony_count);
		if (sunken_colony_total < 2 + _status.being_rushed)
		{
			if (_status.creep_colony_total + _status.sunken_colony_total< 3
				&& _status.creep_colony_in_queue < 2)
			{
				//BWAPI::Broodwar->drawTextScreen(200, 280, "Are we building Creep Colony?");
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), 
					_status.creep_colony_being_built < 1 && 
					_status.being_rushed);
				if (_status.drone_count + _status.drone_in_queue < 12)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
				}
			}
		}
	}
	if (0 < _status.creep_colony_completed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony),
			_status.sunken_colony_count + _status.sunken_colony_being_built < 3 && _status.being_rushed);
	}
}

void ActionZVPHydra::tech(CasiaBot::ProductionQueue & queue, int mode)
{
	switch (mode)
	{
	case 1:
		//being rushed
	{
		if (gas > 200 && minerals > 400 || (_status.larva_count < 2 && gas > 180 && minerals > 220))
		{
			BWAPI::Broodwar->drawTextScreen(200, 300, "update tech");
			if (_status.muscular_arguments_count < 1)
			{
				BWAPI::Broodwar->drawTextScreen(300, 300, "no speed");
				if (_status.hydralisk_den_completed > 0 && 
					_status.hydralisk_count + _status.hydralisk_in_queue > 7)
				{
					BWAPI::Broodwar->drawTextScreen(400, 300, "update speed");
					queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
				}
			}
			else if (_status.grooved_spines_count < 1) {
				BWAPI::Broodwar->drawTextScreen(300, 300, "no range");
				if (_status.hydralisk_den_completed > 0 && 
					_status.hydralisk_count + _status.hydralisk_in_queue > 12)
				{
					BWAPI::Broodwar->drawTextScreen(400, 300, "update range");
					queue.add(MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
				}
			}
		}
	}

	case 2: 
	{
		if (gas > 150 && minerals > 200 )
		{
			BWAPI::Broodwar->drawTextScreen(200, 300, "update tech");
			if (_status.muscular_arguments_count < 1)
			{
				BWAPI::Broodwar->drawTextScreen(300, 300, "no speed");
				if (_status.hydralisk_den_completed > 0 && 
					_status.hydralisk_count + _status.hydralisk_in_queue > 7
					&& gas > 150 && minerals > 150)
				{
					BWAPI::Broodwar->drawTextScreen(400, 300, "update speed");
					queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
				}
			}
			else if (_status.grooved_spines_count < 1) {
				BWAPI::Broodwar->drawTextScreen(300, 300, "no range");
				if (_status.hydralisk_den_completed > 0 && 
					_status.hydralisk_count + _status.hydralisk_in_queue > 12
					&& gas > 150 && minerals > 150)
				{
					BWAPI::Broodwar->drawTextScreen(400, 300, "update range");
					queue.add(MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
				}
			}
		}
		//base tech
		if (!isHiveExist &&
			(mineralDequePositive && gasDequePositive ||
			(minerals > 500 && gas > 300)))	// Èô·ä³²²»´æÔÚ
		{
			if (isQueenNestExist)	// Èô»Êºó³²´æÔÚ
			{
				BWAPI::Broodwar->drawTextScreen(200, 340, "for the queen");
				if (currentFrameCount > 10800 && _status.army_supply > 40)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
				}
			}
			else	// Èô»Êºó³²²»´æÔÚ
			{
				BWAPI::Broodwar->drawTextScreen(200, 340, "Long live the king");
				if (isLairExist)	// ÈôÊÞÑ¨´æÔÚ
				{
					if (currentFrameCount > 9000 && _status.army_supply > 20)
					{
						queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
					}
				}
				else if (currentFrameCount > 4800 && _status.hydralisk_count > 12)	// ÈôÊÞÑ¨²»´æÔÚ
				{
					BWAPI::Broodwar->drawTextScreen(200, 320, "lion lair");
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair));
				}
			}
		}
	}
		

	case 3:
		// normal
	{
		if (_status.muscular_arguments_count < 1)
		{
			if (_status.hydralisk_den_completed > 0 &&
				minerals > 150 && gas >= 150 && 
				_status.hydralisk_count + _status.hydralisk_in_queue > 7)
			{
				queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
			}
		}
		else if (_status.grooved_spines_count < 1) {
			if (_status.hydralisk_den_completed > 0 && gas >= 150 &&
				_status.hydralisk_count + _status.hydralisk_in_queue > 12)
			{
				queue.add(MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
			}
		}

		//base tech
		if (!isHiveExist &&
			(mineralDequePositive && gasDequePositive ||
			(minerals > 400 && gas > 200)))	// Èô·ä³²²»´æÔÚ
		{
			if (isQueenNestExist)	// Èô»Êºó³²´æÔÚ
			{
				BWAPI::Broodwar->drawTextScreen(200, 340, "for the queen");
				if (currentFrameCount > 10800 && _status.army_supply > 40)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
				}
			}
			else	// Èô»Êºó³²²»´æÔÚ
			{
				BWAPI::Broodwar->drawTextScreen(200, 340, "Long live the king");
				if (isLairExist)	// ÈôÊÞÑ¨´æÔÚ
				{
					if (currentFrameCount > 9000 && _status.army_supply > 20)
					{
						queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
					}
				}
				else if (currentFrameCount > 4800 && _status.hydralisk_count > 12)	// ÈôÊÞÑ¨²»´æÔÚ
				{
					BWAPI::Broodwar->drawTextScreen(200, 320, "lion lair");
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair));
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
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	if (!isExtractorExist && _status.drone_count >= 11 && _status.spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
	}

	if (isSpawningPoolExist && !isHydraliskDenExist && gas > 50)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den), true);
	}

	//being rushed
	if (_status.being_rushed && !able_defend)
	{
		BWAPI::Broodwar->drawTextScreen(200, 240, "We are rushed, with able defend %d", able_defend);
		able_defend = _status.sunken_colony_count >= 3 || _status.army_supply > 15;

		staticDefend(queue);
		army(queue, 1);
		tech(queue, 1);
	}
	//not rushed
	else {
		BWAPI::Broodwar->drawTextScreen(200, 260, "larvaQ, %d, %d, %d ,%d, %d",
			_status.larvaQueue.at(0), _status.larvaQueue.at(1), _status.larvaQueue.at(2), _status.larvaQueue.at(3),
			larva_lacking ? 1 : 0);
		BWAPI::Broodwar->drawTextScreen(200, 250, "We are here, with able_defend %d", able_defend);
		able_defend = _status.sunken_colony_count >= 3 || _status.army_supply > 15;

		if (!isExtractorExist && _status.spawning_pool_count > 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
		}

		//we need some zerglings for the very beginning game
		if (currentFrameCount < 5040 && _status.zergling_count < 7 &&
			_status.zergling_in_queue < 1 && _status.spawning_pool_completed)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling), _status.zergling_count < 5);
		}
		
		if (_status.drone_count > 11)
		{
			staticDefend(queue);
		}
		//more bases
		if (larva_lacking && minerals > 400 && gas > 100 &&
			_status.base_count - _status.real_base_completed < 2 &&
			_status.hatchery_in_queue < 1)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main"));
		}

		//need more army force
		else if (_status.enemy_army_supply > _status.army_supply * 1.3 && _status.larva_count > 0 ) {

			BWAPI::Broodwar->drawTextScreen(200, 290, "too few army, enemy %d, self %d",
				_status.enemy_army_supply, _status.army_supply);
			
			army(queue, 2);
			tech(queue, 2);
		}
		
		//train more workers
		else
		{
			if (_status.drone_count + _status.drone_in_queue < patches * 1.8 + _status.extractor_count * 3
				&& _status.drone_in_queue < 2 && _status.larva_count > 0)
			{
				BWAPI::Broodwar->drawTextScreen(200, 330, "more drones! %d",
					(_status.larva_count) / _status.real_base_completed > 1);
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
			}

			//search for more treasures
			
			else if (_status.real_base_completed < _status.enemy_nexus_count ||
				_status.drone_count + _status.drone_in_queue >= _status.real_base_completed * 1.7 + _status.extractor_count * 3 &&
				_status.real_base_completed + _status.hatchery_in_queue < int(double(currentFrameCount) / 7200 + 0.3) &&
				_status.hatchery_in_queue < 2 && minerals > 350
				)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
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

		//²¹Æø¿ó
		int extractorUpperBound = std::min(_status.base_completed, 3);
		if (isExtractorExist &&
			_status.extractor_count + _status.extractor_being_built + _status.extractor_in_queue < extractorUpperBound &&
			_status.extractor_in_queue < 1 &&
			((gas < 100 && minerals > 450) ||
			currentFrameCount / _status.extractor_count < 9600))
		{
			BWAPI::Broodwar->drawTextScreen(200, 330, "GAS!GAS!GAS!");
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		}
	}
}
/*
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
*/