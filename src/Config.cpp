#include "PCH.h"
#include "Config.h"
#include "Globals.h"

#include <fstream>
#include "yaml-cpp/yaml.h"

bool Config::SerializePreset(const std::string& presetName)
{
	std::string configDirectory = "Data\\SKSE\\Plugins\\TogglerConfigs";
	std::filesystem::create_directories(configDirectory);
	std::string fullPath = configDirectory + "\\" + presetName + ".yaml";

	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		{
			out << YAML::Key << "General" << YAML::Value;
			out << YAML::BeginMap; // General
			out << YAML::Key << "EnableMenus" << YAML::Value << m_Config.EnableMenus;
			out << YAML::Key << "EnableTime" << YAML::Value << m_Config.EnableTime;
			out << YAML::Key << "EnableInterior" << YAML::Value << m_Config.EnableInterior;
			out << YAML::Key << "EnableWeather" << YAML::Value << m_Config.EnableWeather;
			out << YAML::EndMap; // General

			// MenusGeneral Section
			out << YAML::Key << "MenusGeneral" << YAML::Value;
			out << YAML::BeginMap; // MenusGeneral
			out << YAML::Key << "MenuToggleOption" << YAML::Value << m_Config.ToggleStateMenus;
			out << YAML::Key << "MenuToggleAllState" << YAML::Value << m_Config.ToggleAllStateMenus;
			for (size_t i = 0; i < m_Config.TechniqueMenuInfoList.size(); i++)
			{
				const auto& menuInfo = m_Config.TechniqueMenuInfoList[i];
				out << YAML::Key << "MenuToggleSpecificFile" << i + 1 << YAML::Value << menuInfo.Filename;
				out << YAML::Key << "MenuToggleSpecificState" << i + 1 << YAML::Value << menuInfo.State;
				out << YAML::Key << "MenuToggleSpecificMenu" << i + 1 << YAML::Value << menuInfo.Name;
			}
			out << YAML::EndMap; // MenusGeneral

			// Save MenusProcess section
			out << YAML::Key << "MenusProcess" << YAML::Value;
			out << YAML::BeginMap; // MenusProcess
			for (size_t i = 0; i < m_Config.MenuList.size(); i++)
			{
				const auto& menuData = m_Config.MenuList[i];
				out << YAML::Key << "Menu" << i + 1 << YAML::Value << menuData.Name;
			}
			out << YAML::EndMap; // MenusProcess

			// Save Time Section
			out << YAML::Key << "Time" << YAML::Value;
			out << YAML::BeginMap; // Time
			out << YAML::Key << "TimeUpdateInterval" << YAML::Value << m_Config.TimeUpdateIntervalTime;
			out << YAML::Key << "TimeToggleOption" << YAML::Value << m_Config.ToggleStateTime;
			out << YAML::Key << "TimeToggleAllState" << YAML::Value << m_Config.ToggleAllStateTime;
			for (const auto& info : m_Config.TechniqueTimeInfoListAll)
			{
				out << YAML::Key << "TimeToggleAllTimeStart" << YAML::Value << info.StartTime;
				out << YAML::Key << "TimeToggleAllTimeStop" << YAML::Value << info.StopTime;
			}
			for (size_t i = 0; i < m_Config.TechniqueTimeInfoList.size(); i++)
			{
				const auto& timeInfo = m_Config.TechniqueTimeInfoList[i];
				out << YAML::Key << "TimeToggleSpecificFile" << i + 1 << YAML::Value << timeInfo.Filename;
				out << YAML::Key << "TimeToggleSpecificState" << i + 1 << YAML::Value << timeInfo.State;
				out << YAML::Key << "TimeToggleSpecificTimeStart" << i + 1 << YAML::Value << timeInfo.StartTime;
				out << YAML::Key << "TimeToggleSpecificTimeStop" << i + 1 << YAML::Value << timeInfo.StopTime;
			}
			out << YAML::EndMap; // Time

			// Save Interior section
			out << YAML::Key << "Interior" << YAML::Value;
			out << YAML::BeginMap; // Interior
			out << YAML::Key << "InteriorUpdateInterval" << YAML::Value << m_Config.TimeUpdateIntervalInterior;
			out << YAML::Key << "InteriorToggleOption" << YAML::Value << m_Config.ToggleStateInterior;
			out << YAML::Key << "InteriorToggleAllState" << YAML::Value << m_Config.ToggleAllStateInterior;
			for (size_t i = 0; i < m_Config.TechniqueInteriorInfoList.size(); i++)
			{
				const auto& interiorInfo = m_Config.TechniqueInteriorInfoList[i];
				out << YAML::Key << "InteriorToggleSpecificFile" << i + 1 << YAML::Value << interiorInfo.Filename;
				out << YAML::Key << "InteriorToggleSpecificState" << i + 1 << YAML::Value << interiorInfo.State;
			}
			out << YAML::EndMap; // Interior

			// Save Weather section
			out << YAML::Key << "Weather" << YAML::Value;
			out << YAML::BeginMap; // Weather
			out << YAML::Key << "WeatherUpdateInterval" << YAML::Value << m_Config.TimeUpdateIntervalWeather;
			out << YAML::Key << "WeatherToggleOption" << YAML::Value << m_Config.ToggleStateWeather;
			out << YAML::Key << "WeatherToggleAllState" << YAML::Value << m_Config.ToggleAllStateWeather;
			for (size_t i = 0; i < m_Config.TechniqueWeatherInfoList.size(); i++)
			{
				const auto& weatherInfo = m_Config.TechniqueWeatherInfoList[i];
				out << YAML::Key << "WeatherToggleSpecificFile" << i + 1 << YAML::Value << weatherInfo.Filename;
				out << YAML::Key << "WeatherToggleSpecificState" << i + 1 << YAML::Value << weatherInfo.State;
				out << YAML::Key << "WeatherToggleSpecificWeather" << i + 1 << YAML::Value << weatherInfo.Name;
			}
			out << YAML::EndMap; // Weather

			// Save WeatherProcess section
			out << YAML::Key << "WeatherProcess" << YAML::Value;
			out << YAML::BeginMap; // WeatherProcess
			for (size_t i = 0; i < m_Config.WeatherList.size(); i++)
			{
				const auto& weatherData = m_Config.WeatherList[i];
				out << YAML::Key << "Weather" << i + 1 << YAML::Value << weatherData.Name;
			}
			out << YAML::EndMap; // WeatherProcess
		}
		out << YAML::EndMap; // Root

		std::ofstream fout(fullPath);
		fout << out.c_str();
		return true;
	}
}

