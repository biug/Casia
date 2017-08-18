#include "ProductionManager.h"

using namespace CasiaBot;

ProductionManager::ProductionManager() 
	: _enemyCloakedDetected          (false)
{
    setOpenningBuildOrder(StrategyManager::Instance().getOpeningBookBuildOrder());
}

void ProductionManager::setOpenningBuildOrder(const BuildOrder & buildOrder)
{
	_queue.clear();
	_openingQueue.clear();

	for (size_t i(0); i<buildOrder.size(); ++i)
	{
		_openingQueue.emplace_back(ProductionItem(buildOrder[i]));
	}
}

void ProductionManager::performBuildOrderSearch()
{	
    if (!Config::Modules::UsingBuildOrderSearch || !canPlanBuildOrderNow())
    {
        return;
    }

	StrategyManager::Instance().updateProductionQueue(_queue);
}

void ProductionManager::update() 
{
	// update status
	ActionZergBase::updateStatus(_queue);
	for (const auto & item : _openingQueue)
	{
		if (item._unit.isUpgrade()
			&& item._unit.getUpgradeType() == BWAPI::UpgradeTypes::Metabolic_Boost)
		{
			ActionZergBase::_status.metabolic_boost_in_queue = 1;
			break;
		}
	}

	// check the _queue for stuff we can build
	manageBuildOrderQueue();
    
	// if nothing is currently building, get a new goal from the strategy manager
	if ((BWAPI::Broodwar->getFrameCount() > 10))
	{
        if (Config::Debug::DrawBuildOrderSearchInfo)
        {
		    BWAPI::Broodwar->drawTextScreen(150, 10, "Nothing left to build, new search!");
        }
		if (_openingQueue.empty())
		{
			performBuildOrderSearch();
		}
	}

	// if they have cloaked units get a new goal asap
	if (!_enemyCloakedDetected && InformationManager::Instance().enemyHasCloakedUnits())
	{        
        if (Config::Debug::DrawBuildOrderSearchInfo)
        {
		    BWAPI::Broodwar->printf("Enemy Cloaked Unit Detected!");
        }

		_enemyCloakedDetected = true;
	}
}

// on unit destroy
void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{
	// we don't care if it's not our unit
	if (!unit || unit->getPlayer() != BWAPI::Broodwar->self())
	{
		return;
	}
}

void ProductionManager::openingCheck()
{
	// check overlord
	if (_openingQueue.empty()) return;
	auto & item = _openingQueue.front();
	// if we create overlord, continue
	if (item._unit.isUnit() && item._unit.getUnitType() == BWAPI::UnitTypes::Zerg_Overlord)
	{
		return;
	}
	// if we building, continue
	if (item._unit.isBuilding())
	{
		return;
	}
	// calculate supply
	int supply =
		InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Overlord, BWAPI::Broodwar->self()) * 8
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

	if (supplyUsed > supply)
	{
		MetaType meta(BWAPI::UnitTypes::Zerg_Overlord);
		ProductionItem item(meta);
		_openingQueue.push_front(item);
		BWAPI::Broodwar->printf("used %d total %d", supplyUsed, supply);
		BWAPI::Broodwar->printf("add one");
	}
}

