#pragma once

class Menu
{
public:
	static Menu* GetSingleton()
	{
		static Menu menu;
		return &menu;
	}
	void DrawSettingsOverlay(reshade::api::effect_runtime* runtime);
	
	static void DrawSettingsOverlayCallback(reshade::api::effect_runtime*);
};