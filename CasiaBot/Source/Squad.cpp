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
	bool gRegroup = groundNeedsToRegroup();
	bool aRegroup = airNeedsToRegroup();

	checkEnemy();
	// 陆军
	if (gRegroup)
	{
		BWAPI::Position regroupPosition = calcGroundRegroupPosition();

		BWAPI::Broodwar->drawCircleMap(regroupPosition.x, regroupPosition.y, 30, BWAPI::Colors::Purple, true);

		_meleeManager.regroup(regroupPosition);
		_rangedManager.regroup(regroupPosition);
		_hydraliskManager.regroup(regroupPosition);
		_zerglingManager.regroup(regroupPosition);
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
		BWAPI::Broodwar->drawCircleMap(_order.getPosition(), 10, BWAPI::Colors::Orange, true);
		_meleeManager.execute(_order);
		_rangedManager.execute(_order);
		_lurkerManager.execute(_order);
		_hydraliskManager.execute(_order);
		_zerglingManager.execute(_order);
	}
	// 空军
	if (aRegroup)
	{
		BWAPI::Position regroupPosition = calcAirRegroupPosition();

		BWAPI::Broodwar->drawCircleMap(regroupPosition.x, regroupPosition.y, 30, BWAPI::Colors::Purple, true);

		_mutaliskManager.regroup(regroupPosition);
		_scourgeManager.regroup(regroupPosition);
	}
	else
	{
		_mutaliskManager.execute(_order);
		_scourgeManager.execute(_order);
	}
	_overlordManager.executeMove(_order);
}
void Squad::checkEnemy()
{
	_noAirWeapon = true;
	_noShowHidden = true;
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
	BWAPI::Unitset scourgeUnits;
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
			else if (unit->getType() == BWAPI::UnitTypes::Zerg_Scourge)
			{
				scourgeUnits.insert(unit);
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
	_scourgeManager.setUnits(scourgeUnits);
	_overlordManager.setUnits(overlordUnits);
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
		float factor = weapon.maxRange() - weapon.minRange();
		if (!type.isBuilding() && type.canMove())
		{
			auto speed = type.topSpeed();
			if (type == BWAPI::UnitTypes::Zerg_Zergling)
			{
				speed *= 1.5f;
			}
			factor += speed * 15.0f / 2.0f;
		}
		else
		{
			factor /= 2.0f;
		}
		score *= (1 + factor / 128.0f);
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
	if ((weapon.isValid() && weapon.targetsGround())
		|| type == BWAPI::UnitTypes::Terran_Bunker)
	{
		score
			= (float)weapon.damageAmount() * 15.0f
			/ (float)weapon.damageCooldown();
		// 地堡四倍伤害
		if (type == BWAPI::UnitTypes::Terran_Bunker) score *= 4.0f;
		// 射程和移动速度加成
		float factor = weapon.maxRange() - weapon.minRange();
		if (!type.isBuilding() && type.canMove())
		{
			factor += type.topSpeed() * 15.0f / 2.0f;
		}
		else
		{
			factor /= 2.0f;
		}
		score *= (1 + factor / 128.0f);
	}
	return score;
}

bool Squad::groundRetreat(BWAPI::Position center)
{
	BWAPI::Unitset ourCombatUnits;
	std::vector<UnitInfo> enemyCombatUnits;

	MapGrid::Instance().GetUnits(ourCombatUnits, center, Config::Micro::CombatRegroupRadius * 2, true, false);
	InformationManager::Instance().getNearbyForce(enemyCombatUnits, center, BWAPI::Broodwar->enemy(), Config::Micro::CombatRegroupRadius);

	// enemy force
	std::hash_map<int, float> groundScore;
	int eGroundNum = 0, sGroundNum = 0;
	float eGroundForce = 0, sGroundForce = 0;
	float eGroundHealth = 0, sGroundHealth = 0;
	// enemy force and health
	for (const auto & eunit : enemyCombatUnits)
	{
		if (!InformationManager::Instance().isCombatUnit(eunit.type)) continue;
		auto type = eunit.type;
		int id = type.getID();
		if (groundScore.find(id) == groundScore.end())
		{
			groundScore[id] = groundForceScore(type);
		}
		// 血量会对力量打折扣
		eGroundForce += groundScore[id] * std::sqrtf(eunit.lastHealth / eunit.type.maxHitPoints());
		if (!type.isFlyer())
		{
			++eGroundNum;
			eGroundHealth += eunit.lastHealth + eunit.lastShields + eunit.type.armor();
		}
	}
	int num = 0;
	for (const auto & score : groundScore)
	{
		BWAPI::Broodwar->drawTextScreen(180, 220 + num * 10, "eg %s : %.2f", BWAPI::UnitType(score.first).getName().c_str(), score.second);
		++num;
	}
	// self force and health
	groundScore.clear();
	for (const auto & sunit : ourCombatUnits)
	{
		if (!InformationManager::Instance().isCombatUnit(sunit->getType())) continue;
		auto type = sunit->getType();
		int id = type.getID();
		if (groundScore.find(id) == groundScore.end())
		{
			groundScore[id] = groundForceScore(type);
		}
		// 血量会对力量打折扣
		sGroundForce += groundScore[id] * std::sqrtf(sunit->getHitPoints() / sunit->getType().maxHitPoints());
		if (!type.isFlyer())
		{
			++sGroundNum;
			sGroundHealth += sunit->getHitPoints() + sunit->getShields() + sunit->getType().armor();
		}
	}
	for (const auto & score : groundScore)
	{
		BWAPI::Broodwar->drawTextScreen(180, 220 + num*10, "sg %s : %.2f", BWAPI::UnitType(score.first).getName().c_str(), score.second);
		++num;
	}
	// 血量平衡数值
	float healthRate = 2.0f;
	eGroundHealth /= healthRate;
	sGroundHealth /= healthRate;
	// 如果某一方没有空军/陆军，那么另一方的空军火力/陆军火力无效
	if (sGroundNum == 0) eGroundForce = 0.0f;
	if (eGroundNum == 0) sGroundForce = 0.0f;
	float groundOffset = (sGroundForce - eGroundHealth) - (eGroundForce - sGroundHealth);
	BWAPI::Broodwar->drawTextScreen(430, 280, "gOffset: %.2f", groundOffset);
	// 有一方劣势就该撤退
	return groundOffset < -0.1f;
}

bool Squad::airRetreat(BWAPI::Position center)
{
	BWAPI::Unitset ourCombatUnits;
	std::vector<UnitInfo> enemyCombatUnits;

	MapGrid::Instance().GetUnits(ourCombatUnits, center, Config::Micro::CombatRegroupRadius * 2, true, false);
	InformationManager::Instance().getNearbyForce(enemyCombatUnits, center, BWAPI::Broodwar->enemy(), Config::Micro::CombatRegroupRadius);

	// enemy force
	std::hash_map<int, float> airScore;
	int eAirNum = 0, sAirNum = 0;
	float eAirForce = 0, sAirForce = 0;
	float eAirHealth = 0, sAirHealth = 0;
	// enemy force and health
	for (const auto & eunit : enemyCombatUnits)
	{
		if (!InformationManager::Instance().isCombatUnit(eunit.type)) continue;
		auto type = eunit.type;
		int id = type.getID();
		if (airScore.find(id) == airScore.end())
		{
			airScore[id] = airForceScore(type);
		}
		// 血量会对力量打折扣
		eAirForce += airScore[id] * std::sqrtf(eunit.lastHealth / eunit.type.maxHitPoints());
		if (type.isFlyer())
		{
			++eAirNum;
			eAirHealth += eunit.lastHealth + eunit.lastShields + eunit.type.armor();
		}
	}
	int num = 0;
	for (const auto & score : airScore)
	{
		BWAPI::Broodwar->drawTextScreen(240, 220 + num * 10, "ea %s : %.2f", BWAPI::UnitType(score.first).getName().c_str(), score.second);
		++num;
	}
	// self force and health
	airScore.clear();
	for (const auto & sunit : ourCombatUnits)
	{
		if (!InformationManager::Instance().isCombatUnit(sunit->getType())) continue;
		auto type = sunit->getType();
		int id = type.getID();
		if (airScore.find(id) == airScore.end())
		{
			airScore[id] = airForceScore(type);
		}
		// 血量会对力量打折扣
		sAirForce += airScore[id] * std::sqrtf(sunit->getHitPoints() / sunit->getType().maxHitPoints());
		if (type.isFlyer())
		{
			++sAirNum;
			sAirHealth += sunit->getHitPoints() + sunit->getShields() + sunit->getType().armor();
		}
	}
	for (const auto & score : airScore)
	{
		BWAPI::Broodwar->drawTextScreen(240, 220 + num * 10, "sa %s : %.2f", BWAPI::UnitType(score.first).getName().c_str(), score.second);
		++num;
	}
	// 血量平衡数值
	float healthRate = 2.0f;
	eAirHealth /= healthRate;
	sAirHealth /= healthRate;
	// 如果某一方没有空军/陆军，那么另一方的空军火力/陆军火力无效
	if (sAirNum == 0) eAirForce = 0.0f;
	if (eAirNum == 0) sAirForce = 0.0f;
	float airOffset = (sAirForce - eAirHealth) - (eAirForce - sAirHealth);
	BWAPI::Broodwar->drawTextScreen(430, 280, "aOffset: %.2f", airOffset);
	// 有一方劣势就该撤退
	return airOffset < -0.1f;
}

// calculates whether or not to regroup
bool Squad::groundNeedsToRegroup()
{
	// if we are not attacking, never regroup
	if (_units.empty() || (_order.getType() != SquadOrderTypes::Attack))
	{
		_regroupStatus = std::string("\x04 No combat units available");
		return false;
	}

	BWAPI::Unit unitClosest = groundUnitClosestToEnemy();

	if (!unitClosest)
	{
		_regroupStatus = std::string("\x04 No closest unit");
		return false;
	}

	// if none of our units are in attack range of any enemy units, don't retreat

	bool gRetreat = groundRetreat(unitClosest->getPosition());

	int switchTime = 15;

	return gRetreat;
}

bool Squad::airNeedsToRegroup()
{
	// if we are not attacking, never regroup
	if (_units.empty() || (_order.getType() != SquadOrderTypes::Attack))
	{
		_regroupStatus = std::string("\x04 No combat units available");
		return false;
	}

	BWAPI::Unit unitClosest = airUnitClosestToEnemy();

	if (!unitClosest)
	{
		_regroupStatus = std::string("\x04 No closest unit");
		return false;
	}

	// if none of our units are in attack range of any enemy units, don't retreat

	bool aRetreat = airRetreat(unitClosest->getPosition());

	int switchTime = 15;

	return aRetreat;
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

BWAPI::Position Squad::calcGroundRegroupPosition()
{
	BWAPI::Position regroup(0, 0);

	int minDist = 100000;

	for (auto & unit : _units)
	{
		// 跳过飞行单位
		if (unit->isFlying()) continue;
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
		return BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	}
	else
	{
		return regroup;
	}
}

BWAPI::Position Squad::calcAirRegroupPosition()
{
	BWAPI::Position regroup(0, 0);

	int minDist = 100000;

	for (auto & unit : _units)
	{
		// 跳过地面单位
		if (!unit->isFlying()) continue;
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
		return BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	}
	else
	{
		return regroup;
	}
}

BWAPI::Unit Squad::groundUnitClosestToEnemy()
{
	BWAPI::Unit closest = nullptr;
	int closestDist = 100000;

	for (auto & unit : _units)
	{
		// 跳过飞行单位
		if (unit->isFlying()) continue;
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
			// 跳过飞行单位
			if (unit->isFlying()) continue;
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

BWAPI::Unit Squad::airUnitClosestToEnemy()
{
	BWAPI::Unit closest = nullptr;
	int closestDist = 100000;

	for (auto & unit : _units)
	{
		// 跳过地面单位
		if (!unit->isFlying()) continue;
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
			// 跳过地面单位
			if (!unit->isFlying()) continue;
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