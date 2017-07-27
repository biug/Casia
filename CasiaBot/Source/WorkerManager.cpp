#include "Common.h"
#include "WorkerManager.h"
#include "Micro.h"

using namespace CasiaBot;

WorkerManager::WorkerManager() 
{
	needLessGas = needMoreGas = needLessMineral = needMoreMineral = false;
    previousClosestWorker = nullptr;
}

WorkerManager & WorkerManager::Instance() 
{
	static WorkerManager instance;
	return instance;
}

void WorkerManager::update() 
{
	updateResourceStatus();
	updateWorkerStatus();
	handleIdleWorkers();
	handleGasWorkers();
	handleMineralWorkers();
	handleMoveWorkers();
	handleCombatWorkers();

	workerData.drawWorkerDebugInfo();
	workerData.drawResourceDebugInfo(540, 200);
}

void WorkerManager::updateResourceStatus()
{
	if (BWAPI::Broodwar->getFrameCount() % 10 == 0)
	{
		needLessGas = needMoreGas = needLessMineral = needMoreMineral = false;
		gasNotUsed = false;
		gasUsed.push_back(BWAPI::Broodwar->self()->spentGas());
		if (gasUsed.size() >= 20)
		{
			if (gasUsed.front() == gasUsed.back()) {
				gasNotUsed = true;
			}
			gasUsed.pop_front();
		}
		int mineral = BWAPI::Broodwar->self()->minerals();
		int gas = BWAPI::Broodwar->self()->gas();
		if (mineral < 50)
		{
			needMoreMineral = true;
		}
		if (gas > 448 && gasNotUsed)
		{
			needLessGas = true;
		}
		else if (gas < 100 || !gasNotUsed)
		{
			needMoreGas = true;
		}
	}
}

void WorkerManager::updateWorkerStatus() 
{
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		// add the depot if it exists
		if (workerData.isMineralBase(unit))
		{
			workerData.addMineralBase(unit);
		}

		if (workerData.isRefinery(unit))
		{
			workerData.addRefinery(unit);
		}

		// if something morphs into a worker, add it
		if (unit->getType().isWorker() && unit->getHitPoints() >= 0)
		{
			//BWAPI::Broodwar->printf("A worker was shown %d", unit->getID());
			workerData.addWorker(unit);
		}
	}
	// for each of our Workers
	for (auto & worker : workerData.getWorkers())
	{
		if (!worker->isCompleted())
		{
			continue;
		}

		if (BWAPI::Broodwar->getFrameCount() > 10 && BWAPI::Broodwar->getFrameCount() % 10 == 0)
		{
			if (worker->getType().isWorker())
			{
				workersPos[worker].push_back(worker->getPosition());
				if (workersPos[worker].size() >= 12)
				{
					auto job = workerData.getWorkerJob(worker);
					// 如果非采矿农民连续120帧没有位移，说明它闲置了，应该设置为Idle
					if (isWorkerIdle(worker))
					{
						workerData.setWorkerIdle(worker);
					}
					workersPos[worker].pop_front();
				}
			}
		}

		// if it's idle
		if (worker->isIdle() && 
			(workerData.getWorkerJob(worker) != WorkerData::Build) && 
			(workerData.getWorkerJob(worker) != WorkerData::Move) &&
			(workerData.getWorkerJob(worker) != WorkerData::Scout)) 
		{
			workerData.setWorkerIdle(worker);
		}

		// if its job is gas
		if (workerData.getWorkerJob(worker) == WorkerData::Gas)
		{
			BWAPI::Unit refinery = workerData.getWorkerRefinery(worker);

			// if the refinery doesn't exist anymore
			if (!refinery || !refinery->exists() ||	refinery->getHitPoints() <= 0)
			{
				workerData.setWorkerIdle(worker);
			}
		}

		// 清空Idle农民的位置信息，重新结算
		if (workerData.getWorkerJob(worker) == WorkerData::Idle)
		{
			workersPos[worker].clear();
		}
	}
}

void WorkerManager::setWorkerRepairing(BWAPI::Unit worker, BWAPI::Unit unitToRepair)
{
    workerData.setWorkerRepairing(worker, unitToRepair);
}

