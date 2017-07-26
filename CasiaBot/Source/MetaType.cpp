#include "MetaType.h"
#include <numeric>

using namespace CasiaBot;

MetaCondition::MetaCondition(const std::string & cond)
	: _condition(cond)
{

}

bool MetaCondition::isNone() const
{
	return _condition.empty();
}

bool MetaCondition::isCancel() const
{
	return _condition.compare("Cancel") == 0;
}

bool MetaCondition::isMain() const
{
	return _condition.compare("Main") == 0;
}

bool MetaCondition::isMineral() const
{
	auto itr = std::find_if_not(_condition.begin(), _condition.end(), [](char c) { return isdigit(c); });
	return !_condition.empty() && itr == _condition.end() - 1 && *itr == 'M';
}

bool MetaCondition::isGas() const
{
	auto itr = std::find_if_not(_condition.begin(), _condition.end(), [](char c) { return isdigit(c); });
	return !_condition.empty() && itr == _condition.end() - 1 && *itr == 'G';
}

bool MetaCondition::isUnit() const
{
	auto itr = std::find_if_not(_condition.begin(), _condition.end(), [](char c) { return isdigit(c); });
	if (itr == _condition.begin() || itr == _condition.end()) return false;
	auto type = _condition.substr(itr - _condition.begin());
	return MetaType::_unitTypes.find(type) != MetaType::_unitTypes.end();
}

bool MetaCondition::isPercent() const
{
	auto itr = std::find_if_not(_condition.begin(), _condition.end(), [](char c) { return isdigit(c); });
	if (itr == _condition.begin() || itr == _condition.end() || *itr != '%') return false;
	auto type = _condition.substr(itr + 1 - _condition.begin());
	return MetaType::_unitTypes.find(type) != MetaType::_unitTypes.end();
}

int MetaCondition::getMineral() const
{
	return std::accumulate<std::string::const_iterator, int>
		(_condition.begin(), _condition.end() - 1, 0, [](int high, int low) {return high * 10 + low - '0'; });
}

int MetaCondition::getGas() const
{
	return std::accumulate<std::string::const_iterator, int>
		(_condition.begin(), _condition.end() - 1, 0, [](int high, int low) {return high * 10 + low - '0'; });
}

std::pair<BWAPI::UnitType, int> MetaCondition::getUnit() const
{
	auto itr = std::find_if_not(_condition.begin(), _condition.end(), [](char c) { return isdigit(c); });
	int num = std::accumulate<std::string::const_iterator, int>
		(_condition.begin(), itr, 0, [](int high, int low) {return high * 10 + low - '0'; });
	auto type = MetaType::_unitTypes.at(_condition.substr(itr - _condition.begin()));
	return { type, num };
}

std::pair<BWAPI::UnitType, float> MetaCondition::getPercent() const
{
	auto itr = std::find_if_not(_condition.begin(), _condition.end(), [](char c) { return isdigit(c); });
	int num = std::accumulate<std::string::const_iterator, int>
		(_condition.begin(), itr, 0, [](int high, int low) {return high * 10 + low - '0'; });
	auto type = MetaType::_unitTypes.at(_condition.substr(itr + 1 - _condition.begin()));
	return { type, (float)num / 100.0 };
}

std::hash_map<std::string, BWAPI::UnitType>		MetaType::_unitTypes;
std::hash_map<std::string, BWAPI::TechType>		MetaType::_techTypes;
std::hash_map<std::string, BWAPI::UpgradeType>	MetaType::_upgradeTypes;

MetaType::MetaType()
	: _type(MetaTypes::Default)
	, _race(BWAPI::Races::None)
	, _condition("")
{
}

