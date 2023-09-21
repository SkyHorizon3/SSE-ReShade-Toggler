#include "../include/Globals.h"
#include "../include/Menu.h"

void Menu::SettingsMenu()
{	
	if (ImGui::Button("Hello"))
	{
		DEBUG_LOG(g_Logger, "Hello!", nullptr);
	}
}
