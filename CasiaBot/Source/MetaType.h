#pragma once

#include "Common.h"

namespace CasiaBot
{

namespace MetaTypes
{
    enum {Unit, Tech, Upgrade, Command, Default};
}

enum class MetaLocation
{
	Anywhere     // default location
	, Macro        // macro hatchery
	, Expo         // gas expansion hatchery
	, MinOnly      // any expansion hatchery (mineral-only or gas, whatever's next)
	, Hidden       // gas expansion hatchery far from both main bases
	, Main         // starting base
	, Natural      // "natural" first expansion base
};

class MetaCondition
{
	std::string _condition;

public:

	MetaCondition(const std::string & cond);

	bool isNone() const;
	bool isCancel() const;
	bool isMain() const;
	bool isMineral() const;
	bool isGas() const;
	bool isUnit() const;
	bool isPercent() const;

	int									getMineral() const;
	int									getGas() const;
	std::pair<BWAPI::UnitType, int>		getUnit() const;
	std::pair<BWAPI::UnitType, float>	getPercent() const;
};

class MetaType 
{
	size_t                  _type;
    BWAPI::Race             _race;

	BWAPI::UnitType         _unitType;
	BWAPI::TechType         _techType;
	BWAPI::UpgradeType      _upgradeType;
	MetaCondition			_condition;

public:

	MetaType ();
    MetaType (const std::string & name);
	MetaType (BWAPI::UnitType t, const std::string & cond = "");
	MetaType (BWAPI::TechType t, const std::string & cond = "");
	MetaType (BWAPI::UpgradeType t, const std::string & cond = "");

	static std::hash_map<std::string, BWAPI::UnitType>		_unitTypes;
	static std::hash_map<std::string, BWAPI::TechType>		_techTypes;
	static std::hash_map<std::string, BWAPI::UpgradeType>	_upgradeTypes;

	static void	initTypes();

	bool    isUnit()		const;
	bool    isTech()		const;
	bool    isUpgrade()	    const;
	bool    isCommand()	    const;
	bool    isBuilding()	const;
	bool    isRefinery()	const;
    
    const size_t & type() const;
    const BWAPI::Race & getRace() const;

    const BWAPI::UnitType & getUnitType() const;
    const BWAPI::TechType & getTechType() const;
    const BWAPI::UpgradeType & getUpgradeType() const;
	const MetaCondition & getCond() const;

	int     supplyRequired();
	int     mineralPrice()  const;
	int     gasPrice()      const;

	BWAPI::UnitType whatBuilds() const;
	std::string getName() const;
};
}