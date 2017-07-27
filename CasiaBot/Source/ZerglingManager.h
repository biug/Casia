#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace CasiaBot
{
class ZerglingManager : public MicroManager
{
public:

	ZerglingManager();
	void execute(const SquadOrder & inputOrder) override;
	void executeMicro(const BWAPI::Unitset & targets);

	BWAPI::Unit chooseTarget(BWAPI::Unit zerglingUnit, const BWAPI::Unitset & targets, std::map<BWAPI::Unit, int> & numTargeting);
	BWAPI::Unit closestzerglingUnit(BWAPI::Unit target, const BWAPI::Unitset & zerglingUnitToAssign);
	int getAttackPriority(BWAPI::Unit zerglingUnit, BWAPI::Unit target);
	BWAPI::Unit getTarget(BWAPI::Unit zerglingUnit, const BWAPI::Unitset & targets);
    bool zerglingUnitShouldRetreat(BWAPI::Unit zerglingUnit, const BWAPI::Unitset & targets);
    std::pair<BWAPI::Unit, BWAPI::Unit> findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets);

    void assignTargetsNew(const BWAPI::Unitset & targets);
    void assignTargetsOld(const BWAPI::Unitset & targets);
};
}