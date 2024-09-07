#include "Menu.h"
#include "Manager.h"


void Menu::SettingsMenu()
{
	if (ImGui::Button("[PH] Configure SSEReshadeToggler"))
		m_openSettingsMenu = true;

	if (m_openSettingsMenu)
	{
		// TODO: ensure that we are only putting the colors onto our own window and its subwindows
		SetColors();

		// Create the main settings window with docking enabled
		ImGui::Begin("[PH] Settings Window", &m_openSettingsMenu);
		// Create a DockSpace inside the main settings window
		ImGuiID dockspaceId = ImGui::GetID("SettingsDockspace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		SpawnMainPage(dockspaceId);
		SpawnMenuSettings(dockspaceId);
		SpawnTimeSettings(dockspaceId);
		SpawnInteriorSettings(dockspaceId);
		SpawnWeatherSettings(dockspaceId);

		RemoveColors();
	}
}

void Menu::SpawnMainPage(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoCollapse);
	m_presets = Manager::GetSingleton()->EnumeratePresets();

	CreateCombo("Select Preset", m_selectedPreset, m_presets, ImGuiComboFlags_None);
	ImGui::SameLine();
	if (ImGui::Button("Reload Preset List"))
	{
		m_presets.clear();
		m_presets = Manager::GetSingleton()->EnumeratePresets();
	}

	if (ImGui::Button("Load Preset"))
	{
		std::string selectedPresetPath = "Data\\SKSE\\Plugins\\ReShadeEffectTogglerPresets\\" + m_selectedPreset;
		if (std::filesystem::exists(selectedPresetPath))
		{
			Manager::GetSingleton()->parseJSONPreset(m_selectedPreset);
		}
		else
		{
			SKSE::log::error("Tried to log preset '{}'. Preset doesn't exist", selectedPresetPath);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		m_saveConfigPopupOpen = true;
		m_inputBuffer[0] = '\0';
	}
	SaveFile();

	ImGui::End();
}
void Menu::SpawnMenuSettings(ImGuiID dockspace_id)
{
	// Use SetNextWindowDockID to dock this window to the dockspace
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Menu Settings", &m_showMenuSettings, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Configure menu toggling settings here.");
	ImGui::End();
}

void Menu::SpawnTimeSettings(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Time Settings", &m_showTimeSettings, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Configure time toggling settings here.");
	ImGui::End();
}

void Menu::SpawnInteriorSettings(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Interior Settings", &m_showInteriorSettings, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Configure interior toggling settings here.");
	ImGui::End();
}

void Menu::SpawnWeatherSettings(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Weather Settings", &m_showWeatherSettings, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Configure weather toggling settings here.");
	ImGui::End();
}


bool Menu::CreateCombo(const char* label, std::string& currentItem, std::vector<std::string>& items, ImGuiComboFlags_ flags)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float w = 250.0f;
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

void Menu::SaveFile()
{
	if (m_saveConfigPopupOpen)
	{
		ImGui::OpenPopup("Save Config");

		// Check if the "Save Config" modal is open
		if (ImGui::BeginPopupModal("Save Config", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Enter the filename:");

			ImGui::InputText("##FileName", m_inputBuffer, sizeof(m_inputBuffer));

			if (ImGui::Button("Ok, Save!"))
			{
				// Use the provided filename or the default if empty
				std::string filename = (m_inputBuffer[0] != '\0') ? m_inputBuffer : "NewPreset";

				Manager::GetSingleton()->serializeJSONPreset(filename);

				//Refresh
				m_presets.clear();
				m_presets = Manager::GetSingleton()->EnumeratePresets();

				ImGui::CloseCurrentPopup();
				m_saveConfigPopupOpen = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
				m_saveConfigPopupOpen = false;
			}

			ImGui::EndPopup();
		}
	}
}

#pragma region Colors
void Menu::SetColors()
{
	auto& style = ImGui::GetStyle();
	auto& colors = ImGui::GetStyle().Colors;

	//========================================================
	/// Colours

	// Headers
	colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
	colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
	colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);

	// Buttons
	colors[ImGuiCol_Button] = ImColor(56, 56, 56, 200);
	colors[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
	colors[ImGuiCol_ButtonActive] = ImColor(56, 56, 56, 150);

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);
	colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);
	colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);

	// Tabs
	colors[ImGuiCol_Tab] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
	colors[ImGuiCol_TabHovered] = ImColor(255, 225, 135, 30);
	colors[ImGuiCol_TabActive] = ImColor(255, 225, 135, 60);
	colors[ImGuiCol_TabUnfocused] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
	colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];

	// Title
	colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
	colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Resize Grip
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

	// Check Mark
	colors[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

	// Slider
	colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

	// Text
	colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::text);

	// Checkbox
	colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::text);

	// Separator
	colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);
	colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::highlight);
	colors[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

	// Window Background
	colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
	colors[ImGuiCol_ChildBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::background);
	colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundPopup);
	colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);

	// Tables
	colors[ImGuiCol_TableHeaderBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
	colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);

	// Menubar
	colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };

	//========================================================
	/// Style
	style.FrameRounding = 2.5f;
	style.FrameBorderSize = 1.0f;
	style.IndentSpacing = 11.0f;
}

void Menu::RemoveColors()
{
	ImGui::PopStyleColor(38);
	ImGui::PopStyleVar(3);
}
#pragma endregion
