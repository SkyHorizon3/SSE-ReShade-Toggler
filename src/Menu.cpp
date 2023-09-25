#include "../include/Globals.h"
#include "../include/Menu.h"
#include "../include/ReshadeToggler.h"
#include "../include/ReshadeIntegration.h"

void Menu::SettingsMenu()
{
	if (ImGui::Button("Save"))
	{
	}

	if (ImGui::Button("Refresh"))
	{
		//ReshadeToggler::GetSingleton().LoadINI();
	}

	if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		RenderInfoPage();
	}
	if (ImGui::CollapsingHeader("Menus", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		RenderMenusPage();
	}
	if (ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		RenderTimePage();

		if (ImGui::Button("+", ImVec2(30, 30)))
		{
			// Fill new info with default values
			TechniqueInfo info;
			info.filename = "Default.fx";
			info.state = "off";
			info.startTime = 0.0;
			info.stopTime = 0.0;

			techniqueTimeInfoList.push_back(info);
		}
	}
}

bool Menu::CreateCombo(const char* label, std::string& currentItem, std::vector<std::string>& items, ImGuiComboFlags_ flags)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float w = 200.0f;
	float spacing = style.ItemInnerSpacing.x;
	float button_sz = ImGui::GetFrameHeight();
	ImGui::PushItemWidth(w - spacing - button_sz * 2.0f);

	bool itemChanged = false;

	if (ImGui::BeginCombo(label, currentItem.c_str(), flags))
	{
		for (std::string& item : items)
		{
			bool isSelected = (currentItem == item);
			if (ImGui::Selectable(item.c_str(), isSelected))
			{
				currentItem = item;
				itemChanged = true;
			}
			if (isSelected) { ImGui::SetItemDefaultFocus(); }
		}
		ImGui::EndCombo();
	}

	ImGui::PopItemWidth();

	return itemChanged;
}


void Menu::RenderInfoPage()
{
	ImGui::Checkbox("Enable Menu", &EnableMenus);
	ImGui::Checkbox("Enable Time", &EnableTime);
	ImGui::Checkbox("Enable Weather", &EnableWeather);
	ImGui::Checkbox("Enable Interior", &EnableInterior);
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
		for (int i = 0; i < techniqueTimeInfoList.size(); i++)
		{
			auto& timeInfo = techniqueTimeInfoList[i];

			if (timeInfo.filename != "" && timeInfo.state != "")
			{
				// Create IDs for every element in the vector that is to be rendered
				std::string effectComboID = "##Effect" + std::to_string(i);
				std::string effectStateID = "##State" + std::to_string(i);
				std::string startTimeID = "##StartTime" + std::to_string(i);
				std::string stopTimeID = "##StopTime" + std::to_string(i);

				CreateCombo(effectComboID.c_str(), timeInfo.filename, g_Effects, ImGuiComboFlags_None);
				ImGui::SameLine();
				CreateCombo(effectStateID.c_str(), timeInfo.state, g_State, ImGuiComboFlags_None);

				ImGui::SetNextItemWidth(200.0f);
				ImGui::SliderScalar(startTimeID.c_str(), ImGuiDataType_Double, &timeInfo.startTime, &minTime, &maxTime, "%.2f");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200.0f);
				ImGui::SliderScalar(stopTimeID.c_str(), ImGuiDataType_Double, &timeInfo.stopTime, &minTime, &maxTime, "%.2f");

				ImGui::Separator();
			}
		}
	}
}