#pragma once

#include <Common.h>
#include "BuildingManager.h"
#include "WorkerData.h"

namespace CasiaBot
{
class Building;

class WorkerManager
{
    WorkerData  workerData;
	BWAPI::Unit previousClosestWorker;
	bool		gasNotUsed;
	int			gasNeed;

	std::deque<int>										gasUsed;
	std::map<BWAPI::Unit, std::deque<BWAPI::Position>>	workersPos;

    void        setWorkerGatheringMineral(BWAPI::Unit unit);
	void		setWorkerGatheringGas(BWAPI::Unit unit);
	bool		isWorkerIdle(BWAPI::Unit unit);
    bool        isGasStealRefinery(BWAPI::Unit unit);
    
    void        handleIdleWorkers();
    void        handleGasWorkers();
	void		handleMineralWorkers();
    void        handleMoveWorkers();
    void        handleCombatWorkers();

    WorkerManager();

public:

    void        update();
    void        onUnitDestroy(BWAPI::Unit unit);
    void        onUnitMorph(BWAPI::Unit unit);
    void        onUnitShow(BWAPI::Unit unit);
    void        onUnitRenegade(BWAPI::Unit unit);
    void        finishedWithWorker(BWAPI::Unit unit);

    void        finishedWithCombatWorkers();

    void        drawResourceDebugInfo(int x, int y);
    void        updateWorkerStatus();
	void		updateResourceStatus();
    void        drawWorkerDebugInfo();

    int         getNumMineralWorkers();
    int         getNumGasWorkers();
    int         getNumIdleWorkers();
	int			getNumMineralPatches();
	int			getMaxNumWorkers();
    void        setScoutWorker(BWAPI::Unit worker);

    bool        isWorkerScout(BWAPI::Unit worker);
    bool        isFree(BWAPI::Unit worker);
    bool        isBuilder(BWAPI::Unit worker);

    BWAPI::Unit getBuilder(Building & b,bool setJobAsBuilder = false);
    BWAPI::Unit getMoveWorker(BWAPI::Position p);
	BWAPI::Unit getClosestResource(BWAPI::Unit worker, BWAPI::Unitset & poses);
    BWAPI::Unit getGasWorker(BWAPI::Unit refinery);
    BWAPI::Unit getClosestEnemyUnit(BWAPI::Unit worker);
    BWAPI::Unit getClosestMineralWorkerTo(BWAPI::Unit enemyUnit);
    BWAPI::Unit getWorkerScout();
	BWAPI::Unit getLarvaDepot();
	const BWAPI::Unitset & getRefineries() const;
	const BWAPI::Unitset & getMineralBases() const;
	const BWAPI::Unitset & getMineralPatches(BWAPI::Unit base) const;
	void addCanceledRefineryLocation(BWAPI::TilePosition position);
	const std::set<BWAPI::TilePosition> & getCanceledRefineryLocations() const;

	void        setWorkerBuilding(BWAPI::Unit worker, Building & b);
    void        setWorkerRepairing(BWAPI::Unit worker,BWAPI::Unit unitToRepair);
    void        stopRepairing(BWAPI::Unit worker);
    void        setWorkerMoving(int m,int g,BWAPI::Position p);
    void        setWorkerCombating(BWAPI::Unit worker);

    bool        willHaveResources(int mineralsRequired,int gasRequired,double distance);
    void        rebalanceMineralWorkers();

    static WorkerManager &  Instance();
};
}