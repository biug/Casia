#include "Common.h"
#include "BuildingPlacer.h"
#include "MapGrid.h"
#include "WorkerManager.h"

using namespace CasiaBot;

BuildingPlacer::BuildingPlacer()
    : _boxTop       (std::numeric_limits<int>::max())
    , _boxBottom    (std::numeric_limits<int>::lowest())
    , _boxLeft      (std::numeric_limits<int>::max())
    , _boxRight     (std::numeric_limits<int>::lowest())
{
    _reserveMap = std::vector< std::vector<bool> >(BWAPI::Broodwar->mapWidth(),std::vector<bool>(BWAPI::Broodwar->mapHeight(),false));

    computeResourceBox();
}

BuildingPlacer & BuildingPlacer::Instance() 
{
    static BuildingPlacer instance;
    return instance;
}

bool BuildingPlacer::isInResourceBox(int x, int y) const
{
    int posX(x * 32);
    int posY(y * 32);

    return (posX >= _boxLeft) && (posX < _boxRight) && (posY >= _boxTop) && (posY < _boxBottom);
}

void BuildingPlacer::computeResourceBox()
{
    BWAPI::Position start(BWAPI::Broodwar->self()->getStartLocation());
    BWAPI::Unitset unitsAroundNexus;

    for (auto & unit : BWAPI::Broodwar->getAllUnits())
    {
        // if the units are less than 400 away add them if they are resources
        if (unit->getDistance(start) < 300 && unit->getType().isMineralField())
        {
            unitsAroundNexus.insert(unit);
        }
    }

    for (auto & unit : unitsAroundNexus)
    {
        int x = unit->getPosition().x;
        int y = unit->getPosition().y;

        int left = x - unit->getType().dimensionLeft();
        int right = x + unit->getType().dimensionRight() + 1;
        int top = y - unit->getType().dimensionUp();
        int bottom = y + unit->getType().dimensionDown() + 1;

        _boxTop     = top < _boxTop       ? top    : _boxTop;
        _boxBottom  = bottom > _boxBottom ? bottom : _boxBottom;
        _boxLeft    = left < _boxLeft     ? left   : _boxLeft;
        _boxRight   = right > _boxRight   ? right  : _boxRight;
    }

    //BWAPI::Broodwar->printf("%d %d %d %d", boxTop, boxBottom, boxLeft, boxRight);
}

// makes final checks to see if a building can be built at a certain location
bool BuildingPlacer::canBuildHere(BWAPI::TilePosition position,const Building & b) const
{
    /*if (!b.type.isRefinery() && !InformationManager::Instance().tileContainsUnit(position))
    {
    return false;
    }*/

    //returns true if we can build this type of unit here. Takes into account reserved tiles.
    if (!BWAPI::Broodwar->canBuildHere(position,b.type,b.builderUnit))
    {
        return false;
    }

    // check the reserve map
    for (int x = position.x; x < position.x + b.type.tileWidth(); x++)
    {
        for (int y = position.y; y < position.y + b.type.tileHeight(); y++)
		{
		if (_reserveMap[x][y])
		{
			return false;
		}
		}
	}

	// check base

	return true;
}

//returns true if we can build this type of unit here with the specified amount of space.
//space value is stored in this->buildDistance.
bool BuildingPlacer::canBuildHereWithSpace(BWAPI::TilePosition position, const Building & b, int buildDist, bool horizontalOnly) const
{
	BWAPI::UnitType type = b.type;

	//if we can't build here, we of course can't build here with space
	if (!canBuildHere(position, b))
	{
		return false;
	}

	// height and width of the building
	int width(b.type.tileWidth());
	int height(b.type.tileHeight());

	// define the rectangle of the building spot
	int startx = position.x - buildDist;
	int starty = position.y - buildDist;
	int endx = position.x + width + buildDist;
	int endy = position.y + height + buildDist;

	if (horizontalOnly)
	{
		starty += buildDist;
		endy -= buildDist;
	}

	// if this rectangle doesn't fit on the map we can't build here
	if (startx < 0 || starty < 0 || endx > BWAPI::Broodwar->mapWidth() || endx < position.x + width || endy > BWAPI::Broodwar->mapHeight())
	{
		return false;
	}

	// if we can't build here, or space is reserved, or it's in the resource box, we can't build here
	for (int x = startx; x < endx; x++)
	{
		for (int y = starty; y < endy; y++)
		{
			if (!b.type.isRefinery())
			{
				if (!buildable(b, x, y) || _reserveMap[x][y] || ((b.type != BWAPI::UnitTypes::Protoss_Photon_Cannon) && isInResourceBox(x, y)))
				{
					return false;
				}
			}
		}
	}

	return true;
}

