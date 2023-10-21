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
	std::string Name = "";
	double startTime = 0.0;
	double stopTime = 0.0;
	bool enable = true;
};

struct Info
{
	std::string Index = "";
	std::string Name = "";
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
inline std::vector<std::string> g_Presets;

inline std::vector<std::string> g_EffectStateMenu = { "on", "off" };
inline std::vector<std::string> g_EffectStateTime = { "on", "off" };
inline std::vector<std::string> g_EffectStateInterior = { "on", "off" };
inline std::vector<std::string> g_EffectStateWeather = { "on", "off" };

inline std::vector<std::string> g_ToggleState = { "All", "Specific" };

inline std::string selectedPreset = "Default.ini";
inline std::string selectedPresetPath = "Data\\SKSE\\Plugins\\TogglerConfigs\\Default.ini";

inline std::vector<std::string> g_MenuNames;

inline std::vector<std::string> g_WeatherFlags = {
	"kNone",
	"kPleasant",
	"kCloudy",
	"kRainy",
	"kSnow",
	"kPermAurora",
	"kAuroraFollowsSun"
};

inline bool isLoaded = false;

// General
inline bool EnableMenus = true;
inline bool EnableTime = true;
inline bool EnableInterior = true;
inline bool EnableWeather = true;


// Menus
inline std::unordered_set<std::string> g_MenuToggleFile;
inline std::unordered_set<std::string> g_MenuToggleState;
inline std::unordered_set<std::string> g_MenuSpecificMenu;


inline std::vector<std::string> g_SpecificMenu;
inline std::vector<std::string> g_INImenus;
inline std::vector<TechniqueInfo> techniqueMenuInfoList;
inline std::vector<Info> menuList;

inline std::string ToggleStateMenus;
inline std::string ToggleAllStateMenus;

inline const char* itemMenuShaderToToggle;
inline const char* itemMenuStateValue;
inline const char* itemSpecificMenu;

// Time
inline std::unordered_set<std::string> g_TimeToggleFile;
inline std::unordered_set<std::string> g_TimeToggleState;

inline std::vector<TechniqueInfo> techniqueTimeInfoList;
inline std::vector<TechniqueInfo> techniqueTimeInfoListAll;
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

inline bool IsInInteriorCell = false;

//Weather
inline std::unordered_set<std::string> g_WeatherValue;
inline std::unordered_set<std::string> g_WeatherToggleFile;
inline std::unordered_set<std::string> g_WeatherToggleState;
inline std::unordered_set<std::string> g_WeatherSpecificWeather;

inline std::vector<std::string> g_SpecificWeather;
inline std::vector<std::string> g_INIweather;
inline std::vector<Info> weatherList;
inline std::vector<TechniqueInfo> techniqueWeatherInfoList;

inline std::string ToggleStateWeather;
inline std::string ToggleAllStateWeather;
inline std::string weatherflags;

inline const char* itemWeatherShaderToToggle;
inline const char* itemWeatherStateValue;
inline const char* itemSpecificWeather;

inline int TimeUpdateIntervalWeather;

// Thread
inline std::mutex timeMutexTime;
inline std::mutex vectorMutexTime;
inline std::mutex timeMutexInterior;
inline std::mutex timeMutexWeather;