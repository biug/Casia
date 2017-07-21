#pragma once

#include "Common.h"

namespace CasiaBot
{
class WorkerMoveData
{
public:

	int mineralsNeeded;
	int gasNeeded;
	BWAPI::Position position;

	WorkerMoveData(int m, int g, BWAPI::Position p)
        : mineralsNeeded(m)
        , gasNeeded(g)
        , position(p)
	{
		
	}

	WorkerMoveData() {}
};

class WorkerData 
{

public:

	enum WorkerJob {Minerals, Gas, Build, Combat, Idle, Repair, Move, Scout, Default};

private:

	BWAPI::Unitset workers;
	BWAPI::Unitset mineralBases;
	BWAPI::Unitset refineries;

	std::hash_map<BWAPI::Unit, enum WorkerJob>				workerJobMap;
	std::hash_map<BWAPI::Unit, BWAPI::Unit>					workerRepairMap;
	std::hash_map<BWAPI::Unit, WorkerMoveData>				workerMoveMap;
	std::hash_map<BWAPI::Unit, BWAPI::UnitType>				workerBuildingTypeMap;

	std::hash_map<BWAPI::Unit, BWAPI::Unit>					workerMineralBaseMap;
	std::hash_map<BWAPI::Unit, std::hash_set<BWAPI::Unit>>	mineralBaseWorkersMap;
	std::hash_map<BWAPI::Unit, std::hash_set<BWAPI::Unit>>	mineralBaseMineralPatchMap;

	std::hash_map<BWAPI::Unit, BWAPI::Unit>					workerMineralPatchMap;
	std::hash_map<BWAPI::Unit, std::hash_set<BWAPI::Unit>>	mineralPatchWorkersMap;
	std::hash_map<BWAPI::Unit, BWAPI::Unit>					mineralPatchMineralBaseMap;
	std::hash_map<BWAPI::Unit, int>							mineralPatchWorkersCountMap;

	std::hash_map<BWAPI::Unit, BWAPI::Unit>					workerRefineryMap;
	std::hash_map<BWAPI::Unit, std::hash_set<BWAPI::Unit>>	refineryWorkersMap;
	std::hash_map<BWAPI::Unit, int>							refineryWorkersCountMap;

	const int		mineralPatchRadius = 480;
	const int		sameMineralBaseRadius = 480;

	void clearPreviousJob(BWAPI::Unit unit);

public:

	WorkerData();

	void					checkResources();
	void					workerDestroyed(BWAPI::Unit worker);
	void					addMineralBase(BWAPI::Unit mineralBase);
	void					addRefinery(BWAPI::Unit refinery);
	void					removeMineralBase(BWAPI::Unit mineralBase);
	void					removeMineralPatch(BWAPI::Unit mineralPatch);
	void					removeRefinery(BWAPI::Unit refinery);
	void					addWorker(BWAPI::Unit unit);
	void					addGatheringWorker(BWAPI::Unit worker, WorkerJob job);
	void					addBuildingWorker(BWAPI::Unit worker, BWAPI::UnitType buildingType);
	void					setWorkerIdle(BWAPI::Unit);
	void					setWorkerGatheringMineral(BWAPI::Unit worker);
	void					setWorkerGatheringGas(BWAPI::Unit worker);
	void					setWorkerBuilding(BWAPI::Unit worker, BWAPI::UnitType bulidingType);
	void					setWorkerMoving(BWAPI::Unit worker, WorkerMoveData wmd);
	void					setWorkerRepairing(BWAPI::Unit worker, BWAPI::Unit building);
	void					setWorkerScouting(BWAPI::Unit worker);
	void					setWorkerCombating(BWAPI::Unit worker);

	int						getNumWorkers() const;
	int						getNumMineralWorkers() const;
	int						getNumGasWorkers() const;
	int						getNumIdleWorkers() const;
	int						getNumMineralPatchWorkers(BWAPI::Unit mineralPatch) const;
	int						getNumRefineryWorkers(BWAPI::Unit refinery) const;
	char					getJobCode(BWAPI::Unit unit) const;
	std::string				getJobString(BWAPI::Unit unit) const;
	
	bool					isMineralBase(BWAPI::Unit base) const;
	bool					isRefinery(BWAPI::Unit refinery) const;
	bool					isWorkerInOverloadMineralPatch(BWAPI::Unit unit) const;

	std::pair<BWAPI::Unit, BWAPI::Unit>				getClosestMineral(BWAPI::Unit worker) const;
	BWAPI::Unit										getClosestRefinery(BWAPI::Unit worker) const;
	BWAPI::Unit										getLarvaDepot() const;

	enum WorkerJob	getWorkerJob(BWAPI::Unit unit) const;
	BWAPI::Unit		getWorkerMineralBase(BWAPI::Unit unit) const;
	BWAPI::Unit		getWorkerMineralPatch(BWAPI::Unit unit) const;
	BWAPI::Unit		getWorkerRefinery(BWAPI::Unit unit) const;
	BWAPI::Unit		getRefineryWorker(BWAPI::Unit unit) const;
	BWAPI::Unit		getWorkerRepairUnit(BWAPI::Unit unit) const;
	BWAPI::UnitType	getWorkerBuildingType(BWAPI::Unit unit) const;
	WorkerMoveData	getWorkerMoveData(BWAPI::Unit unit) const;

	void					drawWorkerDebugInfo() const;
	void					drawResourceDebugInfo(int x, int y) const;

	const BWAPI::Unitset & getWorkers() const { return workers; }

};
}