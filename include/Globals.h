#pragma once

#include <unordered_set>
#include <string>
#include <memory>
#include "Reshade/reshade.hpp"
#include <spdlog/sinks/basic_file_sink.h>

struct TechniqueInfo
{
	std::string filename = "";
	std::string state = "";
	double startTime = 0.0;
	double stopTime = 0.0;
};

struct Menus
{
	std::string menuIndex = "";
	std::string menuName = "";
};

enum class Categories
{
	Menu,
	Time,
	Weather,
	Interior
};

inline HMODULE g_hModule = nullptr;
extern reshade::api::effect_runtime* s_pRuntime;
inline std::shared_ptr<spdlog::logger> g_Logger;

inline std::vector<std::string> g_Effects;
inline std::vector<std::string> g_EffectState = { "on", "off" };
inline std::vector<std::string> g_ToggleState = { "All", "Specific" };

// Hard coding probably isn't a good idea... idgaf rn tho, migh fix later
// TODO: Don't Hardcode
inline std::vector<std::string> g_MenuNames = {
	"Tutorial Menu",
	"TweenMenu",
	"BarterMenu",
	"Book Menu",
	"Console",
	"Console Native UI Menu",
	"ContainerMenu",
	"Crafting Menu",
	"Creation Club Menu",
	"Credits Menu",
	"Cursor Menu",
	"Dialogue Menu",
	"Fader Menu",
	"FavoritesMenu",
	"GiftMenu",
	"HUD Menu",
	"InventoryMenu",
	"Journal Menu",
	"Kinect Menu",
	"LevelUp Menu",
	"Loading Menu",
	"LoadWaitSpinner",
	"Lockpicking Menu",
	"MagicMenu",
	"Main Menu",
	"MapMenu",
	"MessageBoxMenu",
	"Mist Menu",
	"Mod Manager Menu",
	"RaceSex Menu",
	"SafeZoneMenu",
	"Sleep/Wait Menu",
	"StatsMenu",
	"TitleSequence Menu",
	"Training Menu"
};

inline bool loaded = false;

// General
inline bool EnableMenus = true;
inline bool EnableTime = true;
inline bool EnableInterior = true;
inline bool EnableWeather = true;


// Menus
inline std::unordered_set<std::string> g_MenuValue;
inline std::unordered_set<std::string> g_MenuToggleFile;
inline std::unordered_set<std::string> g_MenuToggleState;

inline std::vector<std::string> g_SpecificMenu;
inline std::vector<std::string> g_INImenus;
inline std::vector<TechniqueInfo> techniqueMenuInfoList;
inline std::vector<Menus> menuList;

inline std::string ToggleStateMenus;
inline std::string ToggleAllStateMenus;

inline const char* itemMenuShaderToToggle;
inline const char* itemMenuStateValue;


// Time
inline std::unordered_set<std::string> g_TimeToggleFile;
inline std::unordered_set<std::string> g_TimeToggleState;

inline std::vector<TechniqueInfo> techniqueTimeInfoList;
inline std::vector<std::string> g_SpecificTime;

inline std::string ToggleStateTime;
inline std::string ToggleAllStateTime;

inline const char* itemTimeShaderToToggle;
inline const char* itemTimeStateValue;

inline double itemTimeStartHour;
inline double itemTimeStopHour;
inline double itemTimeStartHourAll;
inline double itemTimeStopHourAll;

inline int TimeUpdateIntervalTime;


//Interior
inline std::unordered_set<std::string> g_InteriorToggleFile;
inline std::unordered_set<std::string> g_InteriorToggleState;

inline std::vector<TechniqueInfo> techniqueInteriorInfoList;
inline std::vector<std::string> g_SpecificInterior;

inline std::string ToggleStateInterior;
inline std::string ToggleAllStateInterior;

inline const char* itemInteriorShaderToToggle;
inline const char* itemInteriorStateValue;

inline int TimeUpdateIntervalInterior;

// Thread
inline std::mutex timeMutexTime;
inline std::mutex vectorMutexTime;
inline std::mutex timeMutexInterior;