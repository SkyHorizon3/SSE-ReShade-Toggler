#pragma once

struct TechniqueInfo
{
	std::string Filename = "";
	std::string State = "";
	std::string Name = "";
	double StartTime = 0.0;
	double StopTime = 0.0;
	bool Enable = true;
};

struct GeneralInformation
{
	bool EnableMenus;
	bool EnableTime;
	bool EnableInterior;
	bool EnableWeather;
};

struct MenuInformation
{
	std::string MenuToggleOption;
	std::string MenuToggleAllState;
	std::vector<TechniqueInfo> TechniqueMenuInfoList;
	std::vector<std::string> MenuList;
};

struct TimeInformation
{
	int TimeUpdateInterval;
	std::string TimeToggleOption;
	std::string TimeToggleAllState;
	double TimeToggleAllTimeStart;
	double TimeToggleAllTimeStop;
	std::vector<TechniqueInfo> TechniqueTimeInfoList;
};

struct InteriorInformation
{
	int InteriorUpdateInterval;
	std::string InteriorToggleOption;
	std::string InteriorToggleAllState;
	std::vector<TechniqueInfo> TechniqueInteriorInfoList;
};

struct WeatherInformation
{
	int WeatherUpdateInterval;
	std::string WeatherToggleOption;
	std::string WeatherToggleAllState;
	std::vector<TechniqueInfo> TechniqueWeatherInfoList;
	std::vector<std::string> WeatherList;
};

class Config
{
public:
	static bool SerializePreset(const std::string& presetName);

	static bool DeserializePreset(const std::string& presetName);
	static void LoadPreset(const std::string& Preset);

	static GeneralInformation m_GeneralInformation;
	static MenuInformation m_MenuInformation;
	static TimeInformation m_TimeInformation;
	static InteriorInformation m_InteriorInformation;
	static WeatherInformation m_WeatherInformation;

private:

	static void DeserializeGeneral(YAML::Node preset);
	static void DeserializeMenu(YAML::Node mainNode);
	static void DeserializeTime(YAML::Node mainNode);
	static void DeserializeInterior(YAML::Node mainNode);
	static void DeserializeWeather(YAML::Node mainNode);
};