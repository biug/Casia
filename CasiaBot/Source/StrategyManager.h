#pragma once

#include "Common.h"
#include "BWTA.h"
#include "BuildOrderQueue.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "BuildOrder.h"
#include "ProductionQueue.h"
#include "ActionZVTBarracksUnits.h"
#include "ActionZVTFactoriesUnits.h"
#include "ActionZVZZerglingLurker.h"
#include "ActionZVZZerglingMutalisk.h"
#include "ActionZVPZealot.h"
#include "ActionZVPDragoon.h"
#include "ActionZVPZerglingRush.h"
#include "ActionZVPHydra.h"

namespace CasiaBot
{
	using namespace CasiaBot;
typedef std::pair<MetaType, size_t> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;

struct Opening
{
    std::string _name;
    BWAPI::Race _race;
    int         _wins;
    int         _losses;
    BuildOrder  _buildOrder;

	Opening()
        : _name("None")
        , _race(BWAPI::Races::None)
        , _wins(0)
        , _losses(0)
    {
    
    }

	Opening(const std::string & name, const BWAPI::Race & race, const BuildOrder & buildOrder)
        : _name(name)
        , _race(race)
        , _wins(0)
        , _losses(0)
        , _buildOrder(buildOrder)
    {
    
    }
};

class StrategyManager 
{
	StrategyManager();

	BWAPI::Race					    _selfRace;
	BWAPI::Race					    _enemyRace;
    std::map<std::string, Opening>	_openings;
    int                             _totalGamesPlayed;
	const BuildOrder                _emptyBuildOrder;
	ActionZergBase*					_action;
	int								_lastChangeFrame;
	ActionZVTBarracksUnits			_actionZVTBarracks;
	ActionZVTFactoriesUnits			_actionZVTFactories;
	ActionZVZZerglingLurker			_actionZVZLurker;
	ActionZVZZerglingMutalisk		_actionZVZMutalisk;
	ActionZVPZealot					_actionZVPZealot;
	ActionZVPDragoon				_actionZVPDragoon;
	ActionZVPZerglingRush			_actionZVPZerglingRush;
	ActionZVPHydra					_actionZVPHydra;

	        void	                writeResults();
	const	int					    getScore(BWAPI::Player player) const;
	const	double				    getUCBValue(const size_t & strategy) const;
	const	bool				    shouldExpandNow() const;

public:
    
	static	StrategyManager &	    Instance();

			void				    onEnd(const bool isWinner);
            void                    addOpening(const std::string & name, Opening & strategy);
            void                    setLearnedOpening();
            void	                readOpeningResults();
	const	bool				    regroup(int numInRadius);
	const	bool				    rushDetected();
	const	int				        defendWithWorkers();
	const	BuildOrder &            getOpeningBookBuildOrder() const;
	void					updateProductionQueue(ProductionQueue & queue);
};
}