BWAPI::TilePosition BuildingPlacer::GetBuildLocation(const Building & b, int padding) const
{
	return BWAPI::TilePosition(0, 0);
}

BWAPI::TilePosition BuildingPlacer::getBuildLocationNear(const Building & b, int buildDist, bool horizontalOnly) const
{
	SparCraft::Timer t;
	t.start();

	// get the precomputed vector of tile positions which are sorted closes to this location
	const std::vector<BWAPI::TilePosition> & closestToBuilding = MapTools::Instance().getClosestTilesTo(BWAPI::Position(b.desiredPosition));

	double ms1 = t.getElapsedTimeInMilliSec();

	// special easy case of having no pylons
	int numPylons = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Pylon);
	if (b.type.requiresPsi() && numPylons == 0)
	{
		return BWAPI::TilePositions::None;
	}

	// iterate through the list until we've found a suitable location
	for (size_t i(0); i < closestToBuilding.size(); ++i)
	{
		if (canBuildHereWithSpace(closestToBuilding[i], b, buildDist, horizontalOnly))
		{
			double ms = t.getElapsedTimeInMilliSec();
			//BWAPI::Broodwar->printf("Building Placer Took %d iterations, lasting %lf ms @ %lf iterations/ms, %lf setup ms", i, ms, (i / ms), ms1);
            return closestToBuilding[i];
        }
    }

    double ms = t.getElapsedTimeInMilliSec();
    //BWAPI::Broodwar->printf("Building Placer Took %lf ms", ms);

    return  BWAPI::TilePositions::None;
}

bool BuildingPlacer::buildable(const Building & b,int x,int y) const
{
    BWAPI::TilePosition tp(x,y);

    //returns true if this tile is currently buildable, takes into account units on tile
    if (!BWAPI::Broodwar->isBuildable(x,y))
    {
        return false;
    }

    for (auto & unit : BWAPI::Broodwar->getUnitsOnTile(x,y))
    {
        if ((b.builderUnit != nullptr) && (unit != b.builderUnit))
        {
            return false;
        }
    }

    if (!tp.isValid())
    {
        return false;
    }

    return true;
}

void BuildingPlacer::reserveTiles(BWAPI::TilePosition position,int width,int height)
{
    int rwidth = _reserveMap.size();
    int rheight = _reserveMap[0].size();
    for (int x = position.x; x < position.x + width && x < rwidth; x++)
    {
        for (int y = position.y; y < position.y + height && y < rheight; y++)
        {
            _reserveMap[x][y] = true;
        }
    }
}

void BuildingPlacer::drawReservedTiles()
{
    if (!Config::Debug::DrawReservedBuildingTiles)
    {
        return;
    }

    int rwidth = _reserveMap.size();
    int rheight = _reserveMap[0].size();

    for (int x = 0; x < rwidth; ++x)
    {
        for (int y = 0; y < rheight; ++y)
        {
            if (_reserveMap[x][y] || isInResourceBox(x,y))
            {
                int x1 = x*32 + 8;
                int y1 = y*32 + 8;
                int x2 = (x+1)*32 - 8;
                int y2 = (y+1)*32 - 8;

                BWAPI::Broodwar->drawBoxMap(x1,y1,x2,y2,BWAPI::Colors::Yellow,false);
            }
        }
    }
}

void BuildingPlacer::freeTiles(BWAPI::TilePosition position, int width, int height)
{
    int rwidth = _reserveMap.size();
    int rheight = _reserveMap[0].size();

    for (int x = position.x; x < position.x + width && x < rwidth; x++)
    {
        for (int y = position.y; y < position.y + height && y < rheight; y++)
        {
            _reserveMap[x][y] = false;
        }
    }
}

