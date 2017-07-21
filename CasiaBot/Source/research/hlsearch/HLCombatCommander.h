#pragma once
#include "Common.h"
#include "Squad.h"
#include "HLSearch.h"
#include "SquadData.h"
#include "CombatCommander.h"

namespace CasiaBot
{
	class HLCombatCommander
	{
		SquadData			squadData;
		CombatCommander		combatCommander;
	public:
		HLCombatCommander();
		~HLCombatCommander();
		void update(std::set<BWAPI::UnitInterface*> combatUnits);
		bool squadUpdateFrame() const;
	};

}