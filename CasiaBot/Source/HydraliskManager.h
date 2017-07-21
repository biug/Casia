#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace CasiaBot
{
class HydraliskManager : public MicroManager
{
public:

	HydraliskManager();
	void executeMicro(const BWAPI::Unitset & targets);

	BWAPI::Unit chooseTarget(BWAPI::Unit rangedUnit, const BWAPI::Unitset & targets, std::map<BWAPI::Unit, int> & numTargeting);
	BWAPI::Unit closestrangedUnit(BWAPI::Unit target, std::set<BWAPI::Unit> & rangedUnitsToAssign);
    std::pair<BWAPI::Unit, BWAPI::Unit> findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets);

	int getAttackPriority(BWAPI::Unit hydraliskUnit, BWAPI::Unit target);
	BWAPI::Unit getTarget(BWAPI::Unit hydraliskUnit, const BWAPI::Unitset & targets);

    void assignTargetsNew(const BWAPI::Unitset & targets);
    void assignTargetsOld(const BWAPI::Unitset & targets);
};
}