#include "ProductionQueue.h"
#include "InformationManager.h"
using namespace CasiaBot;

ProductionQueue::ProductionQueue()
{
	clear();
}

void ProductionQueue::updateCount(const MetaType & unit, int offset)
{
	if (unit.getCond().isCancel()) return;
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

	if (unit.getUnitType() == BWAPI::UnitTypes::Zerg_Overlord)
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
	if (!_overlordQueue.empty())
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
		// lurker需要刺蛇
		if (item._unit.getUnitType() == BWAPI::UnitTypes::Zerg_Lurker)
		{
			if (InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::Broodwar->self()) == 0)
			{
				return false;
			}
		}
		// 防御塔需要Creep
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
	
	//处理reserveQueue的情况
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
		&& _workerQueue.empty()
		&& _techUpgradeQueue.empty()
		&& _reserveQueue.empty();
}

void ProductionQueue::printQueues(int x, int y){
	std::map<std::string, int> armyMap;
	std::map<std::string, int> overlordMap;
	std::map<std::string, int> priorityMap;
	std::map<std::string, int> openningMap;
	std::map<std::string, int> workerMap;
	std::map<std::string, int> techUpgradeMap;
	std::map<std::string, int> buildingMap;
	
	for (unsigned int i = 0; i < _armyQueue.size(); i++){
		if (armyMap.find(_armyQueue.at(i)._unit.getName()) == armyMap.end())
			armyMap[_armyQueue.at(i)._unit.getName()] = 1;
		else
			armyMap[_armyQueue.at(i)._unit.getName()] += 1;
	}
	
	for (unsigned int i = 0; i < _overlordQueue.size(); i++){
		if (overlordMap.find(_overlordQueue.at(i)._unit.getName()) == overlordMap.end())
			overlordMap[_overlordQueue.at(i)._unit.getName()] = 1;
		else
			overlordMap[_overlordQueue.at(i)._unit.getName()] += 1;
	}
	
	for (unsigned int i = 0; i < _priorityQueue.size(); i++){
		if (priorityMap.find(_priorityQueue.at(i)._unit.getName()) == priorityMap.end())
			priorityMap[_priorityQueue.at(i)._unit.getName()] = 1;
		else
			priorityMap[_priorityQueue.at(i)._unit.getName()] += 1;
	}
	
	for (unsigned int i = 0; i < _workerQueue.size(); i++){
		if (workerMap.find(_workerQueue.at(i)._unit.getName()) == workerMap.end())
			workerMap[_workerQueue.at(i)._unit.getName()] = 1;
		else
			workerMap[_workerQueue.at(i)._unit.getName()] += 1;
	}
	
	for (unsigned int i = 0; i <_techUpgradeQueue.size(); i++){
		if (techUpgradeMap.find(_techUpgradeQueue.at(i)._unit.getName()) == techUpgradeMap.end())
			techUpgradeMap[_techUpgradeQueue.at(i)._unit.getName()] = 1;
		else
			techUpgradeMap[_techUpgradeQueue.at(i)._unit.getName()] += 1;
	}

	for (unsigned int i = 0; i <_buildingQueue.size(); i++){
		if (buildingMap.find(_buildingQueue.at(i)._unit.getName()) == buildingMap.end())
			buildingMap[_buildingQueue.at(i)._unit.getName()] = 1;
		else
			buildingMap[_buildingQueue.at(i)._unit.getName()] += 1;
	}
	
	std::string info = "\x04 ";
	BWAPI::Broodwar->drawTextScreen(x - 60, y,		  "\x04 army:  %d", _armyQueue.size());
	for (std::map <std::string, int>::iterator Iter = armyMap.begin(); Iter != armyMap.end(); Iter++) {
		info = info + " " + Iter->first + ": " + std::to_string(Iter->second) + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());

	y += 10;
	info = "\x04 ";
	for (unsigned int j = 0; j < _armyQueue.size() && j < 4; j++) {
		info = info +  " " + _armyQueue.at(j)._unit.getName() + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x , y, "%s", info.c_str());
	

	y += 10;
	info = "\x11";
	BWAPI::Broodwar->drawTextScreen(x - 60, y, "\x11 ovld:  %d", _overlordQueue.size());
	for (std::map <std::string, int>::iterator Iter = overlordMap.begin(); Iter != overlordMap.end(); Iter++) {
		info = info + " " + Iter->first + ": " + std::to_string(Iter->second) + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());

	y += 10;
	info = "\x11 ";
	for (unsigned int j = 0; j < _overlordQueue.size() && j < 4; j++) {
		info = info + " " + _overlordQueue.at(j)._unit.getName() + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());


	y += 10;
	info = "\x03";
	BWAPI::Broodwar->drawTextScreen(x - 60, y, "\x03 prior: %d", _priorityQueue.size());
	for (std::map <std::string, int>::iterator Iter = priorityMap.begin(); Iter != priorityMap.end(); Iter++) {
		info = info + " " + Iter->first + ": " + std::to_string(Iter->second) + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());

	y += 10;
	info = "\x03 ";
	for (unsigned int j = 0; j < _priorityQueue.size() && j < 4; j++) {
		info = info + " " + _priorityQueue.at(j)._unit.getName() + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());

	y += 10;
	info = "\x04";
	BWAPI::Broodwar->drawTextScreen(x - 60, y, "\x03 drone: %d", _workerQueue.size());
	for (std::map <std::string, int>::iterator Iter = workerMap.begin(); Iter != workerMap.end(); Iter++){
		info = info + " " + Iter->first + ": " + std::to_string(Iter->second) + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());

	y += 10;
	info = "\x04 ";
	for (unsigned int j = 0; j < _workerQueue.size() && j < 4; j++) {
		info = info + " " + _workerQueue.at(j)._unit.getName() + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());


	y += 10;
	info = "\x11";
	BWAPI::Broodwar->drawTextScreen(x - 60, y, "\x11 tech:  %d", _techUpgradeQueue.size());
	for (std::map <std::string, int>::iterator Iter = techUpgradeMap.begin(); Iter != techUpgradeMap.end(); Iter++){
		info = info + " " + Iter->first + ": " + std::to_string(Iter->second) + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());

	y += 10;
	info = "\x11 ";
	for (unsigned int j = 0; j < _techUpgradeQueue.size() && j < 4; j++) {
		info = info + " " + _techUpgradeQueue.at(j)._unit.getName() + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());

	y += 10;
	info = "\x04";
	BWAPI::Broodwar->drawTextScreen(x - 60, y, "\x04 build: %d", _buildingQueue.size());
	for (std::map <std::string, int>::iterator Iter = buildingMap.begin(); Iter != buildingMap.end(); Iter++) {
		info = info + " " + Iter->first + ": " + std::to_string(Iter->second) + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());

	y += 10;
	info = "\x04 ";
	for (unsigned int j = 0; j < _buildingQueue.size() && j < 4; j++) {
		info = info + " " + _buildingQueue.at(j)._unit.getName() + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x, y, "%s", info.c_str());
	
}