void ProductionManager::manageBuildOrderQueue() 
{
	bool useQueue = _openingQueue.empty();
	if (useQueue)
	{
		_queue.checkSupply();
		_queue.popReserve();
	}
	else
	{
		// check opening queue
		openingCheck();
	}
	ProductionItem item = useQueue ? _queue.popItem() : _openingQueue.front();
	int freem = getFreeMinerals();
	int freeg = getFreeGas();
	std::string info = "free mineral " + std::to_string(freem) + " free gas " + std::to_string(freeg);
	BWAPI::Broodwar->drawTextScreen(480, 300, info.c_str());
	if (!useQueue) _openingQueue.pop_front();
	if (item._unit.type() == MetaTypes::Default)
	{
		return;
	}
	MetaType & unit = item._unit;
	// this is the unit which can produce the currentItem
    BWAPI::Unit producer = getProducer(unit);

	// check to see if we can make it right now
	bool canMake = canMakeNow(producer, unit);

	// if we should cancel
	if (item._unit.getCond().isCancel())
	{
		if (unit.isUnit())
		{
			BWAPI::Broodwar->printf("cancel extractor");
			auto u = BuildingManager::Instance().cancelBuildingTask(unit.getUnitType());
			if (!u)
			{
				BWAPI::Broodwar->printf("cancel fail");
			}
		}
		return;
	}
	else if (item._unit.getCond().isMineral())
	{
		int mineral = item._unit.getCond().getMineral();
		std::string info = std::to_string(mineral) + "M";
		CAB_ASSERT(false, info.c_str());
		if (BWAPI::Broodwar->self()->minerals() < mineral)
		{
			canMake = false;
		}
	}
	else if (item._unit.getCond().isGas())
	{
		int gas = item._unit.getCond().getGas();
		std::string info = std::to_string(gas) + "G";
		CAB_ASSERT(false, info.c_str());
		if (BWAPI::Broodwar->self()->minerals() < gas)
		{
			canMake = false;
		}
	}
	else if (item._unit.getCond().isPercent())
	{
		auto percent = item._unit.getCond().getPercent();
		for (const auto & unit : BWAPI::Broodwar->self()->getUnits())
		{
			if (unit->getType() == percent.first)
			{
				float rate = (float)unit->getRemainingBuildTime() / (float)percent.first.buildTime();
				if (rate > percent.second)
				{
					canMake = false;
				}
			}
		}
	}
	else if (item._unit.getCond().isUnit())
	{
		auto unit = item._unit.getCond().getUnit();
		int num = InformationManager::Instance().getNumUnits(unit.first, BWAPI::Broodwar->self());
		if (num < unit.second)
		{
			canMake = false;
		}
	}

	if (unit.isBuilding())
	{
		Building b(unit.getUnitType(), item._desiredPosition);
		b.nexpHatchery = item._unit.getCond().isMain();
		b.isGasSteal = false;
		BWAPI::TilePosition testLocation = BuildingManager::Instance().getBuildingLocation(b);
		if (!testLocation.isValid())
		{
			canMake = false;
		}
		else
		{
			b.finalPosition = testLocation;

			// grab a worker unit from WorkerManager which is closest to this final position
			BWAPI::Unit workerToAssign = WorkerManager::Instance().getBuilder(b, false);
			if (!workerToAssign || !workerToAssign->getType().isWorker())
			{
				canMake = false;
			}
			else
			{
				const auto & area1 = InformationManager::Instance().getTileArea(workerToAssign->getTilePosition());
				const auto & area2 = InformationManager::Instance().getTileArea(b.finalPosition);
				auto p1 = workerToAssign->getPosition();
				auto p2 = BWAPI::Position(b.finalPosition);
				const auto & chokes = BWEM::Map::Instance().GetPath(area1, area2);
				double distance = MapTools::Instance().getPathDistance(p1, p2, chokes);

				canMake = WorkerManager::Instance().willHaveResources(
					b.type.mineralPrice() - getFreeMinerals(),
					b.type.gasPrice() - getFreeGas(),
					distance);
			}
		}
	}

	// if we can make the current item
	if ((producer && canMake)) 
	{
		// create it
		create(producer, item);
	}
	else 
	{
		// retreat
		if (unit.type() == MetaTypes::Tech && BWAPI::Broodwar->self()->hasResearched(unit.getTechType()))
		{
			return;
		}
		if (unit.type() == MetaTypes::Upgrade && BWAPI::Broodwar->self()->getUpgradeLevel(unit.getUpgradeType()) > 0)
		{
			return;
		}
		if (useQueue)
		{
			_queue.retreat();
		}
		else
		{
			_openingQueue.emplace_front(item);
		}
	}
}

