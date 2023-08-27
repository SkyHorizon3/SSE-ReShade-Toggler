#pragma once

#include "PCH.h"
#include <unordered_set>
#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "../include/reshade.hpp"


struct TechniqueInfo;

HMODULE g_hModule = nullptr;
static reshade::api::effect_runtime* s_pRuntime = nullptr;
std::shared_ptr<spdlog::logger> g_Logger;
std::unordered_set<std::string> g_MenuValue;
std::unordered_set<std::string> g_MenuToggleFile;
std::unordered_set<std::string> g_MenuToggleState;
std::unordered_set<std::string> g_TimeToggleFile;
std::unordered_set<std::string> g_TimeToggleState;
std::vector<TechniqueInfo> techniqueInfoList;


inline static bool EnableMenus = true;
inline static bool EnableTime = true;
inline static bool EnableInterior = true;
inline static bool EnableWeather = true;



std::string ToggleStateMenus;
std::string ToggleAllStateMenus;
std::string ToggleStateTime;
std::string ToggleAllStateTime;
const char* itemShaderToToggle;
const char* itemMenuStateValue;
const char* itemValueTimeGeneral01;
const char* itemValueTimeGeneral02;

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

class ReshadeToggler
{
public:

    void SetupLog();
    void Load();
    void MenusInINI();

private:
    std::vector<std::string> m_INImenus;
    std::vector<std::string> m_SpecificMenu;
    std::vector<std::string> m_SpecificTime;
};

struct TechniqueInfo
{
    std::string filename;
    std::string state;
};