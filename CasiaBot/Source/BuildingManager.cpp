#include "Common.h"
#include "BuildingManager.h"
#include "Micro.h"
#include "ScoutManager.h"

using namespace CasiaBot;

BuildingManager::BuildingManager()
    : _debugMode(false)
    , _reservedMinerals(0)
    , _reservedGas(0)
{
	int maxTypeID(0);
	for (const BWAPI::UnitType & t : BWAPI::UnitTypes::allUnitTypes())
	{
		maxTypeID = maxTypeID > t.getID() ? maxTypeID : t.getID();
	}

	_numBuildings = std::vector<int>(maxTypeID + 1, 0);
}

// gets called every frame from GameCommander
void BuildingManager::update()
{
    validateWorkersAndBuildings();          // check to see if assigned workers have died en route or while constructing
    assignWorkersToUnassignedBuildings();   // assign workers to the unassigned buildings and label them 'planned'    
    constructAssignedBuildings();           // for each planned building, if the worker isn't constructing, send the command    
    checkForStartedConstruction();          // check to see if any buildings have started construction and update data structures     
	checkForConstructingBuildings();           // check to see if any buildings have completed and update data structures
}

int	BuildingManager::numBeingBuilt(BWAPI::UnitType type)
{
	return _numBuildings[type.getID()];
}

int BuildingManager::getFreeMinerals()
{
	return BWAPI::Broodwar->self()->minerals() - _reservedMinerals;
}

int BuildingManager::getFreeGas()
{
	return BWAPI::Broodwar->self()->gas() - _reservedGas;
}

// STEP 1: DO BOOK KEEPING ON WORKERS WHICH MAY HAVE DIED
void BuildingManager::validateWorkersAndBuildings()
{
    std::vector<Building> toRemove;
    
    // find any buildings which have become obsolete
    for (auto & b : _buildings)
    {
        if (b.status != BuildingStatus::UnderConstruction)
        {
            continue;
        }

        if (b.buildingUnit == nullptr
			|| !b.buildingUnit->getType().isBuilding()
			|| b.buildingUnit->getHitPoints() <= 0
			|| !b.buildingUnit->exists())
        {
            toRemove.push_back(b);
        }
    }

    removeBuildings(toRemove);
}

// STEP 2: ASSIGN WORKERS TO BUILDINGS WITHOUT THEM
void BuildingManager::assignWorkersToUnassignedBuildings()
{
    // for each building that doesn't have a builder, assign one
    for (Building & b : _buildings)
    {
        if (b.status != BuildingStatus::Unassigned)
        {
            continue;
        }

		BWAPI::TilePosition testLocation = getBuildingLocation(b);
		if (!testLocation.isValid())
		{
			continue;
		}
		b.finalPosition = testLocation;

        if (_debugMode) { BWAPI::Broodwar->printf("Assigning Worker To: %s",b.type.getName().c_str()); }

        // grab a worker unit from WorkerManager which is closest to this final position
        BWAPI::Unit workerToAssign = WorkerManager::Instance().getBuilder(b, true);

		if (!workerToAssign || !workerToAssign->getType().isWorker()) continue;

		WorkerManager::Instance().setWorkerBuilding(workerToAssign, b);

        b.builderUnit = workerToAssign;

        // reserve this building's space
        BuildingPlacer::Instance().reserveTiles(b.finalPosition,b.type.tileWidth(),b.type.tileHeight());

        b.status = BuildingStatus::Assigned;
    }
}

// STEP 3: ISSUE CONSTRUCTION ORDERS TO ASSIGN BUILDINGS AS NEEDED
void BuildingManager::constructAssignedBuildings()
{
    for (auto & b : _buildings)
    {
        if (b.status != BuildingStatus::Assigned)
        {
            continue;
        }

        // if that worker is not currently constructing
		// for zerg, it not morph to a building
        if (b.builderUnit->exists() && b.builderUnit->getType().isWorker())
        {
			if (!b.builderUnit->isConstructing())
			{
				// if we haven't explored the build position, go there
				if (!isBuildingPositionExplored(b)
					|| b.builderUnit->getDistance(BWAPI::Position(b.finalPosition)) > 128)
				{
					Micro::SmartMove(b.builderUnit, BWAPI::Position(b.finalPosition));
				}
				// if this is not the first time we've sent this guy to build this
				// it must be the case that something was in the way of building
				else if (b.buildCommandGiven)
				{
					// tell worker manager the unit we had is not needed now, since we might not be able
					// to get a valid location soon enough
					WorkerManager::Instance().finishedWithWorker(b.builderUnit);

					// free the previous location in reserved
					BuildingPlacer::Instance().freeTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

					// nullify its current builder unit
					b.builderUnit = nullptr;

					// reset the build command given flag
					b.buildCommandGiven = false;

					// add the building back to be assigned
					b.status = BuildingStatus::Unassigned;
				}
				else
				{
					// issue the build order!
					if (b.builderUnit->build(b.type, b.finalPosition))
					{
						// set the flag to true
						b.buildCommandGiven = true;
					}
					else
					{
						BWAPI::Broodwar->drawLineMap(b.builderUnit->getPosition(), BWAPI::Position(b.finalPosition), BWAPI::Colors::Orange);
					}
				}
			}
        }
		else if ((b.buildCommandGiven && !b.builderUnit->exists()) && !b.type.isRefinery())
		{
			// free the previous location in reserved
			BuildingPlacer::Instance().freeTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

			// nullify its current builder unit
			b.builderUnit = nullptr;

			// reset the build command given flag
			b.buildCommandGiven = false;

			// add the building back to be assigned
			b.status = BuildingStatus::Unassigned;
		}
    }
}

