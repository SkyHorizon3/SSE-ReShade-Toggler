#pragma once

struct TechniqueInfo
{
	std::string filename = "";
	std::string state = "";
	std::string Name = "";
	double startTime = 0.0;
	double stopTime = 0.0;
	bool enable = true;
};

struct Info
{
	std::string Index = "";
	std::string Name = "";
};

inline bool isLoaded = false;

// General
inline bool EnableMenus = true;
inline bool EnableTime = true;
inline bool EnableInterior = true;
inline bool EnableWeather = true;


// Menus
inline std::vector<TechniqueInfo> techniqueMenuInfoList;
inline std::vector<Info> menuList;

inline std::string ToggleStateMenus;
inline std::string ToggleAllStateMenus;

// Time
inline std::vector<TechniqueInfo> techniqueTimeInfoList;
inline std::vector<TechniqueInfo> techniqueTimeInfoListAll;

inline std::string ToggleStateTime;
inline std::string ToggleAllStateTime;

inline int TimeUpdateIntervalTime;

//Interior
inline std::vector<TechniqueInfo> techniqueInteriorInfoList;

inline std::string ToggleStateInterior;
inline std::string ToggleAllStateInterior;

inline int TimeUpdateIntervalInterior;

inline bool IsInInteriorCell = false;

//Weather
inline std::vector<Info> weatherList;
inline std::vector<TechniqueInfo> techniqueWeatherInfoList;

inline std::string ToggleStateWeather;
inline std::string ToggleAllStateWeather;
inline std::string weatherflags;

inline int TimeUpdateIntervalWeather;