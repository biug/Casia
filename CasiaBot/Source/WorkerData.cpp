#include "WorkerData.h"
#include "Micro.h"

using namespace CasiaBot;

WorkerData::WorkerData() 
{
}

void WorkerData::checkWorkersStatus()
{
	// add new worker
	patchWorkers.clear();
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		// add the depot if it exists
		if (isMineralBase(unit))
		{
			addMineralBase(unit);
		}

		if (isRefinery(unit))
		{
			addRefinery(unit);
		}

		// if something morphs into a worker, add it
		if (unit->getType().isWorker() && unit->isCompleted() && unit->getHitPoints() > 0)
		{
			//BWAPI::Broodwar->printf("A worker was shown %d", unit->getID());
			addWorker(unit);
		}
	}
	BWAPI::Unitset newWorkers;
	int frame = BWAPI::Broodwar->getFrameCount();
	for (auto & worker : workers)
	{
		if (!worker || !worker->getPosition().isValid() || !worker->exists())
		{
			continue;
		}
		else
		{
			newWorkers.insert(worker);
			int frameOffset = frame - workerJobMap[worker].second;
			if (frameOffset > 80)
			{
				// 没有在采气，但是Job是采气
				if (!worker->isGatheringGas()
					&& workerJobMap[worker].first == WorkerJob::Gas
					&& workerRefineryMap.find(worker) != workerRefineryMap.end()
					&& worker->getDistance(workerRefineryMap.at(worker)) > 300)
				{
					setWorkerIdle(worker);
				}
				// 没有在采矿，但是Job是采矿
				if (!worker->isGatheringMinerals()
					&& workerJobMap[worker].first == WorkerJob::Minerals
					&& workerMineralBaseMap.find(worker) != workerMineralBaseMap.end()
					&& worker->getDistance(workerMineralBaseMap.at(worker)) > 300)
				{
					setWorkerIdle(worker);
				}
				// 在采气，但是Job不是采气
				if (workerJobMap[worker].first != WorkerJob::Gas
					&& worker->isCarryingGas()
					&& workerRefineryMap.find(worker) != workerRefineryMap.end()
					&& worker->getDistance(workerRefineryMap.at(worker)) < 300)
				{
					setWorkerGatheringGas(worker);
				}
				// 在采矿，但是Job不是采矿
				if (workerJobMap[worker].first != WorkerJob::Minerals
					&& worker->isCarryingMinerals()
					&& workerMineralBaseMap.find(worker) != workerMineralBaseMap.end()
					&& worker->getDistance(workerMineralBaseMap.at(worker)) < 300)
				{
					setWorkerGatheringMineral(worker);
				}
			}
		}
	}
	if (newWorkers.size() != workers.size())
	{
		BWAPI::Broodwar->printf("worker bad size");
		workers.clear();
		for (const auto & worker : newWorkers)
		{
			workers.insert(worker);
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
		workerJobMap[unit] = { Idle,BWAPI::Broodwar->getFrameCount() };
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
	if (mineralBaseMineralPatchInitMap.find(mineralBase) == mineralBaseMineralPatchInitMap.end())
	{
		auto & initMap = mineralBaseMineralPatchInitMap[mineralBase];
		initMap.clear();
		for (const auto & patch : patches)
		{
			initMap.push_back({ patch, false });
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
	mineralBaseMineralPatchInitMap.erase(mineralBase);
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
	auto & initMap = mineralBaseMineralPatchInitMap[mineralBase];
	auto itr = std::find(initMap.begin(), initMap.end(), std::make_pair(mineralPatch, true));
	if (itr == initMap.end()) itr = std::find(initMap.begin(), initMap.end(), std::make_pair(mineralPatch, false));
	if (itr != initMap.end()) initMap.erase(itr);
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
	workerJobMap[worker] = { Idle,BWAPI::Broodwar->getFrameCount() };
}

void WorkerData::setWorkerGatheringMineral(BWAPI::Unit worker)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = { Minerals,BWAPI::Broodwar->getFrameCount() };

	// find best mineral patch
	auto mineral = getClosestMineral(worker);
	auto mineralPatch = mineral.first;
	auto mineralBase = mineral.second;
	if (!mineralPatch || !mineralBase)
	{
		workerJobMap[worker] = { Idle,BWAPI::Broodwar->getFrameCount() };
		return;
	}

	// right click the mineral to start mining
	if (!Micro::SmartRightClick(worker, mineralPatch))
	{
		// if click fail, return
		workerJobMap[worker] = { Idle,BWAPI::Broodwar->getFrameCount() };
		return;
	}

	// set the mineral the worker is working on
	workerMineralBaseMap[worker] = mineralBase;
	mineralBaseWorkersMap[mineralBase].insert(worker);
	for (auto & initMap : mineralBaseMineralPatchInitMap[mineralBase])
	{
		if (initMap.first == mineralPatch)
		{
			initMap.second = true;
			break;
		}
	}
}

void WorkerData::setWorkerGatheringGas(BWAPI::Unit worker)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = { Gas,BWAPI::Broodwar->getFrameCount() };

	auto refinery = getClosestRefinery(worker);
	if (!refinery)
	{
		workerJobMap[worker] = { Idle,BWAPI::Broodwar->getFrameCount() };
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
	workerJobMap[worker] = { Build,BWAPI::Broodwar->getFrameCount() };

	workerBuildingTypeMap[worker] = bulidingType;
}

void WorkerData::setWorkerMoving(BWAPI::Unit worker, WorkerMoveData wmd)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = { Move,BWAPI::Broodwar->getFrameCount() };

	workerMoveMap[worker] = wmd;
}

void WorkerData::setWorkerRepairing(BWAPI::Unit worker, BWAPI::Unit building)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = { Repair,BWAPI::Broodwar->getFrameCount() };

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
	workerJobMap[worker] = { Scout,BWAPI::Broodwar->getFrameCount() };
}