// STEP 4: UPDATE DATA STRUCTURES FOR BUILDINGS STARTING CONSTRUCTION
void BuildingManager::checkForStartedConstruction()
{
    // for each building unit which is being constructed
    for (auto & buildingStarted : BWAPI::Broodwar->self()->getUnits())
    {
        // filter out units which aren't buildings under construction
        if (!buildingStarted->getType().isBuilding() || !buildingStarted->isBeingConstructed())
        {
            continue;
        }

        // check all our building status objects to see if we have a match and if we do, update it
        for (auto & b : _buildings)
        {
            if (b.status != BuildingStatus::Assigned)
            {
                continue;
            }
        
            // check if the positions match
            if (b.finalPosition == buildingStarted->getTilePosition())
            {
                // the resources should now be spent, so unreserve them
                _reservedMinerals -= buildingStarted->getType().mineralPrice();
                _reservedGas      -= buildingStarted->getType().gasPrice();

                // flag it as started and set the buildingUnit
                b.buildingUnit = buildingStarted;

                // if we are zerg, the buildingUnit now becomes nullptr since it's destroyed
				b.builderUnit = nullptr;

                // put it in the under construction vector
                b.status = BuildingStatus::UnderConstruction;

                // free this space
                BuildingPlacer::Instance().freeTiles(b.finalPosition,b.type.tileWidth(),b.type.tileHeight());

                // only one building will match
                break;
            }
        }
    }
}

// STEP 5: CHECK FOR COMPLETED BUILDINGS
void BuildingManager::checkForConstructingBuildings()
{
    std::vector<Building> toRemove;

    // for each of our buildings under construction
    for (auto & b : _buildings)
    {
        if (b.status != BuildingStatus::UnderConstruction)
        {
            continue;       
        }

        // if the unit is under construction and not refinery
		float ratio =
			(float)b.buildingUnit->getHitPoints()
			/ (float)b.buildingUnit->getType().maxHitPoints();
		BWAPI::Broodwar->drawTextMap(b.buildingUnit->getPosition(), ".2f", ratio);
		if (!b.buildingUnit->getType().isRefinery())
		{
			// 普通建筑在20%时移除
			if (b.buildingUnit->isBeingConstructed() && ratio > 0.2)
			{
				toRemove.push_back(b);
				if (b.beCanceled) b.buildingUnit->cancelConstruction();
			}
		}
		else
		{
			if (b.beCanceled)
			{
				b.buildingUnit->cancelConstruction();
			}
			// 气矿在80%时移除
			if (b.buildingUnit->isBeingConstructed() && ratio > 0.8)
			{
				toRemove.push_back(b);
			}
		}
    }

    removeBuildings(toRemove);
}

// COMPLETED
bool BuildingManager::isEvolvedBuilding(BWAPI::UnitType type) 
{
    if (type == BWAPI::UnitTypes::Zerg_Sunken_Colony ||
        type == BWAPI::UnitTypes::Zerg_Spore_Colony ||
        type == BWAPI::UnitTypes::Zerg_Lair ||
        type == BWAPI::UnitTypes::Zerg_Hive ||
        type == BWAPI::UnitTypes::Zerg_Greater_Spire)
    {
        return true;
    }

    return false;
}

// add a new building to be constructed
void BuildingManager::addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation, bool isGasSteal)
{
	_reservedMinerals += type.mineralPrice();
	_reservedGas += type.gasPrice();

    Building b(type, desiredLocation);
    b.isGasSteal = isGasSteal;
    b.status = BuildingStatus::Unassigned;

    _buildings.push_back(b);
	_numBuildings[b.type.getID()] += 1;
}

void BuildingManager::addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation, bool isGasSteal, bool nexpHatchery)
{
	_reservedMinerals += type.mineralPrice();
	_reservedGas += type.gasPrice();

	Building b(type, desiredLocation);
	b.isGasSteal = isGasSteal;
	b.nexpHatchery = nexpHatchery;
	b.status = BuildingStatus::Unassigned;

	_buildings.push_back(b);
	_numBuildings[b.type.getID()] += 1;
}

BWAPI::Unit BuildingManager::cancelBuildingTask(BWAPI::UnitType type)
{
	for (auto & itr = _buildings.rbegin(); itr < _buildings.rend(); ++itr)
	{
		auto & b = *itr;
		if (itr->type == type)
		{
			itr->beCanceled = true;
			return itr->buildingUnit;
		}
	}
	return nullptr;
}

