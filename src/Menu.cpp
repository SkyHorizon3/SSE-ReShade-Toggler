#include "../include/Globals.h"
#include "../include/Menu.h"
#include "../include/ReshadeToggler.h"
#include "../include/ReshadeIntegration.h"
#include "../include/Processor.h"

// Forward declare the threads
void TimeThread();
void InteriorThread();

void Menu::SettingsMenu()
{
	if (ImGui::Button("Save"))
	{
	}

	if (ImGui::Button("Refresh"))
	{
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

		// Add new 
		if (ImGui::Button("Add", ImVec2(45, 20)))
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

	if (ImGui::Checkbox("Enable Time", &EnableTime))
	{
		if (EnableTime && loaded)
		{
			Processor::GetSingleton().ProcessTimeBasedToggling();
			std::thread(TimeThread).detach();
		}
	}

	if (ImGui::Checkbox("Enable Interior", &EnableInterior))
	{
		if (EnableInterior && loaded)
		{
			Processor::GetSingleton().ProcessInteriorBasedToggling();
			std::thread(InteriorThread).detach();
		}
	}

	ImGui::Checkbox("Enable Weather", &EnableWeather);
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
				std::string effectComboID = "Effect##" + std::to_string(i);
				std::string effectStateID = "State##" + std::to_string(i);
				std::string startTimeID = "StartTime##" + std::to_string(i);
				std::string stopTimeID = "StopTime##" + std::to_string(i);
				std::string removeID = "Remove##" + std::to_string(i);

				std::string currentEffectFileName = timeInfo.filename;
				std::string currentEffectState = timeInfo.state;
				double currentStartTime = timeInfo.startTime;
				double currentStopTime = timeInfo.stopTime;

				bool valueChanged = false;

				if (CreateCombo(effectComboID.c_str(), currentEffectFileName, g_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
				ImGui::SameLine();
				if (CreateCombo(effectStateID.c_str(), currentEffectState, g_State, ImGuiComboFlags_None)) { valueChanged = true; }
				ImGui::SetNextItemWidth(200.0f);
				if (ImGui::SliderScalar(startTimeID.c_str(), ImGuiDataType_Double, &currentStartTime, &minTime, &maxTime, "%.2f")) { valueChanged = true; }
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200.0f);
				if (ImGui::SliderScalar(stopTimeID.c_str(), ImGuiDataType_Double, &currentStopTime, &minTime, &maxTime, "%.2f")) { valueChanged = true; }

				// Add a button to remove the effect
				if (ImGui::Button(removeID.c_str(), ImVec2(45, 20)))
				{
					if (!techniqueTimeInfoList.empty())
					{
						techniqueTimeInfoList.erase(techniqueTimeInfoList.begin() + i);
						i--;  // Decrement i to stay at the current index after removing the element
					}
				}

				ImGui::Separator();

				if (valueChanged)
				{
					// Update new values
					timeInfo.filename = currentEffectFileName;
					timeInfo.state = currentEffectState;
					timeInfo.startTime = currentStartTime;
					timeInfo.stopTime = currentStopTime;

					//ImGui::Text("New Values for %i: Effect: %s - State: %s - Start: %.2f - Stop: %.2f", i, timeInfo.filename.c_str(), timeInfo.state.c_str(), timeInfo.startTime, timeInfo.stopTime);
				}
			}
		}
	}
}