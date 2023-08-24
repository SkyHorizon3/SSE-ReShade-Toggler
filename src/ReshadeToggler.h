#pragma once

#include "PCH.h"
#include <unordered_set>
#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "../include/reshade.hpp"

HMODULE g_hModule = nullptr;
static reshade::api::effect_runtime* s_pRuntime = nullptr;
std::shared_ptr<spdlog::logger> g_Logger;
std::unordered_set<std::string> g_MenuValue;
std::unordered_set<std::string> g_MenuGeneralValue01;
std::unordered_set<std::string> g_MenuGeneralValue02;


inline static bool EnableMenus = true;
//inline static bool EnableInterior = false;
//inline static bool EnableTime = false;
//inline static bool EnableWeather = false



std::string ToggleStateMenus;
std::string ToggleAllStateMenus;
const char* itemValuegeneral01;
const char* itemValuegeneral02;

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
    std::vector<std::string> m_Specific;
};