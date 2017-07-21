#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVPDragoon : public ActionZergBase
	{
	public:
		ActionZVPDragoon();
		~ActionZVPDragoon() {}
		void init() override;
		bool canDeployAction() override;
		bool tick() override;
		void getBuildOrderList(CasiaBot::ProductionQueue &queue) override;
		void updateCurrentState(CasiaBot::ProductionQueue &queue) override;

	private:
		double enemyDragoonOverZealotRate = 0;
	};
}