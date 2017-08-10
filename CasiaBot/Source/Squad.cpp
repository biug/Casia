#include "Squad.h"
#include "UnitUtil.h"

using namespace CasiaBot;

Squad::Squad()
	: _lastRetreatSwitch(0)
	, _lastRetreatSwitchVal(false)
	, _priority(0)
	, _name("Default")
{
	int a = 10;
}

Squad::Squad(const std::string & name, SquadOrder order, size_t priority)
	: _name(name)
	, _order(order)
	, _lastRetreatSwitch(0)
	, _lastRetreatSwitchVal(false)
	, _priority(priority)
{
}

Squad::~Squad()
{
	clear();
}

void Squad::update()
{
	// update all necessary unit information within this squad
	updateUnits();

	// determine whether or not we should regroup
	bool needToRegroup = needsToRegroup();

	// draw some debug info
	if (Config::Debug::DrawSquadInfo && _order.getType() == SquadOrderTypes::Attack)
	{
		BWAPI::Broodwar->drawTextScreen(200, 350, "%s", _regroupStatus.c_str());

		BWAPI::Unit closest = unitClosestToEnemy();
	}

	checkEnemy();
	// if we do need to regroup, do it
	if (needToRegroup)
	{
		BWAPI::Position regroupPosition = calcRegroupPosition();

		if (Config::Debug::DrawCombatSimulationInfo)
		{
			BWAPI::Broodwar->drawTextScreen(200, 150, "REGROUP");
		}

		BWAPI::Broodwar->drawCircleMap(regroupPosition.x, regroupPosition.y, 30, BWAPI::Colors::Purple, true);

		_meleeManager.regroup(regroupPosition);
		_rangedManager.regroup(regroupPosition);
		_hydraliskManager.regroup(regroupPosition);
		_zerglingManager.regroup(regroupPosition);
		if (_noAirWeapon)
		{
			_mutaliskManager.execute(_order);
		}
		else
		{
			_mutaliskManager.regroup(regroupPosition);
		}
		if (_noShowHidden)
		{
			_lurkerManager.execute(_order);
		}
		else
		{
			_lurkerManager.regroup(regroupPosition);
		}
	}
	else // otherwise, execute micro
	{
		_meleeManager.execute(_order);
		_rangedManager.execute(_order);
		_lurkerManager.execute(_order);
		_hydraliskManager.execute(_order);
		_zerglingManager.execute(_order);
		_mutaliskManager.execute(_order);
	}
	_overlordManager.executeMove(_order);
	/*if (_numHarassZergling > 5)
	{
	_harassZerglingManager.setPattern(true);
	}
	else
	{
	_harassZerglingManager.setPattern(false);
	}*/
	/*if (_numHarassMutalisk > 1 || _noAirWeapon)
	{
	_harassMutaliskManager.setPattern(true);
	}
	else
	{
	_harassMutaliskManager.setPattern(false);
	}
	_harassZerglingManager.execute(_order);
	_harassMutaliskManager.execute(_order);*/
}
void Squad::checkEnemy()
{
	_noAirWeapon = true;
	_noShowHidden = true;
	//for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	//{
	//	if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker)
	//	{
	//		if (unit->isDetected() && unit->isBurrowed())
	//		{
	//			_noShowHidden = false;
	//			break;
	//		}
	//	}
	//}
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		BWAPI::UnitType unitType = unit->getType();
		if (_noAirWeapon)
		{
			if (unitType.airWeapon() != BWAPI::WeaponTypes::None)
			{
				_noAirWeapon = false;
				if (!_noShowHidden)
				{
					return;
				}
			}
		}
		if (_noShowHidden)
		{
			if (unitType == BWAPI::UnitTypes::Terran_Missile_Turret ||
				unitType == BWAPI::UnitTypes::Terran_Science_Vessel ||
				unitType == BWAPI::UnitTypes::Protoss_Photon_Cannon ||
				unitType == BWAPI::UnitTypes::Protoss_Observer ||
				unitType == BWAPI::UnitTypes::Zerg_Spore_Colony ||
				unitType == BWAPI::UnitTypes::Zerg_Overlord)
			{
				_noShowHidden = false;
				if (!_noAirWeapon)
				{
					return;
				}
			}
		}
	}
}
bool Squad::isEmpty() const
{
	return _units.empty();
}

