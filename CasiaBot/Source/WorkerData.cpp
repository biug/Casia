#include "WorkerData.h"
#include "Micro.h"

using namespace CasiaBot;

WorkerData::WorkerData() 
{
}

void WorkerData::checkResources()
{
	for (auto & worker : workers)
	{
		CAB_ASSERT(worker && worker->exists(), "worker has been killed");
		if (!worker->isGatheringGas() && workerJobMap[worker] == WorkerJob::Gas)
		{
			setWorkerIdle(worker);
		}
		if (!worker->isGatheringMinerals() && workerJobMap[worker] == WorkerJob::Minerals)
		{
			setWorkerIdle(worker);
		}
	}
}

void WorkerData::workerDestroyed(BWAPI::Unit unit)
{
	if (!unit) { return; }

	clearPreviousJob(unit);
	workers.erase(unit);
}

void WorkerData::addWorker(BWAPI::Unit unit)
{
	if (!unit) { return; }

	if (workers.find(unit) == workers.end())
	{
		workers.insert(unit);
		workerJobMap[unit] = Default;
	}
}

void WorkerData::addGatheringWorker(BWAPI::Unit worker, WorkerJob job)
{
	if (!worker) { return; }

	assert(workers.find(worker) == workers.end());

	workers.insert(worker);
	if (job == Minerals)
	{
		setWorkerGatheringMineral(worker);
	}
	if (job == Gas)
	{
		setWorkerGatheringGas(worker);
	}
}

void WorkerData::addBuildingWorker(BWAPI::Unit worker, BWAPI::UnitType buildingType)
{
	if (!worker) { return; }

	assert(workers.find(worker) == workers.end());
	workers.insert(worker);
	setWorkerBuilding(worker, buildingType);
}

void WorkerData::addMineralBase(BWAPI::Unit mineralBase)
{
	if (!mineralBase) { return; }

	if (mineralBases.find(mineralBase) == mineralBases.end())
	{
		mineralBases.insert(mineralBase);
		mineralBaseWorkersMap[mineralBase].clear();
	}
	auto & patches = mineralBaseMineralPatchMap[mineralBase];
	for (auto & unit : BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field
			&& unit->getDistance(mineralBase) < mineralPatchRadius
			&& patches.find(unit) == patches.end())
		{
			patches.insert(unit);
			mineralPatchMineralBaseMap[unit] = mineralBase;
		}
	}
}

void WorkerData::addRefinery(BWAPI::Unit refinery)
{
	if (!refinery) { return; }

	if (canceledRefineryLocations.find(refinery->getTilePosition())
		!= canceledRefineryLocations.end())
	{
		canceledRefineryLocations.erase(refinery->getTilePosition());
	}

	if (refineries.find(refinery) == refineries.end())
	{
		refineries.insert(refinery);
		refineryWorkersMap[refinery].clear();
	}
}

void WorkerData::addCanceledRefineryLocation(BWAPI::TilePosition position)
{
	canceledRefineryLocations.insert(position);
}

void WorkerData::removeMineralBase(BWAPI::Unit mineralBase)
{	
	if (!mineralBase) { return; }

	mineralBases.erase(mineralBase);

	for (auto & worker : mineralBaseWorkersMap[mineralBase])
	{
		setWorkerIdle(worker);
	}

	mineralBaseWorkersMap.erase(mineralBase);
	mineralBaseMineralPatchMap.erase(mineralBase);
}

void WorkerData::removeMineralPatch(BWAPI::Unit mineralPatch)
{
	if (!mineralPatch) { return; }

	auto it = mineralPatchMineralBaseMap.find(mineralPatch);

	if (it == mineralPatchMineralBaseMap.end()) { return; }

	auto mineralBase = it->second;
	mineralPatchMineralBaseMap.erase(mineralPatch);

	if (!mineralBase) { return; }
	mineralBaseMineralPatchMap[mineralBase].erase(mineralPatch);
}

