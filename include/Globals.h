#pragma once

#include <unordered_set>
#include <string>
#include <memory>
#include "Reshade/reshade.hpp"
#include <spdlog/sinks/basic_file_sink.h>

struct TechniqueInfo
{
    std::string filename;
    std::string state;
    double startTime;
    double stopTime;
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

inline bool EnableMenus = true;
inline bool EnableTime = true;
inline bool EnableInterior = true;
inline bool EnableWeather = true;

inline std::unordered_set<std::string> g_MenuValue;
inline std::unordered_set<std::string> g_MenuToggleFile;
inline std::unordered_set<std::string> g_MenuToggleState;
inline std::unordered_set<std::string> g_TimeToggleFile;
inline std::unordered_set<std::string> g_TimeToggleState;
inline std::vector<TechniqueInfo> techniqueMenuInfoList;
inline std::vector<TechniqueInfo> techniqueTimeInfoList;

inline std::string ToggleStateMenus;
inline std::string ToggleAllStateMenus;
inline std::string ToggleStateTime;
inline std::string ToggleAllStateTime;

inline const char* itemMenuShaderToToggle;
inline const char* itemMenuStateValue;
inline const char* itemTimeShaderToToggle;
inline const char* itemTimeStateValue;
inline double itemTimeStartHour;
inline double itemTimeStopHour;

inline long TimeUpdateIntervall;

inline std::chrono::steady_clock::time_point lastTimeCalled;
inline std::chrono::steady_clock::time_point latestTime;
inline std::mutex timeMutex;
