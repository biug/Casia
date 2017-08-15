#pragma once

#include "Common.h"
#include "MapGrid.h"
#include "SquadOrder.h"
#include "MapTools.h"
#include "InformationManager.h"
#include "Micro.h"

namespace CasiaBot
{
struct AirThreat
{
	BWAPI::Unit	unit;
	double			weight;
};

struct GroundThreat
{
	BWAPI::Unit	unit;
	double			weight;
};

class MicroManager
{

protected:

	BWAPI::Unitset		_units;
	SquadOrder			order;

	virtual void        executeMicro(const BWAPI::Unitset & targets) = 0;
	bool                checkPositionWalkable(BWAPI::Position pos);
	void                drawOrderText();
	bool                unitNearEnemy(BWAPI::Unit unit);
    

public:
						MicroManager();
    virtual				~MicroManager(){}

	const BWAPI::Unitset & getUnits() const;
	BWAPI::Position     calcCenter() const;

	void				setUnits(const BWAPI::Unitset & u);
	virtual void		execute(const SquadOrder & order);
	void				regroup(const BWAPI::Position & regroupPosition) const;

};
}