size_t Squad::getPriority() const
{
	return _priority;
}

void Squad::setPriority(const size_t & priority)
{
	_priority = priority;
}

void Squad::updateUnits()
{
	setAllUnits();
	setNearEnemyUnits();
	addUnitsToMicroManagers();
}

void Squad::setAllUnits()
{
	// clean up the _units vector just in case one of them died
	BWAPI::Unitset goodUnits;
	for (auto & unit : _units)
	{
		if (unit->isCompleted() &&
			unit->getHitPoints() > 0 &&
			unit->exists() &&
			unit->getPosition().isValid() &&
			unit->getType() != BWAPI::UnitTypes::Unknown)
		{
			goodUnits.insert(unit);
		}
	}
	_units.clear();
	_units.insert(goodUnits.begin(), goodUnits.end());
}

void Squad::setNearEnemyUnits()
{
	_nearEnemy.clear();
	for (auto & unit : _units)
	{
		int x = unit->getPosition().x;
		int y = unit->getPosition().y;

		int left = unit->getType().dimensionLeft();
		int right = unit->getType().dimensionRight();
		int top = unit->getType().dimensionUp();
		int bottom = unit->getType().dimensionDown();

		_nearEnemy[unit] = unitNearEnemy(unit);
		if (_nearEnemy[unit])
		{
			if (Config::Debug::DrawSquadInfo) BWAPI::Broodwar->drawBoxMap(x - left, y - top, x + right, y + bottom, Config::Debug::ColorUnitNearEnemy);
		}
		else
		{
			if (Config::Debug::DrawSquadInfo) BWAPI::Broodwar->drawBoxMap(x - left, y - top, x + right, y + bottom, Config::Debug::ColorUnitNotNearEnemy);
		}
	}
}

void Squad::addUnitsToMicroManagers()
{
	BWAPI::Unitset meleeUnits;
	BWAPI::Unitset rangedUnits;
	BWAPI::Unitset lurkerUnits;
	BWAPI::Unitset hydraliskUnits;
	BWAPI::Unitset zerglingUnits;
	BWAPI::Unitset mutaliskUnits;
	BWAPI::Unitset overlordUnits;
	BWAPI::Unitset harassZerglingUnits;
	BWAPI::Unitset harassMutaliskUnits;
	_numHarassZergling = 0;
	_numHarassMutalisk = 0;
	_numZergling = 0;
	// add _units to micro managers
	for (auto & unit : _units)
	{
		if (unit->isCompleted() && unit->getHitPoints() > 0 && unit->exists())
		{
			// select dector _units
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker)
			{
				lurkerUnits.insert(unit);
			}
			else if (unit->getType() == BWAPI::UnitTypes::Zerg_Hydralisk)
			{
				hydraliskUnits.insert(unit);
			}
			else if (unit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
			{
				zerglingUnits.insert(unit);
			}
			else if (unit->getType() == BWAPI::UnitTypes::Zerg_Mutalisk)
			{
				mutaliskUnits.insert(unit);
			}
			else if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
			{
				overlordUnits.insert(unit);
			}
			// select ranged _units
			else if ((unit->getType().groundWeapon().maxRange() > 32) || (unit->getType() == BWAPI::UnitTypes::Zerg_Scourge))
			{
				rangedUnits.insert(unit);
			}
			// select melee _units
			else if (unit->getType().groundWeapon().maxRange() <= 32)
			{
				meleeUnits.insert(unit);
			}
		}
	}
	_meleeManager.setUnits(meleeUnits);
	_rangedManager.setUnits(rangedUnits);
	_lurkerManager.setUnits(lurkerUnits);
	_hydraliskManager.setUnits(hydraliskUnits);
	_zerglingManager.setUnits(zerglingUnits);
	_mutaliskManager.setUnits(mutaliskUnits);
	_overlordManager.setUnits(overlordUnits);
	//_harassZerglingManager.setUnits(harassZerglingUnits);
	//_harassMutaliskManager.setUnits(harassMutaliskUnits);
}

