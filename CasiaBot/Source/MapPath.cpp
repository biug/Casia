#include "MapPath.h"
#include <thread>

using namespace CasiaBot;

TilePath::TilePath() : _lastVisit(0)
{
	_positions.clear();
}

TilePath::TilePath(int tick) : _lastVisit(tick)
{
	_positions.clear();
}

MapPath & MapPath::Instance()
{
	static MapPath instance;
	return instance;
}

MapPath::MapPath()
{
	_paths.clear();
}

void MapPath::calcPath(TileRect rect)
{
	_mutex.lock();
	if (_paths.find(rect) == _paths.end())
	{
		//clock_t start = clock();
		_paths[rect] = TilePath(BWAPI::Broodwar->getFrameCount());
		_paths[rect]._positions = BWTA::getShortestPath(rect.first, rect.second);
		//clock_t end = clock();
		//std::string info = "calculate use " + std::to_string(end - start) + "ms";
		//BWAPI::Broodwar->printf(info.c_str());
	}
	_mutex.unlock();
}

void MapPath::update()
{
	// delete most unused paths
	if (_mutex.try_lock())
	{
		auto iter = _paths.begin();
		while (iter != _paths.end())
		{
			// clear every 4 second
			if (BWAPI::Broodwar->getFrameCount() - iter->second._lastVisit > 60)
			{
				iter = _paths.erase(iter);
			}
			else
			{
				for (int i = 0, n = iter->second._positions.size(); i < n - 1; ++i)
				{
					BWAPI::Broodwar->drawLineMap(BWAPI::Position(iter->second._positions[i]), BWAPI::Position(iter->second._positions[i + 1]), BWAPI::Colors::Purple);
				}
				++iter;
			}
		}
		_mutex.unlock();
	}
}

void MapPath::insert(TileRect rect)
{
	ThreadManager::Instance().enqueue(std::mem_fn(&MapPath::calcPath), this, rect);
}

std::vector<BWAPI::TilePosition> MapPath::getPath(TileRect rect)
{
	std::vector<BWAPI::TilePosition> result;
	result.clear();
	if (_mutex.try_lock())
	{
		auto iter = _paths.find(rect);
		if (iter != _paths.end())
		{
			iter->second._lastVisit = BWAPI::Broodwar->getFrameCount();
			result = iter->second._positions;
		}
		_mutex.unlock();
	}
	return result;
}

