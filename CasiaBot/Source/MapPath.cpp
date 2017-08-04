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

void MapPath::calcPath(PosRect rect)
{
	// find in queue
	_betapathmtx.lock();
	for (const auto & betap : _betaPaths)
	{
		if (betap.first == rect)
		{
			_betapathmtx.unlock();
			return;
		}
	}
	_betapathmtx.unlock();

	// find in path
	_pathmtx.lock();
	if (_paths.find(rect) != _paths.end())
	{
		_pathmtx.unlock();
		return;
	}
	_pathmtx.unlock();

	// calculate
	auto path = BWTA::getShortestPath(BWAPI::TilePosition(rect.first), BWAPI::TilePosition(rect.second));
	_betapathmtx.lock();
	_betaPaths.push_back({ rect, path });
	_betapathmtx.unlock();
}

void MapPath::update()
{
	// delete most unused paths
	if (_pathmtx.try_lock())
	{
		// clear every 4 second
		auto iter = _paths.begin();
		int frame = BWAPI::Broodwar->getFrameCount();
		while (iter != _paths.end())
		{
			if (frame - iter->second._lastVisit > 60)
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
		if (_betapathmtx.try_lock())
		{
			for (const auto & bpath : _betaPaths)
			{
				_paths[bpath.first]._lastVisit = frame;
				_paths[bpath.first]._positions = bpath.second;
			}
			_betaPaths.clear();
			_betapathmtx.unlock();
		}
		_pathmtx.unlock();
	}
}

void MapPath::insert(PosRect rect)
{
	ThreadManager::Instance().enqueue(std::mem_fn(&MapPath::calcPath), this, rect);
}

std::vector<BWAPI::TilePosition> MapPath::getPath(PosRect rect)
{
	std::vector<BWAPI::TilePosition> result;
	result.clear();
	if (_pathmtx.try_lock())
	{
		auto iter = _paths.find(rect);
		if (iter != _paths.end())
		{
			iter->second._lastVisit = BWAPI::Broodwar->getFrameCount();
			result = iter->second._positions;
		}
		_pathmtx.unlock();
	}
	return result;
}

