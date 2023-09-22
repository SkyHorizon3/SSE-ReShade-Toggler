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
	void RenderInfoPage();
	void RenderMenusPage();
	void RenderTimePage();
};