BWAPI::Unit ProductionManager::getProducer(MetaType t, BWAPI::Position closestTo)
{
	if (t.isUnit() && t.getUnitType() == BWAPI::UnitTypes::Zerg_Drone)
	{
		BWAPI::Unit base = WorkerManager::Instance().getLarvaDepot();
		if (!base) { return nullptr; }
		BWAPI::Unitset larvas = base->getLarva();
		if (larvas.empty()) { return nullptr; }
		return *larvas.begin();
	}
	else if (t.isUnit()
		&& (t.getUnitType() == BWAPI::UnitTypes::Zerg_Lair
			|| t.getUnitType() == BWAPI::UnitTypes::Zerg_Hive))
	{
		const auto & bases = InformationManager::Instance().getSelfBases();
		if (bases.empty()) return nullptr;
		return bases.front();
	}
    // get the type of unit that builds this
    BWAPI::UnitType producerType = t.whatBuilds();

    // make a set of all candidate producers
    BWAPI::Unitset candidateProducers;
    for (auto & unit : BWAPI::Broodwar->self()->getUnits())
    {
        CAB_ASSERT(unit != nullptr, "Unit was null");

        // reasons a unit can not train the desired type
        if (unit->getType() != producerType)                    { continue; }
        if (!unit->isCompleted())                               { continue; }
        if (unit->isTraining())                                 { continue; }
        if (unit->isLifted())                                   { continue; }
        if (!unit->isPowered())                                 { continue; }

        // if we haven't cut it, add it to the set of candidates
        candidateProducers.insert(unit);
    }

    return getClosestUnitToPosition(candidateProducers, closestTo);
}

BWAPI::Unit ProductionManager::getClosestUnitToPosition(const BWAPI::Unitset & units, BWAPI::Position closestTo)
{
    if (units.size() == 0)
    {
        return nullptr;
    }

    // if we don't care where the unit is return the first one we have
    if (closestTo == BWAPI::Positions::None)
    {
        return *(units.begin());
    }

    BWAPI::Unit closestUnit = nullptr;
    double minDist(1000000);

	for (auto & unit : units) 
    {
        CAB_ASSERT(unit != nullptr, "Unit was null");

		double distance = unit->getDistance(closestTo);
		if (!closestUnit || distance < minDist) 
        {
			closestUnit = unit;
			minDist = distance;
		}
	}

    return closestUnit;
}

// this function will check to see if all preconditions are met and then create a unit
void ProductionManager::create(BWAPI::Unit producer, ProductionItem & item)
{
    if (!producer)
    {
        return;
    }

	MetaType & unit = item._unit;

    // if we're dealing with a building
	if (unit.isUnit() && unit.isBuilding())
    {
        // send the building task to the building manager
        BuildingManager::Instance().addBuildingTask(unit.getUnitType(), item._desiredPosition, false, item._unit.getCond().isMain());
    }
    // if we're dealing with a non-building unit
	else if (unit.isUnit())
    {
        // if the race is zerg, morph the unit
		producer->morph(unit.getUnitType());
    }
    // if we're dealing with a tech research
	else if (unit.isTech())
    {
		producer->research(unit.getTechType());
    }
	else if (unit.isUpgrade())
    {
        //Logger::Instance().log("Produce Upgrade: " + t.getName() + "\n");
		producer->upgrade(unit.getUpgradeType());
    }
    else
    {	
		
    }
}

bool ProductionManager::canMakeNow(BWAPI::Unit producer, MetaType t)
{
    //CAB_ASSERT(producer != nullptr, "Producer was null");

	bool canMake = producer && meetsReservedResources(t);
	if (canMake)
	{
		if (t.isUnit())
		{
			canMake = BWAPI::Broodwar->canMake(t.getUnitType(), producer);
		}
		else if (t.isTech())
		{
			canMake = BWAPI::Broodwar->canResearch(t.getTechType(), producer);
		}
		else if (t.isUpgrade())
		{
			canMake = BWAPI::Broodwar->canUpgrade(t.getUpgradeType(), producer);
		}
		else
		{	
			CAB_ASSERT(false, "Unknown type");
		}
	}

	return canMake;
}

int ProductionManager::getFreeMinerals()
{
	return BuildingManager::Instance().getFreeMinerals();
}

int ProductionManager::getFreeGas()
{
	return BuildingManager::Instance().getFreeGas();
}

// return whether or not we meet resources, including building reserves
bool ProductionManager::meetsReservedResources(MetaType type) 
{
	// return whether or not we meet the resources
	return (type.mineralPrice() <= getFreeMinerals()) && (type.gasPrice() <= getFreeGas());
}


