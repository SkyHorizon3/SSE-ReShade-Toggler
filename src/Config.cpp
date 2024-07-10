#include "Config.h"
#include "Utils.h"
#include "yaml-cpp/yaml.h"

GeneralInformation Config::m_GeneralInformation;
MenuInformation Config::m_MenuInformation;
TimeInformation Config::m_TimeInformation;
InteriorInformation Config::m_InteriorInformation;
WeatherInformation Config::m_WeatherInformation;

bool Config::SerializePreset(const std::string& presetName)
{
	const std::string configDirectory = "Data\\SKSE\\Plugins\\TogglerConfigs";
	std::filesystem::create_directories(configDirectory);
	const std::string& fullPath = configDirectory + "\\" + presetName + ".yaml";

	/*
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
	*/
	return true;
}

void Config::DeserializeGeneral(YAML::Node mainNode)
{
	const auto& generalNode = mainNode["General"];
	m_GeneralInformation.EnableMenus = generalNode["EnableMenus"].as<bool>();
	m_GeneralInformation.EnableTime = generalNode["EnableTime"].as<bool>();
	m_GeneralInformation.EnableInterior = generalNode["EnableInterior"].as<bool>();
	m_GeneralInformation.EnableWeather = generalNode["EnableWeather"].as<bool>();
}

void Config::DeserializeMenu(YAML::Node mainNode)
{
	const auto& menusGeneralNode = mainNode["MenusGeneral"];
	m_MenuInformation.MenuToggleOption = menusGeneralNode["MenuToggleOption"].as<std::string>();
	m_MenuInformation.MenuToggleAllState = menusGeneralNode["MenuToggleAllState"].as<std::string>();

	for (YAML::const_iterator it = menusGeneralNode.begin(); it != menusGeneralNode.end(); ++it)
	{
		const std::string& key = it->first.as<std::string>();
		if (key.find("MenuToggleSpecificFile") != std::string::npos)
		{
			const std::string& line = it->second.as<std::string>();
			std::vector<std::string> tokens = Utils::SplitString(line, '|');

			if (tokens.size() >= 3)
			{
				TechniqueInfo menuInfo;
				menuInfo.Filename = tokens[0];
				menuInfo.State = tokens[1];
				menuInfo.Name = tokens[2];
				m_MenuInformation.TechniqueMenuInfoList.emplace_back(menuInfo);
			}
		}
	}

	// MenusProcess
	const auto& menusProcessNode = mainNode["MenusProcess"];
	for (YAML::const_iterator it = menusProcessNode.begin(); it != menusProcessNode.end(); ++it)
	{
		const std::string& key = it->first.as<std::string>();
		if (key.find("Menu") != std::string::npos)
		{
			const auto& line = it->second.as<std::string>();
			m_MenuInformation.MenuList.emplace_back(line);
		}
	}
}

void Config::DeserializeTime(YAML::Node mainNode)
{
	const auto& timeGeneralNode = mainNode["Time"];
	m_TimeInformation.TimeUpdateInterval = timeGeneralNode["TimeUpdateInterval"].as<int>();
	m_TimeInformation.TimeToggleOption = timeGeneralNode["TimeToggleOption"].as<std::string>();
	m_TimeInformation.TimeToggleAllState = timeGeneralNode["TimeToggleAllState"].as<std::string>();
	m_TimeInformation.TimeToggleAllTimeStart = timeGeneralNode["TimeToggleAllTimeStart"].as<double>();
	m_TimeInformation.TimeToggleAllTimeStop = timeGeneralNode["TimeToggleAllTimeStop"].as<double>();

	for (YAML::const_iterator it = timeGeneralNode.begin(); it != timeGeneralNode.end(); ++it)
	{
		const std::string& key = it->first.as<std::string>();
		if (key.find("TimeToggleSpecificFile") != std::string::npos)
		{
			const std::string& line = it->second.as<std::string>();
			std::vector<std::string> tokens = Utils::SplitString(line, '|');

			if (tokens.size() >= 4)
			{
				TechniqueInfo timeInfo;
				timeInfo.Filename = tokens[0];
				timeInfo.State = tokens[1];
				timeInfo.StartTime = std::stod(tokens[2]);
				timeInfo.StopTime = std::stod(tokens[3]);
				m_TimeInformation.TechniqueTimeInfoList.emplace_back(timeInfo);
			}
		}
	}
}

