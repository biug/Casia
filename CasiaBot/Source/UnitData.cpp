#include "Common.h"
#include "UnitData.h"

using namespace CasiaBot;

UnitData::UnitData() 
	: mineralsLost(0)
	, gasLost(0)
{
	int maxTypeID(0);
	for (const BWAPI::UnitType & t : BWAPI::UnitTypes::allUnitTypes())
	{
		maxTypeID = maxTypeID > t.getID() ? maxTypeID : t.getID();
	}

	numDeadUnits			= std::vector<int>(maxTypeID + 1, 0);
	numConstructedUnits		= std::vector<int>(maxTypeID + 1, 0);
	numConstructingUnits	= std::vector<int>(maxTypeID + 1, 0);
}

void UnitData::updateSelfZerg(BWAPI::Unit unit)
{
	auto lastType = unitMap[unit].type;
	if (lastType != unit->getType())
	{
		if (lastType != BWAPI::UnitTypes::Zerg_Egg
			&& lastType != BWAPI::UnitTypes::Zerg_Lurker_Egg
			&& lastType != BWAPI::UnitTypes::Zerg_Cocoon
			&& !(lastType.isBuilding() && !unit->getType().isBuilding()))
		{
			--numConstructedUnits[lastType.getID()];
		}
		// morphing complete
		else if (lastType == BWAPI::UnitTypes::Zerg_Egg)
		{
			--numConstructingUnits[unit->getType().getID()];
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling
				|| unit->getType() == BWAPI::UnitTypes::Zerg_Scourge)
			{
				--numConstructingUnits[unit->getType().getID()];
			}

			++numConstructedUnits[unit->getType().getID()];
		}
		// morphing lurker complete
		else if (lastType == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		{
			--numConstructingUnits[BWAPI::UnitTypes::Zerg_Lurker.getID()];
			++numConstructedUnits[BWAPI::UnitTypes::Zerg_Lurker.getID()];
		}
		// morphing from mutalisk
		else if (lastType == BWAPI::UnitTypes::Zerg_Cocoon)
		{
			--numConstructingUnits[unit->getType().getID()];
			++numConstructedUnits[unit->getType().getID()];
		}
		// cancel building
		else if (lastType.isBuilding())
		{
			--numConstructingUnits[lastType.getID()];
			++numConstructedUnits[unit->getType().getID()];
		}

		// start morphing
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg)
		{
			++numConstructingUnits[unit->getBuildType().getID()];
			if (unit->getBuildType() == BWAPI::UnitTypes::Zerg_Zergling
				|| unit->getBuildType() == BWAPI::UnitTypes::Zerg_Scourge)
			{
				++numConstructingUnits[unit->getBuildType().getID()];
			}
		}
		// start morphing lurker
		else if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		{
			++numConstructingUnits[BWAPI::UnitTypes::Zerg_Lurker.getID()];
		}
		else if (unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon)
		{
			++numConstructingUnits[unit->getBuildType().getID()];
		}
		// constructing building
		else if (unit->getType().isBuilding())
		{
			++numConstructingUnits[unit->getType().getID()];
		}
	}
	// when building upgrade complete
	if (unit->getType().isBuilding() && !unitMap[unit].completed && unit->isCompleted())
	{
		--numConstructingUnits[unit->getType().getID()];
		++numConstructedUnits[unit->getType().getID()];
	}
}

void UnitData::updateEnemy(BWAPI::Unit unit)
{
	auto lastType = unitMap[unit].type;
	if (lastType != unit->getType())
	{
		--numConstructedUnits[lastType.getID()];
		++numConstructedUnits[unit->getType().getID()];
	}
}