// selects a unit of a given type
BWAPI::Unit ProductionManager::selectUnitOfType(BWAPI::UnitType type, BWAPI::Position closestTo) 
{
	// if we have none of the unit type, return nullptr right away
	if (BWAPI::Broodwar->self()->completedUnitCount(type) == 0) 
	{
		return nullptr;
	}

	BWAPI::Unit unit = nullptr;

	// if we are concerned about the position of the unit, that takes priority
    if (closestTo != BWAPI::Positions::None) 
    {
		double minDist(1000000);

		for (auto & u : BWAPI::Broodwar->self()->getUnits()) 
        {
			if (u->getType() == type) 
            {
				double distance = u->getDistance(closestTo);
				if (!unit || distance < minDist) {
					unit = u;
					minDist = distance;
				}
			}
		}

	// if it is a building and we are worried about selecting the unit with the least
	// amount of training time remaining
	} 
    else if (type.isBuilding()) 
    {
		for (auto & u : BWAPI::Broodwar->self()->getUnits()) 
        {
            CAB_ASSERT(u != nullptr, "Unit was null");

			if (u->getType() == type && u->isCompleted() && !u->isTraining() && !u->isLifted() &&u->isPowered()) {

				return u;
			}
		}
		// otherwise just return the first unit we come across
	} 
    else 
    {
		for (auto & u : BWAPI::Broodwar->self()->getUnits()) 
		{
            CAB_ASSERT(u != nullptr, "Unit was null");

			if (u->getType() == type && u->isCompleted() && u->getHitPoints() > 0 && !u->isLifted() &&u->isPowered()) 
			{
				return u;
			}
		}
	}

	// return what we've found so far
	return nullptr;
}

void ProductionManager::drawProductionInformation(int x, int y)
{
    if (!Config::Debug::DrawProductionInfo)
    {
        return;
    }

	// fill prod with each unit which is under construction
	std::vector<BWAPI::Unit> prod;
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
        CAB_ASSERT(unit != nullptr, "Unit was null");

		if (unit->isBeingConstructed())
		{
			prod.push_back(unit);
		}
	}
	
	// sort it based on the time it was started
	std::sort(prod.begin(), prod.end(), CompareWhenStarted());

    BWAPI::Broodwar->drawTextScreen(x-30, y+20, "\x04 TIME");
	BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04 UNIT NAME");

	size_t reps = prod.size() < 10 ? prod.size() : 10;

	y += 30;
	int yy = y;

	// for each unit in the queue
	for (auto & unit : prod) 
    {
		std::string prefix = "\x07";

		yy += 10;

		BWAPI::UnitType t = unit->getType();
        if (t == BWAPI::UnitTypes::Zerg_Egg)
        {
            t = unit->getBuildType();
        }

		BWAPI::Broodwar->drawTextScreen(x, yy, " %s%s", prefix.c_str(), t.getName().c_str());
		BWAPI::Broodwar->drawTextScreen(x - 35, yy, "%s%6d", prefix.c_str(), unit->getRemainingBuildTime());
	}
}

ProductionManager & ProductionManager::Instance()
{
	static ProductionManager instance;
	return instance;
}


void ProductionManager::queueGasSteal()
{
    _queue.add(MetaType(BWAPI::Broodwar->self()->getRace().getRefinery()), true);
}

void ProductionManager::queuePrint(int x, int y){
	BWAPI::Broodwar->drawTextScreen(x , y - 20, "\x04 opening:  %d", _openingQueue.size());
	std::string info = "\x04 ";
	for (unsigned int j = 0; j < _openingQueue.size() && j < 5; j++) {
		info = info + " " + _openingQueue.at(j)._unit.getName() + " ||";
	}
	BWAPI::Broodwar->drawTextScreen(x - 60 , y - 10, "%s", info.c_str());
	_queue.printQueues(x, y);
}

// this will return true if any unit is on the first frame if it's training time remaining
// this can cause issues for the build order search system so don't plan a search on these frames
bool ProductionManager::canPlanBuildOrderNow() const
{
    for (const auto & unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getRemainingTrainTime() == 0)
        {
            continue;       
        }

        BWAPI::UnitType trainType = unit->getLastCommand().getUnitType();

        if (unit->getRemainingTrainTime() == trainType.buildTime())
        {
            return false;
        }
    }

    return true;
}