#pragma once

class Config
{
public:

	static Config* GetSingleton()
	{
		static Config s;
		return &s;
	}

	void LoadINI(const std::string& presetPath);
	void LoadPreset(const std::string& Preset);

private:

	//Menu
	const char* m_itemMenuShaderToToggle;
	const char* m_itemMenuStateValue;
	const char* m_itemSpecificMenu;

	std::vector<std::string> m_specificMenu;

	//Time
	const char* m_itemTimeShaderToToggle;
	const char* m_itemTimeStateValue;

	double m_itemTimeStartHour;
	double m_itemTimeStopHour;
	double m_itemTimeStartHourAll;
	double m_itemTimeStopHourAll;

	std::vector<std::string> m_specificTime;

	//Interior
	const char* m_itemInteriorShaderToToggle;
	const char* m_itemInteriorStateValue;

	std::vector<std::string> m_specificInterior;


	//Weather
	const char* m_itemWeatherShaderToToggle;
	const char* m_itemWeatherStateValue;
	const char* m_itemSpecificWeather;

	std::vector<std::string> m_specificWeather;

};