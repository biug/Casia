#include "ProductionManager.h"

using namespace CasiaBot;

ProductionManager::ProductionManager() 
	: _assignedWorkerForThisBuilding (false)
	, _haveLocationForThisBuilding   (false)
	, _enemyCloakedDetected          (false)
{
    setOpenningBuildOrder(StrategyManager::Instance().getOpeningBookBuildOrder());
}

void ProductionManager::setOpenningBuildOrder(const BuildOrder & buildOrder)
{
	_queue.clear();

	for (size_t i(0); i<buildOrder.size(); ++i)
	{
		_queue.addOpenning(ProductionItem(buildOrder[i]));
	}
}

void ProductionManager::setBuildOrder(const BuildOrder & buildOrder)
{
	_queue.clear();

	for (size_t i(0); i<buildOrder.size(); ++i)
	{
		_queue.add(ProductionItem(buildOrder[i]), true);
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
	// check the _queue for stuff we can build
	manageBuildOrderQueue();
    
	// if nothing is currently building, get a new goal from the strategy manager
	if ((BWAPI::Broodwar->getFrameCount() > 10))
	{
        if (Config::Debug::DrawBuildOrderSearchInfo)
        {
		    BWAPI::Broodwar->drawTextScreen(150, 10, "Nothing left to build, new search!");
        }

		performBuildOrderSearch();
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
		
	if (Config::Modules::UsingBuildOrderSearch)
	{
		// if it's a worker or a building, we need to re-search for the current goal
		if ((unit->getType().isWorker() && !WorkerManager::Instance().isWorkerScout(unit)) || unit->getType().isBuilding())
		{
			if (unit->getType() != BWAPI::UnitTypes::Zerg_Drone)
			{
				performBuildOrderSearch();
			}
		}
	}
}

void ProductionManager::manageBuildOrderQueue() 
{
	_queue.checkSupply();
	_queue.popReserve();
	ProductionItem item = _queue.popItem();
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
		for (auto & unit : BWAPI::Broodwar->self()->getUnits())
		{
			if (item._unit.isUnit() && unit->getType() == item._unit.getUnitType() && unit->isConstructing())
			{
				if (unit->getType().isRefinery())
				{
					WorkerManager::Instance().addCanceledRefineryLocation(unit->getTilePosition());
				}
				unit->cancelConstruction();
				return;
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

	// if the next item in the list is a building and we can't yet make it
    if (unit.isBuilding() && !(producer && canMake) && unit.whatBuilds().isWorker() && !item._assigned)
	{
		// construct a temporary building object
		Building b(unit.getUnitType(), item._desiredPosition);
        b.isGasSteal = false;
		b.nexpHatchery = item._unit.getCond().isMain();

		// set the producer as the closest worker, but do not set its job yet
		producer = WorkerManager::Instance().getBuilder(b, false);

		// predict the worker movement to that building location
		predictWorkerMovement(b);
		item._assigned = true;
	}

	// if we can make the current item
	if (producer && canMake) 
	{
		// create it
		create(producer, item);
		_assignedWorkerForThisBuilding = false;
		_haveLocationForThisBuilding = false;
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
		_queue.retreat();
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
	if (unit.isUnit() && unit.getUnitType().isBuilding()
		&& unit.getUnitType() != BWAPI::UnitTypes::Zerg_Lair
		&& unit.getUnitType() != BWAPI::UnitTypes::Zerg_Hive
		&& unit.getUnitType() != BWAPI::UnitTypes::Zerg_Greater_Spire
		&& unit.getUnitType() != BWAPI::UnitTypes::Zerg_Sunken_Colony
		&& unit.getUnitType() != BWAPI::UnitTypes::Zerg_Spore_Colony)
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

// When the next item in the _queue is a building, this checks to see if we should move to it
// This function is here as it needs to access prodction manager's reserved resources info
void ProductionManager::predictWorkerMovement(const Building & b)
{
    if (b.isGasSteal)
    {
        return;
    }

	// get a possible building location for the building
	if (!_haveLocationForThisBuilding)
	{
		_predictedTilePosition = BuildingManager::Instance().getBuildingLocation(b);
	}

	if (_predictedTilePosition != BWAPI::TilePositions::None)
	{
		_haveLocationForThisBuilding = true;
	}
	else
	{
		return;
	}
	
	// draw a box where the building will be placed
	int x1 = _predictedTilePosition.x * 32;
	int x2 = x1 + (b.type.tileWidth()) * 32;
	int y1 = _predictedTilePosition.y * 32;
	int y2 = y1 + (b.type.tileHeight()) * 32;
	if (Config::Debug::DrawWorkerInfo) 
    {
        BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Blue, false);
    }

	// where we want the worker to walk to
	BWAPI::Position walkToPosition		= BWAPI::Position(x1 + (b.type.tileWidth()/2)*32, y1 + (b.type.tileHeight()/2)*32);

	// compute how many resources we need to construct this building
	int mineralsRequired				= std::max(0, b.type.mineralPrice() - getFreeMinerals());
	int gasRequired						= std::max(0, b.type.gasPrice() - getFreeGas());

	// get a candidate worker to move to this location
	BWAPI::Unit moveWorker			= WorkerManager::Instance().getMoveWorker(walkToPosition);

	// Conditions under which to move the worker: 
	//		- there's a valid worker to move
	//		- we haven't yet assigned a worker to move to this location
	//		- the build position is valid
	//		- we will have the required resources by the time the worker gets there
	if (moveWorker && _haveLocationForThisBuilding && !_assignedWorkerForThisBuilding && (_predictedTilePosition != BWAPI::TilePositions::None) &&
		WorkerManager::Instance().willHaveResources(mineralsRequired, gasRequired, moveWorker->getDistance(walkToPosition)) )
	{
		// we have assigned a worker
		_assignedWorkerForThisBuilding = true;

		// tell the worker manager to move this worker
		WorkerManager::Instance().setWorkerMoving(mineralsRequired, gasRequired, walkToPosition);
	}
}

void ProductionManager::performCommand(BWAPI::UnitCommandType t) 
{
	// if it is a cancel construction, it is probably the extractor trick
	if (t == BWAPI::UnitCommandTypes::Cancel_Construction)
	{
		BWAPI::Unit extractor = nullptr;
		for (auto & unit : BWAPI::Broodwar->self()->getUnits())
		{
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Extractor)
			{
				extractor = unit;
			}
		}

		if (extractor)
		{
			extractor->cancelConstruction();
		}
	}
}

int ProductionManager::getFreeMinerals()
{
	int minerals = BWAPI::Broodwar->self()->minerals();
	int reservedMinerals = BuildingManager::Instance().getReservedMinerals();
	return minerals - reservedMinerals;
}

int ProductionManager::getFreeGas()
{
	return BWAPI::Broodwar->self()->gas() - BuildingManager::Instance().getReservedGas();
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

	// for each unit in the _queue
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