bool Config::DeserializePreset(const std::string& presetName)
{

	YAML::Node configNode;
	try
	{
		configNode = YAML::LoadFile(presetName);
	}
	catch (const YAML::ParserException& e)
	{
		SKSE::log::critical("Failed to Parse: '{0}' \n\t{1}", presetName.c_str(), e.what());
		return false;
	}

	// General section
	const auto& generalNode = configNode["General"];
	m_Config.EnableMenus = generalNode["EnableMenus"].as<bool>();
	m_Config.EnableTime = generalNode["EnableTime"].as<bool>();
	m_Config.EnableInterior = generalNode["EnableInterior"].as<bool>();
	m_Config.EnableWeather = generalNode["EnableWeather"].as<bool>();

	// MenusGeneral section
	const auto& menusGeneralNode = configNode["MenusGeneral"];
	m_Config.ToggleStateMenus = menusGeneralNode["MenuToggleOption"].as<std::string>();
	m_Config.ToggleAllStateMenus = menusGeneralNode["MenuToggleAllState"].as<std::string>();
	size_t menuCount = menusGeneralNode.size() - 2; // Exclude toggle options
	m_Config.TechniqueMenuInfoList.clear();
	for (size_t i = 1; i <= menuCount; ++i)
	{
		std::string fileKey = "MenuToggleSpecificFile" + std::to_string(i);
		std::string stateKey = "MenuToggleSpecificState" + std::to_string(i);
		std::string menuKey = "MenuToggleSpecificMenu" + std::to_string(i);
		TechniqueInfo menuInfo;
		menuInfo.Filename = menusGeneralNode[fileKey].as<std::string>();
		menuInfo.State = menusGeneralNode[stateKey].as<std::string>();
		menuInfo.Name = menusGeneralNode[menuKey].as<std::string>();
		m_Config.TechniqueMenuInfoList.push_back(menuInfo);
	}

	// Time section
	const auto& timeNode = configNode["Time"];
	m_Config.TimeUpdateIntervalTime = timeNode["TimeUpdateInterval"].as<int>();
	m_Config.ToggleStateTime = timeNode["TimeToggleOption"].as<std::string>();
	m_Config.ToggleAllStateTime = timeNode["TimeToggleAllState"].as<std::string>();

	// Parse TechniqueTimeInfoListAll
	const auto& timeToggleAllNode = timeNode["TimeToggleAll"];
	for (const auto& techniqueNode : timeToggleAllNode)
	{
		TechniqueInfo info;
		info.StartTime = techniqueNode["StartTime"].as<double>();
		info.StopTime = techniqueNode["StopTime"].as<double>();
		m_Config.TechniqueTimeInfoListAll.push_back(info);
	}

	// Parse TechniqueTimeInfoList
	const auto& timeToggleSpecificNode = timeNode["TimeToggleSpecific"];
	for (const auto& techniqueNode : timeToggleSpecificNode) 
	{
		TechniqueInfo info;
		info.Filename = techniqueNode["Filename"].as<std::string>();
		info.State = techniqueNode["State"].as<std::string>();
		info.StartTime = techniqueNode["StartTime"].as<double>();
		info.StopTime = techniqueNode["StopTime"].as<double>();
		m_Config.TechniqueTimeInfoList.push_back(info);
	}

	// Interior section
	const auto& interiorNode = configNode["Interior"];
	m_Config.TimeUpdateIntervalInterior = interiorNode["InteriorUpdateInterval"].as<int>();
	m_Config.ToggleStateInterior = interiorNode["InteriorToggleOption"].as<std::string>();
	m_Config.ToggleAllStateInterior = interiorNode["InteriorToggleAllState"].as<std::string>();

	// Parse TechniqueInteriorInfoList
	const auto& interiorToggleSpecificNode = interiorNode["InteriorToggleSpecific"];
	for (const auto& techniqueNode : interiorToggleSpecificNode) 
	{
		TechniqueInfo info;
		info.Filename = techniqueNode["Filename"].as<std::string>();
		info.State = techniqueNode["State"].as<std::string>();
		m_Config.TechniqueInteriorInfoList.push_back(info);
	}

	// Weather section
	const auto& weatherNode = configNode["Weather"];
	m_Config.TimeUpdateIntervalWeather = weatherNode["WeatherUpdateInterval"].as<int>();
	m_Config.ToggleStateWeather = weatherNode["WeatherToggleOption"].as<std::string>();
	m_Config.ToggleAllStateWeather = weatherNode["WeatherToggleAllState"].as<std::string>();

	// Parse TechniqueWeatherInfoList
	const auto& weatherToggleSpecificNode = weatherNode["WeatherToggleSpecific"];
	for (const auto& techniqueNode : weatherToggleSpecificNode) 
	{
		TechniqueInfo info;
		info.Filename = techniqueNode["Filename"].as<std::string>();
		info.State = techniqueNode["State"].as<std::string>();
		info.Name = techniqueNode["Name"].as<std::string>();
		m_Config.TechniqueWeatherInfoList.push_back(info);
	}

	// WeatherProcess section
	const auto& weatherProcessNode = configNode["WeatherProcess"];
	m_Config.WeatherList.clear();
	for (const auto& weatherData : weatherProcessNode) 
	{
		Info info;
		info.Name = weatherData["Name"].as<std::string>();
		m_Config.WeatherList.push_back(info);
	}

	return true;
}

