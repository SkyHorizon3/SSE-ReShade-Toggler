#include "../include/ReshadeIntegration.h"
#include "../include/Processor.h"
#include "../include/ReshadeToggler.h"
#include "../include/Globals.h"

namespace logger = SKSE::log;

reshade::api::effect_runtime* s_pRuntime = nullptr;
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

void ReshadeToggler::LoadINI()
{
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

        DEBUG_LOG(g_Logger, "\n", nullptr);


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
                    itemMenuShaderToToggle = ini.GetValue(sectionMenusGeneral, key.pItem, nullptr);
                    g_MenuToggleFile.emplace(itemMenuShaderToToggle);
                    g_Logger->info("MenuToggleSpecificFile:  {} - Value: {}", menuItemgeneral, itemMenuShaderToToggle);

                    // Construct the corresponding key for the state
                    std::string stateKeyName = togglePrefix02 + std::to_string(m_SpecificMenu.size());

                    // Retrieve the state using the constructed key
                    itemMenuStateValue = ini.GetValue(sectionMenusGeneral, stateKeyName.c_str(), nullptr);
                    g_MenuToggleState.emplace(itemMenuStateValue);

                    // Populate the technique info
                    TechniqueInfo MenuInfo;
                    MenuInfo.filename = itemMenuShaderToToggle;
                    MenuInfo.state = itemMenuStateValue;
                    techniqueMenuInfoList.push_back(MenuInfo);
                    g_Logger->info("Populated TechniqueMenuInfo: {} - {}", itemMenuShaderToToggle, itemMenuStateValue);
                }
            }
        }

        DEBUG_LOG(g_Logger, "\n", nullptr);


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

        DEBUG_LOG(g_Logger, "\n", nullptr);


        //Time
        ToggleStateTime = ini.GetValue(sectionTimeGeneral, "TimeToggleOption");
        ToggleAllStateTime = ini.GetValue(sectionTimeGeneral, "TimeToggleAllState");

        TimeUpdateIntervall = ini.GetLongValue(sectionTimeGeneral, "TimeUpdateInterval");

        g_Logger->info("General TimeToggleOption:  {} - TimeToggleAllState: {} - TimeUpdateIntervall: {}", ToggleStateTime, ToggleAllStateTime, TimeUpdateIntervall);

        ini.GetAllKeys(sectionTimeGeneral, TimeGeneral_keys);
        m_SpecificTime.reserve(TimeGeneral_keys.size()); // Reserve space for vector

        const char* togglePrefix03 = "TimeToggleSpecificFile";
        const char* togglePrefix04 = "TimeToggleSpecificState";
        const char* togglePrefix05 = "TimeToggleSpecificTimeStart";
        const char* togglePrefix06 = "TimeToggleSpecificTimeStop";

        for (const auto& key : TimeGeneral_keys)
        {
            if (strcmp(key.pItem, "TimeToggleOption") != 0 && strcmp(key.pItem, "TimeToggleAllState") != 0)
            {
                m_SpecificTime.push_back(key.pItem);
                const char* timeItemGeneral = m_SpecificTime.back().c_str();

                if (strncmp(key.pItem, togglePrefix03, strlen(togglePrefix03)) == 0)
                {
                    itemTimeShaderToToggle = ini.GetValue(sectionTimeGeneral, key.pItem, nullptr);
                    g_TimeToggleFile.emplace(itemTimeShaderToToggle);
                    g_Logger->info("TimeToggleSpecificFile:  {} - Value: {}", timeItemGeneral, itemTimeShaderToToggle);

                    // Construct the corresponding key for the state
                    std::string stateKeyName = togglePrefix04 + std::to_string(m_SpecificTime.size());

                    // Retrieve the state using the constructed key
                    itemTimeStateValue = ini.GetValue(sectionTimeGeneral, stateKeyName.c_str(), nullptr);
                    g_TimeToggleState.emplace(itemTimeStateValue);

                    // Construct the corresponding key for the the start and stop times
                    std::string startTimeKey = togglePrefix05 + std::to_string(m_SpecificTime.size());
                    std::string endTimeKey = togglePrefix06 + std::to_string(m_SpecificTime.size());
                    itemTimeStartHour = ini.GetDoubleValue(sectionTimeGeneral, startTimeKey.c_str());
                    itemTimeStopHour = ini.GetDoubleValue(sectionTimeGeneral, endTimeKey.c_str());
                    g_Logger->info("startTime: {}; stopTimeKey: {} ", itemTimeStartHour, itemTimeStopHour);


                    // Populate the technique info
                    TechniqueInfo TimeInfo;
                    TimeInfo.filename = itemTimeShaderToToggle;
                    TimeInfo.state = itemTimeStateValue;
                    TimeInfo.startTime = itemTimeStartHour;
                    TimeInfo.stopTime = itemTimeStopHour;
                    techniqueTimeInfoList.push_back(TimeInfo);
                    g_Logger->info("Set effect {} to {} from {} - {}", itemTimeShaderToToggle, itemTimeStateValue, itemTimeStartHour, itemTimeStopHour);
                }
            }
        }

        DEBUG_LOG(g_Logger, "\n", nullptr);
    }
}

