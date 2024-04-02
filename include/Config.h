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

	static Config* GetSingleton()
	{
		static Config s;
		return &s;
	}

	bool SerializePreset(const std::string& presetName);

	bool DeserializePreset(const std::string& presetName);
	void LoadPreset(const std::string& Preset);

	GeneralInformation GetGeneralInformation() { return m_GeneralInformation; }
	MenuInformation GetMenuInformation() { return m_MenuInformation; }
	TimeInformation GetTimeInformation() { return m_TimeInformation; }
	InteriorInformation GetInteriorInformation() { return m_InteriorInformation; }
	WeatherInformation GetWeatherInformation() { return m_WeatherInformation; }

private:

	void DeserializeGeneral(YAML::Node preset);
	void DeserializeMenu(YAML::Node mainNode);
	void DeserializeTime(YAML::Node mainNode);
	void DeserializeInterior(YAML::Node mainNode);
	void DeserializeWeather(YAML::Node mainNode);

	GeneralInformation m_GeneralInformation;
	MenuInformation m_MenuInformation;
	TimeInformation m_TimeInformation;
	InteriorInformation m_InteriorInformation;
	WeatherInformation m_WeatherInformation;
};