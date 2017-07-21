#include "ProductionQueue.h"
#include "InformationManager.h"

using namespace CasiaBot;

ProductionQueue::ProductionQueue()
{
	clear();
}

void ProductionQueue::updateCount(const MetaType & unit, int offset)
{
	if (unit.isUnit()) _unitCount[unit.getUnitType().getID()] += offset;
	else if (unit.isTech()) _techCount[unit.getTechType().getID()] += offset;
	else if (unit.isUpgrade()) _upgradeCount[unit.getUpgradeType().getID()] += offset;
}

void ProductionQueue::checkSupply()
{
	int supply =
		InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Overlord, BWAPI::Broodwar->self()) * 8
		+ InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hatchery, BWAPI::Broodwar->self())
		+ InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lair, BWAPI::Broodwar->self())
		+ InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hive, BWAPI::Broodwar->self());

	int supplyUsed = 1;
	for (BWAPI::Unit unit : BWAPI::Broodwar->self()->getUnits()) {
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg)
		{
			if (unit->getBuildType() == BWAPI::UnitTypes::Zerg_Zergling
				|| unit->getBuildType() == BWAPI::UnitTypes::Zerg_Scourge)
			{
				supplyUsed += 2;
			}
			else
			{
				supplyUsed += unit->getBuildType().supplyRequired();
			}
		}
		else if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		{
			supplyUsed += unit->getBuildType().supplyRequired();
		}
		else
		{
			supplyUsed += unit->getType().supplyRequired();
		}
	}
	supplyUsed /= 2;

	int overlordInQueue = unitCount(BWAPI::UnitTypes::Zerg_Overlord);
	int overlordInConstructing =
		InformationManager::Instance().getNumConstructingUnits(BWAPI::UnitTypes::Zerg_Overlord, BWAPI::Broodwar->self());
	int constructing = 0;
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->isMorphing() &&
			unit->getBuildType() == BWAPI::UnitTypes::Zerg_Overlord)
		{
			++constructing;
		}
	}
	if (constructing != overlordInConstructing)
	{
		++_straightCheckOverlord;
		if (_straightCheckOverlord >= 10)
		{
			_straightCheckOverlord = 0;
			overlordInConstructing = constructing;
		}
	}
	else
	{
		_straightCheckOverlord = 0;
	}
	int overlordReady = overlordInQueue + overlordInConstructing;
	std::string info = std::to_string(supplyUsed) + " / " + std::to_string(supply);
	std::string overlord = std::to_string(overlordInQueue) + " + " + std::to_string(overlordInConstructing);
	//CAB_ASSERT(false, info.c_str());
	if (supply - supplyUsed <= 7)
	{
		if (supply <= 9)
		{
			if (supply - supplyUsed <= 0 && overlordReady == 0)
			{
				add(MetaType(BWAPI::UnitTypes::Zerg_Overlord));
			}
		}
		else if (supply <= 17)
		{
			if (supply - supplyUsed <= 2 && overlordReady == 0)
			{
				add(MetaType(BWAPI::UnitTypes::Zerg_Overlord));
			}
		}
		else if (supply <= 33)
		{
			if (supply - supplyUsed <= 4 && overlordReady == 0)
			{
				add(MetaType(BWAPI::UnitTypes::Zerg_Overlord));
			}
		}
		else
		{
			if (overlordReady <= 1)
			{
				add(MetaType(BWAPI::UnitTypes::Zerg_Overlord));
			}
		}
	}
}

void ProductionQueue::addOpenning(const ProductionItem & item)
{
	const MetaType & unit = item._unit;

	updateCount(unit, 1);

	
	CAB_ASSERT(_openningQueue.size() < 1000, "openning queue overflow");

	_openningQueue.push_back(item);
}

