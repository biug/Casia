#include "ProductionItem.h"

using namespace CasiaBot;

ProductionItem::ProductionItem(const MetaType & unit)
{
	_unit = unit;
	_assigned = false;
	_desiredPosition = BWAPI::Broodwar->self()->getStartLocation();
}

ProductionItem::ProductionItem(const ProductionItem & item)
{
	_unit = item._unit;
	_assigned = item._assigned;
	_desiredPosition = BWAPI::Broodwar->self()->getStartLocation();
}

ProductionItem::ProductionItem(const MetaType & unit, BWAPI::TilePosition desiredPosition)
{
	_unit = unit;
	_assigned = false;
	_desiredPosition = desiredPosition;
}

ProductionItem::ProductionItem(const ProductionItem & item, BWAPI::TilePosition desiredPosition)
{
	_unit = item._unit;
	_assigned = item._assigned;
	_desiredPosition = desiredPosition;
}

ProductionItem& ProductionItem::operator=(const ProductionItem & item)
{
	if (this == &item) return *this;
	this->_unit = item._unit;
	this->_assigned = item._assigned;
	this->_desiredPosition = item._desiredPosition;
	return *this;
}