float Squad::groundForceScore(BWAPI::UnitType type)
{
	float score = 0.0f;
	auto weapon = type.groundWeapon();
	if (type == BWAPI::UnitTypes::Terran_Bunker)
	{
		weapon = BWAPI::UnitTypes::Terran_Marine.groundWeapon();
	}
	if ((weapon.isValid() && weapon.targetsGround())
		|| type == BWAPI::UnitTypes::Terran_Bunker)
	{
		score
			= (float)weapon.damageAmount() * 15.0f
			/ (float)weapon.damageCooldown();
		// 狂战士双倍伤害
		if (type == BWAPI::UnitTypes::Protoss_Zealot) score *= 2.0f;
		// lurker三倍伤害
		else if (type == BWAPI::UnitTypes::Zerg_Lurker) score *= 3.0f;
		// 地堡四倍伤害
		else if (type == BWAPI::UnitTypes::Terran_Bunker) score *= 4.0f;
		// 射程和移动速度加成
		float factor = weapon.maxRange() - weapon.minRange() + 1;
		if (!type.isBuilding() && type.canMove())
		{
			factor += type.topSpeed() * 15.0f / 2.0f;
		}
		else
		{
			factor /= 2.0f;
		}
		score *= (1 + factor / 8.0f);
	}
	return score;
}

float Squad::airForceScore(BWAPI::UnitType type)
{
	float score = 0.0f;
	auto weapon = type.airWeapon();
	if (type == BWAPI::UnitTypes::Terran_Bunker)
	{
		weapon = BWAPI::UnitTypes::Terran_Marine.airWeapon();
	}
	if (weapon.isValid() && weapon.targetsAir())
	{
		score
			= (float)weapon.damageAmount() * 15.0f
			/ (float)weapon.damageCooldown();
		// 地堡四倍伤害
		if (type == BWAPI::UnitTypes::Terran_Bunker) score *= 4.0f;
		// 射程和移动速度加成
		float factor = weapon.maxRange() - weapon.minRange() + 1;
		if (!type.isBuilding() && type.canMove())
		{
			factor += type.topSpeed() * 15.0f / 2.0f;
		}
		else
		{
			factor /= 2.0f;
		}
		score *= (1 + factor / 8.0f);
	}
	return score;
}

