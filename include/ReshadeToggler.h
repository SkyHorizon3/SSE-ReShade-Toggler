#pragma once

#include "PCH.h"
#include <unordered_set>
#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "../include/Reshade/reshade.hpp"


struct TechniqueInfo
{
    std::string filename;
    std::string state;
    double startTime;
    double stopTime;
};

HMODULE g_hModule = nullptr;
static reshade::api::effect_runtime* s_pRuntime = nullptr;
std::shared_ptr<spdlog::logger> g_Logger;

inline static bool EnableMenus = true;
inline static bool EnableTime = true;
inline static bool EnableInterior = true;
inline static bool EnableWeather = true;

std::unordered_set<std::string> g_MenuValue;
std::unordered_set<std::string> g_MenuToggleFile;
std::unordered_set<std::string> g_MenuToggleState;
std::unordered_set<std::string> g_TimeToggleFile;
std::unordered_set<std::string> g_TimeToggleState;
std::vector<TechniqueInfo> techniqueMenuInfoList;
std::vector<TechniqueInfo> techniqueTimeInfoList;

std::string ToggleStateMenus;
std::string ToggleAllStateMenus;
std::string ToggleStateTime;
std::string ToggleAllStateTime;

const char* itemMenuShaderToToggle;
const char* itemMenuStateValue;
const char* itemTimeShaderToToggle;
const char* itemTimeStateValue;
double itemTimeStartHour;
double itemTimeStopHour;

enum class Categories
{
    Menu,
    Time,
    Weather,
    Interior
};

class EventProcessorMenu : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    static EventProcessorMenu& GetSingleton()
    {
        static EventProcessorMenu singleton;
        return singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;


private:
    EventProcessorMenu() = default;
    ~EventProcessorMenu() = default;
    EventProcessorMenu(const EventProcessorMenu&) = delete;
    EventProcessorMenu(EventProcessorMenu&&) = delete;
    EventProcessorMenu& operator=(const EventProcessorMenu&) = delete;
    EventProcessorMenu& operator=(EventProcessorMenu&&) = delete;

    std::unordered_set<std::string> m_OpenMenus;
};

class ReshadeToggler : public Singleton<ReshadeToggler>
{
public:

    void SetupLog();
    void Load();
    void LoadINI();

    void MessageListener(SKSE::MessagingInterface::Message* message);

    RE::BSEventNotifyControl ProcessTimeBasedToggling();

    static void ApplyReshadeState(bool enableReshade, const std::string& toggleState);
    static void ApplySpecificReshadeStates(bool enableReshade, Categories ProcessState);
    static void ApplyTechniqueState(bool enableReshade, const TechniqueInfo& info);

private:
    bool IsTimeWithinRange(double currentTime, double startTime, double endTime);

    std::vector<std::string> m_SpecificMenu;
    std::vector<std::string> m_SpecificTime;
    std::vector<std::string> m_INImenus;
};