void WorkerData::removeRefinery(BWAPI::Unit refinery)
{
	if (!refinery) { return; }

	refineries.erase(refinery);

	// re-balance workers in here
	for (auto & worker : workers)
	{
		// if a worker was working at this depot
		if (workerRefineryMap.find(worker) != workerRefineryMap.end()
			&& workerRefineryMap[worker] == refinery)
		{
			setWorkerIdle(worker);
		}
	}
	refineryWorkersMap.erase(refinery);
}

void WorkerData::setWorkerIdle(BWAPI::Unit worker)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = WorkerJob::Idle;
}

void WorkerData::setWorkerGatheringMineral(BWAPI::Unit worker)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = WorkerJob::Minerals;

	// find best mineral patch
	auto mineral = getClosestMineral(worker);
	auto mineralPatch = mineral.first;
	auto mineralBase = mineral.second;
	if (!mineralPatch || !mineralBase)
	{
		workerJobMap[worker] = WorkerJob::Idle;
		return;
	}

	// set the mineral the worker is working on
	workerMineralBaseMap[worker] = mineralBase;
	mineralBaseWorkersMap[mineralBase].insert(worker);

	// right click the mineral to start mining
	Micro::SmartRightClick(worker, mineralPatch);
}

void WorkerData::setWorkerGatheringGas(BWAPI::Unit worker)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = WorkerJob::Gas;

	auto refinery = getClosestRefinery(worker);
	if (!refinery)
	{
		workerJobMap[worker] = WorkerJob::Idle;
		return;
	}

	// set the refinery the worker is working on
	workerRefineryMap[worker] = refinery;
	refineryWorkersMap[refinery].insert(worker);

	// right click the refinery to start harvesting
	Micro::SmartRightClick(worker, refinery);
}

void WorkerData::setWorkerBuilding(BWAPI::Unit worker, BWAPI::UnitType bulidingType)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = WorkerJob::Build;

	workerBuildingTypeMap[worker] = bulidingType;
}

void WorkerData::setWorkerMoving(BWAPI::Unit worker, WorkerMoveData wmd)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = WorkerJob::Move;

	workerMoveMap[worker] = wmd;
}

void WorkerData::setWorkerRepairing(BWAPI::Unit worker, BWAPI::Unit building)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = WorkerJob::Repair;

	assert(worker->getType() == BWAPI::UnitTypes::Terran_SCV);

	// set the building the worker is to repair
	workerRepairMap[worker] = building;

	// start repairing 
	if (!worker->isRepairing())
	{
		Micro::SmartRepair(worker, building);
	}
}

void WorkerData::setWorkerScouting(BWAPI::Unit worker)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = WorkerJob::Scout;
}

void WorkerData::setWorkerCombating(BWAPI::Unit worker)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = WorkerJob::Combat;
}

void WorkerData::clearPreviousJob(BWAPI::Unit worker)
{
	if (!worker) { return; }

	WorkerJob previousJob = getWorkerJob(worker);

	if (previousJob == Minerals)
	{
		auto mineralBase = workerMineralBaseMap[worker];

		mineralBaseWorkersMap[mineralBase].erase(worker);
		workerMineralBaseMap.erase(worker);
	}
	else if (previousJob == Gas)
	{
		auto refinery = workerRefineryMap[worker];

		refineryWorkersMap[refinery].erase(worker);
		workerRefineryMap.erase(worker);
	}
	else if (previousJob == Build)
	{
		workerBuildingTypeMap.erase(worker);
	}
	else if (previousJob == Repair)
	{
		workerRepairMap.erase(worker);
	}
	else if (previousJob == Move)
	{
		workerMoveMap.erase(worker);
	}

	workerJobMap.erase(worker);
}

int WorkerData::getNumWorkers() const
{
	return workers.size();
}

int WorkerData::getNumMineralWorkers() const
{
	size_t num = 0;
	for (auto & unit : workers)
	{
		if (workerJobMap.at(unit) == WorkerData::Minerals)
		{
			num++;
		}
	}
	return num;
}

