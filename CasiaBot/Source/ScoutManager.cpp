#include "ScoutManager.h"
#include "ProductionManager.h"
#include "MapPath.h"

using namespace CasiaBot;

ScoutManager::ScoutManager() 
    : _workerScout(nullptr)
    , _numWorkerScouts(0)
    , _scoutUnderAttack(false)
    , _gasStealStatus("None")
    , _scoutStatus("None")
    , _didGasSteal(false)
    , _gasStealFinished(false)
    , _currentRegionVertexIndex(-1)
    , _previousScoutHP(0)
{
}

ScoutManager & ScoutManager::Instance() 
{
	static ScoutManager instance;
	return instance;
}

void ScoutManager::update()
{
    if (!Config::Modules::UsingScoutManager)
    {
        return;
    }

    // calculate enemy region vertices if we haven't yet
    if (_enemyRegionVertices.empty())
    {
        calculateEnemyRegionVertices();
    }

	moveScouts();
    drawScoutInformation(200, 320);
}

void ScoutManager::setWorkerScout(BWAPI::Unit unit)
{
    // if we have a previous worker scout, release it back to the worker manager
    if (_workerScout)
    {
        WorkerManager::Instance().finishedWithWorker(_workerScout);
    }

    _workerScout = unit;
    WorkerManager::Instance().setScoutWorker(_workerScout);
}

void ScoutManager::drawScoutInformation(int x, int y)
{
    if (!Config::Debug::DrawScoutInfo)
    {
        return;
    }

    BWAPI::Broodwar->drawTextScreen(x, y, "ScoutInfo: %s", _scoutStatus.c_str());
    BWAPI::Broodwar->drawTextScreen(x, y+10, "GasSteal: %s", _gasStealStatus.c_str());
    for (size_t i(0); i < _enemyRegionVertices.size(); ++i)
    {
        BWAPI::Broodwar->drawCircleMap(_enemyRegionVertices[i], 4, BWAPI::Colors::Green, false);
        BWAPI::Broodwar->drawTextMap(_enemyRegionVertices[i], "%d", i);
    }
}

