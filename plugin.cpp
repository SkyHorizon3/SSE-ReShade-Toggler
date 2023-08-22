
#include "PCH.h"
#include <unordered_set>
#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "include/reshade.hpp"

namespace logger = SKSE::log;

HMODULE g_hModule = nullptr;
static reshade::api::effect_runtime* s_pRuntime = nullptr;
std::shared_ptr<spdlog::logger> g_Logger;
std::vector<std::string> g_INImenus;
std::vector<std::string> g_menuValue;

class MyEffectRuntime : public reshade::api::effect_runtime
{
public:
    void set_effects_state(bool enabled) override
    {
        reshade::api::effect_runtime::set_effects_state(enabled);
    }
};

class EventProcessor : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    static EventProcessor& GetSingleton()
    {
        static EventProcessor singleton;
        return singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
    {
        const std::string_view menuName = event->menuName;
        bool opening = event->opening;
        if (opening)
        {
            m_OpenMenus.insert(menuName); // Mark menu as open
        }
        else
        {
            m_OpenMenus.erase(menuName); // Mark menu as closed
        }

        // Check if any open menu matches a menu in g_menuValue
        bool enableReshade = true;
        for (const std::string& menuToDisable : g_menuValue)
        {
            if (m_OpenMenus.count(menuToDisable) > 0)
            {
                enableReshade = false;
                break;
            }
        }

        if (s_pRuntime != nullptr)
        {
            logger::info("Menu {} {}", menuName, opening ? "open" : "closed");
            s_pRuntime->set_effects_state(enableReshade);
            logger::info("Reshade {}", enableReshade ? "disabled" : "enabled");
        }

        return RE::BSEventNotifyControl::kContinue;
    }

private:
    EventProcessor() = default;
    ~EventProcessor() = default;
    EventProcessor(const EventProcessor&) = delete;
    EventProcessor(EventProcessor&&) = delete;
    EventProcessor& operator=(const EventProcessor&) = delete;
    EventProcessor& operator=(EventProcessor&&) = delete;

    std::unordered_set<std::string_view> m_OpenMenus;

};

void MenusInINI()
{
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(L"Data\\SKSE\\Plugins\\ReshadeToggler.ini");

    const char* section = "Menus";

    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys(section, keys);

    for (const auto& key : keys)
    {
        g_INImenus.push_back(key.pItem);
        g_menuValue.push_back(ini.GetValue(section, key.pItem, nullptr));

        logger::info("Menu:  {} - Value: {}", g_INImenus.back(), g_menuValue.back());
    }
}

static void on_reshade_begin_effects(reshade::api::effect_runtime* runtime)
{
    s_pRuntime = runtime;
}

void register_addon_events()
{
    reshade::register_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
}

void unregister_addon_events()
{
    reshade::unregister_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
}

void Load()
{
    if (reshade::register_addon(g_hModule))
    {
        g_Logger->info("Registered addon");
        register_addon_events();
    }
    else {
        g_Logger->info("ReShade not present.");
    }
}

void SetupLog()
{
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder)
    {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    }

    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);

    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    g_Logger = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(g_Logger);
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = hModule;
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        unregister_addon_events();
        reshade::unregister_addon(hModule);
    }

    return TRUE;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    SetupLog();
    g_Logger->info("Loaded plugin");

    MenusInINI();

    auto& eventProcessor = EventProcessor::GetSingleton();
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessor);
    Load();

    return true;
}