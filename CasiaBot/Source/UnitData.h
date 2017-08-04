#pragma once

#include "Common.h"
#include "BWTA.h"

namespace CasiaBot
{
struct UnitInfo
{
    // we need to store all of this data because if the unit is not visible, we
    // can't reference it from the unit pointer

    int             unitID;
    int             lastHealth;
    int             lastShields;
    BWAPI::Player   player;
    BWAPI::Unit     unit;
    BWAPI::Position lastPosition;
    BWAPI::UnitType type;
    bool            completed;

    UnitInfo()
        : unitID(0)
        , lastHealth(0)
        , player(nullptr)
        , unit(nullptr)
        , lastPosition(BWAPI::Positions::None)
        , type(BWAPI::UnitTypes::None)
        , completed(false)
    {

    }

    const bool operator == (BWAPI::Unit unit) const
    {
        return unitID == unit->getID();
    }

    const bool operator == (const UnitInfo & rhs) const
    {
        return (unitID == rhs.unitID);
    }

    const bool operator < (const UnitInfo & rhs) const
    {
        return (unitID < rhs.unitID);
    }
};

typedef std::vector<UnitInfo> UnitInfoVector;
typedef std::map<BWAPI::Unit,UnitInfo> UIMap;

class UnitData
{
    UIMap unitMap;

    const bool badUnitInfo(const UnitInfo & ui) const;

    std::vector<int>						numDeadUnits;
    std::vector<int>						numConstructedUnits;
	std::vector<int>						numConstructingUnits;
	std::vector<BWAPI::Unitset>				unitsets;
	std::map<BWAPI::Unit, std::set<BWAPI::Unit>>	baseLarva;

    int										mineralsLost;
    int										gasLost;

	void	updateSelfZerg(BWAPI::Unit unit);
	void	updateEnemy(BWAPI::Unit unit);

public:

    UnitData();

	void	clearUnitsets();
    void	updateUnit(BWAPI::Unit unit);
    void	removeUnit(BWAPI::Unit unit);
    void	removeBadUnits();

    int		getGasLost()                                const;
    int		getMineralsLost()                           const;
    int		getNumUnits(BWAPI::UnitType t)              const;
	int		getNumConstructingUnits(BWAPI::UnitType t)	const;
	int		getNumConstructedUnits(BWAPI::UnitType t)	const;
    int		getNumDeadUnits(BWAPI::UnitType t)          const;
    const	std::map<BWAPI::Unit,UnitInfo> & getUnits() const;
	const	BWAPI::Unitset & getUnitset(BWAPI::UnitType t);
};
}