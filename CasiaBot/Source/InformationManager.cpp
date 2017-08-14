#include "Common.h"
#include "InformationManager.h"

using namespace CasiaBot;

InformationManager::InformationManager()
    : _self(BWAPI::Broodwar->self())
    , _enemy(BWAPI::Broodwar->enemy())
	, _isEncounterRush(false)
	, _scannedColoum(0)
	, _cols(BWAPI::Broodwar->mapWidth())
	, _rows(BWAPI::Broodwar->mapHeight())
	, _tileAreas(_cols * _rows, nullptr)
{
	initializeRegionInformation();
}

InformationManager & InformationManager::Instance() 
{
	static InformationManager instance;
	return instance;
}

void InformationManager::update() 
{
	_unitData.find(BWAPI::Broodwar->self())->second.clearUnitsets();
	updateUnitInfo();
	updateLocationInfo();
	updateRush();
}

void InformationManager::updateUnitInfo() 
{
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		updateUnit(unit);
	}

	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		updateUnit(unit);
	}

	// remove bad enemy units
	_unitData[_enemy].removeBadUnits();
	_unitData[_self].removeBadUnits();
}

void InformationManager::initializeRegionInformation() 
{
	// set initial pointers to null
	for (const auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType().isResourceDepot())
		{
			_selfBases.push_back(unit);
			break;
		}
	}
	_enemyBaseInfos.clear();
}

void InformationManager::updateLocationInfo() 
{
	if (_scannedColoum < _cols)
	{
		// init tile areas
		for (int r = 0; r < _rows; ++r)
		{
			const auto & area = BWEM::Map::Instance().GetNearestArea(BWAPI::TilePosition(_scannedColoum, r));
			_tileAreas[_scannedColoum*_rows + r] = area;
		}
		++_scannedColoum;
	}

	_baseTiles.clear();
	// for each enemy unit we know about
	for (const auto & kv : _unitData[_enemy].getUnits())
	{
		const UnitInfo & ui(kv.second);
		BWAPI::UnitType type = ui.type;

		// if the unit is base
		if (type == BWAPI::UnitTypes::Zerg_Hatchery
			|| type == BWAPI::UnitTypes::Zerg_Lair
			|| type == BWAPI::UnitTypes::Zerg_Hive
			|| type == BWAPI::UnitTypes::Terran_Command_Center
			|| type == BWAPI::UnitTypes::Protoss_Nexus) if (getTileArea(ui.lastTilePosition))
		{
			_baseTiles.emplace_back(kv.second.lastTilePosition);
			auto itr = std::find_if(_enemyBaseInfos.begin(), _enemyBaseInfos.end(), [&ui](const UnitInfo & u)
			{
				return u.lastTilePosition == ui.lastTilePosition;
			});
			if (itr == _enemyBaseInfos.end())
			{
				_enemyBaseInfos.emplace_back(ui);
			}
		}
	}

	// for each of our units
	for (const auto & kv : _unitData[_self].getUnits())
	{
		auto unit = kv.first;
		BWAPI::UnitType type = kv.second.type;

		// if the unit is base
		if (type == BWAPI::UnitTypes::Zerg_Hatchery
			|| type == BWAPI::UnitTypes::Zerg_Lair
			|| type == BWAPI::UnitTypes::Zerg_Hive) if (getTileArea(unit->getTilePosition()))
		{
			_baseTiles.emplace_back(unit->getTilePosition());
			auto itr = std::find(_selfBases.begin(), _selfBases.end(), unit);
			if (itr == _selfBases.end())
			{
				_selfBases.emplace_back(unit);
			}
		}
	}

	for (const auto &baseTP : _baseTiles)
	{
		BWAPI::Position baseP(baseTP);
		BWAPI::Broodwar->drawBoxMap(baseP - BWAPI::Position(16, 16), baseP + BWAPI::Position(16, 16), BWAPI::Colors::Blue, true);
	}
}

bool InformationManager::validTile(int x, int y)
{
	return x >= 0 && x < _cols && y >= 0 && y < _rows;
}

bool InformationManager::beingMarineRushed()
{
	int numBarracks = getNumUnits(BWAPI::UnitTypes::Terran_Barracks, BWAPI::Broodwar->enemy());
	int numMarine = getNumUnits(BWAPI::UnitTypes::Terran_Marine, BWAPI::Broodwar->enemy());
	int numZergling = getNumUnits(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Broodwar->self());
	int frameCount = BWAPI::Broodwar->getFrameCount();
	return (frameCount <= 3600 && numBarracks >= 2) ||
		(frameCount <= 5000 && numMarine >= 5) ||
		(numMarine >= 6 && numZergling <= 12);
}