void ScoutManager::moveScouts()
{
	if (!_workerScout || !_workerScout->exists() || !(_workerScout->getHitPoints() > 0))
	{
		return;
	}

    int scoutHP = _workerScout->getHitPoints() + _workerScout->getShields();
    
    gasSteal();

	// get the enemy base location, if we have one
	const auto & ebases = InformationManager::Instance().getEnemyBaseInfos();

    int scoutDistanceThreshold = 30;

    if (_workerScout->isCarryingGas())
    {
        BWAPI::Broodwar->drawCircleMap(_workerScout->getPosition(), 10, BWAPI::Colors::Purple, true);
    }

    // if we initiated a gas steal and the worker isn't idle, 
    bool finishedConstructingGasSteal = _workerScout->isIdle() || _workerScout->isCarryingGas();
    if (!_gasStealFinished && _didGasSteal && !finishedConstructingGasSteal)
    {
        return;
    }
    // check to see if the gas steal is completed
    else if (_didGasSteal && finishedConstructingGasSteal)
    {
        _gasStealFinished = true;
    }
    
	// if we know where the enemy region is and where our scout is
	if (_workerScout && !ebases.empty())
	{
        int scoutDistanceToEnemy = MapTools::Instance().getGroundDistance(_workerScout->getPosition(), ebases.front().lastPosition);
        bool scoutInRangeOfenemy = scoutDistanceToEnemy <= scoutDistanceThreshold;
        
        // we only care if the scout is under attack within the enemy region
        // this ignores if their scout worker attacks it on the way to their base
        if (scoutHP < _previousScoutHP)
        {
	        _scoutUnderAttack = true;
        }

        if (!_workerScout->isUnderAttack() && !enemyWorkerInRadius())
        {
	        _scoutUnderAttack = false;
        }

		// if the scout is in the enemy region
		if (scoutInRangeOfenemy)
		{
			// get the closest enemy worker
			BWAPI::Unit closestWorker = closestEnemyWorker();

			// if the worker scout is not under attack
			if (!_scoutUnderAttack)
			{
				// if there is a worker nearby, harass it
				if (Config::Opening::ScoutHarassEnemy && (!Config::Opening::GasStealWithScout || _gasStealFinished) && closestWorker && (_workerScout->getDistance(closestWorker) < 800))
				{
                    _scoutStatus = "Harass enemy worker";
                    _currentRegionVertexIndex = -1;
					Micro::SmartAttackUnit(_workerScout, closestWorker);
				}
				// otherwise keep moving to the enemy region
				else
				{
                    _scoutStatus = "Following perimeter";
                    followPerimeter();  
                }
				
			}
			// if the worker scout is under attack
			else
			{
                _scoutStatus = "Under attack inside, fleeing";
                followPerimeter();
			}
		}
		// if the scout is not in the enemy region
		else if (_scoutUnderAttack)
		{
            _scoutStatus = "Under attack inside, fleeing";

            followPerimeter();
		}
		else
		{
            _scoutStatus = "Enemy region known, going there";

			// move to the enemy region
			followPerimeter();
        }
		
	}

	// for each start location in the level
	if (ebases.empty())
	{
        _scoutStatus = "Enemy base unknown, exploring";

		//��ʱ��̽·
		auto baseTPs = BWEM::Map::Instance().StartingLocations();
		auto cmp = [](const BWAPI::TilePosition baseTP1, const BWAPI::TilePosition baseTP2) {
			BWAPI::Position center(BWAPI::Broodwar->mapWidth() / 2, BWAPI::Broodwar->mapHeight() / 2);
			BWAPI::Position baseP1(baseTP1), baseP2(baseTP2);
			int dx1 = baseP1.x - center.x, dy1 = baseP1.y - center.y;
			int dx2 = baseP2.x - center.x, dy2 = baseP2.y - center.y;
			double theta1 =
				dx1 == 0 ?
					theta1 = dy1 > 0 ?
						M_PI / 2.0 :
						M_PI * 3.0 / 2.0 :
					std::atan2(dy1, dx1);
			double theta2 =
				dx2 == 0 ?
					theta1 = dy2 > 0 ?
						M_PI / 2.0 :
						M_PI * 3.0 / 2.0 :
					std::atan2(dy2, dx2);
			//������->��ʱ��
			return theta1 > theta2;
		};

		std::sort(baseTPs.begin(), baseTPs.end(), cmp);
		auto iter = std::find(baseTPs.begin(), baseTPs.end(), BWAPI::Broodwar->self()->getStartLocation());
		std::reverse(baseTPs.begin(), iter);
		std::reverse(iter, baseTPs.end());
		std::reverse(baseTPs.begin(), baseTPs.end());

		for (const auto baseTP : baseTPs)
		{
			// if we haven't explored it yet
			if (!BWAPI::Broodwar->isExplored(baseTP))
			{
				// assign a zergling to go scout it
				Micro::SmartMove(_workerScout, BWAPI::Position(baseTP));
				return;
			}
		}
	}

    _previousScoutHP = scoutHP;
}

void ScoutManager::followPerimeter()
{
    BWAPI::Position fleeTo = getFleePosition();

    if (Config::Debug::DrawScoutInfo)
    {
        BWAPI::Broodwar->drawCircleMap(fleeTo, 5, BWAPI::Colors::Red, true);
    }

	Micro::SmartMove(_workerScout, fleeTo);
}

void ScoutManager::gasSteal()
{
    if (!Config::Opening::GasStealWithScout)
    {
        _gasStealStatus = "Not using gas steal";
        return;
    }

    if (_didGasSteal)
    {
        return;
    }

    if (!_workerScout)
    {
        _gasStealStatus = "No worker scout";
        return;
    }

	const auto & ebases = InformationManager::Instance().getEnemyBaseInfos();
    if (ebases.empty())
    {
        _gasStealStatus = "No enemy base location found";
        return;
    }

    BWAPI::Unit enemyGeyser = getEnemyGeyser();
    if (!enemyGeyser)
    {
        _gasStealStatus = "No enemy geyser found";
        return;
    }

    if (!_didGasSteal)
    {
        ProductionManager::Instance().queueGasSteal();
        _didGasSteal = true;
        Micro::SmartMove(_workerScout, enemyGeyser->getPosition());
        _gasStealStatus = "Did Gas Steal";
    }
}

