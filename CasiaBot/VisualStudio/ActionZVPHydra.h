#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVPHydra : public ActionZergBase
	{
	public:
		ActionZVPHydra();
		~ActionZVPHydra() {}
		void init() override;
		bool canDeployAction() override;
		bool tick() override;
		void getBuildOrderList(CasiaBot::ProductionQueue &queue) override;
		void updateCurrentState(CasiaBot::ProductionQueue &queue) override;

	protected:
		void tryAddInQueue(CasiaBot::ProductionQueue &queue, const CasiaBot::ProductionItem & item, bool priority = false) override;

	private:
		double enemyDragoonOverZealotRate = 0;
		int droneLimit = 16;
		const int zerglingLimit = 20;
		const int mutaliskLimit = 15;
		const int creepColonyLimit = 7;
		const int sunkenColonyLimit = 7;
		const int spawningPoolLimit = 1;
		const int extractorLimit = 3;
		const int spireLimit = 1;
		const int hatcheryLimit = 4;
		const int lairLimit = 1;
		const int hiveLimit = 1;
		const int queenNestLimit = 1;
		const int metabolicBoostLimit = 1;
		const int adrenalGlandsLimit = 1;
	};
}#pragma once
