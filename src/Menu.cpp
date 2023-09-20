#include "../include/PCH.h"
#include "../include/Globals.h"
#include "../include/Menu.h"
#include "../include/Reshade/reshade_overlay.hpp"

void Menu::DrawSettingsOverlay(reshade::api::effect_runtime*)
{	
	// Why tf am I not allowed to render text unto the screen?????
	//ImGui::Begin("SSEReshadeToggler");
	//ImGui::Text("Hello World!");
	//ImGui::End();

	bool CoolBool = true;
	DEBUG_LOG(g_Logger, "Hello from the Menu Stuff!", nullptr);
	//ImGui::Checkbox("E", &CoolBool);
}


void Menu::DrawSettingsOverlayCallback(reshade::api::effect_runtime* runtime)
{
	GetSingleton()->DrawSettingsOverlay(runtime);
}
