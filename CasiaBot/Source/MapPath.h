#pragma once

#include "Common.h"
#include <vector>
#include "BWAPI.h"
#include <mutex>

namespace CasiaBot
{
	typedef std::pair<BWAPI::Position, BWAPI::Position> PosRect;

	struct TilePath
	{
		int									_lastVisit;
		std::vector<BWAPI::TilePosition>	_positions;

		TilePath();
		TilePath(int tick);
	};
	// provides useful tools for analyzing the starcraft map
	// calculates connectivity and distances using flood fills
	class MapPath
	{
		MapPath();
		std::mutex															_pathmtx;
		std::mutex															_betapathmtx;
		std::map<PosRect, TilePath>											_paths;
		std::vector<std::pair<PosRect, std::vector<BWAPI::TilePosition>>>	_betaPaths;

		void					calcPath(PosRect rect);

	public:

		static MapPath &		Instance();

		void					update();
		void					insert(PosRect rect);

		std::vector<BWAPI::TilePosition>	getPath(PosRect rect);
	};
}