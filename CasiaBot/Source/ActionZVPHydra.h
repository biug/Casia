#pragma once

#include "ActionZergBase.h"
#include <deque>

namespace CasiaBot
{
	class ActionZVPHydra : public ActionZergBase
	{
	public:
		ActionZVPHydra			()											;
		~ActionZVPHydra			(){}
		void init				()									override;
		bool canDeployAction	()									override;
		bool tick				()									override;
		void getBuildOrderList	(CasiaBot::ProductionQueue &queue)	override;


	private:
		double enemyDragoonOverZealotRate	= 0;
		bool able_defend					= false;
		bool larva_lacking					= false;
		const int zerglingLimit				= 20;
		const int hydraliskLimit			= 6;
		const int lurkerLimit				= 15;
		const int creepColonyLimit			= 5;
		const int sunkenColonyLimit			= 5;
		const int spawningPoolLimit			= 1;
		const int extractorLimit			= 3;
		const int hydraliskDenLimit			= 1;
		const int hatcheryLimit				= 4;
		const int lairLimit					= 1;
		const int hiveLimit					= 1;
		const int queenNestLimit			= 1;
		const int metabolicBoostLimit		= 1;
		const int lurkerAspectLimit			= 1;
		const int adrenalGlandsLimit		= 1;

		int		  currentFrameCount			= 0;
		int		  gas						= 0;
		int		  minerals					= 0;
		int		  patches					= 0;
		bool	  mineralDequePositive		= 0;
		bool	  gasDequePositive			= 0;
		bool	  isSunkenColonyExist		= 0;
		bool	  isCreepColonyExist		= 0;
		int		  sunken_colony_total		= 0;
		int		  creep_colony_total		= 0;
		bool	  isHatcheryExist			= 0;
		bool	  isHiveExist				= 0;
		bool	  isQueenNestExist			= 0;
		bool	  isLairExist				= 0;
		bool	  isSpawningPoolExist		= 0;
		bool	  isExtractorExist			= 0;
		bool	  isHydraliskDenExist		= 0;

		void updateInformation	(CasiaBot::ProductionQueue &queue);
		void economy			(CasiaBot::ProductionQueue &queue, int  mode);
		void army				(CasiaBot::ProductionQueue &queue, int  mode);
		void tech				(CasiaBot::ProductionQueue &queue, int  mode);
		void staticDefend		(CasiaBot::ProductionQueue &queue);
	};
}