void TimeThread()
{
    while (true)
    {
        // Call ProcessTimeBasedToggling every 5 seconds
        std::this_thread::sleep_for(std::chrono::seconds(TimeUpdateIntervall));
        Processor::GetSingleton().ProcessTimeBasedToggling();
    }
}

void MessageListener(SKSE::MessagingInterface::Message* message)
{
    auto& processor = Processor::GetSingleton();
    switch (message->type) {
        // Descriptions are taken from the original skse64 library
        // See:
        // https://github.com/ianpatt/skse64/blob/09f520a2433747f33ae7d7c15b1164ca198932c3/skse64/PluginAPI.h#L193-L212
    case SKSE::MessagingInterface::kPostLoad:
        logger::info("kPostLoad: sent to registered plugins once all plugins have been loaded");
        break;
    case SKSE::MessagingInterface::kPostPostLoad:
        logger::info(
            "kPostPostLoad: sent right after kPostLoad to facilitate the correct dispatching/registering of "
            "messages/listeners");
        break;
    case SKSE::MessagingInterface::kPreLoadGame:
        // message->dataLen: length of file path, data: char* file path of .ess savegame file
        logger::info("kPreLoadGame: sent immediately before savegame is read");
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
        // You will probably want to handle this event if your plugin uses a Preload callback
        // as there is a chance that after that callback is invoked the game will encounter an error
        // while loading the saved game (eg. corrupted save) which may require you to reset some of your
        // plugin state.
        logger::info("kPostLoadGame: sent after an attempt to load a saved game has finished");
        processor.ProcessTimeBasedToggling();
        std::thread(TimeThread).detach();
        break;
    case SKSE::MessagingInterface::kSaveGame:
        logger::info("kSaveGame");
        break;
    case SKSE::MessagingInterface::kDeleteGame:
        // message->dataLen: length of file path, data: char* file path of .ess savegame file
        logger::info("kDeleteGame: sent right before deleting the .skse cosave and the .ess save");
        break;
    case SKSE::MessagingInterface::kInputLoaded:
        logger::info("kInputLoaded: sent right after game input is loaded, right before the main menu initializes");
        break;
    case SKSE::MessagingInterface::kNewGame:
        // message-data: CharGen TESQuest pointer (Note: I haven't confirmed the usefulness of this yet!)
        logger::info("kNewGame: sent after a new game is created, before the game has loaded");
        break;
    case SKSE::MessagingInterface::kDataLoaded:
        logger::info("kDataLoaded: sent after the data handler has loaded all its forms");
        break;
    default:
        logger::info("Unknown system message of type: {}", message->type);
        break;
    }
}


void ReshadeToggler::Setup()
{
    SetupLog();
    LoadINI();

    SKSE::GetMessagingInterface()->RegisterListener(MessageListener);

    Load();
    g_Logger->info("Loaded plugin");
    auto& eventProcessorMenu = Processor::GetSingleton();
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessorMenu);
}

int __stdcall DllMain(HMODULE hModule, uint32_t fdwReason, void*)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = hModule;
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        std::thread(TimeThread).join();
        unregister_addon_events();
        reshade::unregister_addon(hModule);
    }

    return 1;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    ReshadeToggler plugin;
    plugin.Setup();

    return true;
}