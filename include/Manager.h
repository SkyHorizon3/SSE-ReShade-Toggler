#pragma once
#include <glaze/json/json_t.hpp>

struct MenuToggleInformation
{
	std::string effectName{};
	std::string menuName{};
	bool state = true;
};

struct WeatherToggleInformation
{
	std::string effectName{};
	std::string weatherFlag{};
	bool state = true;
};

struct InteriorToggleInformation // not final. pair isn't best idea. Will get back to it after weather
{
	std::string effectName{};
	std::pair<RE::FormID, std::string> interiorCell{}; // FormID + Plugin. If not a specific cell, place 0 or something
	bool state = true;
};

struct TimeToggleInformation
{
	std::string effectName{};
	float startTime = 0.f;
	float stopTime = 0.f;
	bool state = true;
};

class Manager : public ISingleton<Manager>
{
	// class for main functions used for all features

public:

	enum class Feature : std::uint32_t
	{
		kMenu,
		kWeather,
		kInterior,
		kTime
	};

	bool parseJSONPreset(const std::string& presetName);

	bool serializeJSONPreset(const std::string& presetName);

	void toggleEffectMenu(const std::set<std::string>& openMenus);

	std::vector<std::string> enumeratePresets();
	std::vector<std::string> enumerateEffects();
	std::vector<std::string> enumerateMenus();
	std::vector<std::string> enumerateWorldSpaces();

	void toggleEffectWeather();

	std::vector<MenuToggleInformation> getMenuToggleInfo() const { return m_menuToggleInfo; }
	void setMenuToggleInfo(const std::vector<MenuToggleInformation>& info) { m_menuToggleInfo = info; }

	std::vector<TimeToggleInformation> getTimeToggleInfo() const { return m_timeToggleInfo; }
	void setTimeToggleInfo(const std::vector<TimeToggleInformation>& info) { m_timeToggleInfo = info; }

	std::unordered_map<std::string, std::vector<WeatherToggleInformation>> getWeatherToggleInfo() const { return m_weatherToggleInfo; }
	void setWeatherToggleInfo(const std::unordered_map<std::string, std::vector<WeatherToggleInformation>>& info) { m_weatherToggleInfo = info; }

private:
	std::string getPresetPath(const std::string& presetName);

	static float getCurrentGameTime();

	void toggleEffect(const char* technique, bool state) const;

	std::vector<MenuToggleInformation> m_menuToggleInfo;
	std::unordered_map<std::string, std::vector<WeatherToggleInformation>> m_weatherToggleInfo;

	std::vector<InteriorToggleInformation> m_interiorToggleInfo;
	std::vector<TimeToggleInformation> m_timeToggleInfo;

	std::pair<RE::TESWorldSpace*, std::vector<WeatherToggleInformation>> m_lastWs;

private:
	template<typename T>
	bool serializeVector(const std::string& key, const std::vector<T>& vec, std::string& output);
	
	template<typename T>
	bool serializeMap(const std::string& key, const std::unordered_map<std::string, std::vector<T>>& map, std::string& output);

	template<typename... Args>
	bool serializeArbitraryData(std::string& output, const Args&... args);

	template<typename T>
	bool deserializeVector(const std::string& key, const glz::json_t& json, std::vector<T>& vec);

	template<typename T>
	bool deserializeMapOfVectors(const std::string& key, const glz::json_t& json, std::unordered_map<std::string, std::vector<T>>& map);

	template<typename... Args>
	bool deserializeArbitraryData(const std::string& buf, Args&... args);

};

extern reshade::api::effect_runtime* s_pRuntime;