BWAPI::TilePosition BuildingPlacer::getRefineryPosition()
{
    BWAPI::TilePosition closestGeyser = BWAPI::TilePositions::None;
    double minGeyserDistanceFromHome = std::numeric_limits<double>::max();
	BWAPI::TilePosition homeTilePosition = BWAPI::Broodwar->self()->getStartLocation();

    // for each geyser
	const auto & bases = InformationManager::Instance().getSelfBases();
	const auto & extractors = InformationManager::Instance().getUnitset(BWAPI::UnitTypes::Zerg_Extractor);
	for (const auto & geyser : BWEM::Map::Instance().Geysers())
	{
		BWAPI::Broodwar->drawCircleMap(geyser->Pos(), 10, BWAPI::Colors::Red, true);

		// invalid geyser
		if (!geyser->IsGeyser())
		{
			continue;
		}

		auto geyserTP = geyser->TopLeft();

		// invalid position
		if (!geyserTP.isValid())
		{
			continue;
		}

		// not used by self
		bool haveUsed = false;
		for (const auto & extractor : extractors)
		{
			if (extractor->getTilePosition() == geyserTP)
			{
				haveUsed = true;
				break;
			}
		}
		if (haveUsed)
		{
			continue;
		}

		// check to see if it's next to one of our depots
		bool nearDepot = false;
		for (const auto base : bases)
		{
			if (base && base->isCompleted() && geyserTP.getDistance(base->getTilePosition()) < 10)
			{
				nearDepot = true;
			}
		}

		// find nearest resource
		if (nearDepot)
		{
			double homeDistance = geyserTP.getDistance(homeTilePosition);

			if (homeDistance < minGeyserDistanceFromHome)
			{
				minGeyserDistanceFromHome = homeDistance;
				closestGeyser = geyserTP;
			}
		}
	}
	if (!closestGeyser.isValid())
	{
		minGeyserDistanceFromHome = std::numeric_limits<double>::max();
		const auto & locations = WorkerManager::Instance().getCanceledRefineryLocations();
		for (const auto & location : locations)
		{
			double homeDistance = location.getDistance(homeTilePosition);

			if (homeDistance < minGeyserDistanceFromHome)
			{
				minGeyserDistanceFromHome = homeDistance;
				closestGeyser = location;
			}
		}
	}
	if (InformationManager::Instance().checkBuildingLocation(closestGeyser))
	{
		//BWAPI::Broodwar->printf("find one refinery at (%d, %d)", closestGeyser.x, closestGeyser.y);
		return closestGeyser;
	}
	return BWAPI::TilePositions::None;
}

bool BuildingPlacer::isReserved(int x, int y) const
{
    int rwidth = _reserveMap.size();
    int rheight = _reserveMap[0].size();
    if (x < 0 || y < 0 || x >= rwidth || y >= rheight)
    {
        return false;
    }

    return _reserveMap[x][y];
}

bool BuildingPlacer::isAwayResource(BWAPI::TilePosition tp, BWAPI::Unit base)
{
	const auto & patches = WorkerManager::Instance().getMineralPatches(base);
	for (const auto & patch : patches)
	{
		if (tp.getDistance(patch->getTilePosition()) <= 5)
		{
			return false;
		}
	}
	return true;
}