BWAPI::Unit ScoutManager::closestEnemyWorker()
{
	BWAPI::Unit enemyWorker = nullptr;
	double maxDist = 0;

	
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker() && unit->isConstructing())
		{
			return unit;
		}
	}

	// for each enemy worker
	const auto & ebases = InformationManager::Instance().getEnemyBaseInfos();
	if (ebases.empty()) return nullptr;
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		auto ebaseP = ebases.front().lastPosition;
		if (unit->getType().isWorker())
		{
			double dist = unit->getDistance(ebaseP);

			if (dist < 800 && dist > maxDist)
			{
				maxDist = dist;
				enemyWorker = unit;
			}
		}
	}

	return enemyWorker;
}

BWAPI::Unit ScoutManager::getEnemyGeyser()
{
	return nullptr;
}

bool ScoutManager::enemyWorkerInRadius()
{
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker() && (unit->getDistance(_workerScout) < 300))
		{
			return true;
		}
	}

	return false;
}

bool ScoutManager::immediateThreat()
{
	BWAPI::Unitset enemyAttackingWorkers;
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker() && unit->isAttacking())
		{
			enemyAttackingWorkers.insert(unit);
		}
	}
	
	if (_workerScout->isUnderAttack())
	{
		return true;
	}

	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		double dist = unit->getDistance(_workerScout);
		double range = unit->getType().groundWeapon().maxRange();

		if (unit->getType().canAttack() && !unit->getType().isWorker() && (dist <= range + 32))
		{
			return true;
		}
	}

	return false;
}

int ScoutManager::getClosestVertexIndex(BWAPI::Unit unit)
{
    int closestIndex = -1;
    double closestDistance = 10000000;

    for (size_t i(0); i < _enemyRegionVertices.size(); ++i)
    {
        double dist = unit->getDistance(_enemyRegionVertices[i]);
        if (dist < closestDistance)
        {
            closestDistance = dist;
            closestIndex = i;
        }
    }

    return closestIndex;
}

BWAPI::Position ScoutManager::getFleePosition()
{
    CAB_ASSERT_WARNING(!_enemyRegionVertices.empty(), "We should have an enemy region vertices if we are fleeing");

    // if this is the first flee, we will not have a previous perimeter index
    if (_currentRegionVertexIndex == -1)
    {
        // so return the closest position in the polygon
        int closestPolygonIndex = getClosestVertexIndex(_workerScout);

        CAB_ASSERT_WARNING(closestPolygonIndex != -1, "Couldn't find a closest vertex");

        if (closestPolygonIndex == -1)
        {
            return BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
        }
        else
        {
            // set the current index so we know how to iterate if we are still fleeing later
            _currentRegionVertexIndex = closestPolygonIndex;
            return _enemyRegionVertices[closestPolygonIndex];
        }
    }
    // if we are still fleeing from the previous frame, get the next location if we are close enough
    else
    {
        double distanceFromCurrentVertex = _enemyRegionVertices[_currentRegionVertexIndex].getDistance(_workerScout->getPosition());

        // keep going to the next vertex in the perimeter until we get to one we're far enough from to issue another move command
        while (distanceFromCurrentVertex < 128)
        {
            _currentRegionVertexIndex = (_currentRegionVertexIndex + 1) % _enemyRegionVertices.size();

            distanceFromCurrentVertex = _enemyRegionVertices[_currentRegionVertexIndex].getDistance(_workerScout->getPosition());
        }

        return _enemyRegionVertices[_currentRegionVertexIndex];
    }

}

