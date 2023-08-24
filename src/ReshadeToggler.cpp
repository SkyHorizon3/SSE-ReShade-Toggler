#include "ReShadeToggler.h"

namespace logger = SKSE::log;

class EffectRuntime : public reshade::api::effect_runtime
{
public:
    void set_effects_state(bool enabled) override
    {
        reshade::api::effect_runtime::set_effects_state(enabled);
    }
};


class TechniqueRuntime : public reshade::api::effect_runtime
{
public:
    void set_technique_state(reshade::api::effect_technique technique, bool enabled) override
    {
        reshade::api::effect_runtime::set_technique_state(technique, enabled);
    }
};


/// <summary>
/// Enables or disables the specified <paramref name="technique"/>.
/// </summary>
/// <param name="technique">Opaque handle to the technique.</param>
/// <param name="enabled">Set to <see langword="true"/> to enable the technique, or <see langword="false"/> to disable it.</param>
//virtual void set_technique_state(effect_technique technique, bool enabled) = 0;

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
RE::BSEventNotifyControl EventProcessorMenu::ProcessEvent(const RE::MenuOpenCloseEvent* event,
    RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{

    if (!EnableMenus) 
    {
        g_Logger->info("!EnableMenus"); // Skip execution if EnableMenus is false
        return RE::BSEventNotifyControl::kContinue; 
    }
    else if (EnableMenus){

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

            g_Logger->info("s_pRuntime not 0!");
            g_Logger->info("General ToggleStateMenus:  {} - ToggleAllStateMenus: {}", ToggleStateMenus, ToggleAllStateMenus);

            if (ToggleStateMenus.find("All") != std::string::npos)
            {
                s_pRuntime->set_effects_state(enableReshade);
#if _DEBUG
                g_Logger->info("Menu {} {}", menuName, opening ? "open" : "closed");
                g_Logger->info("Reshade {}", enableReshade ? "disabled" : "enabled");

#endif
            }

            else if (ToggleStateMenus.find("Specific") != std::string::npos)
            {
                for (const std::string& LoopmenuValue02 : g_MenuGeneralValue02)
                {
                s_pRuntime->enumerate_techniques(LoopmenuValue02.c_str(), [enableReshade](reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique) 
                    {
                    runtime->set_technique_state(technique, enableReshade); 
                    });

#if _DEBUG
                g_Logger->info("Menu {} {}", menuName, opening ? "open" : "closed");
                g_Logger->info("Reshade {}", enableReshade ? "disabled" : "enabled");

#endif

                }



            }
        }
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
    ini.LoadFile(L"Data\\SKSE\\Plugins\\ReShadeToggler.ini");


    const char* sectionGeneral = "General";
    const char* sectionMenusGeneral = "MenusGeneral";
    const char* sectionMenusProcess = "MenusProcess";
    //const char* sectionGeneral = "Interior";
    //const char* sectionGeneral = "Weather";

    CSimpleIniA::TNamesDepend MenusGeneral_keys;
    CSimpleIniA::TNamesDepend MenusProcess_keys;



    //General
    EnableMenus = ini.GetBoolValue(sectionGeneral, "EnableMenus");
    g_Logger->info("{}: EnableMenus: {}", sectionGeneral, EnableMenus);







    // MenusGeneral
    ToggleStateMenus = ini.GetValue(sectionMenusGeneral, "ToggleState");
    ToggleAllStateMenus = ini.GetValue(sectionMenusGeneral, "ToggleAllState");

    g_Logger->info("General ToggleStateMenus:  {} - ToggleAllStateMenus: {}", ToggleStateMenus, ToggleAllStateMenus);

    ini.GetAllKeys(sectionMenusGeneral, MenusGeneral_keys);
    m_Specific.reserve(MenusGeneral_keys.size()); // Reserve space for vector

    const char* toggleStatePrefix = "aToggleSpecificState";
    const char* togglePrefix = "bToggleSpecific";

    for (const auto& key : MenusGeneral_keys)
    {
        if (strcmp(key.pItem, "ToggleState") != 0 && strcmp(key.pItem, "ToggleAllState") != 0)
        {
            m_Specific.push_back(key.pItem);
            const char* menuItemgeneral = m_Specific.back().c_str();

            // Check if the key starts with ToggleSpecific
            if (strncmp(key.pItem, togglePrefix, strlen(togglePrefix)) == 0)
            {
                // Process ToggleSpecific entries
                itemValuegeneral01 = ini.GetValue(sectionMenusGeneral, key.pItem, nullptr);
                g_MenuGeneralValue01.emplace(itemValuegeneral01);
                g_Logger->info("ToggleSpecific Menu:  {} - Value: {}", menuItemgeneral, itemValuegeneral01);
            }

            // Check if the key starts with ToggleSpecificState
            else if (strncmp(key.pItem, toggleStatePrefix, strlen(toggleStatePrefix)) == 0)
            {
                // Process ToggleSpecificState entries
                itemValuegeneral02 = ini.GetValue(sectionMenusGeneral, key.pItem, nullptr);
                g_MenuGeneralValue02.emplace(itemValuegeneral02);
                g_Logger->info("ToggleSpecificState Menu:  {} - Value: {}", menuItemgeneral, itemValuegeneral02);
            }
        }
    }


 


//MenusProcess
    ini.GetAllKeys(sectionMenusProcess, MenusProcess_keys);
    m_INImenus.reserve(MenusProcess_keys.size()); // Reserve space for vector

    for (const auto& key : MenusProcess_keys)
    {
        m_INImenus.push_back(key.pItem);
        const char* menuItem = m_INImenus.back().c_str();
        const char* itemValue = ini.GetValue(sectionMenusProcess, key.pItem, nullptr);
        g_MenuValue.emplace(itemValue);
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
    auto& eventProcessorMenu = EventProcessorMenu::GetSingleton();
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessorMenu);

    return true;
}
