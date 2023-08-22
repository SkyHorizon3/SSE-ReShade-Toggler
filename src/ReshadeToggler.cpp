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


RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::MenuOpenCloseEvent* event,
    RE::BSTEventSource<RE::MenuOpenCloseEvent>*) 
{

    const auto& menuName = event->menuName;
    auto& opening = event->opening;

    // emplace_hint to improve insertion performance
    auto it = m_OpenMenus.emplace_hint(m_OpenMenus.end(), menuName);

    if (!opening)
    {
        m_OpenMenus.erase(menuName); // Mark menu as closed
    }

    // Check if any open menu matches a menu in g_menuValue
    bool enableReshade = true;
    for (const auto& menuToDisable : g_menuValue)
    {
        if (m_OpenMenus.find(menuToDisable) != m_OpenMenus.end())
        {
            enableReshade = false;
            break;
        }
    }
    if (s_pRuntime != nullptr)
    {
        s_pRuntime->set_effects_state(enableReshade);
#if _DEBUG
        logger::info("Menu {} {}", menuName, opening ? "open" : "closed");
        logger::info("Reshade {}", enableReshade ? "disabled" : "enabled");
#endif
    }

    return RE::BSEventNotifyControl::kContinue;
}


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

void ReshadeToggler::MenusInINI()
{
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(L"Data\\SKSE\\Plugins\\ReshadeToggler.ini");

    const char* section = "Menus";

    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys(section, keys);

    for (const auto& key : keys)
    {
        m_INImenus.push_back(key.pItem);
        g_menuValue.emplace(ini.GetValue(section, key.pItem, nullptr));

        logger::info("Menu:  {} - Value: {}", m_INImenus.back(), key.pItem);
    }
}

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
    ReshadeToggler reshadeToggler;
    reshadeToggler.SetupLog();
    reshadeToggler.MenusInINI();
    reshadeToggler.Load();
    g_Logger->info("Loaded plugin");
    auto& eventProcessor = EventProcessor::GetSingleton();
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessor);

    return true;
}
