#include "Common.h"
#include "StrategyManager.h"
#include "UnitUtil.h"

using namespace CasiaBot;

// constructor
StrategyManager::StrategyManager()
	: _selfRace(BWAPI::Broodwar->self()->getRace())
	, _lastEnemyRace(BWAPI::Broodwar->enemy()->getRace())
	, _emptyBuildOrder(BWAPI::Broodwar->self()->getRace())
	, _action(nullptr)
	, _lastChangeFrame(0)
{
	_actionZVTBarracks.init();
	_actionZVZLurker.init();
	_actionZVPZealot.init();
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

void StrategyManager::addOpening(const std::string & name, Opening & opening)
{
	_openings[name] = opening;
}

void StrategyManager::updateProductionQueue(ProductionQueue & queue)
{
	auto race = BWAPI::Broodwar->enemy()->getRace();
	int currentFrame = BWAPI::Broodwar->getFrameCount();
	if (race != _lastEnemyRace)
	{
		BWAPI::Broodwar->printf("find race, race change");
	}
	// 人族action
	if (race == BWAPI::Races::Terran) {
		if (_action == nullptr)
		{
			_action = &_actionZVTBarracks;
		}
		else if (currentFrame - _lastChangeFrame >= 1000 || queue.empty() || _lastEnemyRace != race)
		{
			_lastChangeFrame = currentFrame;
			if (_lastEnemyRace != race)
			{
				queue.clear();
				_action = &_actionZVTBarracks;
			}
			else if (_action->tick())
			{
				queue.clear();
				bool useBarracks = _actionZVTBarracks.canDeployAction();
				_action = &_actionZVTBarracks;
				if (useBarracks)
				{
					_action = &_actionZVTBarracks;
				}
			}
		}
		if (_action != nullptr)
			_action->getBuildOrderList(queue);
	}
	// 虫族action
	else if (race == BWAPI::Races::Zerg) {
		if (_action == nullptr) {
			_action = &_actionZVZLurker;
		}
		else if (currentFrame - _lastChangeFrame >= 1000 || queue.empty() || _lastEnemyRace != race) {
			_lastChangeFrame = currentFrame;
			if (_lastEnemyRace != race)
			{
				queue.clear();
				_action = &_actionZVZLurker;
			}
			else if (_action->tick()) {
				queue.clear();
				bool useLurker = _actionZVZLurker.canDeployAction();
				_action = &_actionZVZLurker;
				if (useLurker) {
					_action = &_actionZVZLurker;
				}
			}
		}
		if (_action != nullptr)
			_action->getBuildOrderList(queue);
	}
	// 神族action
	else if (race == BWAPI::Races::Protoss) {
		if (_action == nullptr) {
			_action = &_actionZVPZealot;
		}
		else if (currentFrame - _lastChangeFrame >= 1000 || queue.empty() || _lastEnemyRace != race) {
			_lastChangeFrame = currentFrame;
			if (_lastEnemyRace != race)
			{
				queue.clear();
				_action = &_actionZVPZealot;
			}
			else if (_action->tick()) {
				queue.clear();
				bool useZealot = _actionZVPZealot.canDeployAction();
				_action = &_actionZVPZealot;
				if (useZealot) {
					_action = &_actionZVPZealot;
				}
				
			}
		}
		if (_action != nullptr)
			_action->getBuildOrderList(queue);
	}
	else {
		if (_action == nullptr)
		{
			_action = &_actionZVPZealot;
		}
		if (_action != nullptr)
			_action->getBuildOrderList(queue);
	}
	_lastEnemyRace = race;
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