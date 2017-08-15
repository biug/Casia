#pragma once

#include <Common.h>
#include "BuildOrderQueue.h"
#include "BuildingManager.h"
#include "StrategyManager.h"
#include "BuildOrder.h"
#include "ProductionQueue.h"

namespace CasiaBot
{
typedef unsigned char Action;

class ProductionManager
{
    ProductionManager();
    
    ProductionQueue				_queue;
	std::deque<ProductionItem>	_openingQueue;
    BWAPI::TilePosition _predictedTilePosition;
    bool                _enemyCloakedDetected;
    
    BWAPI::Unit         getClosestUnitToPosition(const BWAPI::Unitset & units,BWAPI::Position closestTo);
    BWAPI::Unit         selectUnitOfType(BWAPI::UnitType type,BWAPI::Position closestTo = BWAPI::Position(0,0));

    bool                hasResources(BWAPI::UnitType type);
    bool                canMake(BWAPI::UnitType type);
    bool                hasNumCompletedUnitType(BWAPI::UnitType type,int num);
    bool                meetsReservedResources(MetaType type);
	void				setOpenningBuildOrder(const BuildOrder & buildOrder);
	void				create(BWAPI::Unit producer, ProductionItem & unit);
    void                manageBuildOrderQueue();
	void				openingCheck();
    bool                canMakeNow(BWAPI::Unit producer,MetaType t);

    int                 getFreeMinerals();
    int                 getFreeGas();
    bool                canPlanBuildOrderNow() const;

public:

    static ProductionManager &	Instance();

    void        drawQueueInformation(std::map<BWAPI::UnitType,int> & numUnits,int x,int y,int index);
    void        update();
    void        onUnitMorph(BWAPI::Unit unit);
    void        onUnitDestroy(BWAPI::Unit unit);
    void        performBuildOrderSearch();
    void        drawProductionInformation(int x,int y);
    void        setSearchGoal(MetaPairVector & goal);
    void        queueGasSteal();
	void		queuePrint(int x, int y);

    BWAPI::Unit getProducer(MetaType t,BWAPI::Position closestTo = BWAPI::Positions::None);
};


class CompareWhenStarted
{

public:

    CompareWhenStarted() {}

    // the sorting operator
    bool operator() (BWAPI::Unit u1,BWAPI::Unit u2)
    {
        int startedU1 = BWAPI::Broodwar->getFrameCount() - (u1->getType().buildTime() - u1->getRemainingBuildTime());
        int startedU2 = BWAPI::Broodwar->getFrameCount() - (u2->getType().buildTime() - u2->getRemainingBuildTime());
        return startedU1 > startedU2;
    }
};
}