BWAPI::Unit BuildingPlacer::getDefenseBase()
{
	const auto & ebases = InformationManager::Instance().getEnemyBaseInfos();
	auto enemyP = BWAPI::Positions::None;
	if (!ebases.empty())
	{
		enemyP = ebases.front().lastPosition;
	}
	if (!enemyP.isValid())
	{
		return nullptr;
	}
	auto enemyTP = BWAPI::TilePosition(enemyP);
	const auto & bases = InformationManager::Instance().getSelfBases();
	for (const auto & base : bases)
	{
		const auto & infos = InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->self());
		if (!base || infos.find(base) == infos.end()) continue;
		// 寻找这个基地到对方家中的路径
		int len = 1;
		const auto & chokes = InformationManager::Instance().getPath(base->getTilePosition(), enemyTP, &len);
		if (len < 0) continue;
		BWAPI::Broodwar->drawLineMap(base->getPosition(),
			BWAPI::Position(chokes.front()->Center()),
			BWAPI::Colors::Blue);
		for (int i = 0; i < chokes.size() - 1; ++i)
		{
			BWAPI::Broodwar->drawLineMap(BWAPI::Position(chokes[i]->Center()),
				BWAPI::Position(chokes[i + 1]->Center()), BWAPI::Colors::Blue);
		}
		BWAPI::Broodwar->drawLineMap(enemyP,
			BWAPI::Position(chokes.back()->Center()),
			BWAPI::Colors::Blue);
		// 确保路径上没有其他的我方基地
		bool badPath = false;
		for (const auto & otherBase : bases)
		{
			if (otherBase->getPosition().getDistance(base->getPosition()) < 300) continue;
			len = 1;
			const auto & otherChokes = InformationManager::Instance().getPath(otherBase->getTilePosition(), enemyTP, &len);
			if (len < 0) continue;
			if (otherChokes.size() <= chokes.size())
			{
				int i = 0;
				for (int offset = chokes.size() - otherChokes.size(); i < otherChokes.size(); ++i)
				{
					if (otherChokes[i] != chokes[i + offset])
					{
						break;
					}
				}
				if (i == otherChokes.size())
				{
					badPath = true;
					break;
				}
			}
		}
		if (!badPath) return base;
	}
	BWAPI::Broodwar->printf("defense loc not found");
	return nullptr;
}

BWAPI::TilePosition BuildingPlacer::getCreepPosition(int numCreep, BWAPI::Unit base)
{
	if (base == nullptr) base = getDefenseBase();
	if (!base) return BWAPI::TilePositions::None;
	auto baseP = base->getPosition();
	auto baseTP = base->getTilePosition();
	int numColony = numCreep
		+ InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Creep_Colony, BWAPI::Broodwar->self())
		+ InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Sunken_Colony, BWAPI::Broodwar->self());
	const auto & ebases = InformationManager::Instance().getEnemyBaseInfos();
	auto enemyP = BWAPI::Positions::None;
	if (!ebases.empty())
	{
		enemyP = ebases.front().lastPosition;
	}
	if (!enemyP.isValid())
	{
		return BWAPI::TilePositions::None;
	}
	auto enemyTP = BWAPI::TilePosition(enemyP);
	// 第一个地堡
	if (numColony == 0)
	{
		// 找路口
		int len = 0;
		const auto & chokes = InformationManager::Instance().getPath(baseTP, enemyTP, &len);
		if (len < 0) return BWAPI::TilePositions::None;
		auto chokeTP = BWAPI::TilePositions::None;
		if (!chokes.empty())
		{
			chokeTP = BWAPI::TilePosition(chokes[0]->Center());
		}
		if (!chokeTP.isValid())
		{
			chokeTP = enemyTP;
		}
		double bestTargetScore = 10.0;
		BWAPI::TilePosition bestCreep = BWAPI::TilePositions::None;
		for (int x = baseTP.x - 10; x <= baseTP.x + 10; ++x)
		{
			for (int y = baseTP.y - 10; y <= baseTP.y + 10; ++y)
			{
				// 找离路口最近，离基地最远的位置
				BWAPI::TilePosition targetTP(x, y);
				// 尽量远离基地
				if (targetTP.getDistance(baseTP) <= 5)
				{
					continue;
				}
				// 尽量远离资源
				if (!isAwayResource(targetTP, base)) continue;
				// 在可建造位置建造
				Building b;
				b.type = BWAPI::UnitTypes::Zerg_Creep_Colony;
				auto drones = InformationManager::Instance().getUnitset(BWAPI::UnitTypes::Zerg_Drone);
				if (drones.size() > 0) b.builderUnit = *drones.begin();
				if (canBuildHere(targetTP, b))
				{
					double disA = targetTP.getDistance(chokeTP);
					double disB = targetTP.getDistance(baseTP);
					double score = disB < 0.1 ? 0.08 * disA : disA / disB + 0.08 * (disA + disB);
					// 尽可能远离中心又靠近路口
					if (score < bestTargetScore)
					{
						bestTargetScore = score;
						bestCreep = targetTP;
					}
				}
			}
		}
		if (!bestCreep.isValid()) BWAPI::Broodwar->printf("creep loc not found");
		return bestCreep;
	}
	else
	{
		auto & creepSet = InformationManager::Instance().getUnitset(BWAPI::UnitTypes::Zerg_Creep_Colony);
		auto & sunkenSet = InformationManager::Instance().getUnitset(BWAPI::UnitTypes::Zerg_Sunken_Colony);
		BWAPI::Unitset creeps;
		creeps.insert(creepSet.begin(), creepSet.end());
		creeps.insert(sunkenSet.begin(), sunkenSet.end());
		//找到地堡中心
		auto center = BWAPI::TilePosition(creeps.getPosition());
		// 找路口
		int len = 0;
		const auto & chokes = InformationManager::Instance().getPath(baseTP, enemyTP, &len);
		if (len < 0) return BWAPI::TilePositions::None;
		auto chokeTP = BWAPI::TilePositions::None;
		if (!chokes.empty())
		{
			chokeTP = BWAPI::TilePosition(chokes[0]->Center());
		}
		if (!chokeTP.isValid())
		{
			chokeTP = enemyTP;
		}
		int radius = 3;
		double bestTargetScore = 100000;
		BWAPI::TilePosition bestTile = BWAPI::TilePositions::None;
		// 中心半径为3的区域内找位置
		for (int x = center.x - radius; x <= center.x + radius; ++x)
		{
			for (int y = center.y - radius; y <= center.y + radius; ++y)
			{
				BWAPI::TilePosition tile(x, y);
				Building b;
				b.type = BWAPI::UnitTypes::Zerg_Creep_Colony;
				auto drones = InformationManager::Instance().getUnitset(BWAPI::UnitTypes::Zerg_Drone);
				if (drones.size() > 0) b.builderUnit = *drones.begin();
				if (canBuildHere(tile, b))
				{
					double disA = tile.getDistance(chokeTP);
					double disB = tile.getDistance(center);
					double score = disA + disB;
					// 既靠近中心又靠近路口
					if (!bestTile.isValid() || score < bestTargetScore)
					{
						bestTargetScore = score;
						bestTile = tile;
					}
				}
			}
		}
		return bestTile;
	}
}

