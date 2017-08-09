#include "Common.h"
#include "StrategyManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

// constructor
StrategyManager::StrategyManager()
	: _selfRace(BWAPI::Broodwar->self()->getRace())
	, _enemyRace(BWAPI::Broodwar->enemy()->getRace())
	, _emptyBuildOrder(BWAPI::Broodwar->self()->getRace())
	, _action(nullptr)
	, _lastChangeFrame(0)
{
	_actionZVTBarracks.init();
	_actionZVTFactories.init();
	_actionZVZLurker.init();
	_actionZVZMutalisk.init();
	_actionZVPZealot.init();
	_actionZVPDragoon.init();
	_actionZVPZerglingRush.init();
	_actionZVPHydra.init();
}

// get an instance of this
StrategyManager & StrategyManager::Instance() 
{
	static StrategyManager instance;
	return instance;
}

const int StrategyManager::getScore(BWAPI::Player player) const
{
	return player->getBuildingScore() + player->getKillScore() + player->getRazingScore() + player->getUnitScore();
}

const BuildOrder & StrategyManager::getOpeningBookBuildOrder() const
{
    auto buildOrderIt = _openings.find(Config::Opening::OpeningName);

    // look for the build order in the build order map
	if (buildOrderIt != std::end(_openings))
    {
        return (*buildOrderIt).second._buildOrder;
    }
    else
    {
        CAB_ASSERT_WARNING(false, "Opening not found: %s, returning empty initial build order", Config::Opening::OpeningName.c_str());
        return _emptyBuildOrder;
    }
}

const bool StrategyManager::shouldExpandNow() const
{
	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
        BWAPI::Broodwar->printf("No valid expansion location");
		return false;
	}

	size_t numDepots    = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hatchery)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Lair)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hive);
	int frame           = BWAPI::Broodwar->getFrameCount();
    int minute          = frame / (24*60);

	// if we have a ton of idle workers then we need a new expansion
	if (WorkerManager::Instance().getNumIdleWorkers() > 10)
	{
		return true;
	}

    // if we have a ridiculous stockpile of minerals, expand
    if (BWAPI::Broodwar->self()->minerals() > 3000)
    {
        return true;
    }

    // we will make expansion N after array[N] minutes have passed
    std::vector<int> expansionTimes = {5, 10, 20, 30, 40 , 50};

    for (size_t i(0); i < expansionTimes.size(); ++i)
    {
        if (numDepots < (i+2) && minute > expansionTimes[i])
        {
            return true;
        }
    }

	return false;
}

void StrategyManager::addOpening(const std::string & name, Opening & opening)
{
	_openings[name] = opening;
}

void StrategyManager::updateProductionQueue(ProductionQueue & queue)
{
	/*
	int currentFrame = BWAPI::Broodwar->getFrameCount();
	if (_enemyRace == BWAPI::Races::Terran) {
		_actionZVTBarracks.updateCurrentState(queue);
		_actionZVTFactories.updateCurrentState(queue);

		// need to be update
		if (_action == nullptr)
		{
			if (queue.empty())
				_action = &_actionZVTBarracks;
		}
		else if (currentFrame - _lastChangeFrame >= 1000 || queue.empty())
		{
			_lastChangeFrame = currentFrame;
			if (_action->tick())
			{
				queue.clear();
				bool useBarracks = _actionZVTBarracks.canDeployAction();
				bool useFactories = _actionZVTFactories.canDeployAction();
				_action = &_actionZVTBarracks;
				if (useBarracks)
				{
					_action = &_actionZVTBarracks;
				}
				else if (useFactories)
				{
					_action = &_actionZVTFactories;
				}
			}
		}
		if (_action != nullptr)
			_action->getBuildOrderList(queue);
	}
	else if (_enemyRace == BWAPI::Races::Zerg) {
		_actionZVZLurker.updateCurrentState(queue);
		_actionZVZMutalisk.updateCurrentState(queue);

		// to do
		if (_action == nullptr) {
			if (queue.empty())
				_action = &_actionZVZLurker;
		}
		else if (currentFrame - _lastChangeFrame >= 1000 || queue.empty()) {
			_lastChangeFrame = currentFrame;
			if (_action->tick()) {
				queue.clear();
				bool useLurker = _actionZVZLurker.canDeployAction();
				bool useMutalisk = _actionZVZMutalisk.canDeployAction();
				_action = &_actionZVZLurker;
				if (useLurker) {
					_action = &_actionZVZLurker;
				}
				else if (useMutalisk) {
					_action = &_actionZVZMutalisk;
				}
			}
		}
		if (_action != nullptr)
			_action->getBuildOrderList(queue);
	}
	else if (_enemyRace == BWAPI::Races::Protoss) {
		_actionZVPZealot.updateCurrentState(queue);
		_actionZVPDragoon.updateCurrentState(queue);
		_actionZVPZerglingRush.updateCurrentState(queue);
		_actionZVPHydra.updateCurrentState(queue);

		//to do
		if (_action == nullptr) {
			if (queue.empty())
				_action = &_actionZVPZealot;
			
			//queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
		else if (currentFrame - _lastChangeFrame >= 1000 || queue.empty()) {
			_lastChangeFrame = currentFrame;
			if (_action->tick()) {
				queue.clear();
				bool useZealot = _actionZVPZealot.canDeployAction();
				bool useDragoon = _actionZVPDragoon.canDeployAction();
				//bool useZergling = _actionZVPZerglingRush.canDeployAction();
				bool useZergling = false;
				_action = &_actionZVPZealot;
				if (useZealot) {
					_action = &_actionZVPZealot;
				}
				else if (useDragoon) {
					_action = &_actionZVPDragoon;
				}
				else if (useZergling) {
					_action = &_actionZVPZerglingRush;
				}
				
			}
		}
		if (_action != nullptr)
			_action->getBuildOrderList(queue);
	}
	else {
		_actionZVZMutalisk.updateCurrentState(queue);
		_actionZVPZerglingRush.updateCurrentState(queue);
		if (_action == nullptr)
		{
			if (queue.empty())
				_action = &_actionZVPZerglingRush;
		}
		if (_action != nullptr)
			_action->getBuildOrderList(queue);
	}
	*/
	_actionZVPHydra.updateCurrentState(queue);
	_action = &_actionZVPHydra;
	_action->getBuildOrderList(queue);	
}

