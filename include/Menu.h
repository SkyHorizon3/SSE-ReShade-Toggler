#pragma once
#include "Config.h"

class Menu
{
public:
	static Menu* GetSingleton()
	{
		static Menu menu;
		return &menu;
	}

	void SettingsMenu();

	void EnumerateEffects();
	void EnumeratePresets();
	void EnumerateMenus();
private:

	bool CreateCombo(const char* label, std::string& currentItem, std::vector<std::string>& items, ImGuiComboFlags_ flags);

	void SaveConfig();

	void RenderInfoPage();
	void RenderMenusPage();
	void RenderTimePage();
	void RenderInteriorPage();
	void RenderWeatherPage();

private:
	double minTime = 0.0;
	double maxTime = 23.59;

	std::string m_SaveFilename = "Default2"; // Default filename

	bool saveConfigPopupOpen = false; // Flag to control the visibility of the Save Config popup
	char inputBuffer[256] = { 0 };    // Initialize the input buffer

	bool m_LoadPresetPopupOpen = false;

	std::vector<std::string> m_Effects;
	std::vector<std::string> m_Presets;
	std::vector<std::string> m_MenuNames;

	std::vector<std::string> m_EffectState = { "on", "off" };
	std::vector<std::string> m_ToggleState = { "All", "Specific" };

	std::vector<std::string> m_WeatherFlags = {
	"kNone",
	"kPleasant",
	"kCloudy",
	"kRainy",
	"kSnow",
	"kPermAurora",
	"kAuroraFollowsSun"
	};

	Configuration m_Conf = Config::GetSingleton()->GetConfig();

public:

	std::string selectedPreset = "Default.yaml";
	std::string selectedPresetPath = "Data\\SKSE\\Plugins\\TogglerConfigs\\Default.yaml";
};