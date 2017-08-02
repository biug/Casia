/* 
 +----------------------------------------------------------------------+
 | CasiaBot                                                          |
 +----------------------------------------------------------------------+
 | University of Alberta - AIIDE StarCraft Competition                  |
 +----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------+
 | Author: David Churchill <dave.churchill@gmail.com>                   |
 +----------------------------------------------------------------------+
*/

#include "Common.h"
#include "CasiaBotModule.h"
#include "JSONTools.h"
#include "ParseUtils.h"
#include "UnitUtil.h"

using namespace CasiaBot;

// This gets called when the bot starts!
void CasiaBotModule::onStart()
{
    // Initialize SparCraft, the combat simulation package
    SparCraft::init();

    // Parse the bot's configuration file if it has one, change this file path to where your config file is
    // Any relative path name will be relative to Starcraft installation folder
	MetaType::initTypes();
    ParseUtils::ParseConfigFile(Config::ConfigFile::ConfigFileLocation);

    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(Config::BWAPIOptions::SetLocalSpeed);
	BWAPI::Broodwar->setFrameSkip(Config::BWAPIOptions::SetFrameSkip);
    
    if (Config::BWAPIOptions::EnableCompleteMapInformation)
    {
        BWAPI::Broodwar->enableFlag(BWAPI::Flag::CompleteMapInformation);
    }

    if (Config::BWAPIOptions::EnableUserInput)
    {
        BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);
    }

    if (Config::BotInfo::PrintInfoOnStart)
    {
        BWAPI::Broodwar->printf("Hello! I am %s, written by %s", Config::BotInfo::BotName.c_str(), Config::BotInfo::Authors.c_str());
    }

    // Call BWTA to read and analyze the current map
    if (Config::Modules::UsingGameCommander)
	{
		//BWTA::readMap();
		BWTA::analyze();

        if (Config::Modules::UsingOpeningIO)
        {
            StrategyManager::Instance().readOpeningResults();
            StrategyManager::Instance().setLearnedOpening();
        }
	}

	try
	{
		BWEM::Map & theMap = BWEM::Map::Instance();
		theMap.Initialize();
		theMap.Initialize();
		theMap.EnableAutomaticPathAnalysis();
		bool startingLocationsOK = theMap.FindBasesForStartingLocations();
		assert(startingLocationsOK);

		BWEM::utils::MapPrinter::Initialize(&theMap);
		BWEM::utils::printMap(theMap);      // will print the map into the file <StarCraftFolder>bwapi-data/map.bmp
		BWEM::utils::pathExample(theMap);   // add to the printed map a path between two starting locations
	}
	catch (const std::exception & e)
	{
		BWAPI::Broodwar->printf("Exception %s", e.what());
	}
}

void CasiaBotModule::onEnd(bool isWinner) 
{
	if (Config::Modules::UsingGameCommander)
	{
		StrategyManager::Instance().onEnd(isWinner);
	}	
}

void CasiaBotModule::onFrame()
{
    char red = '\x08';
    char green = '\x07';
    char white = '\x04';

    if (!Config::ConfigFile::ConfigFileFound)
    {
        BWAPI::Broodwar->drawBoxScreen(0,0,450,100, BWAPI::Colors::Black, true);
        BWAPI::Broodwar->setTextSize(BWAPI::Text::Size::Huge);
        BWAPI::Broodwar->drawTextScreen(10, 5, "%c%s Config File Not Found", red, Config::BotInfo::BotName.c_str());
        BWAPI::Broodwar->setTextSize(BWAPI::Text::Size::Default);
        BWAPI::Broodwar->drawTextScreen(10, 30, "%c%s will not run without its configuration file", white, Config::BotInfo::BotName.c_str());
        BWAPI::Broodwar->drawTextScreen(10, 45, "%cCheck that the file below exists. Incomplete paths are relative to Starcraft directory", white);
        BWAPI::Broodwar->drawTextScreen(10, 60, "%cYou can change this file location in Config::ConfigFile::ConfigFileLocation", white);
        BWAPI::Broodwar->drawTextScreen(10, 75, "%cFile Not Found (or is empty): %c %s", white, green, Config::ConfigFile::ConfigFileLocation.c_str());
        return;
    }
    else if (!Config::ConfigFile::ConfigFileParsed)
    {
        BWAPI::Broodwar->drawBoxScreen(0,0,450,100, BWAPI::Colors::Black, true);
        BWAPI::Broodwar->setTextSize(BWAPI::Text::Size::Huge);
        BWAPI::Broodwar->drawTextScreen(10, 5, "%c%s Config File Parse Error", red, Config::BotInfo::BotName.c_str());
        BWAPI::Broodwar->setTextSize(BWAPI::Text::Size::Default);
        BWAPI::Broodwar->drawTextScreen(10, 30, "%c%s will not run without a properly formatted configuration file", white, Config::BotInfo::BotName.c_str());
        BWAPI::Broodwar->drawTextScreen(10, 45, "%cThe configuration file was found, but could not be parsed. Check that it is valid JSON", white);
        BWAPI::Broodwar->drawTextScreen(10, 60, "%cFile Not Parsed: %c %s", white, green, Config::ConfigFile::ConfigFileLocation.c_str());
        return;
    }

	if (Config::Modules::UsingGameCommander) 
	{ 
		_gameCommander.update(); 
	}

    if (Config::Modules::UsingAutoObserver)
    {
        _autoObserver.onFrame();
    }
}

void CasiaBotModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (Config::Modules::UsingGameCommander) { _gameCommander.onUnitDestroy(unit); }
}

void CasiaBotModule::onUnitMorph(BWAPI::Unit unit)
{
	if (Config::Modules::UsingGameCommander) { _gameCommander.onUnitMorph(unit); }
}

void CasiaBotModule::onSendText(std::string text) 
{ 
	ParseUtils::ParseTextCommand(text);
}

void CasiaBotModule::onUnitCreate(BWAPI::Unit unit)
{ 
	if (Config::Modules::UsingGameCommander) { _gameCommander.onUnitCreate(unit); }
}

void CasiaBotModule::onUnitComplete(BWAPI::Unit unit)
{
	if (Config::Modules::UsingGameCommander) { _gameCommander.onUnitComplete(unit); }
}

void CasiaBotModule::onUnitShow(BWAPI::Unit unit)
{ 
	if (Config::Modules::UsingGameCommander) { _gameCommander.onUnitShow(unit); }
}

void CasiaBotModule::onUnitHide(BWAPI::Unit unit)
{ 
	if (Config::Modules::UsingGameCommander) { _gameCommander.onUnitHide(unit); }
}

void CasiaBotModule::onUnitRenegade(BWAPI::Unit unit)
{ 
	if (Config::Modules::UsingGameCommander) { _gameCommander.onUnitRenegade(unit); }
}