bool InformationManager::beingZealotRushed()
{
	int numGateway = getNumUnits(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->enemy());
	int numZealot = getNumUnits(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->enemy());
	int numZergling = getNumUnits(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Broodwar->self());
	int frameCount = BWAPI::Broodwar->getFrameCount();
	return (frameCount <= 3300 && numGateway >= 2) ||
		(frameCount <= 4000 && numZealot >= 2) ||
		(frameCount <= 5000 && numZealot >= 4) ||
		(numZealot >= 4 && numZergling <= 12);
}

bool InformationManager::beingZerglingRushed()
{
	int numZergling = getNumUnits(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Broodwar->enemy());
	int frameCount = BWAPI::Broodwar->getFrameCount();
	return (frameCount <= 3200 && numZergling >= 6);
}

void InformationManager::updateRush()
{
	if (getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Lurker, BWAPI::Broodwar->self()) >= 2
		|| getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Mutalisk, BWAPI::Broodwar->self()) >= 4)
	{
		_isEncounterRush = false;
	}
	else if (!_isEncounterRush)
	{
		_isEncounterRush = beingMarineRushed() || beingZerglingRushed() || beingZealotRushed();
	}
}

const UIMap & InformationManager::getUnitInfo(BWAPI::Player player) const
{
	return getUnitData(player).getUnits();
}

const std::vector<BWAPI::Unit> & InformationManager::getSelfBases() const
{
	return _selfBases;
}

const std::vector<UnitInfo> & InformationManager::getEnemyBaseInfos() const
{
	return _enemyBaseInfos;
}

const std::vector<BWAPI::TilePosition> & InformationManager::getBaseTiles() const
{
	return _baseTiles;
}

const BWEM::Area * InformationManager::getTileArea(BWAPI::TilePosition base) const
{
	return _tileAreas[base.x*_rows + base.y];
}

const BWEM::CPPath & InformationManager::getBasePath(BWAPI::TilePosition base1, BWAPI::TilePosition base2, int * length)
{
	*length = 1;
	const auto & area1 = getTileArea(base1);
	const auto & area2 = getTileArea(base2);
	return BWEM::Map::Instance().GetPath(area1, area2, length);
}

BWAPI::Position InformationManager::getLastPosition(BWAPI::Unit unit, BWAPI::Player player) const
{
	const auto & units = getUnitData(player).getUnits();
	if (units.find(unit) == units.end()) return BWAPI::Positions::None;
	const auto & uinfo = units.at(unit);
	return uinfo.lastPosition;
}