MetaType::MetaType(const std::string & name)
    : _type(MetaTypes::Default) 
    , _race(BWAPI::Races::None)
	, _condition("")
{
    std::string inputName(name);
	std::string condition = "";
	std::size_t condPos = inputName.find('@');
	if (condPos != std::string::npos)
	{
		condition = inputName.substr(condPos + 1);
		inputName = inputName.substr(0, condPos);
	}

	if (_unitTypes.find(inputName) != _unitTypes.end())
	{
		*this = MetaType(_unitTypes.at(inputName), condition);
		return;
	}
	else if (_techTypes.find(inputName) != _techTypes.end())
	{
		*this = MetaType(_techTypes.at(inputName), condition);
		return;
	}
	else if (_upgradeTypes.find(inputName) != _upgradeTypes.end())
	{
		*this = MetaType(_upgradeTypes.at(inputName), condition);
		return;
	}

    CAB_ASSERT_WARNING(false, "Could not find MetaType with name: %s", name.c_str());
}

MetaType::MetaType (BWAPI::UnitType t, const std::string & cond)
    : _unitType(t)
    , _type(MetaTypes::Unit) 
    , _race(t.getRace())
	, _condition(cond)
{
}

MetaType::MetaType (BWAPI::TechType t, const std::string & cond)
    : _techType(t)
    , _type(MetaTypes::Tech) 
    , _race(t.getRace())
	, _condition(cond)
{
}

MetaType::MetaType (BWAPI::UpgradeType t, const std::string & cond)
    : _upgradeType(t)
    , _type(MetaTypes::Upgrade) 
    , _race(t.getRace())
	, _condition(cond)
{
}

void MetaType::initTypes()
{
	for (const auto & unitType : BWAPI::UnitTypes::allUnitTypes())
	{
		std::string name = unitType.getName();
		std::string race = unitType.getRace().getName();
		if (name.find(race) == 0 && name != race) name = name.substr(race.length() + 1);
		MetaType::_unitTypes.insert({ name, unitType });
	}
	for (const auto & techType : BWAPI::TechTypes::allTechTypes())
	{
		MetaType::_techTypes.insert({ techType.getName(), techType });
	}
	for (const auto & upgradeType : BWAPI::UpgradeTypes::allUpgradeTypes())
	{
		MetaType::_upgradeTypes.insert({ upgradeType.getName(), upgradeType });
	}
}

const size_t & MetaType::type() const
{
    return _type;
}

bool MetaType::isUnit() const 
{
    return _type == MetaTypes::Unit; 
}

bool MetaType::isTech() const 
{ 
    return _type == MetaTypes::Tech; 
}

bool MetaType::isUpgrade() const 
{ 
    return _type == MetaTypes::Upgrade; 
}

bool MetaType::isCommand() const 
{ 
    return _type == MetaTypes::Command; 
}

const BWAPI::Race & MetaType::getRace() const
{
    return _race;
}

bool MetaType::isBuilding()	const 
{ 
    return _type == MetaTypes::Unit && _unitType.isBuilding(); 
}

bool MetaType::isRefinery()	const 
{ 
    return isBuilding() && _unitType.isRefinery(); 
}

const BWAPI::UnitType & MetaType::getUnitType() const
{
    return _unitType;
}

const BWAPI::TechType & MetaType::getTechType() const
{
    return _techType;
}

const BWAPI::UpgradeType & MetaType::getUpgradeType() const
{
    return _upgradeType;
}

const MetaCondition & MetaType::getCond() const
{
	return _condition;
}

int MetaType::supplyRequired()
{
	if (isUnit())
	{
		return _unitType.supplyRequired();
	}
	else
	{
		return 0;
	}
}

int MetaType::mineralPrice() const
{
	return isUnit() ? _unitType.mineralPrice() : (isTech() ? _techType.mineralPrice() : _upgradeType.mineralPrice());
}

int MetaType::gasPrice() const
{
	return isUnit() ? _unitType.gasPrice() : (isTech() ? _techType.gasPrice() : _upgradeType.gasPrice());
}

BWAPI::UnitType MetaType::whatBuilds() const
{
	return isUnit() ? _unitType.whatBuilds().first : (isTech() ? _techType.whatResearches() : _upgradeType.whatUpgrades());
}

std::string MetaType::getName() const
{
	if (isUnit())
	{
		return _unitType.getName();
	}
	else if (isTech())
	{
		return _techType.getName();
	}
	else if (isUpgrade())
	{
		return _upgradeType.getName();
	}
	else
	{
		CAB_ASSERT(false, "MetaType not found");
		return "LOL";	
	}
}