#pragma once

#include "PCH.h"
#include <unordered_set>
#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "../include/reshade.hpp"

HMODULE g_hModule = nullptr;
static reshade::api::effect_runtime* s_pRuntime = nullptr;
std::shared_ptr<spdlog::logger> g_Logger;
std::unordered_set<std::string> g_menuValue;


class EventProcessor : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    static EventProcessor& GetSingleton()
    {
        static EventProcessor singleton;
        return singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

private:
    EventProcessor() = default;
    ~EventProcessor() = default;
    EventProcessor(const EventProcessor&) = delete;
    EventProcessor(EventProcessor&&) = delete;
    EventProcessor& operator=(const EventProcessor&) = delete;
    EventProcessor& operator=(EventProcessor&&) = delete;

    std::unordered_set<std::string_view> m_OpenMenus;
};

class ReshadeToggler
{
public:

    void SetupLog();
    void Load();
    void MenusInINI();

private:
    std::vector<std::string> m_INImenus;
};