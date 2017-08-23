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
		BWAPI::Broodwar->drawTextScreen(200, 290, "army mode 111111");
		//train some force
		if (_status.larva_count > 1) {
			if (sunken_colony_total > 2 &&
				_status.drone_count + _status.drone_in_queue < 12)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, DRONE!");
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), 
					_status.larva_count > 2);
			}
			else if (minerals > 300 /*&& gas > 50*/ && _status.hydralisk_completed > 0 && 
				_status.hydralisk_in_queue < 2)
			{
				BWAPI::Broodwar->drawTextScreen(200, 270, "Here, Hydra!");
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk), 
					_status.larva_count > 2 && gas > 100 );
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
		BWAPI::Broodwar->drawTextScreen(200, 290, "army mode 222222");
		if (!isHydraliskDenExist) {
			if (isExtractorExist &&
				!isHydraliskDenExist && gas > 40)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den), true);
			}
			if (_status.zergling_in_queue < 3 
				&& _status.zergling_count < 17)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
			}
		}
		else
		{
			BWAPI::Broodwar->drawTextScreen(300, 300, "more hydralisks");
			if (_status.hydralisk_in_queue < 3)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
			}

			if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect) 
				/*&& gas > 100*/ && _status.lurker_count * 5 < _status.hydralisk_count
				&& _status.enemy_air_army_supply < 10)
			{
				BWAPI::Broodwar->drawTextScreen(350, 300, "want lurkers");
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker));
			}
			else if (_status.lurker_aspect_total == 0 &&
				_status.hydralisk_count > 15 &&
				_status.enemy_air_army_supply < 8 && 
				_status.lurker_aspect_total > 0 &&
				_status.lair_completed)
			{
				BWAPI::Broodwar->drawTextScreen(250, 290, "Are we looking for lurkers?");
				queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect));
			}
		}
	}

	case 3:
	{
		BWAPI::Broodwar->drawTextScreen(200, 290, "army mode 333333");
		if (_status.hydralisk_in_queue < 3 && _status.hydralisk_den_completed > 0)
		{
			BWAPI::Broodwar->drawTextScreen(200, 310, "why not more hydras");
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
		}

		if (_status.hydralisk_den_completed > 0 && 
			(_status.lair_completed > 0 || _status.hive_completed > 0) &&
			_status.lurker_aspect_total == 0 && 
			/* gas >= 125 &&*/
			_status.hydralisk_count + _status.hydralisk_in_queue > 15)
		{
			queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect));
		}

		if (_status.hydralisk_den_completed > 0 &&
			BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
			&& _status.lurker_count * 7 < _status.hydralisk_count
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
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), false);
		while (_status.drone_count + _status.drone_in_queue < 9)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), false);
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
	if (0 < _status.creep_colony_completed && 
		_status.sunken_colony_in_queue < _status.creep_colony_completed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony),
			_status.sunken_colony_total < 1 || _status.being_rushed);
	}
}

