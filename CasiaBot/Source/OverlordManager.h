#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace CasiaBot
{
class OverlordManager : public MicroManager
{
public:

	OverlordManager();
	void executeMicro(const BWAPI::Unitset & targets);
	void executeMove(const SquadOrder & inputOrder);
};
}