void WorkerManager::stopRepairing(BWAPI::Unit worker)
{
	workerData.setWorkerIdle(worker);
}

void WorkerManager::handleGasWorkers()
{
	// for each unit we have
	bool firstRefinery = true;
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		// if that unit is a refinery
		if (workerData.isRefinery(unit) && !isGasStealRefinery(unit))
		{
			if (needLessGas)
			{
				BWAPI::Unit gasWorker = workerData.getRefineryWorker(unit);
				if (gasWorker != nullptr && gasWorker->isMoving())
				{
					setWorkerGatheringMineral(gasWorker);
				}
			}
			else
			{
				// get the number of workers currently assigned to it
				int workersNeeded = Config::Macro::WorkersPerRefinery
					- workerData.getNumRefineryWorkers(unit);
				if (!firstRefinery && !needMoreGas) workersNeeded = 0;

				// if it's less than we want it to be, fill 'er up
				for (int i = 0; i < workersNeeded; ++i)
				{
					BWAPI::Unit gasWorker = getGasWorker(unit);
					if (gasWorker)
					{
						setWorkerGatheringGas(gasWorker);
					}
				}
				firstRefinery = false;
			}
		}
	}
}

bool WorkerManager::isGasStealRefinery(BWAPI::Unit unit)
{
    BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
    if (!enemyBaseLocation)
    {
        return false;
    }
    
    if (enemyBaseLocation->getGeysers().empty())
    {
        return false;
    }
    
	for (auto & u : enemyBaseLocation->getGeysers())
	{
        if (unit->getTilePosition() == u->getTilePosition())
        {
            return true;
        }
	}

    return false;
}

void WorkerManager::handleMineralWorkers()
{
	rebalanceMineralWorkers();
}

void WorkerManager::handleIdleWorkers() 
{
	// for each of our workers
	for (auto & worker : workerData.getWorkers())
	{
        CAB_ASSERT(worker != nullptr, "Worker was null");

		// if it is idle
		if (workerData.getWorkerJob(worker) == WorkerData::Idle) 
		{
			// send it to the nearest mineral patch
			setWorkerGatheringMineral(worker);
		}
	}
}

// bad micro for combat workers
void WorkerManager::handleCombatWorkers()
{
	for (auto & worker : workerData.getWorkers())
	{
        CAB_ASSERT(worker != nullptr, "Worker was null");

		if (workerData.getWorkerJob(worker) == WorkerData::Combat)
		{
			BWAPI::Unit target = getClosestEnemyUnit(worker);

			if (target)
			{
				Micro::SmartAttackUnit(worker, target);
			}
		}
	}
}

BWAPI::Unit WorkerManager::getClosestEnemyUnit(BWAPI::Unit worker)
{
    CAB_ASSERT(worker != nullptr, "Worker was null");

	BWAPI::Unit closestUnit = nullptr;
	double closestDist = 10000;

	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		double dist = unit->getDistance(worker);

		if ((dist < 400) && (!closestUnit || (dist < closestDist)))
		{
			closestUnit = unit;
			closestDist = dist;
		}
	}

	return closestUnit;
}

void WorkerManager::finishedWithCombatWorkers()
{
	for (auto & worker : workerData.getWorkers())
	{
        CAB_ASSERT(worker != nullptr, "Worker was null");

		if (workerData.getWorkerJob(worker) == WorkerData::Combat)
		{
			setWorkerGatheringMineral(worker);
		}
	}
}

BWAPI::Unit WorkerManager::getClosestMineralWorkerTo(BWAPI::Unit enemyUnit)
{
    CAB_ASSERT(enemyUnit != nullptr, "enemyUnit was null");

    BWAPI::Unit closestMineralWorker = nullptr;
    double closestDist = 100000;

    if (previousClosestWorker)
    {
        if (previousClosestWorker->getHitPoints() > 0)
        {
            return previousClosestWorker;
        }
    }

    // for each of our workers
	for (auto & worker : workerData.getWorkers())
	{
        CAB_ASSERT(worker != nullptr, "Worker was null");
		if (!worker)
		{
			continue;
		}
		// if it is a move worker
        if (workerData.getWorkerJob(worker) == WorkerData::Minerals) 
		{
			double dist = worker->getDistance(enemyUnit);

            if (!closestMineralWorker || dist < closestDist)
            {
                closestMineralWorker = worker;
                dist = closestDist;
            }
		}
	}

    previousClosestWorker = closestMineralWorker;
    return closestMineralWorker;
}

