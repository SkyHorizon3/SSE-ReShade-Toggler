#include "Config.h"
#include "Globals.h"


void Config::LoadINI(const std::string& presetPath)
{
#ifndef NDEBUG
	SKSE::log::debug("Starting to load: {}", presetPath.c_str());
#endif

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

#ifndef NDEBUG
	SKSE::log::debug("{}: EnableMenus: {} - EnableTime: {} - EnableInterior: {} - EnableWeather: {}", sectionGeneral, EnableMenus, EnableTime, EnableInterior, EnableWeather);
	SKSE::log::debug("\n");
#endif

#pragma region Menus

	// MenusGeneral
	ToggleStateMenus = ini.GetValue(sectionMenusGeneral, "MenuToggleOption");
	ToggleAllStateMenus = ini.GetValue(sectionMenusGeneral, "MenuToggleAllState");

#ifndef NDEBUG
	SKSE::log::debug("General MenuToggleOption:  {} - MenuToggleAllState: {}", ToggleStateMenus, ToggleAllStateMenus);
#endif

	ini.GetAllKeys(sectionMenusGeneral, MenusGeneral_keys);
	m_specificMenu.reserve(MenusGeneral_keys.size()); // Reserve space for vector

	const char* togglePrefix01 = "MenuToggleSpecificFile";
	const char* togglePrefix02 = "MenuToggleSpecificState";
	const char* togglePrefixMenu = "MenuToggleSpecificMenu";

	for (const auto& key : MenusGeneral_keys)
	{
		if (strcmp(key.pItem, "MenuToggleOption") != 0 && strcmp(key.pItem, "MenuToggleAllState") != 0)
		{
			m_specificMenu.push_back(key.pItem);
			//const char* menuItemgeneral = m_SpecificMenu.back().c_str();

			// Check if the key starts with MenuToggleSpecificFile
			if (strncmp(key.pItem, togglePrefix01, strlen(togglePrefix01)) == 0)
			{
				m_itemMenuShaderToToggle = ini.GetValue(sectionMenusGeneral, key.pItem, nullptr);
				//DEBUG_LOG(g_Logger, "MenuToggleSpecificFile:  {} - Value: {}", menuItemgeneral, itemMenuShaderToToggle);

				// Construct the corresponding key for the state
				std::string stateKeyName = togglePrefix02 + std::to_string(m_specificMenu.size());

				// Retrieve the state using the constructed key
				m_itemMenuStateValue = ini.GetValue(sectionMenusGeneral, stateKeyName.c_str(), nullptr);

				std::string menuKeyName = togglePrefixMenu + std::to_string(m_specificMenu.size());
				m_itemSpecificMenu = ini.GetValue(sectionMenusGeneral, menuKeyName.c_str(), nullptr);

				// Populate the technique info
				TechniqueInfo MenuInfo;
				MenuInfo.filename = m_itemMenuShaderToToggle;
				MenuInfo.state = m_itemMenuStateValue;
				MenuInfo.Name = m_itemSpecificMenu;
				techniqueMenuInfoList.push_back(MenuInfo);
#ifndef NDEBUG
				SKSE::log::debug("Populated TechniqueMenuInfo: {} - {}", m_itemMenuShaderToToggle, m_itemMenuStateValue);
#endif
			}
		}
	}

#ifndef NDEBUG
	SKSE::log::debug("\n");
#endif

	//MenusProcess
	ini.GetAllKeys(sectionMenusProcess, MenusProcess_keys);
	m_iniMenus.reserve(MenusProcess_keys.size()); // Reserve space for vector

	for (const auto& key : MenusProcess_keys)
	{
		Info menus;
		m_iniMenus.push_back(key.pItem);
		const char* menuItem = m_iniMenus.back().c_str();
		const char* itemValue = ini.GetValue(sectionMenusProcess, key.pItem, nullptr);

		menus.Index = menuItem;
		menus.Name = itemValue;
		menuList.push_back(menus);

#ifndef NDEBUG
		SKSE::log::debug("Menu:  {} - Value: {}", menuItem, itemValue);
#endif
	}

#ifndef NDEBUG
	SKSE::log::debug("\n");
#endif

#pragma endregion

#pragma region Time
	//Time
	ToggleStateTime = ini.GetValue(sectionTimeGeneral, "TimeToggleOption");
	TimeUpdateIntervalTime = ini.GetLongValue(sectionTimeGeneral, "TimeUpdateInterval");

#ifndef NDEBUG
	SKSE::log::debug("General TimeToggleOption:  {} - TimeUpdateInterval: {}", ToggleStateTime, TimeUpdateIntervalTime);
#endif

	// All Time
	ToggleAllStateTime = ini.GetValue(sectionTimeGeneral, "TimeToggleAllState");
	m_itemTimeStartHourAll = ini.GetDoubleValue(sectionTimeGeneral, "TimeToggleAllTimeStart");
	m_itemTimeStopHourAll = ini.GetDoubleValue(sectionTimeGeneral, "TimeToggleAllTimeStop");

	TechniqueInfo TimeInfoAll;
	TimeInfoAll.state = ToggleAllStateTime;
	TimeInfoAll.startTime = m_itemTimeStartHourAll;
	TimeInfoAll.stopTime = m_itemTimeStopHourAll;
	techniqueTimeInfoListAll.push_back(TimeInfoAll);
#ifndef NDEBUG
	SKSE::log::debug("Set all effects to {} from {} - {}", ToggleAllStateTime, m_itemTimeStartHourAll, m_itemTimeStopHourAll);
#endif

	// Specific Time
	ini.GetAllKeys(sectionTimeGeneral, TimeGeneral_keys);
	m_specificTime.reserve(TimeGeneral_keys.size()); // Reserve space for vector

	const char* togglePrefix03 = "TimeToggleSpecificFile";
	const char* togglePrefix04 = "TimeToggleSpecificState";
	const char* togglePrefix05 = "TimeToggleSpecificTimeStart";
	const char* togglePrefix06 = "TimeToggleSpecificTimeStop";

	for (const auto& key : TimeGeneral_keys)
	{
		if (strcmp(key.pItem, "TimeToggleOption") != 0 && strcmp(key.pItem, "TimeToggleAllState") != 0 && strcmp(key.pItem, "TimeToggleAllTimeStart") != 0 && strcmp(key.pItem, "TimeToggleAllTimeStop") != 0)
		{
			m_specificTime.push_back(key.pItem);

			//const char* timeItemGeneral = m_SpecificTime.back().c_str();

			if (strncmp(key.pItem, togglePrefix03, strlen(togglePrefix03)) == 0)
			{
				m_itemTimeShaderToToggle = ini.GetValue(sectionTimeGeneral, key.pItem, nullptr);
				//DEBUG_LOG(g_Logger, "TimeToggleSpecificFile:  {} - Value: {}", timeItemGeneral, itemTimeShaderToToggle);

				// Construct the corresponding key for the state
				std::string stateKeyName = togglePrefix04 + std::to_string(m_specificTime.size());

				// Retrieve the state using the constructed key
				m_itemTimeStateValue = ini.GetValue(sectionTimeGeneral, stateKeyName.c_str(), nullptr);

				// Construct the corresponding key for the the start and stop times
				std::string startTimeKey = togglePrefix05 + std::to_string(m_specificTime.size());
				std::string endTimeKey = togglePrefix06 + std::to_string(m_specificTime.size());
				m_itemTimeStartHour = ini.GetDoubleValue(sectionTimeGeneral, startTimeKey.c_str());
				m_itemTimeStopHour = ini.GetDoubleValue(sectionTimeGeneral, endTimeKey.c_str());
#ifndef NDEBUG
				SKSE::log::debug("startTime: {}; stopTimeKey: {} ", m_itemTimeStartHour, m_itemTimeStopHour);
#endif

				// Populate the technique info
				TechniqueInfo TimeInfo;
				TimeInfo.filename = m_itemTimeShaderToToggle;
				TimeInfo.state = m_itemTimeStateValue;
				TimeInfo.startTime = m_itemTimeStartHour;
				TimeInfo.stopTime = m_itemTimeStopHour;
				techniqueTimeInfoList.push_back(TimeInfo);
#ifndef NDEBUG
				SKSE::log::debug("Set effect {} to {} from {} - {}", m_itemTimeShaderToToggle, m_itemTimeStateValue, m_itemTimeStartHour, m_itemTimeStopHour);
#endif
			}
		}
	}

#ifndef NDEBUG
	SKSE::log::debug("\n");
#endif

#pragma endregion 

#pragma region Interior
	//Interior
	ToggleStateInterior = ini.GetValue(sectionInteriorGeneral, "InteriorToggleOption");
	ToggleAllStateInterior = ini.GetValue(sectionInteriorGeneral, "InteriorToggleAllState");
	TimeUpdateIntervalInterior = ini.GetLongValue(sectionInteriorGeneral, "InteriorUpdateInterval");
#ifndef NDEBUG
	SKSE::log::debug("General InteriorToggleOption:  {} - InteriorToggleAllState: {} - InteriorUpdateInterval: {}", ToggleStateInterior, ToggleAllStateInterior, TimeUpdateIntervalInterior);
#endif

	ini.GetAllKeys(sectionInteriorGeneral, InteriorGeneral_keys);
	m_specificInterior.reserve(InteriorGeneral_keys.size()); // Reserve space for vector

	const char* togglePrefix07 = "InteriorToggleSpecificFile";
	const char* togglePrefix08 = "InteriorToggleSpecificState";

	for (const auto& key : InteriorGeneral_keys)
	{
		if (strcmp(key.pItem, "InteriorToggleOption") != 0 && strcmp(key.pItem, "InteriorToggleAllState") != 0)
		{
			m_specificInterior.push_back(key.pItem);
			//const char* interiorItemgeneral = m_SpecificInterior.back().c_str();

			// Check if the key starts with InteriorToggleSpecificFile
			if (strncmp(key.pItem, togglePrefix07, strlen(togglePrefix07)) == 0)
			{
				m_itemInteriorShaderToToggle = ini.GetValue(sectionInteriorGeneral, key.pItem, nullptr);
				//DEBUG_LOG(g_Logger, "InteriorToggleSpecificFile:  {} - Value: {}", interiorItemgeneral, itemInteriorShaderToToggle);

				// Construct the corresponding key for the state
				std::string stateKeyName = togglePrefix08 + std::to_string(m_specificInterior.size());

				// Retrieve the state using the constructed key
				m_itemInteriorStateValue = ini.GetValue(sectionInteriorGeneral, stateKeyName.c_str(), nullptr);

				// Populate the technique info
				TechniqueInfo InteriorInfo;
				InteriorInfo.filename = m_itemInteriorShaderToToggle;
				InteriorInfo.state = m_itemInteriorStateValue;
				techniqueInteriorInfoList.push_back(InteriorInfo);
#ifndef NDEBUG
				SKSE::log::debug("Populated TechniqueInteriorInfo: {} - {}", m_itemInteriorShaderToToggle, m_itemInteriorStateValue);
#endif
			}
		}
	}

#ifndef NDEBUG
	SKSE::log::debug("\n");
#endif

#pragma endregion

#pragma region Weather
	//Weather
	ToggleStateWeather = ini.GetValue(sectionWeatherGeneral, "WeatherToggleOption");
	ToggleAllStateWeather = ini.GetValue(sectionWeatherGeneral, "WeatherToggleAllState");
	TimeUpdateIntervalWeather = ini.GetLongValue(sectionWeatherGeneral, "WeatherUpdateInterval");
#ifndef NDEBUG
	SKSE::log::debug("General WeatherToggleOption:  {} - WeatherToggleAllState: {} - WeatherUpdateInterval: {}", ToggleStateWeather, ToggleAllStateWeather, TimeUpdateIntervalWeather);
#endif

	ini.GetAllKeys(sectionWeatherGeneral, WeatherGeneral_keys);
	m_specificWeather.reserve(WeatherGeneral_keys.size());

	const char* togglePrefix09 = "WeatherToggleSpecificFile";
	const char* togglePrefix10 = "WeatherToggleSpecificState";
	const char* togglePrefixItemWeather = "WeatherToggleSpecificWeather";

	for (const auto& key : WeatherGeneral_keys)
	{
		if (strcmp(key.pItem, "WeatherToggleOption") != 0 && strcmp(key.pItem, "WeatherToggleAllState") != 0)
		{
			m_specificWeather.push_back(key.pItem);
			//const char* weatherItemgeneral = m_SpecificWeather.back().c_str();

			if (strncmp(key.pItem, togglePrefix09, strlen(togglePrefix09)) == 0)
			{
				m_itemWeatherShaderToToggle = ini.GetValue(sectionWeatherGeneral, key.pItem, nullptr);
				//DEBUG_LOG(g_Logger, "WeatherToggleSpecificFile:  {} - Value: {}", weatherItemgeneral, itemWeatherShaderToToggle);

				std::string stateKeyName = togglePrefix10 + std::to_string(m_specificWeather.size());

				m_itemWeatherStateValue = ini.GetValue(sectionWeatherGeneral, stateKeyName.c_str(), nullptr);

				std::string weatherKeyName = togglePrefixItemWeather + std::to_string(m_specificWeather.size());
				m_itemSpecificWeather = ini.GetValue(sectionWeatherGeneral, weatherKeyName.c_str(), nullptr);

				TechniqueInfo WeatherInfo;
				WeatherInfo.filename = m_itemWeatherShaderToToggle;
				WeatherInfo.state = m_itemWeatherStateValue;
				WeatherInfo.Name = m_itemSpecificWeather;
				techniqueWeatherInfoList.push_back(WeatherInfo);
#ifndef NDEBUG
				SKSE::log::debug("Populated TechniqueWeatherInfo: {} - {}", m_itemWeatherShaderToToggle, m_itemWeatherStateValue);
#endif
			}
		}
	}

#ifndef NDEBUG
	SKSE::log::debug("\n");
#endif

	//WeatherProcess
	ini.GetAllKeys(sectionWeatherProcess, WeatherProcess_keys);
	m_iniWeather.reserve(WeatherProcess_keys.size()); // Reserve space for vector

	for (const auto& key : WeatherProcess_keys)
	{
		Info weather;
		m_iniWeather.push_back(key.pItem);
		const char* weatherItem = m_iniWeather.back().c_str();
		const char* weatheritemValue = ini.GetValue(sectionWeatherProcess, key.pItem, nullptr);

		weather.Index = weatherItem;
		weather.Name = weatheritemValue;
		weatherList.push_back(weather);
#ifndef NDEBUG
		SKSE::log::debug("Weather:  {} - Value: {}", weatherItem, weatheritemValue);
#endif
	}

#ifndef NDEBUG
	SKSE::log::debug("\n");
#endif

#pragma endregion

	if (TimeUpdateIntervalTime < 0) { TimeUpdateIntervalTime = 0; }
	if (TimeUpdateIntervalInterior < 0) { TimeUpdateIntervalInterior = 0; }
	if (TimeUpdateIntervalWeather < 0) { TimeUpdateIntervalWeather = 0; }
}

