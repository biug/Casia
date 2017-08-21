#include "ParseUtils.h"
#include "rapidjson\document.h"
#include "JSONTools.h"
#include "BuildOrder.h"
#include "StrategyManager.h"

using namespace CasiaBot;

void ParseUtils::ParseConfigFile(const std::string & filename)
{
    rapidjson::Document doc;
    BWAPI::Race race = BWAPI::Broodwar->self()->getRace();
	BWAPI::Race erace = BWAPI::Broodwar->enemy()->getRace();

    std::string config = FileUtils::ReadFile(filename);

    if (config.length() == 0)
    {
        return;
    }

    Config::ConfigFile::ConfigFileFound = true;

    bool parsingFailed = doc.Parse(config.c_str()).HasParseError();
    if (parsingFailed)
    {
        return;
    }

    // Parse the Bot Info
    if (doc.HasMember("Bot Info") && doc["Bot Info"].IsObject())
    {
        const rapidjson::Value & info = doc["Bot Info"];
        JSONTools::ReadString("BotName", info, Config::BotInfo::BotName);
        JSONTools::ReadString("Authors", info, Config::BotInfo::Authors);
        JSONTools::ReadBool("PrintInfoOnStart", info, Config::BotInfo::PrintInfoOnStart);
    }

    // Parse the BWAPI Options
    if (doc.HasMember("BWAPI") && doc["BWAPI"].IsObject())
    {
        const rapidjson::Value & bwapi = doc["BWAPI"];
        JSONTools::ReadInt("SetLocalSpeed", bwapi, Config::BWAPIOptions::SetLocalSpeed);
        JSONTools::ReadInt("SetFrameSkip", bwapi, Config::BWAPIOptions::SetFrameSkip);
        JSONTools::ReadBool("UserInput", bwapi, Config::BWAPIOptions::EnableUserInput);
        JSONTools::ReadBool("CompleteMapInformation", bwapi, Config::BWAPIOptions::EnableCompleteMapInformation);
    }

    // Parse the Micro Options
    if (doc.HasMember("Micro") && doc["Micro"].IsObject())
    {
        const rapidjson::Value & micro = doc["Micro"];
        JSONTools::ReadBool("UseSparcraftSimulation", micro, Config::Micro::UseSparcraftSimulation);
        JSONTools::ReadBool("KiteWithRangedUnits", micro, Config::Micro::KiteWithRangedUnits);
        JSONTools::ReadBool("WorkersDefendRush", micro, Config::Micro::WorkersDefendRush);
        JSONTools::ReadInt("RetreatMeleeUnitShields", micro, Config::Micro::RetreatMeleeUnitShields);
        JSONTools::ReadInt("RetreatMeleeUnitHP", micro, Config::Micro::RetreatMeleeUnitHP);
        JSONTools::ReadInt("InCombatRadius", micro, Config::Micro::CombatRadius);
        JSONTools::ReadInt("RegroupRadius", micro, Config::Micro::CombatRegroupRadius);
        JSONTools::ReadInt("UnitNearEnemyRadius", micro, Config::Micro::UnitNearEnemyRadius);
		JSONTools::ReadInt("ScoutRound", micro, Config::Micro::ScoutRound);

        if (micro.HasMember("KiteLongerRangedUnits") && micro["KiteLongerRangedUnits"].IsArray())
        {
            const rapidjson::Value & kite = micro["KiteLongerRangedUnits"];

            for (size_t i(0); i < kite.Size(); ++i)
            {
                if (kite[i].IsString())
                {
                    MetaType type(kite[i].GetString());
                    Config::Micro::KiteLongerRangedUnits.insert(type.getUnitType());
                }
            }
        }
    }

    // Parse the Macro Options
    if (doc.HasMember("Macro") && doc["Macro"].IsObject())
    {
        const rapidjson::Value & macro = doc["Macro"];
        JSONTools::ReadInt("BuildingSpacing", macro, Config::Macro::BuildingSpacing);
        JSONTools::ReadInt("PylongSpacing", macro, Config::Macro::PylonSpacing);
        JSONTools::ReadInt("WorkersPerRefinery", macro, Config::Macro::WorkersPerRefinery);
    }

    // Parse the Debug Options
    if (doc.HasMember("Debug") && doc["Debug"].IsObject())
    {
        const rapidjson::Value & debug = doc["Debug"];
        JSONTools::ReadString("ErrorLogFilename", debug, Config::Debug::ErrorLogFilename);
        JSONTools::ReadBool("LogAssertToErrorFile", debug, Config::Debug::LogAssertToErrorFile);
        JSONTools::ReadBool("DrawGameInfo", debug, Config::Debug::DrawGameInfo);
        JSONTools::ReadBool("DrawBuildOrderSearchInfo", debug, Config::Debug::DrawBuildOrderSearchInfo);
        JSONTools::ReadBool("DrawUnitHealthBars", debug, Config::Debug::DrawUnitHealthBars);
        JSONTools::ReadBool("DrawResourceInfo", debug, Config::Debug::DrawResourceInfo);
        JSONTools::ReadBool("DrawWorkerInfo", debug, Config::Debug::DrawWorkerInfo);
        JSONTools::ReadBool("DrawProductionInfo", debug, Config::Debug::DrawProductionInfo);
        JSONTools::ReadBool("DrawScoutInfo", debug, Config::Debug::DrawScoutInfo);
        JSONTools::ReadBool("DrawSquadInfo", debug, Config::Debug::DrawSquadInfo);
		JSONTools::ReadBool("DrawSquadTypeInfo", debug, Config::Debug::DrawSquadTypeInfo);
        JSONTools::ReadBool("DrawCombatSimInfo", debug, Config::Debug::DrawCombatSimulationInfo);
        JSONTools::ReadBool("DrawBuildingInfo", debug, Config::Debug::DrawBuildingInfo);
        JSONTools::ReadBool("DrawModuleTimers", debug, Config::Debug::DrawModuleTimers);
        JSONTools::ReadBool("DrawMouseCursorInfo", debug, Config::Debug::DrawMouseCursorInfo);
        JSONTools::ReadBool("DrawEnemyUnitInfo", debug, Config::Debug::DrawEnemyUnitInfo);
        JSONTools::ReadBool("DrawMapGrid", debug, Config::Debug::DrawMapGrid);
        JSONTools::ReadBool("DrawUnitTargetInfo", debug, Config::Debug::DrawUnitTargetInfo);
        JSONTools::ReadBool("DrawReservedBuildingTiles", debug, Config::Debug::DrawReservedBuildingTiles);
        JSONTools::ReadBool("PrintModuleTimeout", debug, Config::Debug::PrintModuleTimeout);
    }

    // Parse the Module Options
    if (doc.HasMember("Modules") && doc["Modules"].IsObject())
    {
        const rapidjson::Value & module = doc["Modules"];

        JSONTools::ReadBool("UseGameCommander", module, Config::Modules::UsingGameCommander);
        JSONTools::ReadBool("UseScoutManager", module, Config::Modules::UsingScoutManager);
        JSONTools::ReadBool("UseCombatCommander", module, Config::Modules::UsingCombatCommander);
        JSONTools::ReadBool("UseBuildOrderSearch", module, Config::Modules::UsingBuildOrderSearch);
        JSONTools::ReadBool("UseOpeningIO", module, Config::Modules::UsingOpeningIO);
        JSONTools::ReadBool("UseUnitCommandManager", module, Config::Modules::UsingUnitCommandManager);
        JSONTools::ReadBool("UseAutoObserver", module, Config::Modules::UsingAutoObserver);
    }

    // Parse the Tool Options
    if (doc.HasMember("Tools") && doc["Tools"].IsObject())
    {
        const rapidjson::Value & tool = doc["Tools"];

        JSONTools::ReadInt("MapGridSize", tool, Config::Tools::MAP_GRID_SIZE);
    }

    // Parse the Opening Options
    if (doc.HasMember("Opening") && doc["Opening"].IsObject())
    {
        const rapidjson::Value & opening = doc["Opening"];

        // read in the various strategic elements
        JSONTools::ReadBool("ScoutGasSteal", opening, Config::Opening::GasStealWithScout);
        JSONTools::ReadBool("ScoutHarassEnemy", opening, Config::Opening::ScoutHarassEnemy);
        JSONTools::ReadString("ReadDirectory", opening, Config::Opening::ReadDir);
        JSONTools::ReadString("WriteDirectory", opening, Config::Opening::WriteDir);

        // we should use zerg as our race
		CAB_ASSERT(race == BWAPI::Races::Zerg, "should choose zerg");
		std::string gameType = "Random";
		if (erace == BWAPI::Races::Terran)			gameType = "ZVT";
		else if (erace == BWAPI::Races::Protoss)	gameType = "ZVP";
		else if (erace == BWAPI::Races::Zerg)		gameType = "ZVZ";
		if (opening.HasMember(gameType.c_str()))
		{
			Config::Opening::OpeningName = opening[gameType.c_str()].GetString();
		}

        // check if we are using an enemy specific opening
        JSONTools::ReadBool("UseEnemySpecificOpening", opening, Config::Opening::UseEnemySpecificOpening);
        if (Config::Opening::UseEnemySpecificOpening && opening.HasMember("EnemySpecificOpening") && opening["EnemySpecificOpening"].IsObject())
        {
            const std::string enemyName = BWAPI::Broodwar->enemy()->getName();
            const rapidjson::Value & specific = opening["EnemySpecificOpening"];

            // check to see if our current enemy name is listed anywhere in the specific strategies
            if (specific.HasMember(enemyName.c_str()) && specific[enemyName.c_str()].IsString())
            {
                // if that enemy has a opening, use it
				Config::Opening::OpeningName = specific[enemyName.c_str()].GetString();
				Config::Opening::FoundEnemySpecificOpening = true;
            }
        }

        // Parse all the Strategies
        if (opening.HasMember("Openings") && opening["Openings"].IsObject())
        {
            const rapidjson::Value & openings = opening["Openings"];
            for (rapidjson::Value::ConstMemberIterator itr = openings.MemberBegin(); itr != openings.MemberEnd(); ++itr)
            {
                const std::string &         name = itr->name.GetString();
                const rapidjson::Value &    val  = itr->value;
				
				// must use specific game type
				if (gameType != "Random" && name.find(gameType) != 0) continue;

                BuildOrder buildOrder(race);
                if (val.HasMember("OpeningBuildOrder") && val["OpeningBuildOrder"].IsArray())
                {
                    const rapidjson::Value & build = val["OpeningBuildOrder"];

                    for (size_t b(0); b < build.Size(); ++b)
                    {
                        if (build[b].IsString())
                        {
                            MetaType type(build[b].GetString());

                            if (type.getRace() != BWAPI::Races::None)
                            {
                                buildOrder.add(type);
                            }
                        }
                        else
                        {
                            CAB_ASSERT_WARNING(false, "Build order item must be a string %s", name.c_str());
                            continue;
                        }
                    }
                }

                StrategyManager::Instance().addOpening(name, Opening(name, race, buildOrder));
            }
        }
    }

    Config::ConfigFile::ConfigFileParsed = true;
}