bool Squad::terranRetreat(BWAPI::Position center)
{
	BWAPI::Unitset ourCombatUnits;
	std::vector<UnitInfo> enemyCombatUnits;

	MapGrid::Instance().GetUnits(ourCombatUnits, center, Config::Micro::CombatRegroupRadius, true, false);
	InformationManager::Instance().getNearbyForce(enemyCombatUnits, center, BWAPI::Broodwar->enemy(), Config::Micro::CombatRegroupRadius);

	// enemy force
	std::hash_map<int, float> groundScore, airScore;
	int eGroundNum = 0, eAirNum = 0, sGroundNum = 0, sAirNum = 0;
	float eGroundForce = 0, eAirForce = 0, sGroundForce = 0, sAirForce = 0;
	float eGroundHealth = 0, eAirHealth = 0, sGroundHealth = 0, sAirHealth = 0;
	// enemy force and health
	for (const auto & eunit : enemyCombatUnits)
	{
		if (!eunit.type.canAttack()) continue;
		auto type = eunit.type;
		int id = type.getID();
		if (groundScore.find(id) == groundScore.end())
		{
			groundScore[id] = groundForceScore(type);
		}
		eGroundForce += groundScore[id];
		if (airScore.find(id) == airScore.end())
		{
			airScore[id] = airForceScore(type);
		}
		eAirForce += airScore[id];
		if (!type.isFlyer())
		{
			eGroundHealth += eunit.lastHealth + eunit.lastShields;
			++eGroundNum;
		}
		else
		{
			eAirHealth += eunit.lastHealth + eunit.lastShields;
			++eAirNum;
		}
	}
	// self force and health
	for (const auto & sunit : ourCombatUnits)
	{
		if (!sunit->canAttack()) continue;
		auto type = sunit->getType();
		int id = type.getID();
		if (groundScore.find(id) == groundScore.end())
		{
			groundScore[id] = groundForceScore(type);
		}
		sGroundForce += groundScore[id];
		if (airScore.find(id) == airScore.end())
		{
			airScore[id] = airForceScore(type);
		}
		sAirForce += airScore[id];
		if (!type.isFlyer())
		{
			sGroundHealth += sunit->getHitPoints() + sunit->getShields();
			++sGroundNum;
		}
		else
		{
			sAirHealth += sunit->getHitPoints() + sunit->getShields();
			++sAirNum;
		}
	}
	// 血量除以三，平衡数值
	eGroundHealth /= 3.0f;
	sGroundHealth /= 3.0f;
	eAirHealth /= 3.0f;
	sAirHealth /= 3.0f;
	// 如果某一方没有空军/陆军，那么另一方的空军火力/陆军火力无效
	if (sGroundNum == 0) eGroundForce = 0.0f;
	if (eGroundNum == 0) sGroundForce = 0.0f;
	if (sAirNum == 0) eAirForce = 0.0f;
	if (eAirNum == 0) sAirForce = 0.0f;
	BWAPI::Broodwar->drawTextScreen(440, 200, "sGroundNum: %d", sGroundNum);
	BWAPI::Broodwar->drawTextScreen(440, 210, "eGroundNum: %d", eGroundNum);
	BWAPI::Broodwar->drawTextScreen(440, 220, "sAirNum: %d", sAirNum);
	BWAPI::Broodwar->drawTextScreen(440, 230, "eAirNum: %d", eAirNum);
	BWAPI::Broodwar->drawTextScreen(430, 240, "sGForce: %.2f | eGHP: %.2f", sGroundForce, eGroundHealth);
	BWAPI::Broodwar->drawTextScreen(430, 250, "eGForce: %.2f | sGHP: %.2f", eGroundForce, sGroundHealth);
	BWAPI::Broodwar->drawTextScreen(430, 260, "sAForce: %.2f | eAHP: %.2f", sAirForce, eAirHealth);
	BWAPI::Broodwar->drawTextScreen(430, 270, "eAForce: %.2f | sAHP: %.2f", eAirForce, sAirHealth);
	float groundOffset = (sGroundForce - eGroundHealth) - (eGroundForce - sGroundHealth);
	float airOffset = (sAirForce - eAirHealth) - (eAirForce - sAirHealth);
	BWAPI::Broodwar->drawTextScreen(430, 280, "gOffset: %.2f | aOffset: %.2f", groundOffset, airOffset);
	// 有一方劣势就该撤退
	return groundOffset < -0.1f || airOffset < -0.1f;
}

// calculates whether or not to regroup
bool Squad::needsToRegroup()
{
	if (!Config::Micro::UseSparcraftSimulation)
	{
		return false;
	}

	// if we are not attacking, never regroup
	if (_units.empty() || (_order.getType() != SquadOrderTypes::Attack))
	{
		_regroupStatus = std::string("\x04 No combat units available");
		return false;
	}

	BWAPI::Unit unitClosest = unitClosestToEnemy();

	if (!unitClosest)
	{
		_regroupStatus = std::string("\x04 No closest unit");
		return false;
	}

	// if none of our units are in attack range of any enemy units, don't retreat
	std::vector<UnitInfo> enemyCombatUnits;
	const auto & enemyUnitInfo = InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy());

	SparCraft::ScoreType score = 0;
	bool retreat = false;

	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		retreat = terranRetreat(unitClosest->getPosition());
	}
	else
	{
		//do the SparCraft Simulation!
		CombatSimulation sim;

		sim.setCombatUnits(unitClosest->getPosition(), Config::Micro::CombatRegroupRadius);
		score = sim.simulateCombat();
		retreat = score < 0;
		std::string info = "combat score is " + std::to_string(score);
		BWAPI::Broodwar->drawTextScreen(480, 280, info.c_str());
	}
	int switchTime = 100;
	bool waiting = false;

	// we should not attack unless 5 seconds have passed since a retreat
	if (retreat != _lastRetreatSwitchVal)
	{
		if (!retreat && (BWAPI::Broodwar->getFrameCount() - _lastRetreatSwitch < switchTime))
		{
			waiting = true;
			retreat = _lastRetreatSwitchVal;
		}
		else
		{
			waiting = false;
			_lastRetreatSwitch = BWAPI::Broodwar->getFrameCount();
			_lastRetreatSwitchVal = retreat;
		}
	}

	if (retreat)
	{
		_regroupStatus = std::string("\x04 Retreat - simulation predicts defeat");
	}
	else
	{
		_regroupStatus = std::string("\x04 Attack - simulation predicts success");
	}

	return retreat;
}

