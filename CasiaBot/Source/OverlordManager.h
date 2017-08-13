#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace CasiaBot
{
class OverlordManager : public MicroManager
{
protected:
	bool needDetect[3];
	BWAPI::TilePosition startBase[3];
	bool initializeFlag;
public:

	OverlordManager();
	void executeMicro(const BWAPI::Unitset & targets);
	void initEnemyBase();
	void executeMove(const SquadOrder & inputOrder);
};
}