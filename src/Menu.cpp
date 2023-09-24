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
	if (!menuList.empty())
	{
		for (auto& menu : menuList)
		{
			ImGui::Text("%s: %s", menu.menuIndex.c_str(), menu.menuName.c_str());
		}
	}
}

void Menu::RenderTimePage()
{
	if (!techniqueTimeInfoList.empty())
	{
		for (auto& timeInfo : techniqueTimeInfoList)
		{
			ImGui::Text("Effect: %s - ToggleState:", timeInfo.filename.c_str(), timeInfo.state.c_str());
			ImGui::Text("StartTime: %.2f StopTime: %.2f", timeInfo.startTime, timeInfo.stopTime);
		}
	}
}

