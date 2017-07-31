#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace CasiaBot
{
class OverlordManager : public MicroManager
{
protected:
	bool detect[3];
	bool initializeFlag = false;
	BWTA::BaseLocation *startBase[3];//0 最近的，1最远的，2剩下那个
public:

	OverlordManager();
	void executeMicro(const BWAPI::Unitset & targets);
	
	void executeMove(const SquadOrder & inputOrder);
};
}