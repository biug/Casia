#pragma once

#include "Common.h"

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

    std::hash_map<BWAPI::Player, UnitData>						_unitData;
	std::vector<BWAPI::Unit>									_selfBases;
	std::vector<UnitInfo>										_enemyBaseInfos;
	std::vector<BWAPI::TilePosition>							_baseTiles;
	std::map<BWAPI::TilePosition, const BWEM::Area *>			_baseAreas;
	std::map<std::pair<BWAPI::TilePosition, BWAPI::TilePosition> , std::vector<BWAPI::Position>> _basePaths;
	std::vector<BWAPI::Position>								_emptyPath;

	bool					_isEncounterRush;
	bool					_isBasePathFound;

    void                    updateUnit(BWAPI::Unit unit);
    void                    initializeRegionInformation();
    void                    updateUnitInfo();
    void                    updateBaseLocationInfo();
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

    int						getNumUnits(BWAPI::UnitType type,BWAPI::Player player);
	int						getNumConstructedUnits(BWAPI::UnitType type, BWAPI::Player player);
	int						getNumConstructingUnits(BWAPI::UnitType type, BWAPI::Player player);
    bool					isCombatUnit(BWAPI::UnitType type) const;

    void                    getNearbyForce(std::vector<UnitInfo> & unitInfo,BWAPI::Position p,BWAPI::Player player,int radius);

    const UIMap &           getUnitInfo(BWAPI::Player player) const;

    const std::vector<BWAPI::Unit> &			getSelfBases() const;
	const std::vector<UnitInfo> &				getEnemyBaseInfos() const;
	const std::vector<BWAPI::TilePosition> &	getBaseTiles() const;
	BWAPI::Position	getLastPosition(BWAPI::Unit unit, BWAPI::Player player) const;
	const BWEM::Area *					getBaseArea(BWAPI::TilePosition base) const;
	const std::vector<BWAPI::Position>& getBasePath(BWAPI::TilePosition base1, BWAPI::TilePosition base2, int * length) const;

    bool                    enemyHasCloakedUnits();

    void                    drawExtendedInterface();
    void                    drawUnitInformation(int x,int y);
	void					PrintInfo(int x, int y);

	bool					isEncounterRush();
	bool					checkBuildingLocation(const BWAPI::TilePosition & tp);

    const UnitData &        getUnitData(BWAPI::Player player) const;
	const BWAPI::Unitset &	getUnitset(BWAPI::UnitType t);
};
}