void ProductionQueue::add(const ProductionItem & item, bool priority)
{
	const MetaType & unit = item._unit;

	updateCount(unit, 1);

	
	CAB_ASSERT(_priorityQueue.size() < 1000, "priority queue overflow");
	CAB_ASSERT(_overlordQueue.size() < 1000, "overlord queue overflow");
	CAB_ASSERT(_buildingQueue.size() < 1000, "building queue overflow");
	CAB_ASSERT(_workerQueue.size() < 1000, "worker queue overflow");
	CAB_ASSERT(_armyQueue.size() < 1000, "army queue overflow");
	CAB_ASSERT(_techUpgradeQueue.size() < 1000, "tech queue overflow");

	if (unit.getUnitType() == BWAPI::UnitTypes::Zerg_Overlord)
	{
		_overlordQueue.push_back(item);
	}
	else if (priority)
	{
		_priorityQueue.push_back(item);
	}
	else if (unit.isBuilding())
	{
		_buildingQueue.push_back(item);
	}
	else if (unit.getUnitType().isWorker())
	{
		_workerQueue.push_back(item);
	}
	else if (unit.isUnit())
	{
		_armyQueue.push_back(item);
	}
	else
	{
		_techUpgradeQueue.push_back(item);
	}
}

void ProductionQueue::retreat()
{
	ProductionItem item = _reserveQueue.back().first;
	ProductionPriority ptype = _reserveQueue.back().second.second;
	const MetaType & unit = item._unit;
	_reserveQueue.pop_back();

	if (ptype == Openning)
	{
		_openningQueue.push_front(item);
	}
	else if (unit.getUnitType() == BWAPI::UnitTypes::Zerg_Overlord)
	{
		_overlordQueue.push_front(item);
	}
	else if (ptype == Priority)
	{
		_priorityQueue.push_front(item);
	}
	else if (unit.isBuilding())
	{
		_buildingQueue.push_front(item);
	}
	else if (unit.getUnitType().isWorker())
	{
		_workerQueue.push_front(item);
	}
	else if (unit.isUnit())
	{
		_armyQueue.push_front(item);
	}
	else
	{
		_techUpgradeQueue.push_front(item);
	}
}

void ProductionQueue::popReserve()
{
	int frame = BWAPI::Broodwar->getFrameCount();
	while (!_reserveQueue.empty())
	{
		if (frame - _reserveQueue.front().second.first < _reserveFrame)
		{
			break;
		}
		updateCount(_reserveQueue.front().first._unit, -1);
		_reserveQueue.pop_front();
	}
}

ProductionItem ProductionQueue::popItem()
{
	MetaType meta;
	ProductionItem retItem(meta);
	ProductionPriority ptype = Normal;
	int larva_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Larva, BWAPI::Broodwar->self());
	if (!_openningQueue.empty())
	{
		retItem = _openningQueue.front();
		_openningQueue.pop_front();
		ptype = Openning;
	}
	else if (!_overlordQueue.empty())
	{
		retItem = _overlordQueue.front();
		_overlordQueue.pop_front();
	}
	else if (!_priorityQueue.empty())
	{
		retItem = _priorityQueue.front();
		_priorityQueue.pop_front();
		ptype = Priority;
	}
	else
	{
		_buildID += 1;
		if (_buildID == ProductionTypeID::TYPE_MAX) _buildID = 0;
		switch (_buildID)
		{
		case ProductionTypeID::BUILDING:
			if (!_buildingQueue.empty())
			{
				retItem = _buildingQueue.front();
				_buildingQueue.pop_front();
			}
			break;
		case ProductionTypeID::ARMY:
			if (!_armyQueue.empty())
			{
				_straightWorkerCount = 0;
				if (larva_count == 0 && _unitCount[BWAPI::UnitTypes::Zerg_Lurker.getID()] > 0)
				{
					std::deque<ProductionItem> items;
					while (!_armyQueue.empty())
					{
						retItem = _armyQueue.front();
						if (retItem._unit.getUnitType() == BWAPI::UnitTypes::Zerg_Lurker)
						{
							break;
						}
						items.push_back(retItem);
						_armyQueue.pop_front();
					}
					if (_armyQueue.empty())
					{
						retItem = items.front();
						items.pop_front();
					}
					else
					{
						retItem = _armyQueue.front();
						_armyQueue.pop_front();
						std::string info = "bad type is " + retItem._unit.getUnitType().getName();
						CAB_ASSERT(retItem._unit.getUnitType() == BWAPI::UnitTypes::Zerg_Lurker,
							info.c_str());
					}
					while (!items.empty())
					{
						_armyQueue.push_front(items.back());
						items.pop_back();
					}
				}
				else
				{
					retItem = _armyQueue.front();
					_armyQueue.pop_front();
				}
			}
			break;
		case ProductionTypeID::WORKER:
			if (!_workerQueue.empty())
			{
				retItem = _workerQueue.front();
				_workerQueue.pop_front();
			}
			break;
		case ProductionTypeID::TECH:
			if (!_techUpgradeQueue.empty())
			{
				retItem = _techUpgradeQueue.front();
				_techUpgradeQueue.pop_front();
			}
			break;
		}
	}
	if (retItem._unit.type() != MetaTypes::Default && popCheck(retItem))
	{
		_reserveQueue.push_back(std::pair<ProductionItem, std::pair<int, ProductionPriority>>
			(retItem, std::pair<int, ProductionPriority>(BWAPI::Broodwar->getFrameCount(), ptype)));
	}
	else
	{
		retItem._unit = meta;
	}
	return retItem;
}

