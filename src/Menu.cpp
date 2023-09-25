#include "../include/Globals.h"
#include "../include/Menu.h"
#include "../include/ReshadeToggler.h"
#include "../include/ReshadeIntegration.h"

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
	if (ImGui::CollapsingHeader("Menus", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		RenderMenusPage();
	}
	if (ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		RenderTimePage();
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

    double minTime = 0.0;
    double maxTime = 23.59;

    if (!techniqueTimeInfoList.empty())
    {
        for (auto& timeInfo : techniqueTimeInfoList)
        {
            if (timeInfo.filename != "" && timeInfo.state != "")
            {
                CreateCombo("##Effect", timeInfo.filename, g_Effects, ImGuiComboFlags_None);
                ImGui::SameLine();
                CreateCombo("##State", timeInfo.state, g_State, ImGuiComboFlags_None);

                ImGui::SetNextItemWidth(200.0f);
                ImGui::SliderScalar("Start Time", ImGuiDataType_Double, &timeInfo.startTime, &minTime, &maxTime, "%.2f");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(200.0f);
                ImGui::SliderScalar("Stop Time", ImGuiDataType_Double, &timeInfo.stopTime, &minTime, &maxTime, "%.2f");
            }
        }
    }
}