void ActionZVPHydra::tech(CasiaBot::ProductionQueue & queue, int mode)
{
	switch (mode)
	{
	case 1:
		//being rushed
	{
		BWAPI::Broodwar->drawTextScreen(200, 300, "tech mode 111111");
		if (gas > 200 && minerals > 400 || (_status.larva_count < 2 && gas > 180 && minerals > 220))
		{
			//BWAPI::Broodwar->drawTextScreen(200, 300, "update tech");
			if (_status.muscular_arguments_total < 1)
			{
				//BWAPI::Broodwar->drawTextScreen(300, 300, "no speed");
				if (_status.hydralisk_den_completed > 0 && 
					_status.hydralisk_count + _status.hydralisk_in_queue > 7)
				{
					//BWAPI::Broodwar->drawTextScreen(400, 300, "update speed");
					queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
				}
			}
			else if (_status.grooved_spines_total < 1) {
				//BWAPI::Broodwar->drawTextScreen(300, 300, "no range");
				if (_status.hydralisk_den_completed > 0 && 
					_status.hydralisk_count + _status.hydralisk_in_queue > 12)
				{
					//BWAPI::Broodwar->drawTextScreen(400, 300, "update range");
					queue.add(MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
				}
			}
		}
	}

	case 2: 
	{
		BWAPI::Broodwar->drawTextScreen(200, 300, "tech mode 222222");
		if (/*gas > 150 &&*/ minerals > 200 )
		{
			if (_status.muscular_arguments_total < 1)
			{
				if (_status.hydralisk_den_completed > 0 && 
					_status.hydralisk_count + _status.hydralisk_in_queue > 7
					/*&& gas > 150*/ && minerals > 150)
				{
					queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
				}
			}
			else if (_status.grooved_spines_total < 1) {
				if (_status.hydralisk_den_completed > 0 && 
					_status.hydralisk_count + _status.hydralisk_in_queue > 12
					/*&& gas > 150*/ && minerals > 150)
				{
					queue.add(MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
				}
			}
		}
		//base tech
		if (!isHiveExist &&
			(mineralDequePositive && gasDequePositive ||
			(minerals > 500  /*&& gas > 300*/)))	// Èô·ä³²²»´æÔÚ
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
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair));
				}
			}
		}
	}
		

	case 3:
		// normal
	{
		BWAPI::Broodwar->drawTextScreen(200, 300, "tech mode 333333");
		if (_status.muscular_arguments_total < 1)
		{
			if (_status.hydralisk_den_completed > 0 &&
				minerals > 150 /*&& gas >= 150*/ && 
				_status.hydralisk_count + _status.hydralisk_in_queue > 7)
			{
				queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
			}
		}
		else if (_status.grooved_spines_total < 1) {
			if (_status.hydralisk_den_completed > 0 /*&& gas >= 150*/ &&
				_status.hydralisk_count + _status.hydralisk_in_queue > 12)
			{
				queue.add(MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
			}
		}

		//base tech
		if (!isHiveExist &&
			(mineralDequePositive && gasDequePositive ||
			(minerals > 400 /*&& gas > 200*/)))	// Èô·ä³²²»´æÔÚ
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

	if (!isExtractorExist && _status.drone_count >= 11 && _status.spawning_pool_count > 0 )
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
	}

	if (isSpawningPoolExist && !isHydraliskDenExist && gas > 40)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den), true);
	}


	BWAPI::Broodwar->drawTextScreen(200, 270, "base count: %d , real_c: %d, in queue %d, need %d", 
		_status.base_count, _status.real_base_completed, _status.hatchery_in_queue, 
		int(double(currentFrameCount) / 6600 + 1));
	BWAPI::Broodwar->drawTextScreen(200, 280, "total %d , %d", 
		_status.hatchery_total, _status.real_base_completed * 1.5 - 0.1);
	//being rushed
	if (_status.being_rushed && !able_defend)
	{
		BWAPI::Broodwar->drawTextScreen(400, 240, "We are rushed, with able defend %d", able_defend);
		able_defend = _status.sunken_colony_count >= 3 || _status.army_supply > 15;

		staticDefend(queue);
		army(queue, 1);
		tech(queue, 1);
	}
	//not rushed
	else {
		BWAPI::Broodwar->drawTextScreen(400, 250, "larvaQ, %d, %d, %d ,%d, %d",
			_status.larvaQueue.at(0), _status.larvaQueue.at(1), _status.larvaQueue.at(2), _status.larvaQueue.at(3),
			larva_lacking ? 1 : 0);
		BWAPI::Broodwar->drawTextScreen(400, 240, "We are here, with able_defend %d", able_defend);
		able_defend = _status.sunken_colony_count >= 3 || _status.army_supply > 15;

		if (!isExtractorExist && _status.spawning_pool_count > 0 && _status.drone_count > 15)
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
			_status.base_total < _status.real_base_completed * 1.5 - 0.1 &&
			_status.hatchery_in_queue < 1)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main"));
		}

		//need more army force
		else if (_status.enemy_army_supply > _status.army_supply * 1.2 + _status.sunken_colony_count && 
			_status.larva_count > 0 ) {

			BWAPI::Broodwar->drawTextScreen(400, 260, "too few army, enemy %d, self %d",
				_status.enemy_army_supply, _status.army_supply);
			
			army(queue, 2);
			tech(queue, 2);
		}

		else
		{
			//economy
			if (_status.drone_count + _status.drone_in_queue < patches * 1.8 + _status.extractor_count * 3
				&& _status.drone_in_queue < 2 && _status.larva_count > 0)
			{
				BWAPI::Broodwar->drawTextScreen(400, 270, "more drones! %d",
					(_status.larva_count) / _status.real_base_completed > 1);
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
			}

			//search for more treasures
			
			if (//_status.real_base_completed < _status.enemy_base_count ||
				_status.real_base_completed + _status.hatchery_being_built < int(double(currentFrameCount) / 6600 + 1) &&
				_status.drone_count + _status.drone_in_queue >= _status.real_base_completed * 1.5 + _status.extractor_count * 3 &&
				_status.hatchery_in_queue < 2
				)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery), true);
			}
			
			//train more army & upgrade tech
			//else
			{
				BWAPI::Broodwar->drawTextScreen(400, 280, "tech is first power %d");
				tech(queue, 3);
				army(queue, 3);
			}
		}

		//²¹Æø¿ó
		int extractorUpperBound = std::min(_status.base_completed, 4);
		if (isExtractorExist &&
			_status.extractor_total < extractorUpperBound &&
			((gas < 100 && minerals > 450) &&
			currentFrameCount / _status.extractor_count > 9600))
		{
			BWAPI::Broodwar->drawTextScreen(200, 330, "GAS!GAS!GAS!");
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		}
	}
}