#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVZZerglingLurker : public ActionZergBase
	{
	public:
		ActionZVZZerglingLurker();
		~ActionZVZZerglingLurker() {}
		void init() override;
		bool canDeployAction() override;
		bool tick() override;
		void getBuildOrderList(CasiaBot::ProductionQueue &queue) override;

	private:
	};
}