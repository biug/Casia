#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVPZerglingRush : public ActionZergBase
	{
	public:
		ActionZVPZerglingRush();
		~ActionZVPZerglingRush() {}
		void init() override;
		bool canDeployAction() override;
		bool tick() override;
		void getBuildOrderList(CasiaBot::ProductionQueue &queue) override;
		void updateCurrentState(CasiaBot::ProductionQueue &queue) override;

	private:
		bool isFirstDroneInQueue = false;
	};
}