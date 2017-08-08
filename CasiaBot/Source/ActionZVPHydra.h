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
		void getBuildOrderListNew(CasiaBot::ProductionQueue &queue);

	protected:
		void tryAddInQueue(CasiaBot::ProductionQueue &queue, const CasiaBot::ProductionItem & item, bool priority = false) override;

	private:
		double enemyDragoonOverZealotRate = 0;
		bool able_defend = false;
		int droneLimit = 16;
		const int zerglingLimit = 20;
		const int hydraliskLimit = 6;
		const int lurkerLimit = 15;
		const int creepColonyLimit = 7;
		const int sunkenColonyLimit = 7;
		const int spawningPoolLimit = 1;
		const int extractorLimit = 3;
		const int hydraliskDenLimit = 1;
		const int hatcheryLimit = 4;
		const int lairLimit = 1;
		const int hiveLimit = 1;
		const int queenNestLimit = 1;
		const int metabolicBoostLimit = 1;
		const int lurkerAspectLimit = 1;
		const int adrenalGlandsLimit = 1;
	};
}