void Config::LoadPreset(const std::string& Preset)
{
	SKSE::log::info("Starting clear procedure...");
	std::string fullPath = "Data\\SKSE\\Plugins\\TogglerConfigs\\" + Preset + ".yaml";

	m_Config.EnableMenus = false;
	m_Config.EnableTime = false;
	m_Config.EnableInterior = false;
	m_Config.EnableWeather = false;

	// Empty every vector
	m_Config.SpecificMenu.clear();
	m_Config.TechniqueMenuInfoList.clear();
	m_Config.MenuList.clear();
	m_Config.ToggleStateMenus.clear();
	m_Config.ToggleAllStateMenus.clear();
	m_Config.ItemMenuShaderToToggle = nullptr;
	m_Config.ItemMenuStateValue = nullptr;
	m_Config.ItemSpecificMenu = nullptr;

	m_Config.TechniqueTimeInfoList.clear();
	m_Config.TechniqueTimeInfoListAll.clear();
	m_Config.SpecificTime.clear();
	m_Config.ToggleStateTime.clear();
	m_Config.ToggleAllStateTime.clear();
	m_Config.ItemTimeShaderToToggle = nullptr;
	m_Config.ItemTimeStateValue = nullptr;
	m_Config.ItemTimeStartHour = 0.0;
	m_Config.ItemTimeStopHour = 0.0;
	m_Config.ItemTimeStartHourAll = 0.0;
	m_Config.ItemTimeStopHourAll = 0.0;
	m_Config.TimeUpdateIntervalTime = 0;

	m_Config.TechniqueInteriorInfoList.clear();
	m_Config.SpecificInterior.clear();
	m_Config.ToggleStateInterior.clear();
	m_Config.ToggleAllStateInterior.clear();
	m_Config.ItemInteriorShaderToToggle = nullptr;
	m_Config.ItemInteriorStateValue = nullptr;
	m_Config.TimeUpdateIntervalInterior = 0;

	m_Config.SpecificWeather.clear();
	m_Config.WeatherList.clear();
	m_Config.TechniqueWeatherInfoList.clear();
	m_Config.ToggleStateWeather.clear();
	m_Config.ToggleAllStateWeather.clear();
	m_Config.Weatherflags.clear();
	m_Config.ItemWeatherShaderToToggle = nullptr;
	m_Config.ItemWeatherStateValue = nullptr;
	m_Config.ItemSpecificWeather = nullptr;
	m_Config.TimeUpdateIntervalWeather = 0;

	SKSE::log::info("Finished clearing procedure...");

	// Load the new INI
	DeserializePreset(fullPath);
}