void Config::LoadPreset(const std::string& Preset)
{
	const std::string& fullPath = "Data\\SKSE\\Plugins\\TogglerConfigs\\" + Preset;


	SKSE::log::info("Starting clear procedure...");

	EnableMenus = false;
	EnableTime = false;
	EnableInterior = false;
	EnableWeather = false;

	// Empty every vector
	m_specificMenu.clear();
	m_iniMenus.clear();
	techniqueMenuInfoList.clear();
	menuList.clear();
	ToggleStateMenus.clear();
	ToggleAllStateMenus.clear();
	m_itemMenuShaderToToggle = nullptr;
	m_itemMenuStateValue = nullptr;
	m_itemSpecificMenu = nullptr;

	techniqueTimeInfoList.clear();
	techniqueTimeInfoListAll.clear();
	m_specificTime.clear();
	ToggleStateTime.clear();
	ToggleAllStateTime.clear();
	m_itemTimeShaderToToggle = nullptr;
	m_itemTimeStateValue = nullptr;
	m_itemTimeStartHour = 0.0;
	m_itemTimeStopHour = 0.0;
	m_itemTimeStartHourAll = 0.0;
	m_itemTimeStopHourAll = 0.0;
	TimeUpdateIntervalTime = 0;

	techniqueInteriorInfoList.clear();
	m_specificInterior.clear();
	ToggleStateInterior.clear();
	ToggleAllStateInterior.clear();
	m_itemInteriorShaderToToggle = nullptr;
	m_itemInteriorStateValue = nullptr;
	TimeUpdateIntervalInterior = 0;

	m_specificWeather.clear();
	m_iniWeather.clear();
	weatherList.clear();
	techniqueWeatherInfoList.clear();
	ToggleStateWeather.clear();
	ToggleAllStateWeather.clear();
	weatherflags.clear();
	m_itemWeatherShaderToToggle = nullptr;
	m_itemWeatherStateValue = nullptr;
	m_itemSpecificWeather = nullptr;
	TimeUpdateIntervalWeather = 0;

	SKSE::log::info("Finished clearing procedure...");

	// Load the new INI
	LoadINI(fullPath);
}