int WorkerData::getNumGasWorkers() const
{
	size_t num = 0;
	for (auto & unit : workers)
	{
		if (workerJobMap.at(unit) == WorkerData::Gas)
		{
			num++;
		}
	}
	return num;
}

int WorkerData::getNumRefineryWorkers(BWAPI::Unit refinery) const
{
	auto it = refineryWorkersMap.find(refinery);
	if (it == refineryWorkersMap.end())
	{
		return 0;
	}
	return it->second.size();
}

int WorkerData::getNumIdleWorkers() const
{
	size_t num = 0;
	for (auto & unit : workers)
	{
		if (workerJobMap.at(unit) == WorkerData::Idle)
		{
			num++;
		}
	}
	return num;
}


enum WorkerData::WorkerJob WorkerData::getWorkerJob(BWAPI::Unit worker) const
{
	if (!worker) { return Default; }

	auto it = workerJobMap.find(worker);

	if (it != workerJobMap.end())
	{
		return it->second;
	}

	return Default;
}

bool WorkerData::isWorkerInOverloadMineral(BWAPI::Unit worker) const
{
	if (!worker) { return false; }

	auto itBase = workerMineralBaseMap.find(worker);

	if (itBase == workerMineralBaseMap.end())
	{
		return true;
	}

	auto base = itBase->second;

	auto itWorkers = mineralBaseWorkersMap.find(base);

	if (itWorkers == mineralBaseWorkersMap.end())
	{
		return true;
	}

	auto itPatch = mineralBaseMineralPatchMap.find(base);

	if (itPatch == mineralBaseMineralPatchMap.end())
	{
		return true;
	}

	return itWorkers->second.size() >
		Config::Macro::WorkersPerMineralPatch * itPatch->second.size();
}

