#pragma once

#include "Common.h"
#include "BWTA.h"

#include "UnitData.h"

#include "..\..\SparCraft\source\SparCraft.h"

namespace CasiaBot
{
struct BaseInfo;
typedef std::vector<BaseInfo> BaseInfoVector;

class InformationManager 
{
    InformationManager();
    
    BWAPI::Player       _self;
    BWAPI::Player       _enemy;

    std::map<BWAPI::Player, UnitData>                   _unitData;
    std::map<BWAPI::Player, BWTA::BaseLocation *>       _mainBaseLocations;
    std::map<BWAPI::Player, std::set<BWTA::Region *> >  _occupiedRegions;

	bool					_isEncounterRush;

    int                     getIndex(BWAPI::Player player) const;

    void                    updateUnit(BWAPI::Unit unit);
    void                    initializeRegionInformation();
    void                    initializeBaseInfoVector();
    void                    updateUnitInfo();
    void                    updateBaseLocationInfo();
    void                    updateOccupiedRegions(BWTA::Region * region,BWAPI::Player player);
    bool                    isValidUnit(BWAPI::Unit unit);
	void					updateRush();

	bool					beingMarineRushed();
	bool					beingZerglingRushed();
	bool					beingZealotRushed();

	std::string				formatSelfInfo(BWAPI::UnitType type);
	std::string				formatEnemyInfo(BWAPI::UnitType type);

public:

    // yay for singletons!
    static InformationManager & Instance();

    void                    update();

    // event driven stuff
    void					onUnitShow(BWAPI::Unit unit)        { updateUnit(unit); }
    void					onUnitHide(BWAPI::Unit unit)        { updateUnit(unit); }
    void					onUnitCreate(BWAPI::Unit unit)		{ updateUnit(unit); }
    void					onUnitComplete(BWAPI::Unit unit)    { updateUnit(unit); }
    void					onUnitMorph(BWAPI::Unit unit)       { updateUnit(unit); }
    void					onUnitRenegade(BWAPI::Unit unit)    { updateUnit(unit); }
    void					onUnitDestroy(BWAPI::Unit unit);

    bool					isEnemyBuildingInRegion(BWTA::Region * region);
    int						getNumUnits(BWAPI::UnitType type,BWAPI::Player player);
	int						getNumConstructedUnits(BWAPI::UnitType type, BWAPI::Player player);
	int						getNumConstructingUnits(BWAPI::UnitType type, BWAPI::Player player);
    bool					nearbyForceHasCloaked(BWAPI::Position p,BWAPI::Player player,int radius);
    bool					isCombatUnit(BWAPI::UnitType type) const;

    void                    getNearbyForce(std::vector<UnitInfo> & unitInfo,BWAPI::Position p,BWAPI::Player player,int radius);

    const UIMap &           getUnitInfo(BWAPI::Player player) const;

    std::set<BWTA::Region *> &  getOccupiedRegions(BWAPI::Player player);
    BWTA::BaseLocation *    getMainBaseLocation(BWAPI::Player player);
	BWAPI::Position			getLastPosition(BWAPI::Unit unit, BWAPI::Player player) const;

    bool                    enemyHasCloakedUnits();

    void                    drawExtendedInterface();
    void                    drawUnitInformation(int x,int y);
    void                    drawMapInformation();
	void					PrintInfo(int x, int y);

	bool					isEncounterRush();
	bool					checkBuildingLocation(const BWAPI::TilePosition & tp);

    const UnitData &        getUnitData(BWAPI::Player player) const;
	const std::set<BWAPI::Unit> & getUnitset(const std::string & name);
};
}
