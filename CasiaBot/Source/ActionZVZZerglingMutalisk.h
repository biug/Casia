#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVZZerglingMutalisk : public ActionZergBase
	{
	public:
		ActionZVZZerglingMutalisk();
		~ActionZVZZerglingMutalisk() {}
		void init() override;
		bool canDeployAction() override;
		bool tick() override;
		void getBuildOrderList(CasiaBot::ProductionQueue &queue) override;
		void updateCurrentState(CasiaBot::ProductionQueue &queue) override;

	private:
	};
}