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

	protected:
		void tryAddInQueue(CasiaBot::ProductionQueue &queue, const CasiaBot::ProductionItem & item, bool priority = false) override;

	private:
		bool thirdBase;
		int enemyTerranBarrackUnitsAmount;
		int enemyTerranFactoryUnitsAmount;
		double enemyTerranMechanizationRate;
	};
}