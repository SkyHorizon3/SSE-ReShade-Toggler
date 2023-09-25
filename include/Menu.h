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

	void RenderInfoPage();
	void RenderMenusPage();
	void RenderTimePage();


	double minTime = 0.0;
	double maxTime = 23.59;
};