bool WorkerData::isMineralBase(BWAPI::Unit base) const
{
	if (base->getType() == BWAPI::UnitTypes::Zerg_Hatchery && !base->isCompleted()
		|| base->getPlayer() != BWAPI::Broodwar->self()) {
		return false;
	}
	if (base->getType().isResourceDepot())
	{
		for (auto & mineralBase : mineralBases)
		{
			if (base->getDistance(mineralBase) < sameMineralBaseRadius)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

bool WorkerData::isRefinery(BWAPI::Unit refinery) const
{
	return refinery->getType().isRefinery()
		&& refinery->isCompleted()
		&& refinery->getPlayer() == BWAPI::Broodwar->self();
}

std::pair<BWAPI::Unit, BWAPI::Unit> WorkerData::getClosestMineral(BWAPI::Unit worker) const
{
	if (!worker) { return{ nullptr, nullptr }; }

	// get the depot associated with this unit
	BWAPI::Unit bestPatch = nullptr;
	BWAPI::Unit bestMineralBase = nullptr;
	int			minDist = 1000000;

	for (auto mineralBase : mineralBases)
	{
		auto itWorkers = mineralBaseWorkersMap.find(mineralBase);
		if (itWorkers == mineralBaseWorkersMap.end()) { continue; }
		auto itPatch = mineralBaseMineralPatchMap.find(mineralBase);
		if (itPatch == mineralBaseMineralPatchMap.end()) { continue; }
		// find an unoverload mineral base
		if (itWorkers->second.size() <
			Config::Macro::WorkersPerMineralPatch * itPatch->second.size())
		{
			// find a most valuble patch
			for (const auto & patch : itPatch->second)
			{
				if (worker->getDistance(patch) < minDist)
				{
					bestPatch = patch;
					bestMineralBase = mineralBase;
					minDist = worker->getDistance(patch);
				}
			}
		}
	}
	return{ bestPatch, bestMineralBase };
}

BWAPI::Unit WorkerData::getClosestRefinery(BWAPI::Unit worker) const
{
	if (!worker) { return nullptr; }

	// get the depot associated with this unit
	BWAPI::Unit bestRefinery = nullptr;
	int			bestDist = 100000;

	for (auto refinery : refineries)
	{
		auto it = refineryWorkersMap.find(refinery);
		if (it == refineryWorkersMap.end()) { continue; }
		if (it->second.size() < Config::Macro::WorkersPerRefinery)
		{
			int dist = worker->getDistance(refinery);
			if (dist < bestDist)
			{
				bestDist = dist;
				bestRefinery = refinery;
			}
		}
	}
	return bestRefinery;
}

BWAPI::Unit WorkerData::getLarvaDepot() const
{
	int maxRemain = 0;
	BWAPI::Unit bestMineralBase = nullptr;
	for (auto mineralBase : mineralBases)
	{
		auto itWorkers = mineralBaseWorkersMap.find(mineralBase);
		if (itWorkers == mineralBaseWorkersMap.end()) { continue; }
		auto itPatch = mineralBaseMineralPatchMap.find(mineralBase);
		if (itPatch == mineralBaseMineralPatchMap.end()) { continue; }
		int remain = itPatch->second.size() * Config::Macro::WorkersPerMineralPatch - itWorkers->second.size();
		if (remain > maxRemain)
		{
			maxRemain = remain;
			int maxLarva = 0;
			for (const auto & base : BWAPI::Broodwar->self()->getUnits())
			{
				if (base->getType() == BWAPI::UnitTypes::Zerg_Hatchery ||
					base->getType() == BWAPI::UnitTypes::Zerg_Hive ||
					base->getType() == BWAPI::UnitTypes::Zerg_Lair) 
				{
					//BWAPI::Broodwar->drawTextScreen(200, 200, "\x04 distance:  %d", base->getDistance(mineralBase));
					if (base == mineralBase || base->getDistance(mineralBase) < 300)
					{
						int larvas = base->getLarva().size();
						if (larvas > maxLarva)
						{
							maxLarva = larvas;
							bestMineralBase = base;
						}
					}
				}
			}
		}
	}
	return bestMineralBase;
}

const BWAPI::Unitset & WorkerData::getRefineries() const
{
	return refineries;
}

const std::set<BWAPI::TilePosition> & WorkerData::getCanceledRefineryLocations() const
{
	return canceledRefineryLocations;
}

BWAPI::Unit WorkerData::getWorkerRepairUnit(BWAPI::Unit unit) const
{
	if (!unit) { return nullptr; }

	auto it = workerRepairMap.find(unit);

	if (it != workerRepairMap.end())
	{
		return it->second;
	}	

	return nullptr;
}

BWAPI::Unit WorkerData::getRefineryWorker(BWAPI::Unit refinery) const
{
	if (!refinery) { return nullptr; }

	auto it = refineryWorkersMap.find(refinery);

	if (it != refineryWorkersMap.end() && it->second.size() > 0)
	{
		return *it->second.begin();
	}

	return nullptr;
}

BWAPI::Unit WorkerData::getWorkerMineralBase(BWAPI::Unit worker) const
{
	if (!worker) { return nullptr; }

	auto it = workerMineralBaseMap.find(worker);

	if (it != workerMineralBaseMap.end())
	{
		return it->second;
	}	

	return nullptr;
}

BWAPI::Unit WorkerData::getWorkerRefinery(BWAPI::Unit worker) const
{
	if (!worker) { return nullptr; }

	auto it = workerRefineryMap.find(worker);

	if (it != workerRefineryMap.end())
	{
		return it->second;
	}

	return nullptr;
}

BWAPI::UnitType	WorkerData::getWorkerBuildingType(BWAPI::Unit unit) const
{
	if (!unit) { return BWAPI::UnitTypes::None; }

	auto it = workerBuildingTypeMap.find(unit);

	if (it != workerBuildingTypeMap.end())
	{
		return it->second;
	}	

	return BWAPI::UnitTypes::None;
}

WorkerMoveData WorkerData::getWorkerMoveData(BWAPI::Unit unit) const
{
	auto it = workerMoveMap.find(unit);

	assert(it != workerMoveMap.end());
	
	return (it->second);
}

char WorkerData::getJobCode(BWAPI::Unit unit) const
{
	if (!unit) { return 'X'; }

	WorkerData::WorkerJob j = getWorkerJob(unit);

	if (j == WorkerData::Build) return 'B';
	if (j == WorkerData::Combat) return 'C';
	if (j == WorkerData::Default) return 'D';
	if (j == WorkerData::Gas) return 'G';
	if (j == WorkerData::Idle) return 'I';
	if (j == WorkerData::Minerals) return 'M';
	if (j == WorkerData::Repair) return 'R';
	if (j == WorkerData::Move) return 'O';
	if (j == WorkerData::Scout) return 'S';
	return 'X';
}

std::string WorkerData::getJobString(BWAPI::Unit unit) const
{
	if (!unit) { return "NULL"; }

	WorkerData::WorkerJob j = getWorkerJob(unit);

	switch (j)
	{
	case WorkerData::Build:
		return "Build";
	case WorkerData::Combat:
		return "Combat";
	case WorkerData::Default:
		return "Default";
	case WorkerData::Gas:
		return "Gas";
	case WorkerData::Idle:
		return "Idle";
	case WorkerData::Minerals:
		return "Minerals";
	case WorkerData::Repair:
		return "Repair";
	case WorkerData::Move:
		return "Move";
	case WorkerData::Scout:
		return "Scout";
	}

	return "NULL";
}

void WorkerData::drawWorkerDebugInfo() const
{
	for (auto & worker : workers)
	{
		CAB_ASSERT(worker != nullptr, "Worker was null");

		std::string job = getJobString(worker);

		BWAPI::Position pos = worker->getTargetPosition();

		BWAPI::Broodwar->drawTextMap(worker->getPosition().x, worker->getPosition().y - 5, job.c_str());

		BWAPI::Broodwar->drawLineMap(worker->getPosition().x, worker->getPosition().y, pos.x, pos.y, BWAPI::Colors::Cyan);
	}
}

void WorkerData::drawResourceDebugInfo(int x, int y) const
{
	int mineralPatchCount = 0;
	for (auto & mineralBase : mineralBases)
	{
		auto itWorkers = mineralBaseWorkersMap.find(mineralBase);
		if (itWorkers == mineralBaseWorkersMap.end()) { continue; }
		BWAPI::Broodwar->drawTextMap(mineralBase->getPosition() + BWAPI::Position(0, 3), "\x04 %d", itWorkers->second.size());
		mineralPatchCount += mineralBaseMineralPatchMap.at(mineralBase).size();
		for (auto & mineralPatch : mineralBaseMineralPatchMap.at(mineralBase))
		{
			BWAPI::Broodwar->drawCircleMap(mineralPatch->getPosition(), 3, BWAPI::Colors::Red, true);
		}
	}
	for (auto & refinery : refineries)
	{
		auto itWorkers = refineryWorkersMap.find(refinery);
		if (itWorkers == refineryWorkersMap.end()) { continue; }
		BWAPI::Broodwar->drawCircleMap(refinery->getPosition(), 3, BWAPI::Colors::Green, true);
		BWAPI::Broodwar->drawTextMap(refinery->getPosition() + BWAPI::Position(0, 3), "\x04 %d", itWorkers->second.size());
	}

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Mineral Patches %d", mineralPatchCount);
	BWAPI::Broodwar->drawTextScreen(x, y + 10, "\x04 Mineral Workers %d", getNumMineralWorkers());
	BWAPI::Broodwar->drawTextScreen(x, y + 20, "\x04 Refineries %d", refineries.size());
	BWAPI::Broodwar->drawTextScreen(x, y + 30, "\x04 Gas Workers %d", getNumGasWorkers());
}