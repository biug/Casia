#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVPZealot : public ActionZergBase
	{
	public:
		ActionZVPZealot();
		~ActionZVPZealot() {}
		void init() override;
		bool canDeployAction() override;
		bool tick() override;
		void getBuildOrderList(CasiaBot::ProductionQueue &queue) override;
		void updateCurrentState(CasiaBot::ProductionQueue &queue) override;

	private:
		double enemyDragoonOverZealotRate = 0;
	};
}