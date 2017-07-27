#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVTBarracksUnits : public ActionZergBase
	{
	public:
		ActionZVTBarracksUnits();
		~ActionZVTBarracksUnits() {}
		void init() override;
		bool canDeployAction() override;
		bool tick() override;
		void getBuildOrderList(CasiaBot::ProductionQueue &queue) override;
		void updateCurrentState(CasiaBot::ProductionQueue &queue) override;
		bool beingRush();

	protected:
		void tryAddInQueue(CasiaBot::ProductionQueue &queue, const CasiaBot::ProductionItem & item, bool priority = false) override;

	private:
		int enemyTerranBarrackUnitsAmount = 0;
		int enemyTerranFactoryUnitsAmount = 0;
		double enemyTerranMechanizationRate = 0;
	};
}