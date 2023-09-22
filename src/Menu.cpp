#include "../include/Globals.h"
#include "../include/Menu.h"
#include "../include/ReshadeToggler.h"

void Menu::SettingsMenu()
{	
	if (ImGui::Button("Save"))
	{}

	if (ImGui::Button("Refresh"))
	{
		//ReshadeToggler::GetSingleton().LoadINI();
	}

	if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		RenderInfoPage();
	}
	if (ImGui::CollapsingHeader("Menus"), ImGuiTreeNodeFlags_CollapsingHeader)
	{
		RenderMenusPage();
	}
	if (ImGui::CollapsingHeader("Time"), ImGuiTreeNodeFlags_CollapsingHeader)
	{
		RenderTimePage();
	}
}

void Menu::RenderInfoPage()
{
	ImGui::Checkbox("Enable Menu", &EnableMenus);
	ImGui::Checkbox("Enable Time", &EnableTime);
	ImGui::Checkbox("EnableWeather", &EnableWeather);
	ImGui::Checkbox("EnableInterior", &EnableInterior);
}

void Menu::RenderMenusPage()
{
	for (int i = 0; i < g_INImenus.size(); ++i)
	{
		ImGui::Text("%s: %s", g_INImenus.at(i).c_str());
	}
}

void Menu::RenderTimePage()
{}

