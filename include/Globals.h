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
inline std::vector<std::string> g_State = { "on", "off" };

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

inline uint32_t TimeUpdateIntervalTime;


//Interior
inline std::unordered_set<std::string> g_InteriorToggleFile;
inline std::unordered_set<std::string> g_InteriorToggleState;

inline std::vector<TechniqueInfo> techniqueInteriorInfoList;
inline std::vector<std::string> g_SpecificInterior;

inline std::string ToggleStateInterior;
inline std::string ToggleAllStateInterior;

inline const char* itemInteriorShaderToToggle;
inline const char* itemInteriorStateValue;

inline uint32_t TimeUpdateIntervalInterior;

// Thread
inline std::mutex timeMutexTime;
inline std::mutex timeMutexInterior;