bool BuildingManager::isBuildingPositionExplored(const Building & b) const
{
    BWAPI::TilePosition tile = b.finalPosition;

    // for each tile where the building will be built
    for (int x=0; x<b.type.tileWidth(); ++x)
    {
        for (int y=0; y<b.type.tileHeight(); ++y)
        {
            if (!BWAPI::Broodwar->isExplored(tile.x + x,tile.y + y))
            {
                return false;
            }
        }
    }

    return true;
}


char BuildingManager::getBuildingWorkerCode(const Building & b) const
{
    return b.builderUnit == nullptr ? 'X' : 'W';
}

int BuildingManager::getReservedMinerals() 
{
    return _reservedMinerals;
}

int BuildingManager::getReservedGas() 
{
    return _reservedGas;
}

void BuildingManager::drawBuildingInformation(int x,int y)
{
    if (!Config::Debug::DrawBuildingInfo)
    {
        return;
    }

    //for (auto & unit : BWAPI::Broodwar->self()->getUnits())
    //{
    //    BWAPI::Broodwar->drawTextMap(unit->getPosition().x,unit->getPosition().y+5,"\x07%d",unit->getID());
    //}

    BWAPI::Broodwar->drawTextScreen(x,y,"\x04 Building Information(%d):", _buildings.size());
    BWAPI::Broodwar->drawTextScreen(x,y+20,"\x04 Name");
    BWAPI::Broodwar->drawTextScreen(x+150,y+20,"\x04 State");

    int yspace = 0;

    for (const auto & b : _buildings)
    {
        if (b.status == BuildingStatus::Unassigned)
        {
            BWAPI::Broodwar->drawTextScreen(x,y+40+((yspace)*10),"\x03 %s",b.type.getName().c_str());
            BWAPI::Broodwar->drawTextScreen(x+150,y+40+((yspace++)*10),"\x03 Need %c",getBuildingWorkerCode(b));
        }
        else if (b.status == BuildingStatus::Assigned)
        {
            BWAPI::Broodwar->drawTextScreen(x,y+40+((yspace)*10),"\x03 %s %d",b.type.getName().c_str(),b.builderUnit->getID());
            BWAPI::Broodwar->drawTextScreen(x+150,y+40+((yspace++)*10),"\x03 A %c (%d,%d)",getBuildingWorkerCode(b),b.finalPosition.x,b.finalPosition.y);
			/*
            int x1 = b.finalPosition.x*32;
            int y1 = b.finalPosition.y*32;
            int x2 = (b.finalPosition.x + b.type.tileWidth())*32;
            int y2 = (b.finalPosition.y + b.type.tileHeight())*32;

            BWAPI::Broodwar->drawLineMap(b.builderUnit->getPosition().x,b.builderUnit->getPosition().y,(x1+x2)/2,(y1+y2)/2,BWAPI::Colors::Orange);
            BWAPI::Broodwar->drawBoxMap(x1,y1,x2,y2,BWAPI::Colors::Red,false);
			*/
        }
        else if (b.status == BuildingStatus::UnderConstruction)
        {
            BWAPI::Broodwar->drawTextScreen(x,y+40+((yspace)*10),"\x03 %s %d",b.type.getName().c_str(),b.buildingUnit->getID());
            BWAPI::Broodwar->drawTextScreen(x+150,y+40+((yspace++)*10),"\x03 Const %c",getBuildingWorkerCode(b));
        }
    }
}

BuildingManager & BuildingManager::Instance()
{
    static BuildingManager instance;
    return instance;
}

BWAPI::TilePosition BuildingManager::getBuildingLocation(const Building & b)
{
    if (b.type.isRefinery())
    {
        return BuildingPlacer::Instance().getRefineryPosition();
    }

    else if (b.type.isResourceDepot() && !b.nexpHatchery)
    {
        // get the location 
        BWAPI::TilePosition tile = BuildingPlacer::Instance().getNextExpansion();

        return tile;
    }

	else if (b.type == BWAPI::UnitTypes::Zerg_Creep_Colony)
	{
		int numCreep = 0;
		for (const auto & b : _buildings)
		{
			if (b.buildingUnit && b.buildingUnit->getType() == BWAPI::UnitTypes::Zerg_Creep_Colony)
			{
				++numCreep;
			}
		}
		BWAPI::TilePosition tile = BuildingPlacer::Instance().getCreepPosition(numCreep);
		return tile;
	}

    // set the building padding specifically
    int distance = Config::Macro::BuildingSpacing;

    // get a position within our region
	BWAPI::TilePosition ret = BuildingPlacer::Instance().getBuildLocationNear(b, distance, false);
	if (ret == BWAPI::TilePositions::None)
		return BuildingPlacer::Instance().getBuildLocationNear(b, 0, false);
	else
		return ret;
}

void BuildingManager::removeBuildings(const std::vector<Building> & toRemove)
{
    for (auto & b : toRemove)
    {
        auto & it = std::find(_buildings.begin(), _buildings.end(), b);

        if (it != _buildings.end())
        {
			_numBuildings[it->type.getID()] -= 1;
            _buildings.erase(it);	
        }
    }
}