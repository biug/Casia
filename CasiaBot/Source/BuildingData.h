#pragma once

#include "Common.h"

namespace CasiaBot
{

namespace BuildingStatus
{
    enum { Unassigned = 0, Assigned, UnderConstruction, Size };
}

class Building 
{     
public:
      
	BWAPI::TilePosition     desiredPosition;
	BWAPI::TilePosition     finalPosition;
	BWAPI::UnitType         type;
	BWAPI::Unit             buildingUnit;
	BWAPI::Unit             builderUnit;
    size_t                  status;
	int                     lastOrderFrame;
    bool                    isGasSteal;
	bool					nexpHatchery;
	bool                    buildCommandGiven;
	bool					beCanceled;

	Building() 
		: desiredPosition   (BWAPI::TilePositions::None)
        , finalPosition     (BWAPI::TilePositions::None)
        , type              (BWAPI::UnitTypes::Unknown)
        , buildingUnit      (nullptr)
        , builderUnit       (nullptr)
        , lastOrderFrame    (0)
        , status            (BuildingStatus::Unassigned)
        , buildCommandGiven (false)
        , isGasSteal        (false)
		, nexpHatchery		(false)
		, beCanceled		(false)
    {} 

	// constructor we use most often
	Building(BWAPI::UnitType t, BWAPI::TilePosition desired)
		: desiredPosition   (desired)
        , finalPosition     (BWAPI::TilePositions::None)
        , type              (t)
        , buildingUnit      (nullptr)
        , builderUnit       (nullptr)
        , lastOrderFrame    (0)
        , status            (BuildingStatus::Unassigned)
        , buildCommandGiven (false)
        , isGasSteal        (false)
		, nexpHatchery		(false)
		, beCanceled		(false)
    {}

	// equals operator
	bool operator==(const Building & b) 
    {
		// buildings are equal if their worker unit or building unit are equal
		return (b.buildingUnit == buildingUnit) || (b.builderUnit == builderUnit);
	}
};

class BuildingData 
{
    std::vector<Building>                   _buildings;

public:

	BuildingData();
	
    std::vector<Building> & getBuildings();

	void        addBuilding(const Building & b);
	void        removeBuilding(const Building & b);
    void        removeBuildings(const std::vector<Building> & buildings);
	bool        isBeingBuilt(BWAPI::UnitType type);
};
}