void Config::DeserializeInterior(YAML::Node mainNode)
{
	const auto& interiorGeneralNode = mainNode["Interior"];
	m_InteriorInformation.InteriorUpdateInterval = interiorGeneralNode["InteriorUpdateInterval"].as<int>();
	m_InteriorInformation.InteriorToggleOption = interiorGeneralNode["InteriorToggleOption"].as<std::string>();
	m_InteriorInformation.InteriorToggleAllState = interiorGeneralNode["InteriorToggleAllState"].as<std::string>();

	for (YAML::const_iterator it = interiorGeneralNode.begin(); it != interiorGeneralNode.end(); ++it)
	{
		const std::string& key = it->first.as<std::string>();
		if (key.find("InteriorToggleSpecificFile") != std::string::npos)
		{
			const std::string& line = it->second.as<std::string>();
			std::vector<std::string> tokens = Utils::SplitString(line, '|');

			if (tokens.size() >= 2)
			{
				TechniqueInfo interiorInfo;
				interiorInfo.Filename = tokens[0];
				interiorInfo.State = tokens[1];
				m_InteriorInformation.TechniqueInteriorInfoList.emplace_back(interiorInfo);
			}
		}
	}

}

void Config::DeserializeWeather(YAML::Node mainNode)
{
	const auto& weatherGeneralNode = mainNode["Weather"];
	m_WeatherInformation.WeatherUpdateInterval = weatherGeneralNode["WeatherUpdateInterval"].as<int>();
	m_WeatherInformation.WeatherToggleOption = weatherGeneralNode["WeatherToggleOption"].as<std::string>();
	m_WeatherInformation.WeatherToggleAllState = weatherGeneralNode["WeatherToggleAllState"].as<std::string>();

	for (YAML::const_iterator it = weatherGeneralNode.begin(); it != weatherGeneralNode.end(); ++it)
	{
		const std::string& key = it->first.as<std::string>();
		if (key.find("WeatherToggleSpecificFile") != std::string::npos)
		{
			const std::string& line = it->second.as<std::string>();
			std::vector<std::string> tokens = Utils::SplitString(line, '|');

			if (tokens.size() >= 3)
			{
				TechniqueInfo weatherInfo;
				weatherInfo.Filename = tokens[0];
				weatherInfo.State = tokens[1];
				weatherInfo.Name = tokens[2];
				m_WeatherInformation.TechniqueWeatherInfoList.emplace_back(weatherInfo);
			}
		}
	}

	const auto& weatherProcessNode = mainNode["WeatherProcess"];
	for (YAML::const_iterator it = weatherProcessNode.begin(); it != weatherProcessNode.end(); ++it)
	{
		const std::string& key = it->first.as<std::string>();
		if (key.find("WeatherFlag") != std::string::npos)
		{
			const auto& line = it->second.as<std::string>();
			m_WeatherInformation.WeatherList.emplace_back(line);
		}
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

	DeserializeGeneral(configNode);
	DeserializeMenu(configNode);
	DeserializeTime(configNode);
	DeserializeInterior(configNode);
	DeserializeWeather(configNode);

	return true;
}

void Config::LoadPreset(const std::string& Preset)
{
	SKSE::log::info("Starting clear procedure...");

	// General
	m_GeneralInformation.EnableMenus = false;
	m_GeneralInformation.EnableTime = false;
	m_GeneralInformation.EnableInterior = false;
	m_GeneralInformation.EnableWeather = false;

	// Menu
	m_MenuInformation.MenuToggleOption.clear();
	m_MenuInformation.MenuToggleAllState.clear();
	m_MenuInformation.TechniqueMenuInfoList.clear();
	m_MenuInformation.MenuList.clear();

	// Time
	m_TimeInformation.TimeUpdateInterval = 0;
	m_TimeInformation.TimeToggleOption.clear();
	m_TimeInformation.TimeToggleAllState.clear();
	m_TimeInformation.TimeToggleAllTimeStart = 0.0;
	m_TimeInformation.TimeToggleAllTimeStop = 0.0;
	m_TimeInformation.TechniqueTimeInfoList.clear();

	// Interior
	m_InteriorInformation.InteriorUpdateInterval = 0;
	m_InteriorInformation.InteriorToggleOption.clear();
	m_InteriorInformation.InteriorToggleAllState.clear();
	m_InteriorInformation.TechniqueInteriorInfoList.clear();

	m_WeatherInformation.WeatherUpdateInterval = 0;
	m_WeatherInformation.WeatherToggleOption.clear();
	m_WeatherInformation.WeatherToggleAllState.clear();
	m_WeatherInformation.TechniqueWeatherInfoList.clear();
	m_WeatherInformation.WeatherList.clear();

	SKSE::log::info("Finished clearing procedure...");

	// Load the new INI
	DeserializePreset("Data\\SKSE\\Plugins\\TogglerConfigs\\" + Preset + ".yaml");
}