void InformationManager::drawExtendedInterface()
{
    if (!Config::Debug::DrawUnitHealthBars)
    {
        return;
    }

    int verticalOffset = -10;

    // draw enemy units
    for (const auto & kv : getUnitData(BWAPI::Broodwar->enemy()).getUnits())
	{
        const UnitInfo & ui(kv.second);

		BWAPI::UnitType type(ui.type);
        int hitPoints = ui.lastHealth;
        int shields = ui.lastShields;

        const BWAPI::Position & pos = ui.lastPosition;

        int left    = pos.x - type.dimensionLeft();
        int right   = pos.x + type.dimensionRight();
        int top     = pos.y - type.dimensionUp();
        int bottom  = pos.y + type.dimensionDown();

        if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(ui.lastPosition)))
        {
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, top), BWAPI::Position(right, bottom), BWAPI::Colors::Grey, false);
            BWAPI::Broodwar->drawTextMap(BWAPI::Position(left + 3, top + 4), "%s", ui.type.getName().c_str());
        }
        
        if (!type.isResourceContainer() && type.maxHitPoints() > 0)
        {
            double hpRatio = (double)hitPoints / (double)type.maxHitPoints();
        
            BWAPI::Color hpColor = BWAPI::Colors::Green;
            if (hpRatio < 0.66) hpColor = BWAPI::Colors::Orange;
            if (hpRatio < 0.33) hpColor = BWAPI::Colors::Red;

            int ratioRight = left + (int)((right-left) * hpRatio);
            int hpTop = top + verticalOffset;
            int hpBottom = top + 4 + verticalOffset;

            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), hpColor, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

            int ticWidth = 3;

            for (int i(left); i < right-1; i+=ticWidth)
            {
                BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
            }
        }

        if (!type.isResourceContainer() && type.maxShields() > 0)
        {
            double shieldRatio = (double)shields / (double)type.maxShields();
        
            int ratioRight = left + (int)((right-left) * shieldRatio);
            int hpTop = top - 3 + verticalOffset;
            int hpBottom = top + 1 + verticalOffset;

            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Blue, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

            int ticWidth = 3;

            for (int i(left); i < right-1; i+=ticWidth)
            {
                BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
            }
        }

    }

    // draw neutral units and our units
    for (auto & unit : BWAPI::Broodwar->getAllUnits())
    {
        if (unit->getPlayer() == BWAPI::Broodwar->enemy())
        {
            continue;
        }

        const BWAPI::Position & pos = unit->getPosition();

        int left    = pos.x - unit->getType().dimensionLeft();
        int right   = pos.x + unit->getType().dimensionRight();
        int top     = pos.y - unit->getType().dimensionUp();
        int bottom  = pos.y + unit->getType().dimensionDown();

        //BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, top), BWAPI::Position(right, bottom), BWAPI::Colors::Grey, false);

        if (!unit->getType().isResourceContainer() && unit->getType().maxHitPoints() > 0)
        {
            double hpRatio = (double)unit->getHitPoints() / (double)unit->getType().maxHitPoints();
        
            BWAPI::Color hpColor = BWAPI::Colors::Green;
            if (hpRatio < 0.66) hpColor = BWAPI::Colors::Orange;
            if (hpRatio < 0.33) hpColor = BWAPI::Colors::Red;

            int ratioRight = left + (int)((right-left) * hpRatio);
            int hpTop = top + verticalOffset;
            int hpBottom = top + 4 + verticalOffset;

            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), hpColor, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

            int ticWidth = 3;

            for (int i(left); i < right-1; i+=ticWidth)
            {
                BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
            }
        }

        if (!unit->getType().isResourceContainer() && unit->getType().maxShields() > 0)
        {
            double shieldRatio = (double)unit->getShields() / (double)unit->getType().maxShields();
        
            int ratioRight = left + (int)((right-left) * shieldRatio);
            int hpTop = top - 3 + verticalOffset;
            int hpBottom = top + 1 + verticalOffset;

            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Blue, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

            int ticWidth = 3;

            for (int i(left); i < right-1; i+=ticWidth)
            {
                BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
            }
        }

        if (unit->getType().isResourceContainer() && unit->getInitialResources() > 0)
        {
            
            double mineralRatio = (double)unit->getResources() / (double)unit->getInitialResources();
        
            int ratioRight = left + (int)((right-left) * mineralRatio);
            int hpTop = top + verticalOffset;
            int hpBottom = top + 4 + verticalOffset;

            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Cyan, true);
            BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

            int ticWidth = 3;

            for (int i(left); i < right-1; i+=ticWidth)
            {
                BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
            }
        }
    }
}

void InformationManager::drawUnitInformation(int x, int y) 
{
	if (!Config::Debug::DrawEnemyUnitInfo)
    {
        return;
    }

	std::string prefix = "\x04";

	BWAPI::Broodwar->drawTextScreen(x, y-10, "\x03 Self Loss:\x04 Minerals: \x1f%d \x04Gas: \x07%d", _unitData[_self].getMineralsLost(), _unitData[_self].getGasLost());
    BWAPI::Broodwar->drawTextScreen(x, y, "\x03 Enemy Loss:\x04 Minerals: \x1f%d \x04Gas: \x07%d", _unitData[_enemy].getMineralsLost(), _unitData[_enemy].getGasLost());
	BWAPI::Broodwar->drawTextScreen(x, y+10, "\x04 Enemy: %s", BWAPI::Broodwar->enemy()->getName().c_str());
	BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04 UNIT NAME");
	BWAPI::Broodwar->drawTextScreen(x+140, y+20, "\x04#");
	BWAPI::Broodwar->drawTextScreen(x+160, y+20, "\x04X");

	int yspace = 0;

	// for each unit in the queue
	for (BWAPI::UnitType t : BWAPI::UnitTypes::allUnitTypes()) 
	{
		int numUnits = _unitData[_enemy].getNumUnits(t);
		int numDeadUnits = _unitData[_enemy].getNumDeadUnits(t);

		// if there exist units in the vector
		if (numUnits > 0) 
		{
			if (t.isDetector())			{ prefix = "\x10"; }		
			else if (t.canAttack())		{ prefix = "\x08"; }		
			else if (t.isBuilding())	{ prefix = "\x03"; }
			else						{ prefix = "\x04"; }

			BWAPI::Broodwar->drawTextScreen(x, y+40+((yspace)*10), " %s%s", prefix.c_str(), t.getName().c_str());
			BWAPI::Broodwar->drawTextScreen(x+140, y+40+((yspace)*10), "%s%d", prefix.c_str(), numUnits);
			BWAPI::Broodwar->drawTextScreen(x+160, y+40+((yspace++)*10), "%s%d", prefix.c_str(), numDeadUnits);
		}
	}
}

