#include "Config.h"
#include "CABAssert.h"

namespace Config
{
    namespace ConfigFile
    {
        bool ConfigFileFound                = false;
        bool ConfigFileParsed               = false;
		std::string ConfigFileLocation = "bwapi-data/AI/CasiaBot_Config.txt";
    }

    namespace Opening
    {
        std::string OpeningName            = "ZVP_9D";
        std::string ReadDir                 = "bwapi-data/read/";
        std::string WriteDir                = "bwapi-data/write/";
        bool GasStealWithScout              = false;
        bool ScoutHarassEnemy               = true;
        bool UseEnemySpecificOpening       = false;
        bool FoundEnemySpecificOpening     = false;
    }

    namespace Modules							    
    {
        // the default tournament bot modules
        bool UsingGameCommander             = true;     // toggle GameCommander, effectively CasiaBot
        bool UsingScoutManager              = true;
        bool UsingCombatCommander           = true;
        bool UsingBuildOrderSearch          = true;     // toggle use of Build Order Search, currently no backup
        bool UsingAutoObserver              = false;
        bool UsingOpeningIO					= true;    // toggle the use of file io for strategy
        bool UsingUnitCommandManager        = false;    // handles all unit commands
		
        // extra things, don't enable unless you know what they are
        bool UsingBuildOrderDemo            = false;
    }

    namespace BotInfo
    {
        std::string BotName                 = "CasiaBot";
        std::string Authors                 = "Dave Churchill";
        bool PrintInfoOnStart               = false;
    }

    namespace BWAPIOptions
    {
        int SetLocalSpeed                   = 42;
        int SetFrameSkip                    = 0;
        bool EnableUserInput                = true;
        bool EnableCompleteMapInformation   = false;
    }
    
    namespace Tournament						
    {
        int GameEndFrame                    = 86400;	
    }
    
    namespace Debug								
    {
        bool DrawGameInfo                   = true;
        bool DrawUnitHealthBars             = true;
        bool DrawProductionInfo             = true;
        bool DrawBuildOrderSearchInfo       = false;
        bool DrawScoutInfo                  = false;
        bool DrawResourceInfo               = true;
        bool DrawWorkerInfo                 = true;
        bool DrawModuleTimers               = false;
        bool DrawReservedBuildingTiles      = false;
        bool DrawCombatSimulationInfo       = false;
        bool DrawBuildingInfo               = false;
        bool DrawMouseCursorInfo            = false;
        bool DrawEnemyUnitInfo              = false;
        bool DrawBWTAInfo                   = false;
        bool DrawMapGrid                    = false;
        bool DrawUnitTargetInfo             = false;
        bool DrawSquadInfo                  = false;
        bool PrintModuleTimeout             = false;	

        std::string ErrorLogFilename        = "CAB_ErrorLog.txt";
        bool LogAssertToErrorFile           = false;

        BWAPI::Color ColorLineTarget        = BWAPI::Colors::White;
        BWAPI::Color ColorLineMineral       = BWAPI::Colors::Cyan;
        BWAPI::Color ColorUnitNearEnemy     = BWAPI::Colors::Red;
        BWAPI::Color ColorUnitNotNearEnemy  = BWAPI::Colors::Green;
    }

    namespace Micro								
    {
        bool UseSparcraftSimulation         = true;
        bool KiteWithRangedUnits            = true;
        std::set<BWAPI::UnitType> KiteLongerRangedUnits;
        bool WorkersDefendRush              = false; 
		int RetreatMeleeUnitShields         = 0;
        int RetreatMeleeUnitHP              = 0;
        int CombatRadius                    = 1000;     // radius of combat to consider units for Micro Search
        int CombatRegroupRadius             = 300;      // radius of units around frontmost unit we consider in regroup calculation
        int UnitNearEnemyRadius             = 600;      // radius to consider a unit 'near' to an enemy unit
    }

    namespace Macro
    {
        int WorkersPerRefinery              = 3;
        int WorkersPerMineralPatch          = 3;
        int BuildingSpacing                 = 1;
        int PylonSpacing                    = 3;
    }

    namespace Tools								
    {
        extern int MAP_GRID_SIZE            = 320;      // size of grid spacing in MapGrid
    }
}