void UnitData::updateUnit(BWAPI::Unit unit)
{
	if (!unit) { return; }

	bool firstSeen = false;
    auto & it = unitMap.find(unit);
	if (it == unitMap.end())
    {
        firstSeen = true;
        unitMap[unit] = UnitInfo();
    }
	// unit changed, maybe morph
	else
	{
		if (unit->getPlayer() == BWAPI::Broodwar->self())
		{
			updateSelfZerg(unit);
		}
		else
		{
			updateEnemy(unit);
		}
	}
    
	UnitInfo & ui   = unitMap[unit];
    ui.unit         = unit;
    ui.player       = unit->getPlayer();
	ui.lastPosition = unit->getPosition();
	ui.lastHealth   = unit->getHitPoints();
    ui.lastShields  = unit->getShields();
	ui.unitID       = unit->getID();
	ui.type         = unit->getType();
    ui.completed    = unit->isCompleted();

    if (firstSeen)
    {
		if (unit->getType().isRefinery() && unit->getPlayer() == BWAPI::Broodwar->self())
		{
			++numConstructingUnits[unit->getType().getID()];
		}
		else
		{
			++numConstructedUnits[unit->getType().getID()];
		}
    }
}

void UnitData::removeUnit(BWAPI::Unit unit)
{
	if (!unit) { return; }

	mineralsLost += unit->getType().mineralPrice();
	gasLost += unit->getType().gasPrice();
	auto type = unit->getType();
	if (unit->getPlayer() == BWAPI::Broodwar->self()
		&& (type == BWAPI::UnitTypes::Zerg_Egg
		|| type == BWAPI::UnitTypes::Zerg_Lurker_Egg
		|| type == BWAPI::UnitTypes::Zerg_Cocoon
		|| (type.isBuilding() && unit->isBeingConstructed())))
	{
		type = unit->getBuildType();
		--numConstructingUnits[type.getID()];
		if (type == BWAPI::UnitTypes::Zerg_Zergling
			|| type == BWAPI::UnitTypes::Zerg_Scourge)
		{
			--numConstructingUnits[type.getID()];
		}
	}
	else
	{
		--numConstructedUnits[type.getID()];
		++numDeadUnits[type.getID()];
	}
		
	unitMap.erase(unit);
}

void UnitData::removeBadUnits()
{
	for (auto iter(unitMap.begin()); iter != unitMap.end();)
	{
		if (badUnitInfo(iter->second))
		{
			numConstructedUnits[iter->second.type.getID()]--;
			iter = unitMap.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

const bool UnitData::badUnitInfo(const UnitInfo & ui) const
{
    if (!ui.unit)
    {
        return false;
    }

	// Cull away any refineries/assimilators/extractors that were destroyed and reverted to vespene geysers
	if (ui.unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)
	{ 
		return true;
	}

	// If the unit is a building and we can currently see its position and it is not there
	if (ui.type.isBuilding() && BWAPI::Broodwar->isVisible(ui.lastPosition.x/32, ui.lastPosition.y/32) && !ui.unit->isVisible())
	{
		return true;
	}

	return false;
}

int UnitData::getGasLost() const 
{ 
    return gasLost; 
}

int UnitData::getMineralsLost() const 
{ 
    return mineralsLost; 
}

int UnitData::getNumUnits(BWAPI::UnitType t) const 
{ 
	return getNumConstructedUnits(t) + getNumConstructingUnits(t);
}

int UnitData::getNumConstructedUnits(BWAPI::UnitType t) const
{
	if (t == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode
		|| t == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
	{
		return numConstructedUnits[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode.getID()]
			+ numConstructedUnits[BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode.getID()];
	}
	else
	{
		return numConstructedUnits[t.getID()];
	}
}

int UnitData::getNumConstructingUnits(BWAPI::UnitType t) const
{
	return numConstructingUnits[t.getID()];
}

int UnitData::getNumDeadUnits(BWAPI::UnitType t) const 
{ 
    return numDeadUnits[t.getID()]; 
}

const std::map<BWAPI::Unit,UnitInfo> & UnitData::getUnits() const 
{ 
    return unitMap; 
}