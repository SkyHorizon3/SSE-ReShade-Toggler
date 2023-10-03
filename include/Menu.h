#pragma once

class Menu
{
public:
	static Menu* GetSingleton()
	{
		static Menu menu;
		return &menu;
	}

	void SettingsMenu();

private:
	bool CreateCombo(const char* label, std::string& currentItem, std::vector<std::string>& items, ImGuiComboFlags_ flags);

	void Save(const std::string& filename);
	void SaveConfig();

	void RenderInfoPage();
	void RenderMenusPage();
	void RenderTimePage();
	void RenderInteriorPage();
	void RenderWeatherPage();

private:
	double minTime = 0.0;
	double maxTime = 23.59;

	std::string m_SaveFilename = "Default2.ini"; // Default filename

	bool saveConfigPopupOpen = false; // Flag to control the visibility of the Save Config popup
	char inputBuffer[256] = { 0 };    // Initialize the input buffer

	bool m_LoadPresetPopupOpen = false;
};