void Squad::setSquadOrder(const SquadOrder & so)
{
	_order = so;
}

bool Squad::containsUnit(BWAPI::Unit u) const
{
	return _units.contains(u);
}

void Squad::clear()
{
	for (auto & unit : getUnits())
	{
		if (unit->getType().isWorker())
		{
			WorkerManager::Instance().finishedWithWorker(unit);
		}
	}

	_units.clear();
}

bool Squad::unitNearEnemy(BWAPI::Unit unit)
{
	assert(unit);

	BWAPI::Unitset enemyNear;

	MapGrid::Instance().GetUnits(enemyNear, unit->getPosition(), 400, false, true);

	return enemyNear.size() > 0;
}

BWAPI::Position Squad::calcCenter()
{
	if (_units.empty())
	{
		if (Config::Debug::DrawSquadInfo)
		{
			BWAPI::Broodwar->printf("Squad::calcCenter() called on empty squad");
		}
		return BWAPI::Position(0, 0);
	}

	BWAPI::Position accum(0, 0);
	for (auto & unit : _units)
	{
		accum += unit->getPosition();
	}
	return BWAPI::Position(accum.x / _units.size(), accum.y / _units.size());
}

BWAPI::Position Squad::calcRegroupPosition()
{
	BWAPI::Position regroup(0, 0);

	int minDist = 100000;

	for (auto & unit : _units)
	{
		if (!_nearEnemy[unit])
		{
			int dist = unit->getDistance(_order.getPosition());
			if (dist < minDist)
			{
				minDist = dist;
				regroup = unit->getPosition();
			}
		}
	}

	if (regroup == BWAPI::Position(0, 0))
	{
		return BWTA::getRegion(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition())->getCenter();
	}
	else
	{
		return regroup;
	}
}

BWAPI::Unit Squad::unitClosestToEnemy()
{
	BWAPI::Unit closest = nullptr;
	int closestDist = 100000;

	for (auto & unit : _units)
	{
		// the distance to the order position
		int dist = MapTools::Instance().getGroundDistance(unit->getPosition(), _order.getPosition());

		if (dist != -1 && (!closest || dist < closestDist))
		{
			closest = unit;
			closestDist = dist;
		}
	}

	if (!closest)
	{
		for (auto & unit : _units)
		{
			// the distance to the order position
			int dist = unit->getDistance(BWAPI::Position(BWAPI::Broodwar->enemy()->getStartLocation()));

			if (dist != -1 && (!closest || dist < closestDist))
			{
				closest = unit;
				closestDist = dist;
			}
		}
	}

	return closest;
}

int Squad::squadUnitsNear(BWAPI::Position p)
{
	int numUnits = 0;

	for (auto & unit : _units)
	{
		if (unit->getDistance(p) < 600)
		{
			numUnits++;
		}
	}

	return numUnits;
}

const BWAPI::Unitset & Squad::getUnits() const
{
	return _units;
}

const SquadOrder & Squad::getSquadOrder()	const
{
	return _order;
}

void Squad::addUnit(BWAPI::Unit u)
{
	_units.insert(u);
}

void Squad::removeUnit(BWAPI::Unit u)
{
	_units.erase(u);
}

const std::string & Squad::getName() const
{
	return _name;
}