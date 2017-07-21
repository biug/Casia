#pragma once

#include "MetaType.h"

namespace CasiaBot
{

	struct ProductionItem
	{

		MetaType	_unit;
		bool		_assigned;
		bool		_nexpHatchery;
		BWAPI::TilePosition _desiredPosition;

	public:
		ProductionItem(const MetaType & unit);
		ProductionItem(const ProductionItem & item);
		ProductionItem(const MetaType & unit, BWAPI::TilePosition desiredPosition);
		ProductionItem(const ProductionItem & item, BWAPI::TilePosition desiredPosition);
		ProductionItem &operator=(const ProductionItem & item);
		void setNExpHatchery();
	};
}