void ScoutManager::calculateEnemyRegionVertices()
{
	const auto & ebases = InformationManager::Instance().getEnemyBaseInfos();
    //CAB_ASSERT_WARNING(enemyBaseLocation, "We should have an enemy base location if we are fleeing");

    if (ebases.empty())
    {
        return;
    }

    const auto & area = InformationManager::Instance().getTileArea(ebases.front().lastTilePosition);
    //CAB_ASSERT_WARNING(enemyRegion, "We should have an enemy region if we are fleeing");

    if (!area)
    {
        return;
    }

    const BWAPI::Position basePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
    const std::vector<BWAPI::TilePosition> & closestTobase = MapTools::Instance().getClosestTilesTo(basePosition);

    std::set<BWAPI::Position> unsortedVertices;

    // check each tile position
    for (size_t i(0); i < closestTobase.size(); ++i)
    {
		auto & map = BWEM::Map::Instance();

        const BWAPI::TilePosition & tp = closestTobase[i];

        if (map.GetNearestArea(tp) != area)
        {
            continue;
        }

        // a tile is 'surrounded' if
        // 1) in all 4 directions there's a tile position in the current region
        // 2) in all 4 directions there's a buildable tile
        bool surrounded = true;
		auto & info = InformationManager::Instance();
        if (!info.validTile(tp.x+1, tp.y)
			|| info.getTileArea(BWAPI::TilePosition(tp.x+1, tp.y)) != area
			|| !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x+1, tp.y))
			|| !info.validTile(tp.x, tp.y+1)
            || info.getTileArea(BWAPI::TilePosition(tp.x, tp.y+1)) != area
			|| !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x, tp.y+1))
			|| !info.validTile(tp.x - 1, tp.y)
            || info.getTileArea(BWAPI::TilePosition(tp.x-1, tp.y)) != area
			|| !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x-1, tp.y))
			|| !info.validTile(tp.x, tp.y - 1)
            || info.getTileArea(BWAPI::TilePosition(tp.x, tp.y-1)) != area
			|| !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x, tp.y -1)))
        { 
            surrounded = false; 
        }
        
        // push the tiles that aren't surrounded
        if (!surrounded && BWAPI::Broodwar->isBuildable(tp))
        {
            if (Config::Debug::DrawScoutInfo)
            {
                int x1 = tp.x * 32 + 2;
                int y1 = tp.y * 32 + 2;
                int x2 = (tp.x+1) * 32 - 2;
                int y2 = (tp.y+1) * 32 - 2;
        
                BWAPI::Broodwar->drawTextMap(x1+3, y1+2, "%d", MapTools::Instance().getGroundDistance(BWAPI::Position(tp), basePosition));
                BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Green, false);
            }
            
            unsortedVertices.insert(BWAPI::Position(tp) + BWAPI::Position(16, 16));
        }
    }


    std::vector<BWAPI::Position> sortedVertices;
    BWAPI::Position current = *unsortedVertices.begin();

    _enemyRegionVertices.push_back(current);
    unsortedVertices.erase(current);

    // while we still have unsorted vertices left, find the closest one remaining to current
    while (!unsortedVertices.empty())
    {
        double bestDist = 1000000;
        BWAPI::Position bestPos;

        for (const BWAPI::Position & pos : unsortedVertices)
        {
            double dist = pos.getDistance(current);

            if (dist < bestDist)
            {
                bestDist = dist;
                bestPos = pos;
            }
        }

        current = bestPos;
        sortedVertices.push_back(bestPos);
        unsortedVertices.erase(bestPos);
    }

    // let's close loops on a threshold, eliminating death grooves
    int distanceThreshold = 100;

    while (true)
    {
        // find the largest index difference whose distance is less than the threshold
        int maxFarthest = 0;
        int maxFarthestStart = 0;
        int maxFarthestEnd = 0;

        // for each starting vertex
        for (int i(0); i < (int)sortedVertices.size(); ++i)
        {
            int farthest = 0;
            int farthestIndex = 0;

            // only test half way around because we'll find the other one on the way back
            for (size_t j(1); j < sortedVertices.size()/2; ++j)
            {
                int jindex = (i + j) % sortedVertices.size();
            
                if (sortedVertices[i].getDistance(sortedVertices[jindex]) < distanceThreshold)
                {
                    farthest = j;
                    farthestIndex = jindex;
                }
            }

            if (farthest > maxFarthest)
            {
                maxFarthest = farthest;
                maxFarthestStart = i;
                maxFarthestEnd = farthestIndex;
            }
        }
        
        // stop when we have no long chains within the threshold
        if (maxFarthest < 4)
        {
            break;
        }

        double dist = sortedVertices[maxFarthestStart].getDistance(sortedVertices[maxFarthestEnd]);

        std::vector<BWAPI::Position> temp;

        for (size_t s(maxFarthestEnd); s != maxFarthestStart; s = (s+1) % sortedVertices.size())
        {
            temp.push_back(sortedVertices[s]);
        }

        sortedVertices = temp;
    }

    _enemyRegionVertices = sortedVertices;
}