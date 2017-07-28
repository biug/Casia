#pragma once

#include <deque>
#include "ProductionItem.h"

namespace CasiaBot
{
	class ProductionQueue
	{
		enum ProductionPriority { Priority, Normal };
		enum ProductionTypeID { BUILDING, ARMY, WORKER, TECH, TYPE_MAX };
		int						_buildID;
		// unit
		std::deque<ProductionItem>	_buildingQueue;
		std::deque<ProductionItem>	_armyQueue;
		std::deque<ProductionItem>	_workerQueue;
		std::deque<ProductionItem>	_overlordQueue;
		// tech & upgrade
		std::deque<ProductionItem>	_techUpgradeQueue;
		// priority queue
		std::deque<ProductionItem>	_priorityQueue;
		// reserve for some frame
		const int									_reserveFrame = 10;
		std::deque<std::pair<ProductionItem, std::pair<int, ProductionPriority>>>	_reserveQueue;

		std::vector<int>	_unitCount;
		std::vector<int>	_techCount;
		std::vector<int>	_upgradeCount;
		std::vector<int>	_straightCount;
		int					_straightArmyCount;
		int					_straightWorkerCount;
		int					_straightCheckOverlord;

		void updateCount(const MetaType & unit, int offset);

	public:
		ProductionQueue();

		void checkSupply();
		void add(const ProductionItem & item, bool priority = false);
		void retreat();
		void popReserve();
		bool popCheck(const ProductionItem & item);

		int unitCount(BWAPI::UnitType type);
		int techCount(BWAPI::TechType type);
		int upgradeCount(BWAPI::UpgradeType type);
		void clear();

		ProductionItem popItem();

		bool empty();
		void printQueues(int x, int y);
	};
}