bool ProductionQueue::popCheck(const ProductionItem & item)
{
	if (item._unit.isUnit())
	{
		// lurker��Ҫ����
		if (item._unit.getUnitType() == BWAPI::UnitTypes::Zerg_Lurker)
		{
			if (InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::Broodwar->self()) == 0)
			{
				return false;
			}
		}
		// ��������ҪCreep
		else if (item._unit.getUnitType() == BWAPI::UnitTypes::Zerg_Sunken_Colony
			|| item._unit.getUnitType() == BWAPI::UnitTypes::Zerg_Spore_Colony)
		{
			if (InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Creep_Colony, BWAPI::Broodwar->self()) == 0)
			{
				return false;
			}
		}
	}
	return true;
}

void ProductionQueue::clear()
{
	_buildID = -1;
	_buildingQueue.clear();
	_armyQueue.clear();
	_workerQueue.clear();
	_overlordQueue.clear();
	_techUpgradeQueue.clear();
	_priorityQueue.clear();
	_openningQueue.clear();
	_straightArmyCount = 0;
	_straightWorkerCount = 0;
	_straightCheckOverlord = 0;

	// unit count vector
	int maxTypeID(0);
	for (const BWAPI::UnitType & t : BWAPI::UnitTypes::allUnitTypes())
	{
		maxTypeID = maxTypeID > t.getID() ? maxTypeID : t.getID();
	}
	_unitCount = std::vector<int>(maxTypeID + 1, 0);
	_straightCount = std::vector<int>(maxTypeID + 1, 0);

	// tech count vector
	maxTypeID = 0;
	for (const BWAPI::TechType & t : BWAPI::TechTypes::allTechTypes())
	{
		maxTypeID = maxTypeID > t.getID() ? maxTypeID : t.getID();
	}
	_techCount = std::vector<int>(maxTypeID + 1, 0);

	// upgrade count vector
	maxTypeID = 0;
	for (const BWAPI::UpgradeType & t : BWAPI::UpgradeTypes::allUpgradeTypes())
	{
		maxTypeID = maxTypeID > t.getID() ? maxTypeID : t.getID();
	}
	_upgradeCount = std::vector<int>(maxTypeID + 1, 0);
	
	//����reserveQueue�����
	for (auto &i : _reserveQueue) {
		updateCount(i.first._unit, 1);
	}
}

int ProductionQueue::unitCount(BWAPI::UnitType type)
{
	return _unitCount[type.getID()];
}

int ProductionQueue::techCount(BWAPI::TechType type)
{
	return _techCount[type.getID()];
}

int ProductionQueue::upgradeCount(BWAPI::UpgradeType type)
{
	return _upgradeCount[type.getID()];
}

bool ProductionQueue::empty()
{
	return _buildingQueue.empty()
		&& _armyQueue.empty()
		&& _overlordQueue.empty()
		&& _priorityQueue.empty()
		&& _openningQueue.empty()
		&& _workerQueue.empty()
		&& _techUpgradeQueue.empty();
}