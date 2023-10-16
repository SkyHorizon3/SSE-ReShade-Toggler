#include "../include/ReshadeIntegration.h"
#include "../include/Processor.h"
#include "../include/ReshadeToggler.h"
#include "../include/Globals.h"
#include "../include/Menu.h"
namespace logger = SKSE::log;

#define DLLEXPORT __declspec(dllexport)
extern "C" DLLEXPORT const char* NAME = "ReShadeToggler";
extern "C" DLLEXPORT const char* DESCRIPTION = "ReShade Toggler by SkyHorizon and PhilikusHD.";

reshade::api::effect_runtime* s_pRuntime = nullptr;

// Callback when Reshade begins effects
static void on_reshade_begin_effects(reshade::api::effect_runtime* runtime)
{
	s_pRuntime = runtime;
}

static void DrawMenu(reshade::api::effect_runtime*)
{
	Menu::GetSingleton()->SettingsMenu();
}

// Register and unregister addon events
void register_addon_events()
{
	reshade::register_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
	reshade::register_overlay(nullptr, &DrawMenu);
}

void unregister_addon_events()
{
	reshade::unregister_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
	reshade::unregister_overlay(nullptr, &DrawMenu);
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

void RuntimeThread()
{
	g_Logger->info("Attaching RuntimeThread");
	auto MainThread = ReshadeToggler::GetSingleton();

	while (isLoaded)
	{

		if (EnableTime)
		{
			//g_Logger->info("Adding Time to Mainqueue");
			std::this_thread::sleep_for(std::chrono::seconds(TimeUpdateIntervalTime));
			MainThread->SubmitToMainThread("Time", []() -> RE::BSEventNotifyControl {
				return Processor::GetSingleton().ProcessTimeBasedToggling();
				});
		}

		if (EnableInterior)
		{
			//g_Logger->info("Adding Interior to Mainqueue");
			std::this_thread::sleep_for(std::chrono::seconds(TimeUpdateIntervalInterior));
			MainThread->SubmitToMainThread("Interior", []() -> RE::BSEventNotifyControl {
				return Processor::GetSingleton().ProcessInteriorBasedToggling();
				});
		}

		if (EnableWeather && !IsInInteriorCell)
		{

			//g_Logger->info("Adding Weather to Mainqueue");
			std::this_thread::sleep_for(std::chrono::seconds(TimeUpdateIntervalWeather));
			MainThread->SubmitToMainThread("Weather", []() -> RE::BSEventNotifyControl {
				return Processor::GetSingleton().ProcessWeatherBasedToggling();
				});

		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		MainThread->Run();
	}
}

void ReshadeToggler::SubmitToMainThread(const std::string& functionName, FunctionToExecute function)
{
	std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);


	m_MainThreadQueue[functionName] = function;
	//g_Logger->info("Submit {}", functionName);
}

void ReshadeToggler::ExecuteMainThreadQueue()
{
	std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

	for (const auto& func : m_MainThreadQueue)
	{
		//g_Logger->info("Function: {}", func.first.c_str());
		func.second(); // Funktion ausführen
	}
	m_MainThreadQueue.clear();

}

void ReshadeToggler::Run()
{
	if (m_MainThreadQueue.find("Weather") != m_MainThreadQueue.end())
	{
		//g_Logger->info("Attaching WeatherThread");
		ExecuteMainThreadQueue();
	}

	if (m_MainThreadQueue.find("Interior") != m_MainThreadQueue.end())
	{
		//g_Logger->info("Attaching InteriorThread");
		ExecuteMainThreadQueue();
	}

	if (m_MainThreadQueue.find("Time") != m_MainThreadQueue.end())
	{
		//g_Logger->info("Attaching TimeThread");
		ExecuteMainThreadQueue();
	}

	auto& eventProcessorMenu = Processor::GetSingleton();
	if (EnableMenus)
	{
		RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessorMenu);
	}
	else
	{
		RE::UI::GetSingleton()->RemoveEventSink<RE::MenuOpenCloseEvent>(&eventProcessorMenu);
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

void ReshadeToggler::LoadINI(const std::string& presetPath)
{
	DEBUG_LOG(g_Logger, "Starting to load: {}", presetPath.c_str());

	CSimpleIniA ini;
	ini.SetUnicode(false);
	ini.LoadFile(presetPath.c_str());

	const char* sectionGeneral = "General";
	const char* sectionMenusGeneral = "MenusGeneral";
	const char* sectionMenusProcess = "MenusProcess";
	const char* sectionTimeGeneral = "Time";
	const char* sectionInteriorGeneral = "Interior";
	const char* sectionWeatherGeneral = "Weather";
	const char* sectionWeatherProcess = "WeatherProcess";

	CSimpleIniA::TNamesDepend MenusGeneral_keys;
	CSimpleIniA::TNamesDepend MenusProcess_keys;
	CSimpleIniA::TNamesDepend TimeGeneral_keys;
	CSimpleIniA::TNamesDepend InteriorGeneral_keys;
	CSimpleIniA::TNamesDepend WeatherGeneral_keys;
	CSimpleIniA::TNamesDepend WeatherProcess_keys;

	//General
	EnableMenus = ini.GetBoolValue(sectionGeneral, "EnableMenus");
	EnableTime = ini.GetBoolValue(sectionGeneral, "EnableTime");
	EnableInterior = ini.GetBoolValue(sectionGeneral, "EnableInterior");
	EnableWeather = ini.GetBoolValue(sectionGeneral, "EnableWeather");


	DEBUG_LOG(g_Logger, "{}: EnableMenus: {} - EnableTime: {} - EnableInterior: {} - EnableWeather: {}", sectionGeneral, EnableMenus, EnableTime, EnableInterior, EnableWeather);

	DEBUG_LOG(g_Logger, "\n", nullptr);

#pragma region Menus

	// MenusGeneral
	ToggleStateMenus = ini.GetValue(sectionMenusGeneral, "MenuToggleOption");
	ToggleAllStateMenus = ini.GetValue(sectionMenusGeneral, "MenuToggleAllState");

	DEBUG_LOG(g_Logger, "General MenuToggleOption:  {} - MenuToggleAllState: {}", ToggleStateMenus, ToggleAllStateMenus);

	ini.GetAllKeys(sectionMenusGeneral, MenusGeneral_keys);
	g_SpecificMenu.reserve(MenusGeneral_keys.size()); // Reserve space for vector

	const char* togglePrefix01 = "MenuToggleSpecificFile";
	const char* togglePrefix02 = "MenuToggleSpecificState";
	const char* togglePrefixMenu = "MenuToggleSpecificMenu";

	for (const auto& key : MenusGeneral_keys)
	{
		if (strcmp(key.pItem, "MenuToggleOption") != 0 && strcmp(key.pItem, "MenuToggleAllState") != 0)
		{
			g_SpecificMenu.push_back(key.pItem);
			//const char* menuItemgeneral = g_SpecificMenu.back().c_str();

			// Check if the key starts with MenuToggleSpecificFile
			if (strncmp(key.pItem, togglePrefix01, strlen(togglePrefix01)) == 0)
			{
				itemMenuShaderToToggle = ini.GetValue(sectionMenusGeneral, key.pItem, nullptr);
				g_MenuToggleFile.emplace(itemMenuShaderToToggle);
				//DEBUG_LOG(g_Logger, "MenuToggleSpecificFile:  {} - Value: {}", menuItemgeneral, itemMenuShaderToToggle);

				// Construct the corresponding key for the state
				std::string stateKeyName = togglePrefix02 + std::to_string(g_SpecificMenu.size());

				// Retrieve the state using the constructed key
				itemMenuStateValue = ini.GetValue(sectionMenusGeneral, stateKeyName.c_str(), nullptr);
				g_MenuToggleState.emplace(itemMenuStateValue);

				std::string menuKeyName = togglePrefixMenu + std::to_string(g_SpecificMenu.size());
				itemSpecificMenu = ini.GetValue(sectionMenusGeneral, menuKeyName.c_str(), nullptr);
				g_MenuSpecificMenu.emplace(itemSpecificMenu);

				// Populate the technique info
				TechniqueInfo MenuInfo;
				MenuInfo.filename = itemMenuShaderToToggle;
				MenuInfo.state = itemMenuStateValue;
				MenuInfo.Name = itemSpecificMenu;
				techniqueMenuInfoList.push_back(MenuInfo);
				DEBUG_LOG(g_Logger, "Populated TechniqueMenuInfo: {} - {}", itemMenuShaderToToggle, itemMenuStateValue);
			}
		}
	}

	DEBUG_LOG(g_Logger, "\n", nullptr);


	//MenusProcess
	ini.GetAllKeys(sectionMenusProcess, MenusProcess_keys);
	g_INImenus.reserve(MenusProcess_keys.size()); // Reserve space for vector

	for (const auto& key : MenusProcess_keys)
	{
		Info menus;
		g_INImenus.push_back(key.pItem);
		const char* menuItem = g_INImenus.back().c_str();
		const char* itemValue = ini.GetValue(sectionMenusProcess, key.pItem, nullptr);

		menus.Index = menuItem;
		menus.Name = itemValue;
		menuList.push_back(menus);

		DEBUG_LOG(g_Logger, "Menu:  {} - Value: {}", menuItem, itemValue);
	}

	DEBUG_LOG(g_Logger, "\n", nullptr);
#pragma endregion

#pragma region Time
	//Time
	ToggleStateTime = ini.GetValue(sectionTimeGeneral, "TimeToggleOption");
	TimeUpdateIntervalTime = ini.GetLongValue(sectionTimeGeneral, "TimeUpdateInterval");

	DEBUG_LOG(g_Logger, "General TimeToggleOption:  {} - TimeUpdateInterval: {}", ToggleStateTime, TimeUpdateIntervalTime);

	// All Time
	ToggleAllStateTime = ini.GetValue(sectionTimeGeneral, "TimeToggleAllState");
	itemTimeStartHourAll = ini.GetDoubleValue(sectionTimeGeneral, "TimeToggleAllTimeStart");
	itemTimeStopHourAll = ini.GetDoubleValue(sectionTimeGeneral, "TimeToggleAllTimeStop");

	TechniqueInfo TimeInfoAll;
	TimeInfoAll.state = ToggleAllStateTime;
	TimeInfoAll.startTime = itemTimeStartHourAll;
	TimeInfoAll.stopTime = itemTimeStopHourAll;
	techniqueTimeInfoListAll.push_back(TimeInfoAll);
	DEBUG_LOG(g_Logger, "Set all effects to {} from {} - {}", ToggleAllStateTime, itemTimeStartHourAll, itemTimeStopHourAll);


	// Specific Time
	ini.GetAllKeys(sectionTimeGeneral, TimeGeneral_keys);
	g_SpecificTime.reserve(TimeGeneral_keys.size()); // Reserve space for vector

	const char* togglePrefix03 = "TimeToggleSpecificFile";
	const char* togglePrefix04 = "TimeToggleSpecificState";
	const char* togglePrefix05 = "TimeToggleSpecificTimeStart";
	const char* togglePrefix06 = "TimeToggleSpecificTimeStop";

	for (const auto& key : TimeGeneral_keys)
	{
		if (strcmp(key.pItem, "TimeToggleOption") != 0 && strcmp(key.pItem, "TimeToggleAllState") != 0 && strcmp(key.pItem, "TimeToggleAllTimeStart") != 0 && strcmp(key.pItem, "TimeToggleAllTimeStop") != 0)
		{
			g_SpecificTime.push_back(key.pItem);

			// DEBUG_LOG(g_Logger, "Size of m_SpecificTime: {} ", m_SpecificTime.size());

			//const char* timeItemGeneral = g_SpecificTime.back().c_str();

			if (strncmp(key.pItem, togglePrefix03, strlen(togglePrefix03)) == 0)
			{
				itemTimeShaderToToggle = ini.GetValue(sectionTimeGeneral, key.pItem, nullptr);
				g_TimeToggleFile.emplace(itemTimeShaderToToggle);
				//DEBUG_LOG(g_Logger, "TimeToggleSpecificFile:  {} - Value: {}", timeItemGeneral, itemTimeShaderToToggle);

				// Construct the corresponding key for the state
				std::string stateKeyName = togglePrefix04 + std::to_string(g_SpecificTime.size());

				// Retrieve the state using the constructed key
				itemTimeStateValue = ini.GetValue(sectionTimeGeneral, stateKeyName.c_str(), nullptr);
				g_TimeToggleState.emplace(itemTimeStateValue);

				// Construct the corresponding key for the the start and stop times
				std::string startTimeKey = togglePrefix05 + std::to_string(g_SpecificTime.size());
				std::string endTimeKey = togglePrefix06 + std::to_string(g_SpecificTime.size());
				itemTimeStartHour = ini.GetDoubleValue(sectionTimeGeneral, startTimeKey.c_str());
				itemTimeStopHour = ini.GetDoubleValue(sectionTimeGeneral, endTimeKey.c_str());
				DEBUG_LOG(g_Logger, "startTime: {}; stopTimeKey: {} ", itemTimeStartHour, itemTimeStopHour);


				// Populate the technique info
				TechniqueInfo TimeInfo;
				TimeInfo.filename = itemTimeShaderToToggle;
				TimeInfo.state = itemTimeStateValue;
				TimeInfo.startTime = itemTimeStartHour;
				TimeInfo.stopTime = itemTimeStopHour;
				techniqueTimeInfoList.push_back(TimeInfo);
				DEBUG_LOG(g_Logger, "Set effect {} to {} from {} - {}", itemTimeShaderToToggle, itemTimeStateValue, itemTimeStartHour, itemTimeStopHour);
			}
		}
	}


	DEBUG_LOG(g_Logger, "\n", nullptr);
#pragma endregion 

#pragma region Interior
	//Interior
	ToggleStateInterior = ini.GetValue(sectionInteriorGeneral, "InteriorToggleOption");
	ToggleAllStateInterior = ini.GetValue(sectionInteriorGeneral, "InteriorToggleAllState");
	TimeUpdateIntervalInterior = ini.GetLongValue(sectionInteriorGeneral, "InteriorUpdateInterval");

	DEBUG_LOG(g_Logger, "General InteriorToggleOption:  {} - InteriorToggleAllState: {} - InteriorUpdateInterval: {}", ToggleStateInterior, ToggleAllStateInterior, TimeUpdateIntervalInterior);

	ini.GetAllKeys(sectionInteriorGeneral, InteriorGeneral_keys);
	g_SpecificInterior.reserve(InteriorGeneral_keys.size()); // Reserve space for vector

	const char* togglePrefix07 = "InteriorToggleSpecificFile";
	const char* togglePrefix08 = "InteriorToggleSpecificState";

	for (const auto& key : InteriorGeneral_keys)
	{
		if (strcmp(key.pItem, "InteriorToggleOption") != 0 && strcmp(key.pItem, "InteriorToggleAllState") != 0)
		{
			g_SpecificInterior.push_back(key.pItem);
			//const char* interiorItemgeneral = g_SpecificInterior.back().c_str();

			// Check if the key starts with InteriorToggleSpecificFile
			if (strncmp(key.pItem, togglePrefix07, strlen(togglePrefix07)) == 0)
			{
				itemInteriorShaderToToggle = ini.GetValue(sectionInteriorGeneral, key.pItem, nullptr);
				g_InteriorToggleFile.emplace(itemInteriorShaderToToggle);
				//DEBUG_LOG(g_Logger, "InteriorToggleSpecificFile:  {} - Value: {}", interiorItemgeneral, itemInteriorShaderToToggle);

				// Construct the corresponding key for the state
				std::string stateKeyName = togglePrefix08 + std::to_string(g_SpecificInterior.size());

				// Retrieve the state using the constructed key
				itemInteriorStateValue = ini.GetValue(sectionInteriorGeneral, stateKeyName.c_str(), nullptr);
				g_InteriorToggleState.emplace(itemInteriorStateValue);

				// Populate the technique info
				TechniqueInfo InteriorInfo;
				InteriorInfo.filename = itemInteriorShaderToToggle;
				InteriorInfo.state = itemInteriorStateValue;
				techniqueInteriorInfoList.push_back(InteriorInfo);
				DEBUG_LOG(g_Logger, "Populated TechniqueInteriorInfo: {} - {}", itemInteriorShaderToToggle, itemInteriorStateValue);
			}
		}
	}

	DEBUG_LOG(g_Logger, "\n", nullptr);
#pragma endregion

#pragma region Weather
	//Weather
	ToggleStateWeather = ini.GetValue(sectionWeatherGeneral, "WeatherToggleOption");
	ToggleAllStateWeather = ini.GetValue(sectionWeatherGeneral, "WeatherToggleAllState");
	TimeUpdateIntervalWeather = ini.GetLongValue(sectionWeatherGeneral, "WeatherUpdateInterval");

	DEBUG_LOG(g_Logger, "General WeatherToggleOption:  {} - WeatherToggleAllState: {} - WeatherUpdateInterval: {}", ToggleStateWeather, ToggleAllStateWeather, TimeUpdateIntervalWeather);

	ini.GetAllKeys(sectionWeatherGeneral, WeatherGeneral_keys);
	g_SpecificWeather.reserve(WeatherGeneral_keys.size());

	const char* togglePrefix09 = "WeatherToggleSpecificFile";
	const char* togglePrefix10 = "WeatherToggleSpecificState";
	const char* togglePrefixItemWeather = "WeatherToggleSpecificWeather";

	for (const auto& key : WeatherGeneral_keys)
	{
		if (strcmp(key.pItem, "WeatherToggleOption") != 0 && strcmp(key.pItem, "WeatherToggleAllState") != 0)
		{
			g_SpecificWeather.push_back(key.pItem);
			//const char* weatherItemgeneral = g_SpecificWeather.back().c_str();

			if (strncmp(key.pItem, togglePrefix09, strlen(togglePrefix09)) == 0)
			{
				itemWeatherShaderToToggle = ini.GetValue(sectionWeatherGeneral, key.pItem, nullptr);
				g_WeatherToggleFile.emplace(itemWeatherShaderToToggle);
				//DEBUG_LOG(g_Logger, "WeatherToggleSpecificFile:  {} - Value: {}", weatherItemgeneral, itemWeatherShaderToToggle);

				std::string stateKeyName = togglePrefix10 + std::to_string(g_SpecificWeather.size());

				itemWeatherStateValue = ini.GetValue(sectionWeatherGeneral, stateKeyName.c_str(), nullptr);
				g_WeatherToggleState.emplace(itemWeatherStateValue);

				std::string weatherKeyName = togglePrefixItemWeather + std::to_string(g_SpecificWeather.size());
				itemSpecificWeather = ini.GetValue(sectionWeatherGeneral, weatherKeyName.c_str(), nullptr);
				g_WeatherSpecificWeather.emplace(itemSpecificWeather);

				TechniqueInfo WeatherInfo;
				WeatherInfo.filename = itemWeatherShaderToToggle;
				WeatherInfo.state = itemWeatherStateValue;
				WeatherInfo.Name = itemSpecificWeather;
				techniqueWeatherInfoList.push_back(WeatherInfo);
				DEBUG_LOG(g_Logger, "Populated TechniqueWeatherInfo: {} - {}", itemWeatherShaderToToggle, itemWeatherStateValue);
			}
		}
	}

	DEBUG_LOG(g_Logger, "\n", nullptr);

	//WeatherProcess
	ini.GetAllKeys(sectionWeatherProcess, WeatherProcess_keys);
	g_INIweather.reserve(WeatherProcess_keys.size()); // Reserve space for vector

	for (const auto& key : WeatherProcess_keys)
	{
		Info weather;
		g_INIweather.push_back(key.pItem);
		const char* weatherItem = g_INIweather.back().c_str();
		const char* weatheritemValue = ini.GetValue(sectionWeatherProcess, key.pItem, nullptr);
		g_WeatherValue.emplace(weatheritemValue);

		weather.Index = weatherItem;
		weather.Name = weatheritemValue;
		weatherList.push_back(weather);

		DEBUG_LOG(g_Logger, "Weather:  {} - Value: {}", weatherItem, weatheritemValue);
	}

	DEBUG_LOG(g_Logger, "\n", nullptr);
#pragma endregion

	if (TimeUpdateIntervalTime < 0) { TimeUpdateIntervalTime = 0; }
	if (TimeUpdateIntervalInterior < 0) { TimeUpdateIntervalInterior = 0; }
	//if (TimeUpdateIntervalWeather < 0) { TimeUpdateIntervalWeather = 0; }
}

void ReshadeToggler::LoadPreset(const std::string& Preset)
{
	const std::string& fullPath = "Data\\SKSE\\Plugins\\TogglerConfigs\\" + Preset;


	g_Logger->info("Starting clear procedure...");

	EnableMenus = false;
	EnableTime = false;
	EnableInterior = false;
	EnableWeather = false;

	// Empty every vector
	g_MenuToggleFile.clear();
	g_MenuToggleState.clear();
	g_MenuSpecificMenu.clear();
	g_SpecificMenu.clear();
	g_INImenus.clear();
	techniqueMenuInfoList.clear();
	menuList.clear();
	ToggleStateMenus.clear();
	ToggleAllStateMenus.clear();
	itemMenuShaderToToggle = nullptr; // itemMenuShaderToToggle is a pointer, set it to nullptr.
	itemMenuStateValue = nullptr; // Similarly, set itemMenuStateValue to nullptr.
	itemSpecificMenu = nullptr;

	g_TimeToggleFile.clear();
	g_TimeToggleState.clear();
	techniqueTimeInfoList.clear();
	techniqueTimeInfoListAll.clear();
	g_SpecificTime.clear();
	ToggleStateTime.clear();
	ToggleAllStateTime.clear();
	itemTimeShaderToToggle = nullptr; // Set to nullptr.
	itemTimeStateValue = nullptr; // Set to nullptr.
	itemTimeStartHour = 0.0;
	itemTimeStopHour = 0.0;
	itemTimeStartHourAll = 0.0;
	itemTimeStopHourAll = 0.0;
	TimeUpdateIntervalTime = 0;

	g_InteriorToggleFile.clear();
	g_InteriorToggleState.clear();
	techniqueInteriorInfoList.clear();
	g_SpecificInterior.clear();
	ToggleStateInterior.clear();
	ToggleAllStateInterior.clear();
	itemInteriorShaderToToggle = nullptr;
	itemInteriorStateValue = nullptr;
	TimeUpdateIntervalInterior = 0;

	g_WeatherValue.clear();
	g_WeatherToggleFile.clear();
	g_WeatherToggleState.clear();
	g_WeatherSpecificWeather.clear();
	g_SpecificWeather.clear();
	g_INIweather.clear();
	weatherList.clear();
	techniqueWeatherInfoList.clear();
	ToggleStateWeather.clear();
	ToggleAllStateWeather.clear();
	weatherflags.clear();
	itemWeatherShaderToToggle = nullptr; // Set to nullptr.
	itemWeatherStateValue = nullptr; // Set to nullptr.
	itemSpecificWeather = nullptr;
	TimeUpdateIntervalWeather = 0;

	g_Logger->info("Finished clearing procedure...");

	// Load the new INI
	LoadINI(fullPath);
}

void MessageListener(SKSE::MessagingInterface::Message* message)
{
	auto& processor = Processor::GetSingleton();
	switch (message->type) {

		/*
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
		  */

	case SKSE::MessagingInterface::kDataLoaded:
		DEBUG_LOG(g_Logger, "kDataLoaded: sent after the data handler has loaded all its forms", nullptr);
		isLoaded = true;
		if (isLoaded)
		{
			std::thread(RuntimeThread).detach();
		}
		if (EnableTime)
		{
			processor.ProcessTimeBasedToggling();
		}

		if (EnableInterior)
		{
			processor.ProcessInteriorBasedToggling();
		}

		if (EnableWeather)
		{
			processor.ProcessWeatherBasedToggling();
		}

		break;

		/*
	default:
		logger::info("Unknown system message of type: {}", message->type);
		break;
		*/
	}
}

void ReshadeToggler::Setup()
{
	SetupLog();

	ReshadeIntegration::EnumeratePresets();
	ReshadeIntegration::EnumerateEffects();


	CSimpleIniA ini;
	ini.SetUnicode(false);
	ini.LoadFile("Data\\SKSE\\Plugins\\ReShadeToggler.ini");
	selectedPresetPath = ini.GetValue("Presets", "PresetPath");
	selectedPreset = ini.GetValue("Presets", "PresetName");

	if (FileExists(selectedPresetPath))
	{
		LoadINI(selectedPresetPath);
	}
	else
	{
		LoadINI("Data\\SKSE\\Plugins\\TogglerConfigs\\Default.ini");
	}

	// Check if all options are false, and unregister the addon if true
	if (!EnableMenus && !EnableTime && !EnableInterior && !EnableWeather)
	{
		g_Logger->info("All options are set to false.");
	}

	SKSE::GetMessagingInterface()->RegisterListener(MessageListener);

	Load();
	g_Logger->info("Loaded plugin");

	if (!EnableTime)
	{
		g_Logger->info("EnableTime is set to false, time-based toggling won't be processed.");
	}

	if (!EnableInterior)
	{
		g_Logger->info("EnableInterior is set to false, interior-based toggling won't be processed.");
	}

	if (!EnableWeather)
	{
		g_Logger->info("EnableWeather is set to false, weather-based toggling won't be processed.");
	}

	if (EnableMenus)
	{
		auto& eventProcessorMenu = Processor::GetSingleton();
		RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessorMenu);
	}
	else
	{
		g_Logger->info("EnableMenus is set to false, no menus will be processed.");
	}
}

int __stdcall DllMain(HMODULE hModule, uint32_t fdwReason, void*)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		g_hModule = hModule;
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		if (isLoaded)
		{
			std::thread(RuntimeThread).join();
		}
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

extern "C" DLLEXPORT const auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
	}
();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo * pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}