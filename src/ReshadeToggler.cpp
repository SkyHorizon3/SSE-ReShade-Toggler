#include "ReshadeToggler.h"

namespace logger = SKSE::log;

class EffectRuntime : public reshade::api::effect_runtime
{
public:
    void set_effects_state(bool enabled) override
    {
        reshade::api::effect_runtime::set_effects_state(enabled);
    }
};

// Callback when Reshade begins effects
static void on_reshade_begin_effects(reshade::api::effect_runtime* runtime)
{
    s_pRuntime = runtime;
}

// Register and unregister addon events
void register_addon_events()
{
    reshade::register_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
}

void unregister_addon_events()
{
    reshade::unregister_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
}

// Process menu open/close events
RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::MenuOpenCloseEvent* event,
    RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{
    const auto& menuName = event->menuName;
    auto& opening = event->opening;

    auto [it, inserted] = m_OpenMenus.emplace(menuName);

    if (!opening)
    {
        m_OpenMenus.erase(it); // Mark menu as closed using the iterator
    }

    if (m_OpenMenus.empty()) {
        return RE::BSEventNotifyControl::kContinue; // Skip if no open menus
    }

    bool enableReshade = [this]() {
        for (const auto& menuToDisable : g_MenuValue)
        {
            if (m_OpenMenus.find(menuToDisable) != m_OpenMenus.end())
            {
                return false; // If any disabled menu is open, disable Reshade
            }
        }
        return true; // If no disabled menus are open, enable Reshade
    }();

        if (s_pRuntime != nullptr)
        {
            s_pRuntime->set_effects_state(enableReshade);
#if _DEBUG
            g_Logger->info("Menu {} {}", menuName, opening ? "open" : "closed");
            g_Logger->info("Reshade {}", enableReshade ? "disabled" : "enabled");
#endif
        }

        return RE::BSEventNotifyControl::kContinue;
}

// Setup logger for plugin
void ReshadeToggler::SetupLog()
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

// Load menu values from INI
void ReshadeToggler::MenusInINI()
{
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(L"Data\\SKSE\\Plugins\\ReshadeToggler.ini");

    const char* section = "Menus";
    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys(section, keys);

    m_INImenus.reserve(keys.size()); // Reserve space for vector

    for (const auto& key : keys)
    {
        m_INImenus.push_back(key.pItem);
        g_MenuValue.emplace(ini.GetValue(section, key.pItem, nullptr));
        const char* menuItem = m_INImenus.back().c_str();
        const char* itemValue = key.pItem;
        g_Logger->info("Menu:  {} - Value: {}", menuItem, itemValue);
    }
}

// Load Reshade and register events
void ReshadeToggler::Load()
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

// Entry point for DLL
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

// Entry point for SKSE plugin loading
SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    ReshadeToggler reshadeToggler;
    reshadeToggler.SetupLog();
    reshadeToggler.MenusInINI();
    reshadeToggler.Load();
    g_Logger->info("Loaded plugin");
    auto& eventProcessor = EventProcessor::GetSingleton();
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessor);

    return true;
}
