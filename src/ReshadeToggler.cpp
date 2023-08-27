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

    const auto& menuName = event->menuName;
    auto& opening = event->opening;

    auto [it, inserted] = m_OpenMenus.emplace(menuName);

    if (!opening)
    {
        m_OpenMenus.erase(it); // Mark menu as closed using the iterator
    }

    if (m_OpenMenus.empty())
    {
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
        if (ToggleStateMenus.find("All") != std::string::npos)
        {
            g_Logger->info("All is enabled! - EnableReshade: {}", enableReshade);
            if (ToggleAllStateMenus.find("off") != std::string::npos)
            {
                s_pRuntime->set_effects_state(enableReshade);
            }
            else if (ToggleAllStateMenus.find("on") != std::string::npos)
            {
                s_pRuntime->set_effects_state(!enableReshade);
            }
        }
        else if (ToggleStateMenus.find("Specific") != std::string::npos)
        {
            g_Logger->info("Specific is enabled! - EnableReshade: {}", enableReshade);

            for (const TechniqueInfo& info : techniqueInfoList)
            {
                s_pRuntime->enumerate_techniques(info.filename.c_str(), [enableReshade, &info](reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique)
                {
                    g_Logger->info("State: {} for: {}", info.state.c_str(), info.filename.c_str());
                    if (info.state == "off") 
                    {
                        runtime->set_technique_state(technique, enableReshade);
                    }
                    else if (info.state == "on") 
                    {
                        runtime->set_technique_state(technique, !enableReshade);
                    }
                    else
                    {
                        g_Logger->error("Wrong input: MenuToggleSpecificState has to be on/off, input was: {} for: {}", info.state.c_str(), info.filename.c_str());
                    }
                });
            }
        }
#if _DEBUG
    //g_Logger->info("Menu {} {}", menuName, opening ? "open" : "closed");
    //g_Logger->info("Reshade {}", enableReshade ? "enabled" : "disabled");
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
    ini.LoadFile(L"Data\\SKSE\\Plugins\\ReShadeToggler.ini");


    const char* sectionGeneral = "General";
    const char* sectionMenusGeneral = "MenusGeneral";
    const char* sectionMenusProcess = "MenusProcess";
    const char* sectionTimeGeneral = "Time";
    //const char* sectionInteriorGeneral = "Interior";
    //const char* sectionWeatherGeneral = "Weather";

    CSimpleIniA::TNamesDepend MenusGeneral_keys;
    CSimpleIniA::TNamesDepend MenusProcess_keys;  
    CSimpleIniA::TNamesDepend TimeGeneral_keys;

    //General
    EnableMenus = ini.GetBoolValue(sectionGeneral, "EnableMenus");
    EnableTime = ini.GetBoolValue(sectionGeneral, "EnableTime");
    EnableInterior = ini.GetBoolValue(sectionGeneral, "EnableInterior");
    EnableWeather = ini.GetBoolValue(sectionGeneral, "EnableWeather");

    g_Logger->info("{}: EnableMenus: {} - EnableTime: {} - EnableInterior: {} - EnableWeather: {}", sectionGeneral, EnableMenus, EnableTime, EnableInterior, EnableWeather);

#if _DEBUG
    g_Logger->info("\n");
#endif

    // MenusGeneral
    ToggleStateMenus = ini.GetValue(sectionMenusGeneral, "MenuToggleOption");
    ToggleAllStateMenus = ini.GetValue(sectionMenusGeneral, "MenuToggleAllState");

    g_Logger->info("General MenuToggleOption:  {} - MenuToggleAllState: {}", ToggleStateMenus, ToggleAllStateMenus);

    ini.GetAllKeys(sectionMenusGeneral, MenusGeneral_keys);
    m_SpecificMenu.reserve(MenusGeneral_keys.size()); // Reserve space for vector

    const char* togglePrefix01 = "MenuToggleSpecificFile";
    const char* togglePrefix02 = "MenuToggleSpecificState";

    for (const auto& key : MenusGeneral_keys)
    {
        if (strcmp(key.pItem, "MenuToggleOption") != 0 && strcmp(key.pItem, "MenuToggleAllState") != 0)
        {
            m_SpecificMenu.push_back(key.pItem);
            const char* menuItemgeneral = m_SpecificMenu.back().c_str();

            // Check if the key starts with MenuToggleSpecificFile
            if (strncmp(key.pItem, togglePrefix01, strlen(togglePrefix01)) == 0)
            {
                itemShaderToToggle = ini.GetValue(sectionMenusGeneral, key.pItem, nullptr);
                g_MenuToggleFile.emplace(itemShaderToToggle);
                g_Logger->info("MenuToggleSpecificFile:  {} - Value: {}", menuItemgeneral, itemShaderToToggle);

                // Construct the corresponding key for the state
                std::string stateKeyName = togglePrefix02 + std::to_string(m_SpecificMenu.size());

                // Retrieve the state using the constructed key
                itemMenuStateValue = ini.GetValue(sectionMenusGeneral, stateKeyName.c_str(), nullptr);
                g_MenuToggleState.emplace(itemMenuStateValue);

                // Populate the technique info
                TechniqueInfo info;
                info.filename = itemShaderToToggle;
                info.state = itemMenuStateValue;
                techniqueInfoList.push_back(info);
                g_Logger->info("Populated TechniqueInfo: {} - {}", itemShaderToToggle, itemMenuStateValue);
            }
        }
    }
    
#if _DEBUG
    g_Logger->info("\n");
#endif

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

#if _DEBUG
    g_Logger->info("\n");
#endif

    //Time
    ToggleStateTime = ini.GetValue(sectionTimeGeneral, "TimeToggleOption");
    ToggleAllStateTime = ini.GetValue(sectionTimeGeneral, "TimeToggleAllState");

    g_Logger->info("General TimeToggleOption:  {} - TimeToggleAllState: {}", ToggleStateTime, ToggleAllStateTime);

    ini.GetAllKeys(sectionTimeGeneral, TimeGeneral_keys);
    m_SpecificTime.reserve(TimeGeneral_keys.size()); // Reserve space for vector

    const char* togglePrefix03 = "TimeToggleSpecificFile";
    const char* togglePrefix04 = "TimeToggleSpecificState";

    for (const auto& key : TimeGeneral_keys)
    {
        if (strcmp(key.pItem, "TimeToggleOption") != 0 && strcmp(key.pItem, "TimeToggleAllState") != 0)
        {
            m_SpecificTime.push_back(key.pItem);
            const char* TimeItemgeneral = m_SpecificTime.back().c_str();

            // Check if the key starts with TimeToggleSpecificFile
            if (strncmp(key.pItem, togglePrefix03, strlen(togglePrefix03)) == 0)
            {
                itemValueTimeGeneral01 = ini.GetValue(sectionTimeGeneral, key.pItem, nullptr);
                g_TimeToggleFile.emplace(itemValueTimeGeneral01);
                g_Logger->info("TimeToggleSpecificFile:  {} - Value: {}", TimeItemgeneral, itemValueTimeGeneral01);
            }

            // Check if the key starts with TimeToggleSpecificState
            else if (strncmp(key.pItem, togglePrefix04, strlen(togglePrefix04)) == 0)
            {
                itemValueTimeGeneral02 = ini.GetValue(sectionTimeGeneral, key.pItem, nullptr);
                g_TimeToggleState.emplace(itemValueTimeGeneral02);
                g_Logger->info("TimeToggleSpecificState:  {} - Value: {}", TimeItemgeneral, itemValueTimeGeneral02);
            }
        }
    }

#if _DEBUG
    g_Logger->info("\n");
#endif

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