BWAPI::TilePosition BuildingPlacer::getNextExpansion()
{
	BWAPI::TilePosition closestMineBase = BWAPI::TilePositions::None;
	BWAPI::TilePosition closestGasBase = BWAPI::TilePositions::None;
	double minMineDistance = 100000;
	double minGasDistance = 100000;

	BWAPI::TilePosition homeTile = BWAPI::Broodwar->self()->getStartLocation();

	const auto & baseTPs = InformationManager::Instance().getBaseTiles();

	for (const auto & area : BWEM::Map::Instance().Areas())
	{
		for (const auto & base : area.Bases())
		{
			auto baseTile = base.Location();
			bool find = false;
			for (const auto & tp : baseTPs)
			{
				if (tp.getDistance(baseTile) < 10)
				{
					find = true;
					break;
				}
			}
			if (find) continue;
			int len = 1;
			const auto & chokes = InformationManager::Instance().getPath(homeTile, baseTile, &len);
			if (len < 0) continue;
			// the base's distance from our start location
			BWAPI::Position startPosition(homeTile);
			BWAPI::Position basePosition = BWAPI::Position(baseTile);
			double distanceFromHome =
				MapTools::Instance().getPathDistance(startPosition, basePosition, chokes)
				+ startPosition.getDistance(basePosition);
			if (base.Geysers().empty())
			{
				if (!closestMineBase || distanceFromHome < minMineDistance)
				{
					closestMineBase = base.Location();
					minMineDistance = distanceFromHome;
				}
			}
			else
			{
				if (!closestGasBase || distanceFromHome < minGasDistance)
				{
					closestGasBase = base.Location();
					minGasDistance = distanceFromHome;
				}
			}
		}
	}

	if (closestGasBase
		&& InformationManager::Instance().checkBuildingLocation(closestGasBase))
	{
		return closestGasBase;
	}
	else if (closestMineBase
		&& InformationManager::Instance().checkBuildingLocation(closestMineBase))
	{
		return closestMineBase;
	}
	else
	{
		return BWAPI::TilePositions::None;
	}
}