BWAPI::Unit WorkerManager::getWorkerScout()
{
    // for each of our workers
	for (auto & worker : workerData.getWorkers())
	{
        CAB_ASSERT(worker != nullptr, "Worker was null");
		if (!worker)
		{
			continue;
		}
		// if it is a move worker
        if (workerData.getWorkerJob(worker) == WorkerData::Scout) 
		{
			return worker;
		}
	}

    return nullptr;
}

BWAPI::Unit WorkerManager::getLarvaDepot()
{
	return workerData.getLarvaDepot();
}

const BWAPI::Unitset & WorkerManager::getRefineries() const
{
	return workerData.getRefineries();
}

void WorkerManager::addCanceledRefineryLocation(BWAPI::TilePosition position)
{
	return workerData.addCanceledRefineryLocation(position);
}

const std::set<BWAPI::TilePosition> & WorkerManager::getCanceledRefineryLocations() const
{
	return workerData.getCanceledRefineryLocations();
}

void WorkerManager::handleMoveWorkers() 
{
	// for each of our workers
	for (auto & worker : workerData.getWorkers())
	{
        CAB_ASSERT(worker != nullptr, "Worker was null");

		// if it is a move worker
		if (workerData.getWorkerJob(worker) == WorkerData::Move) 
		{
			WorkerMoveData data = workerData.getWorkerMoveData(worker);
			
			Micro::SmartMove(worker, data.position);
		}
	}
}