void WorkerData::setWorkerCombating(BWAPI::Unit worker)
{
	if (!worker) { return; }

	clearPreviousJob(worker);
	workerJobMap[worker] = { Combat,BWAPI::Broodwar->getFrameCount() };
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
		if (workerJobMap.at(unit).first == WorkerData::Minerals)
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
		if (workerJobMap.at(unit).first == WorkerData::Gas)
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
		if (workerJobMap.at(unit).first == WorkerData::Idle)
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
		return it->second.first;
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
		Config::Macro::WorkersPerMineralPatch * itPatch->second.size() + 2;
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

std::pair<BWAPI::Unit, BWAPI::Unit> WorkerData::getClosestMineral(BWAPI::Unit worker)
{
	if (!worker) { return{ nullptr, nullptr }; }

	// get the depot associated with this unit
	int			minDist0 = 1000000, minDist1 = 1000000;
	BWAPI::Unit bestPatch0 = nullptr, bestPatch1 = nullptr;
	BWAPI::Unit bestMineralBase0 = nullptr, bestMineralBase1 = nullptr;
	for (const auto & worker : workers)
	{
		if (getWorkerJob(worker) == Minerals)
		{
			auto target = worker->getTarget();
			if (target && target->exists() && target->getType().isMineralField())
			{
				patchWorkers[target] = 1;
			}
		}
	}

	for (auto mineralBase : mineralBases)
	{
		auto itWorkers = mineralBaseWorkersMap.find(mineralBase);
		if (itWorkers == mineralBaseWorkersMap.end()) { continue; }
		auto itPatch = mineralBaseMineralPatchMap.find(mineralBase);
		if (itPatch == mineralBaseMineralPatchMap.end()) { continue; }
		auto itPatchInit = mineralBaseMineralPatchInitMap.find(mineralBase);
		if (itPatchInit == mineralBaseMineralPatchInitMap.end()) { continue; }
		std::sort(itPatchInit->second.begin(), itPatchInit->second.end(),
			[worker](const std::pair<BWAPI::Unit, bool> & p1, const std::pair<BWAPI::Unit, bool> & p2)
		{
			return p1.first->getDistance(worker) < p2.first->getDistance(worker);
		});
		for (auto & patchInit : itPatchInit->second)
		{
			if (!patchInit.second && patchInit.first->getDistance(worker) < 300)
			{
				patchWorkers[patchInit.first] = 1;
				return { patchInit.first, mineralBase };
			}
		}
		for (const auto & patch : itPatch->second)
		{
			if (!patch || !patch->exists()) continue;
			if (patchWorkers.find(patch) == patchWorkers.end())
			{
				patchWorkers[patch] = patch->isBeingGathered() ? 1 : 0;
			}
			else if (patch->isBeingGathered())
			{
				patchWorkers[patch] = 1;
			}
		}
		// find an unoverload mineral base
		if (itWorkers->second.size() <
			Config::Macro::WorkersPerMineralPatch * itPatch->second.size())
		{
			// find a most valuble patch
			for (const auto & patch : patchWorkers)
			{
				if (patch.second == 0)
				{
					if (worker->getDistance(patch.first->getPosition()) < minDist0)
					{
						bestPatch0 = patch.first;
						bestMineralBase0 = mineralBase;
						minDist0 = worker->getDistance(patch.first);
					}
				}
				else
				{
					if (worker->getDistance(patch.first->getPosition()) < minDist1)
					{
						bestPatch1 = patch.first;
						bestMineralBase1 = mineralBase;
						minDist1 = worker->getDistance(patch.first);
					}
				}
			}
		}
	}
	if ((bestPatch0 && minDist0 - minDist1 < 300) || !bestPatch1)
	{
		if (bestPatch0) patchWorkers[bestPatch0] = 1;
		return { bestPatch0, bestMineralBase0 };
	}
	else
	{
		if (bestPatch1) patchWorkers[bestPatch1] = 1;
		return { bestPatch1, bestMineralBase1 };
	}
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
						//BWAPI::Broodwar->drawTextScreen(200, 210, "\x04 size:  %d", mineralBases.size());
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

const BWAPI::Unitset & WorkerData::getMineralBases() const
{
	return mineralBases;
}

const BWAPI::Unitset & WorkerData::getMineralPatches(BWAPI::Unit base) const
{
	if (mineralBaseMineralPatchMap.find(base) != mineralBaseMineralPatchMap.end())
	{
		return mineralBaseMineralPatchMap.at(base);
	}
	else
	{
		return emptyset;
	}
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