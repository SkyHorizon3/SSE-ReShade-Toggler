#pragma once

#include <filesystem>
#include <vector>


struct TechniqueInfo
{
	std::string Filename = "";
	std::string State = "";
	std::string Name = "";
	double StartTime = 0.0;
	double StopTime = 0.0;
	bool Enable = true;
};

struct Info
{
	std::string Index = "";
	std::string Name = "";
};

struct Configuration
{
    // General
    bool EnableMenus = true;
    bool EnableTime = true;
    bool EnableInterior = true;
    bool EnableWeather = true;

    // Menu
    const char* ItemMenuShaderToToggle;
    const char* ItemMenuStateValue;
    const char* ItemSpecificMenu;
    std::vector<std::string> SpecificMenu;

    // Time
    const char* ItemTimeShaderToToggle;
    const char* ItemTimeStateValue;
    double ItemTimeStartHour;
    double ItemTimeStopHour;
    double ItemTimeStartHourAll;
    double ItemTimeStopHourAll;
    std::vector<std::string> SpecificTime;

    // Interior
    const char* ItemInteriorShaderToToggle;
    const char* ItemInteriorStateValue;
    std::vector<std::string> SpecificInterior;

    // Weather
    const char* ItemWeatherShaderToToggle;
    const char* ItemWeatherStateValue;
    const char* ItemSpecificWeather;
    std::vector<std::string> SpecificWeather;

    // Menus
    std::vector<TechniqueInfo> TechniqueMenuInfoList;
    std::vector<Info> MenuList;
    std::string ToggleStateMenus;
    std::string ToggleAllStateMenus;

    // Time
    std::vector<TechniqueInfo> TechniqueTimeInfoList;
    std::vector<TechniqueInfo> TechniqueTimeInfoListAll;
    std::string ToggleStateTime;
    std::string ToggleAllStateTime;
    int TimeUpdateIntervalTime;

    // Interior
    std::vector<TechniqueInfo> TechniqueInteriorInfoList;
    std::string ToggleStateInterior;
    std::string ToggleAllStateInterior;
    int TimeUpdateIntervalInterior;
    bool IsInInteriorCell = false;

    // Weather
    std::vector<Info> WeatherList;
    std::vector<TechniqueInfo> TechniqueWeatherInfoList;
    std::string ToggleStateWeather;
    std::string ToggleAllStateWeather;
    std::string Weatherflags;
    int TimeUpdateIntervalWeather;
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

	Configuration GetConfig() { return m_Config; }

private:
	Configuration m_Config;

};