bool WorkerManager::isWorkerIdle(BWAPI::Unit unit)
{
	if (unit && workersPos.find(unit) != workersPos.end())
	{
		auto & poses = workersPos[unit];
		for (size_t i = 1; i < poses.size(); ++i)
		{
			if (poses[i - 1] != poses[i])
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

// set a worker to mine minerals
void WorkerManager::setWorkerGatheringMineral(BWAPI::Unit unit)
{
    CAB_ASSERT(unit != nullptr, "Unit was null");

	// update workerData with the new job
	workerData.setWorkerGatheringMineral(unit);
}

// set a worker to mine minerals
void WorkerManager::setWorkerGatheringGas(BWAPI::Unit unit)
{
	CAB_ASSERT(unit != nullptr, "Unit was null");

	// update workerData with the new job
	workerData.setWorkerGatheringGas(unit);
}

BWAPI::Unit WorkerManager::getClosestResource(BWAPI::Unit worker, BWAPI::Unitset & poses)
{
	CAB_ASSERT(worker != nullptr, "Worker was null");

	BWAPI::Unit closestDepot = nullptr;
	double closestDistance = 0;

	for (auto & unit : poses)
	{
		CAB_ASSERT(unit != nullptr, "Unit was null");

		double distance = unit->getDistance(worker);
		if (!closestDepot || distance < closestDistance)
		{
			closestDepot = unit;
			closestDistance = distance;
		}
	}

	return closestDepot;
}

// other managers that need workers call this when they're done with a unit
void WorkerManager::finishedWithWorker(BWAPI::Unit unit) 
{
	CAB_ASSERT(unit != nullptr, "Unit was null");

	//BWAPI::Broodwar->printf("BuildingManager finished with worker %d", unit->getID());
	if (workerData.getWorkerJob(unit) != WorkerData::Scout)
	{
		workerData.setWorkerIdle(unit);
	}
}

BWAPI::Unit WorkerManager::getGasWorker(BWAPI::Unit refinery)
{
	CAB_ASSERT(refinery != nullptr, "Refinery was null");

	BWAPI::Unit closestWorker = nullptr;
	double closestDistance = 0;

	for (auto & unit : workerData.getWorkers())
	{
        CAB_ASSERT(unit != nullptr, "Unit was null");

		if (workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			double distance = unit->getDistance(refinery);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	return closestWorker;
}

void WorkerManager::setWorkerBuilding(BWAPI::Unit worker, Building & b)
 {
     CAB_ASSERT(worker != nullptr, "Worker was null");

     workerData.setWorkerBuilding(worker, b.type);
 }

// gets a builder for BuildingManager to use
// if setJobAsBuilder is true (default), it will be flagged as a builder unit
// set 'setJobAsBuilder' to false if we just want to see which worker will build a building
BWAPI::Unit WorkerManager::getBuilder(Building & b, bool setJobAsBuilder)
{
	// variables to hold the closest worker of each type to the building
	BWAPI::Unit closestMovingWorker = nullptr;
	BWAPI::Unit closestMiningWorker = nullptr;
	double closestMovingWorkerDistance = 0;
	double closestMiningWorkerDistance = 0;

	// look through each worker that had moved there first
	for (auto & worker : workerData.getWorkers())
	{
		CAB_ASSERT(worker != nullptr, "Unit was null");

        // gas steal building uses scout worker
		if (b.isGasSteal && (workerData.getWorkerJob(worker) == WorkerData::Scout))
        {
            if (setJobAsBuilder)
            {
                workerData.setWorkerBuilding(worker, b.type);
            }
			return worker;
        }

		// mining worker check
		if (worker->isCompleted() && (workerData.getWorkerJob(worker) == WorkerData::Minerals))
		{
			// if it is a new closest distance, set the pointer
			double distance = worker->getDistance(BWAPI::Position(b.finalPosition));
			if (!closestMiningWorker || distance < closestMiningWorkerDistance)
			{
				closestMiningWorker = worker;
				closestMiningWorkerDistance = distance;
			}
		}

		// moving worker check
		if (worker->isCompleted() && (workerData.getWorkerJob(worker) == WorkerData::Move))
		{
			// if it is a new closest distance, set the pointer
			double distance = worker->getDistance(BWAPI::Position(b.finalPosition));
			if (!closestMovingWorker || distance < closestMovingWorkerDistance)
			{
				closestMovingWorker = worker;
				closestMovingWorkerDistance = distance;
			}
		}
	}

	// if we found a moving worker, use it, otherwise using a mining worker
	BWAPI::Unit chosenWorker = closestMovingWorker ? closestMovingWorker : closestMiningWorker;

	// if the worker exists (one may not have been found in rare cases)
	if (chosenWorker && setJobAsBuilder)
	{
		workerData.setWorkerBuilding(chosenWorker, b.type);
	}

	// return the worker
	return chosenWorker;
}

// sets a worker as a scout
void WorkerManager::setScoutWorker(BWAPI::Unit worker)
{
	CAB_ASSERT(worker != nullptr, "Worker was null");

	workerData.setWorkerScouting(worker);
}

// gets a worker which will move to a current location
BWAPI::Unit WorkerManager::getMoveWorker(BWAPI::Position p)
{
	// set up the pointer
	BWAPI::Unit closestWorker = nullptr;
	double closestDistance = 0;

	// for each worker we currently have
	for (auto & unit : workerData.getWorkers())
	{
        CAB_ASSERT(unit != nullptr, "Unit was null");

		// only consider it if it's a mineral worker
		if (unit->isCompleted() && workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(p);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	// return the worker
	return closestWorker;
}

// sets a worker to move to a given location
void WorkerManager::setWorkerMoving(int mineralsNeeded, int gasNeeded, BWAPI::Position p)
{
	// set up the pointer
	BWAPI::Unit closestWorker = nullptr;
	double closestDistance = 0;

	// for each worker we currently have
	for (auto & unit : workerData.getWorkers())
	{
        CAB_ASSERT(unit != nullptr, "Unit was null");

		// only consider it if it's a mineral worker
		if (unit->isCompleted() && workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(p);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	if (closestWorker)
	{
		//BWAPI::Broodwar->printf("Setting worker job Move for worker %d", closestWorker->getID());
		workerData.setWorkerMoving(closestWorker, WorkerMoveData(mineralsNeeded, gasNeeded, p));
	}
	else
	{
		//BWAPI::Broodwar->printf("Error, no worker found");
	}
}

// will we have the required resources by the time a worker can travel a certain distance
bool WorkerManager::willHaveResources(int mineralsRequired, int gasRequired, double distance)
{
	// if we don't require anything, we will have it
	if (mineralsRequired <= 0 && gasRequired <= 0)
	{
		return true;
	}

	// the speed of the worker unit
	double speed = BWAPI::Broodwar->self()->getRace().getWorker().topSpeed();

    CAB_ASSERT(speed > 0, "Speed is negative");

	// how many frames it will take us to move to the building location
	// add a second to account for worker getting stuck. better early than late
	double framesToMove = (distance / speed) + 50;

	// magic numbers to predict income rates
	double mineralRate = getNumMineralWorkers() * 0.045;
	double gasRate     = getNumGasWorkers() * 0.07;

	// calculate if we will have enough by the time the worker gets there
	if (mineralRate * framesToMove >= mineralsRequired + 50 &&
		gasRate * framesToMove >= gasRequired + 24)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void WorkerManager::setWorkerCombating(BWAPI::Unit worker)
{
	CAB_ASSERT(worker != nullptr, "Worker was null");

	workerData.setWorkerCombating(worker);
}

void WorkerManager::onUnitMorph(BWAPI::Unit unit)
{
	CAB_ASSERT(unit != nullptr, "Unit was null");

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0)
	{
		workerData.addWorker(unit);
	}

	// if something morphs into a building, it was a worker?
	if (unit->getType().isBuilding() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		//BWAPI::Broodwar->printf("A Drone started building");
		workerData.workerDestroyed(unit);
	}
}

void WorkerManager::onUnitShow(BWAPI::Unit unit)
{
	CAB_ASSERT(unit != nullptr, "Unit was null");

	// add the depot if it exists
	if (workerData.isMineralBase(unit))
	{
		workerData.addMineralBase(unit);
	}

	// add the refinery if it exists
	if (workerData.isRefinery(unit))
	{
		workerData.addRefinery(unit);
	}

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0)
	{
		//BWAPI::Broodwar->printf("A worker was shown %d", unit->getID());
		workerData.addWorker(unit);
	}
}


void WorkerManager::rebalanceMineralWorkers()
{
	// for each worker
	for (auto & worker : workerData.getWorkers())
	{
        CAB_ASSERT(worker != nullptr, "Worker was null");

		if (workerData.getWorkerJob(worker) != WorkerData::Minerals)
		{
			continue;
		}

		if (worker && workerData.isWorkerInOverloadMineralPatch(worker))
		{
			workerData.setWorkerIdle(worker);
		}
	}
}

void WorkerManager::onUnitDestroy(BWAPI::Unit unit) 
{
	CAB_ASSERT(unit != nullptr, "Unit was null");

	if (workerData.isMineralBase(unit))
	{
		workerData.removeMineralBase(unit);
	}

	if (workerData.isRefinery(unit))
	{
		workerData.removeRefinery(unit);
	}

	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self()) 
	{
		workerData.workerDestroyed(unit);
	}

	if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field)
	{
		workerData.removeMineralPatch(unit);
	}
}

bool WorkerManager::isFree(BWAPI::Unit worker)
{
    CAB_ASSERT(worker != nullptr, "Worker was null");

	return workerData.getWorkerJob(worker) == WorkerData::Minerals || workerData.getWorkerJob(worker) == WorkerData::Idle;
}

bool WorkerManager::isWorkerScout(BWAPI::Unit worker)
{
    CAB_ASSERT(worker != nullptr, "Worker was null");

	return (workerData.getWorkerJob(worker) == WorkerData::Scout);
}

bool WorkerManager::isBuilder(BWAPI::Unit worker)
{
    CAB_ASSERT(worker != nullptr, "Worker was null");

	return (workerData.getWorkerJob(worker) == WorkerData::Build);
}

int WorkerManager::getNumMineralWorkers() 
{
	return workerData.getNumMineralWorkers();	
}

int WorkerManager::getNumIdleWorkers() 
{
	return workerData.getNumIdleWorkers();	
}

int WorkerManager::getNumGasWorkers() 
{
	return workerData.getNumGasWorkers();
}