void ParseUtils::ParseTextCommand(const std::string & commandString)
{
    std::stringstream ss(commandString);

    std::string command;
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    std::string variableName;
    std::transform(variableName.begin(), variableName.end(), variableName.begin(), ::tolower);

    std::string val;

    ss >> command;
    ss >> variableName;
    ss >> val;

    if (command == "/set")
    {
        // BWAPI options
        if (variableName == "setlocalspeed") { Config::BWAPIOptions::SetLocalSpeed = GetIntFromString(val); BWAPI::Broodwar->setLocalSpeed(Config::BWAPIOptions::SetLocalSpeed); }
        else if (variableName == "setframeskip") { Config::BWAPIOptions::SetFrameSkip = GetIntFromString(val); BWAPI::Broodwar->setFrameSkip(Config::BWAPIOptions::SetFrameSkip); }
        else if (variableName == "userinput") { Config::BWAPIOptions::EnableUserInput = GetBoolFromString(val); if (Config::BWAPIOptions::EnableUserInput) BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput); }
        else if (variableName == "completemapinformation") { Config::BWAPIOptions::EnableCompleteMapInformation = GetBoolFromString(val); if (Config::BWAPIOptions::EnableCompleteMapInformation) BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput); }
        
        // Micro Options
        else if (variableName == "usesparcraftsimulation") { Config::Micro::UseSparcraftSimulation = GetBoolFromString(val); }
        else if (variableName == "workersdefendrush") { Config::Micro::WorkersDefendRush = GetBoolFromString(val); }
        else if (variableName == "incombatradius") { Config::Micro::CombatRadius = GetIntFromString(val); }
        else if (variableName == "regroupradius") { Config::Micro::CombatRegroupRadius = GetIntFromString(val); }
        else if (variableName == "unitnearenemyradius") { Config::Micro::UnitNearEnemyRadius = GetIntFromString(val); }

        // Macro Options
        else if (variableName == "buildingspacing") { Config::Macro::BuildingSpacing = GetIntFromString(val); }
        else if (variableName == "pylonspacing") { Config::Macro::PylonSpacing = GetIntFromString(val); }

        // Debug Options
        else if (variableName == "errorlogfilename") { Config::Debug::ErrorLogFilename = val; }
        else if (variableName == "printmoduletimeout") { Config::Debug::PrintModuleTimeout = GetBoolFromString(val); }
        else if (variableName == "drawbuildordersearchinfo") { Config::Debug::DrawBuildOrderSearchInfo = GetBoolFromString(val); }
        else if (variableName == "drawunithealthbars") { Config::Debug::DrawUnitHealthBars = GetBoolFromString(val); }
        else if (variableName == "drawproductioninfo") { Config::Debug::DrawProductionInfo = GetBoolFromString(val); }
        else if (variableName == "drawenemyunitinfo") { Config::Debug::DrawEnemyUnitInfo = GetBoolFromString(val); }
        else if (variableName == "drawmoduletimers") { Config::Debug::DrawModuleTimers = GetBoolFromString(val); }
        else if (variableName == "drawresourceinfo") { Config::Debug::DrawResourceInfo = GetBoolFromString(val); }
        else if (variableName == "drawcombatsiminfo") { Config::Debug::DrawCombatSimulationInfo = GetBoolFromString(val); }
        else if (variableName == "drawunittargetinfo") { Config::Debug::DrawUnitTargetInfo = GetBoolFromString(val); }
        else if (variableName == "drawmapgrid") { Config::Debug::DrawMapGrid = GetBoolFromString(val); }
        else if (variableName == "drawsquadinfo") { Config::Debug::DrawSquadInfo = GetBoolFromString(val); }
        else if (variableName == "drawworkerinfo") { Config::Debug::DrawWorkerInfo = GetBoolFromString(val); }
        else if (variableName == "drawmousecursorinfo") { Config::Debug::DrawMouseCursorInfo = GetBoolFromString(val); }
        else if (variableName == "drawbuildinginfo") { Config::Debug::DrawBuildingInfo = GetBoolFromString(val); }
        else if (variableName == "drawreservedbuildingtiles") { Config::Debug::DrawReservedBuildingTiles = GetBoolFromString(val); }

        // Module Options
        else if (variableName == "usegamecommander") { Config::Modules::UsingGameCommander = GetBoolFromString(val); }
        else if (variableName == "usescoutmanager") { Config::Modules::UsingScoutManager = GetBoolFromString(val); }
        else if (variableName == "usecombatcommander") { Config::Modules::UsingCombatCommander = GetBoolFromString(val); }
        else if (variableName == "usebuildordersearch") { Config::Modules::UsingBuildOrderSearch = GetBoolFromString(val); }
        else if (variableName == "useautoobserver") { Config::Modules::UsingAutoObserver = GetBoolFromString(val); }
        else if (variableName == "useopeningio") { Config::Modules::UsingOpeningIO = GetBoolFromString(val); }
        else if (variableName == "useunitcommandmanager") { Config::Modules::UsingUnitCommandManager = GetBoolFromString(val); }

        else { CAB_ASSERT_WARNING(false, "Unknown variable name for /set: %s", variableName.c_str()); }
    }
    else
    {
        CAB_ASSERT_WARNING(false, "Unknown command: %s", command.c_str());
    }
}

BWAPI::Race ParseUtils::GetRace(const std::string & raceName)
{
    if (raceName == "Protoss")
    {
        return BWAPI::Races::Protoss;
    }

    if (raceName == "Terran")
    {
        return BWAPI::Races::Terran;
    }

    if (raceName == "Zerg")
    {
        return BWAPI::Races::Zerg;
    }

    if (raceName == "Random")
    {
        return BWAPI::Races::Random;
    }

    CAB_ASSERT_WARNING(false, "Race not found: %s", raceName.c_str());
    return BWAPI::Races::None;
}

int ParseUtils::GetIntFromString(const std::string & str)
{
    std::stringstream ss(str);
    int a = 0;
    ss >> a;
    return a;
}

bool ParseUtils::GetBoolFromString(const std::string & str)
{
    std::string boolStr(str);
    std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);

    if (boolStr == "true" || boolStr == "t")
    {
        return true;
    }
    else if (boolStr == "false" || boolStr == "f")
    {
        return false;
    }
    else
    {
        CAB_ASSERT_WARNING(false, "Unknown bool from string: %s", str.c_str());
    }

    return false;
}