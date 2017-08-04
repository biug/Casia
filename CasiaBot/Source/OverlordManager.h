#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace CasiaBot
{
class OverlordManager : public MicroManager
{
protected:
	bool needDetect[3];
	bool initializeFlag;
	BWTA::BaseLocation *startBase[3];
public:

	OverlordManager();
	void executeMicro(const BWAPI::Unitset & targets);
	void initEnemyBase();
	void executeMove(const SquadOrder & inputOrder);
};
}