void StrategyManager::readOpeningResults()
{
    if (!Config::Modules::UsingOpeningIO)
    {
        return;
    }

    std::string enemyName = BWAPI::Broodwar->enemy()->getName();
    std::replace(enemyName.begin(), enemyName.end(), ' ', '_');

    std::string enemyResultsFile = Config::Opening::ReadDir + enemyName + ".txt";
    
    std::string strategyName;
    int wins = 0;
    int losses = 0;

    FILE *file = fopen ( enemyResultsFile.c_str(), "r" );
    if ( file != nullptr )
    {
        char line [ 4096 ]; /* or other suitable maximum line size */
        while ( fgets ( line, sizeof line, file ) != nullptr ) /* read a line */
        {
            std::stringstream ss(line);

            ss >> strategyName;
            ss >> wins;
            ss >> losses;

            //BWAPI::Broodwar->printf("Results Found: %s %d %d", strategyName.c_str(), wins, losses);

            if (_openings.find(strategyName) == _openings.end())
            {
                //BWAPI::Broodwar->printf("Warning: Results file has unknown Strategy: %s", strategyName.c_str());
            }
            else
            {
				_openings[strategyName]._wins = wins;
				_openings[strategyName]._losses = losses;
            }
        }

        fclose ( file );
    }
    else
    {
        BWAPI::Broodwar->printf("No results file found: %s", enemyResultsFile.c_str());
    }
}

void StrategyManager::writeResults()
{
    if (!Config::Modules::UsingOpeningIO)
    {
        return;
    }

    std::string enemyName = BWAPI::Broodwar->enemy()->getName();
    std::replace(enemyName.begin(), enemyName.end(), ' ', '_');

    std::string enemyResultsFile = Config::Opening::WriteDir + enemyName + ".txt";

    std::stringstream ss;

    for (auto & kv : _openings)
    {
        const Opening & opening = kv.second;

        ss << opening._name << " " << opening._wins << " " << opening._losses << "\n";
    }

    Logger::LogOverwriteToFile(enemyResultsFile, ss.str());
}

void StrategyManager::onEnd(const bool isWinner)
{
    if (!Config::Modules::UsingOpeningIO)
    {
        return;
    }

    if (isWinner)
    {
		_openings[Config::Opening::OpeningName]._wins++;
    }
    else
    {
		_openings[Config::Opening::OpeningName]._losses++;
    }

    writeResults();
}

void StrategyManager::setLearnedOpening()
{
    // we are currently not using this functionality for the competition so turn it off 
    //return;

    if (!Config::Modules::UsingOpeningIO)
    {
        return;
    }

    const std::string & strategyName = Config::Opening::OpeningName;
    Opening & currentStrategy = _openings[strategyName];

    int totalGamesPlayed = 0;
    int openingGamesPlayed = currentStrategy._wins + currentStrategy._losses;
    double winRate = openingGamesPlayed > 0 ? currentStrategy._wins / static_cast<double>(openingGamesPlayed) : 0;

    // if we are using an enemy specific strategy
    if (Config::Opening::FoundEnemySpecificOpening)
    {        
        return;
    }

    // if our win rate with the current strategy is super high don't explore at all
    // also we're pretty confident in our base strategies so don't change if insufficient games have been played
    if (openingGamesPlayed < 5 || (openingGamesPlayed > 0 && winRate > 0.49))
    {
        BWAPI::Broodwar->printf("Still using default strategy");
        return;
    }

    // get the total number of games played so far with this race
    for (auto & kv : _openings)
    {
        Opening & opening = kv.second;
        if (opening._race == BWAPI::Broodwar->self()->getRace())
        {
            totalGamesPlayed += opening._wins + opening._losses;
        }
    }

    // calculate the UCB value and store the highest
    double C = 0.5;
    std::string bestUCBStrategy;
    double bestUCBStrategyVal = std::numeric_limits<double>::lowest();
    for (auto & kv : _openings)
    {
        Opening & opening = kv.second;
        if (opening._race != BWAPI::Broodwar->self()->getRace())
        {
            continue;
        }

        int sGamesPlayed = opening._wins + opening._losses;
        double sWinRate = sGamesPlayed > 0 ? currentStrategy._wins / static_cast<double>(openingGamesPlayed) : 0;
        double ucbVal = C * sqrt( log( (double)totalGamesPlayed / sGamesPlayed ) );
        double val = sWinRate + ucbVal;

        if (val > bestUCBStrategyVal)
        {
            bestUCBStrategy = opening._name;
            bestUCBStrategyVal = val;
        }
    }

    Config::Opening::OpeningName = bestUCBStrategy;
}