void InformationManager::updateUnit(BWAPI::Unit unit)
{
    if (!(unit->getPlayer() == _self || unit->getPlayer() == _enemy))
    {
        return;
    }

    _unitData[unit->getPlayer()].updateUnit(unit);
}

// is the unit valid?
bool InformationManager::isValidUnit(BWAPI::Unit unit) 
{
	// we only care about our units and enemy units
	if (unit->getPlayer() != BWAPI::Broodwar->self() && unit->getPlayer() != BWAPI::Broodwar->enemy()) 
	{
		return false;
	}

	// if it's a weird unit, don't bother
	if (unit->getType() == BWAPI::UnitTypes::None || unit->getType() == BWAPI::UnitTypes::Unknown ||
		unit->getType() == BWAPI::UnitTypes::Zerg_Larva || unit->getType() == BWAPI::UnitTypes::Zerg_Egg) 
	{
		return false;
	}

	// if the position isn't valid throw it out
	if (!unit->getPosition().isValid()) 
	{
		return false;	
	}

	// s'all good baby baby
	return true;
}

void InformationManager::onUnitDestroy(BWAPI::Unit unit) 
{ 
    if (unit->getType().isNeutral())
    {
        return;
    }
	auto player = unit->getPlayer();
	const auto & uis = _unitData[player].getUnits();
	if (uis.find(unit) == uis.end()) return;
	const auto tile = uis.at(unit).lastTilePosition;
	// remove unit
    _unitData[player].removeUnit(unit);
	if (player == _enemy)
	{
		auto itr = std::find_if(_enemyBaseInfos.begin(), _enemyBaseInfos.end(), [&tile](const UnitInfo & u)
		{
			return u.lastTilePosition == tile;
		});
		if (itr != _enemyBaseInfos.end())
		{
			_enemyBaseInfos.erase(itr);
		}
	}
	else
	{
		auto itr = std::find(_selfBases.begin(), _selfBases.end(), unit);
		if (itr != _selfBases.end())
		{
			_selfBases.erase(itr);
		}
	}
}

bool InformationManager::isCombatUnit(BWAPI::UnitType type) const
{
	if (type == BWAPI::UnitTypes::Zerg_Lurker/* || type == BWAPI::UnitTypes::Protoss_Dark_Templar*/)
	{
		return false;
	}

	// check for various types of combat units
	if (type.canAttack() || 
		type == BWAPI::UnitTypes::Terran_Medic || 
		type == BWAPI::UnitTypes::Protoss_Observer ||
        type == BWAPI::UnitTypes::Terran_Bunker)
	{
		return true;
	}
		
	return false;
}

void InformationManager::getNearbyForce(std::vector<UnitInfo> & unitInfo, BWAPI::Position p, BWAPI::Player player, int radius) 
{
	bool hasBunker = false;
	// for each unit we know about for that player
	for (const auto & kv : getUnitData(player).getUnits())
	{
		const UnitInfo & ui(kv.second);

		// if it's a combat unit we care about
		// and it's finished! 
		if (isCombatUnit(ui.type) && ui.completed)
		{
			// determine its attack range
			int range = 0;
			if (ui.type.groundWeapon() != BWAPI::WeaponTypes::None)
			{
				range = ui.type.groundWeapon().maxRange() + 40;
			}

			// if it can attack into the radius we care about
			if (ui.lastPosition.getDistance(p) <= (radius + range))
			{
				// add it to the vector
				unitInfo.push_back(ui);
			}
		}
		else if (ui.type.isDetector() && ui.lastPosition.getDistance(p) <= (radius + 250))
        {
			// add it to the vector
			unitInfo.push_back(ui);
        }
	}
}

