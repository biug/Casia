#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVTFactoriesUnits : public ActionZergBase
	{
	public:
		ActionZVTFactoriesUnits();
		~ActionZVTFactoriesUnits() {}
		void init() override;
		bool canDeployAction() override;
		bool tick() override;
		void getBuildOrderList(CasiaBot::ProductionQueue &queue) override;
		void updateCurrentState(CasiaBot::ProductionQueue &queue) override;

	protected:
		void tryAddInQueue(CasiaBot::ProductionQueue &queue, const CasiaBot::ProductionItem & item, bool priority = false) override;

	private:
		int enemyTerranBarrackUnitsAmount = 0;
		int enemyTerranFactoryUnitsAmount = 0;
		double enemyTerranMechanizationRate = 0;
		const double escalationMark = 240;	// 敌方人口达到此值时开始造雷兽
	};
}