int InformationManager::getNumUnits(BWAPI::UnitType t, BWAPI::Player player)
{
	return getUnitData(player).getNumUnits(t);
}

int InformationManager::getNumConstructedUnits(BWAPI::UnitType t, BWAPI::Player player)
{
	return getUnitData(player).getNumConstructedUnits(t);
}

int InformationManager::getNumConstructingUnits(BWAPI::UnitType t, BWAPI::Player player)
{
	return getUnitData(player).getNumConstructingUnits(t);
}

const UnitData & InformationManager::getUnitData(BWAPI::Player player) const
{
    return _unitData.find(player)->second;
}

const BWAPI::Unitset & InformationManager::getUnitset(BWAPI::UnitType t)
{
	return _unitData.find(BWAPI::Broodwar->self())->second.getUnitset(t);
}

bool InformationManager::enemyHasCloakedUnits()
{
    for (const auto & kv : getUnitData(_enemy).getUnits())
	{
		const UnitInfo & ui(kv.second);

        if (ui.type.isCloakable())
        {
            return true;
        }

        // assume they're going dts
        if (ui.type == BWAPI::UnitTypes::Protoss_Citadel_of_Adun)
        {
            return true;
        }

        if (ui.type == BWAPI::UnitTypes::Protoss_Observatory)
        {
            return true;
        }
    }

	return false;
}

std::string InformationManager::formatSelfInfo(BWAPI::UnitType type)
{
	int constructed = getNumConstructedUnits(type, _self);
	int constructing = getNumConstructingUnits(type, _self);
	int total = getNumUnits(type, _self);
	if (constructed != 0 || constructing != 0 || total != 0) {
		int needSpace = 20 - type.getName().length();
		if (needSpace <= 0) needSpace = 1;
		std::string info = "\x03" + type.getName() + std::string(needSpace, ' ');
		if (constructed >= 0 && constructed < 10) info += " ";
		info += std::to_string(constructed);
		info += " ";
		if (constructing >= 0 && constructing < 10) info += " ";
		info += std::to_string(constructing);
		info += " ";
		if (total >= 0 && total < 10) info += " ";
		info += std::to_string(total);
		return info;
	}
	else
	{
		return "";
	}
}

std::string InformationManager::formatEnemyInfo(BWAPI::UnitType type)
{
	int total = getNumUnits(type, _enemy);
	if (total != 0) {
		int needSpace = 20 - type.getName().length();
		if (needSpace <= 0) needSpace = 1;
		std::string info = "\x04" + type.getName() + std::string(needSpace, ' ');
		if (total >= 0 && total < 10) info += " ";
		info += std::to_string(total);
		return info;
	}
	else
	{
		return "";
	}
}

void InformationManager::PrintInfo(int x, int y) {
	int i = 0;
	BWAPI::Broodwar->drawTextScreen(x, y - 10, "\x3 self_count");
	BWAPI::Broodwar->drawTextScreen(x + 180, y - 10, "\x4 enemy_count");
	for (auto & type : BWAPI::UnitTypes::allUnitTypes()) {
		std::string info = formatSelfInfo(type);
		if (!info.empty())
		{
			BWAPI::Broodwar->drawTextScreen(x, y + i, info.c_str());
			i = i + 10;
		}
	}
	i = 0;
	for (auto & type : BWAPI::UnitTypes::allUnitTypes()) {
		std::string info = formatEnemyInfo(type);
		if (!info.empty())
		{
			BWAPI::Broodwar->drawTextScreen(x + 180, y + i, info.c_str());
			i = i + 10;
		}
	}

}

bool InformationManager::isEncounterRush()
{
	return _isEncounterRush;
}

bool InformationManager::checkBuildingLocation(const BWAPI::TilePosition & tp)
{
	for (const auto & b : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (b && b->exists() && b->getHitPoints() > 0 && b->getType().isBuilding())
		{
			auto lastP = getLastPosition(b, BWAPI::Broodwar->enemy());
			if (!lastP.isValid()) continue;
			auto lastTP = BWAPI::TilePosition(lastP);
			if (lastTP.getDistance(tp) < 5) return false;
		}
	}
	return true;
}