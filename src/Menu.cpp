#include "Menu.h"
#include "Manager.h"
#include "Utils.h"

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
		static int currentTab = 0;


		if (ImGui::BeginTabBar("SettingsTabBar"))
		{
			if (ImGui::BeginTabItem("Main Page"))
			{
				currentTab = 0;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Menu Settings"))
			{
				currentTab = 1;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Time Settings"))
			{
				currentTab = 2;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Interior Settings"))
			{
				currentTab = 3;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Weather Settings"))
			{
				currentTab = 4;
				ImGui::EndTabItem();
			}
		}

		ImGuiID dockspaceId = ImGui::GetID("SettingsDockspace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		switch (currentTab)
		{
		case 0: SpawnMainPage(dockspaceId); break;
		case 1: SpawnMenuSettings(dockspaceId); break;
		case 2: SpawnTimeSettings(dockspaceId); break;
		case 3: SpawnInteriorSettings(dockspaceId); break;
		case 4: SpawnWeatherSettings(dockspaceId); break;
		}

		RemoveColors();
	}
}

void Menu::SpawnMainPage(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoCollapse);

	CreateCombo("Select Preset", m_selectedPreset, m_presets, ImGuiComboFlags_None);
	ImGui::SameLine();
	if (ImGui::Button("Reload Preset List"))
	{
		m_presets.clear();
		m_presets = Manager::GetSingleton()->enumeratePresets();
	}

	if (ImGui::Button("Load Preset"))
	{
		const std::string selectedPresetPath = Manager::GetSingleton()->getPresetPath(m_selectedPreset);
		if (std::filesystem::exists(selectedPresetPath))
		{
			const auto start = std::chrono::high_resolution_clock::now();
			const bool success = Manager::GetSingleton()->parseJSONPreset(m_selectedPreset);
			const auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> duration = end - start;

			if (!success)
			{
				m_lastMessage = "Failed to load preset: '" + m_selectedPreset + "'.";
				m_lastMessageColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			}
			else
			{
				Manager::GetSingleton()->setLastPreset(m_selectedPreset);
				Manager::GetSingleton()->serializeINI();

				m_lastMessage = "Successfully loaded preset: '" + m_selectedPreset + "'! Took: " + std::to_string(duration.count()) + "ms";
				m_lastMessageColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
			}
		}
		else
		{
			m_lastMessage = "Tried to load preset '" + selectedPresetPath + "'. Preset doesn't exist";
			m_lastMessageColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		m_saveConfigPopupOpen = true;
		m_inputBuffer[0] = '\0';
	}
	SaveFile();

	if (!m_lastMessage.empty())
	{
		ImGui::TextColored(m_lastMessageColor, "%s", m_lastMessage.c_str());
	}

	ImGui::End();
}

void Menu::SpawnMenuSettings(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Menu Settings", &m_showMenuSettings, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Configure menu toggling settings here.");

	ImGui::SeparatorText("Effects");

	// Get the list of toggle information
	std::vector<MenuToggleInformation> infoList = Manager::GetSingleton()->getMenuToggleInfo();
	std::vector<MenuToggleInformation> updatedInfoList = infoList;

	// Group menus
	std::unordered_map<std::string, std::vector<MenuToggleInformation>> menuEffectMap;
	for (auto& info : infoList)
	{
		if (!info.effectName.empty())
		{
			menuEffectMap[info.menuName].emplace_back(info);
		}
	}

	static char inputBuffer[256] = "";
	ImGui::InputTextWithHint("##Search", "Search Menus...", inputBuffer, sizeof(inputBuffer));

	int headerId = -1;
	int globalIndex = 0;
	static std::string currentEditingEffect;

	for (const auto& [menuName, effects] : menuEffectMap)
	{
		if (strlen(inputBuffer) > 0 && menuName.find(inputBuffer) == std::string::npos)
			continue;

		headerId++;
		std::string headerUniqueId = menuName + std::to_string(headerId);

		if (ImGui::CollapsingHeader((menuName + "##" + headerUniqueId + "##Header").c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::BeginTable(("EffectsTable##" + headerUniqueId).c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
			ImGui::TableSetupColumn(("Effect##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("State##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("Actions##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("MenuName##" + headerUniqueId).c_str());
			ImGui::TableHeadersRow();

			for (int i = 0; i < effects.size(); i++, globalIndex++)
			{
				MenuToggleInformation info = effects[i];
				bool valueChanged = false;

				std::string effectComboId = "Effect##" + headerUniqueId + std::to_string(i);
				std::string effectStateId = "State##" + headerUniqueId + std::to_string(i);
				std::string removeId = "RemoveEffect##" + headerUniqueId + std::to_string(i);
				std::string editId = "EditEffect##" + headerUniqueId + std::to_string(i);

				std::string currentEffectName = info.effectName;
				bool currentEffectState = info.state;

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (CreateCombo(effectComboId.c_str(), currentEffectName, m_effects, ImGuiComboFlags_None)) { valueChanged = true; }
				ImGui::TableNextColumn();
				if (ImGui::Checkbox(effectStateId.c_str(), &currentEffectState)) { valueChanged = true; }
				ImGui::TableNextColumn();
				if (ImGui::Button(removeId.c_str()))
				{
					updatedInfoList.erase(updatedInfoList.begin() + globalIndex);
					globalIndex--;
					continue;
				}
				if (ImGui::Button(editId.c_str())) 
				{
					currentEditingEffect = currentEffectName;
					ImGui::OpenPopup("Edit Effect Values");
				}

				ImGui::TableNextColumn();
				ImGui::Text("%s", menuName.c_str());

				if (valueChanged)
				{
					info.menuName = menuName;
					info.effectName = currentEffectName;
					info.state = currentEffectState;
					updatedInfoList[globalIndex] = info;
				}

			}

			EditValues(currentEditingEffect);
			ImGui::EndTable();
		}
	}


	Manager::GetSingleton()->setMenuToggleInfo(updatedInfoList);

	ImGui::SeparatorText("Add New");
	// Add new effect
	if (ImGui::Button("Add New Effect"))
	{
		ImGui::OpenPopup("Create Menu Entry");
	}
	AddNewMenu(updatedInfoList);

	ImGui::End();
}

void Menu::AddNewMenu(std::vector<MenuToggleInformation>& updatedInfoList)
{
	static std::string currentMenu;
	static std::string currentEffect;
	static bool toggled = false;

	if (ImGui::BeginPopupModal("Create Menu Entry", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Reset static variables for each popup
		if (ImGui::IsWindowAppearing())
		{
			currentMenu.clear();
			currentEffect.clear();
			toggled = false;
		}

		ImGui::Text("Select a Menu");
		CreateCombo("Menu", currentMenu, m_menuNames, ImGuiComboFlags_None);
		ImGui::Separator();

		ImGui::Text("Select the Effect");
		CreateCombo("Effect", currentEffect, m_effects, ImGuiComboFlags_None);
		ImGui::SameLine();
		ImGui::Checkbox("Toggled On", &toggled);

		ImGui::Separator();
		if (ImGui::Button("Finish"))
		{
			MenuToggleInformation info;
			info.menuName = currentMenu;
			info.effectName = currentEffect;
			info.state = toggled;

			updatedInfoList.emplace_back(info);
			Manager::GetSingleton()->setMenuToggleInfo(updatedInfoList);

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Menu::SpawnTimeSettings(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Time Settings", &m_showTimeSettings, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Configure time toggling settings here.");

	ImGui::SeparatorText("Effects");

	// Get the list of toggle information
	std::map<std::string, std::vector<TimeToggleInformation>> infoList = Manager::GetSingleton()->getTimeToggleInfo();
	std::map<std::string, std::vector<TimeToggleInformation>> updatedInfoList = infoList;
	static char inputBuffer[256] = "";
	ImGui::InputTextWithHint("##Search", "Search Worldspace...", inputBuffer, sizeof(inputBuffer));

	int headerId = -1;
	int globalIndex = 0;
	for (const auto& [cellName, effects] : infoList)
	{
		if (strlen(inputBuffer) > 0 && cellName.find(inputBuffer) == std::string::npos)
			continue;

		headerId++;
		std::string headerUniqueId = cellName + std::to_string(headerId);

		if (ImGui::CollapsingHeader((cellName + "##" + headerUniqueId + "##Header").c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::BeginTable(("EffectsTable##" + headerUniqueId).c_str(), 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
			ImGui::TableSetupColumn(("Effect##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("State##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("StartTime##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("StopTime##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("Actions##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("Cell##" + headerUniqueId).c_str());
			ImGui::TableHeadersRow();

			for (int i = 0; i < effects.size(); i++, globalIndex++)
			{
				TimeToggleInformation info = effects[i];
				bool valueChanged = false;

				std::string effectComboId = "Effect##" + headerUniqueId + std::to_string(i);
				std::string effectStateId = "State##" + headerUniqueId + std::to_string(i);
				std::string startTimeId = "StartTime##" + headerUniqueId + std::to_string(i);
				std::string stopTimeId = "StopTime##" + headerUniqueId + std::to_string(i);
				std::string removeId = "RemoveEffect##" + headerUniqueId + std::to_string(i);
				std::string editId = "EditEffect##" + headerUniqueId + std::to_string(i);

				std::string currentEffectName = info.effectName;
				float currentStartTime = info.startTime;
				float currentStopTime = info.stopTime;
				bool currentEffectState = info.state;

				int startHours = static_cast<int>(currentStartTime);
				int startMinutes = static_cast<int>((currentStartTime - startHours) * 100);
				int stopHours = static_cast<int>(currentStopTime);
				int stopMinutes = static_cast<int>((currentStopTime - stopHours) * 100);

				static char startHourStr[3] = "00";
				static char startMinuteStr[3] = "00";
				static char stopHourStr[3] = "00";
				static char stopMinuteStr[3] = "00";

				snprintf(startHourStr, sizeof(startHourStr), "%02d", startHours);
				snprintf(startMinuteStr, sizeof(startMinuteStr), "%02d", startMinutes);
				snprintf(stopHourStr, sizeof(stopHourStr), "%02d", stopHours);
				snprintf(stopMinuteStr, sizeof(stopMinuteStr), "%02d", stopMinutes);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (CreateCombo(effectComboId.c_str(), currentEffectName, m_effects, ImGuiComboFlags_None)) { valueChanged = true; }
				ImGui::TableNextColumn();
				if (ImGui::Checkbox(effectStateId.c_str(), &currentEffectState)) { valueChanged = true; }

				// Start time
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(35);
				if (ImGui::InputText(("##StartHours" + startTimeId).c_str(), startHourStr, sizeof(startHourStr), ImGuiInputTextFlags_CharsDecimal)) { valueChanged = true; }
				ClampInputValue(startHourStr, 23);
				if (strcmp(startHourStr, "00") == 0 && strlen(startHourStr) == 0) strcpy(startHourStr, "0");
				ImGui::SameLine();
				ImGui::Text(":");
				ImGui::SameLine();
				if (ImGui::InputText(("##StartMinutes" + startTimeId).c_str(), startMinuteStr, sizeof(startMinuteStr), ImGuiInputTextFlags_CharsDecimal)) { valueChanged = true; }
				ClampInputValue(startMinuteStr, 59);
				if (strcmp(startMinuteStr, "00") == 0 && strlen(startMinuteStr) == 0) strcpy(startMinuteStr, "0");
				ImGui::PopItemWidth();

				// Stop Time
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(35);
				if (ImGui::InputText(("##StopHours" + stopTimeId).c_str(), stopHourStr, sizeof(stopHourStr), ImGuiInputTextFlags_CharsDecimal)) { valueChanged = true; }
				ClampInputValue(stopHourStr, 23);
				if (strcmp(stopHourStr, "00") == 0 && strlen(stopHourStr) == 0) strcpy(stopHourStr, "0");
				ImGui::SameLine();
				ImGui::Text(":");
				ImGui::SameLine();
				if (ImGui::InputText(("##StopMinutes" + stopTimeId).c_str(), stopMinuteStr, sizeof(stopMinuteStr), ImGuiInputTextFlags_CharsDecimal)) { valueChanged = true; }
				ClampInputValue(stopMinuteStr, 59);
				if (strcmp(stopMinuteStr, "00") == 0 && strlen(stopMinuteStr) == 0) strcpy(stopMinuteStr, "0");
				ImGui::PopItemWidth();

				// Convert baby
				startHours = strlen(startHourStr) > 0 ? std::stoi(startHourStr) : 0;
				startMinutes = strlen(startMinuteStr) > 0 ? std::stoi(startMinuteStr) : 0;
				stopHours = strlen(stopHourStr) > 0 ? std::stoi(stopHourStr) : 0;
				stopMinutes = strlen(stopMinuteStr) > 0 ? std::stoi(stopMinuteStr) : 0;

				currentStartTime = startHours + (startMinutes / 100.0f);
				currentStopTime = stopHours + (stopMinutes / 100.0f);

				if (currentStartTime > currentStopTime)
				{
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "Invalid Time Range");
				}

				ImGui::TableNextColumn();
				if (ImGui::Button(removeId.c_str()))
				{
					updatedInfoList[cellName].erase(
						std::remove_if(updatedInfoList[cellName].begin(), updatedInfoList[cellName].end(),
						[&info](const TimeToggleInformation& timeInfo) {
							return timeInfo.effectName == info.effectName;
						}
					),
						updatedInfoList[cellName].end()
					);

					if (updatedInfoList[cellName].empty())
					{
						updatedInfoList.erase(cellName);
					}

					globalIndex--;
					continue;
				}
				if (ImGui::Button(editId.c_str())) { ImGui::Text("I do nothing"); }
				ImGui::TableNextColumn();
				ImGui::Text("%s", cellName.c_str());

				if (valueChanged)
				{
					info.effectName = currentEffectName;
					info.startTime = currentStartTime;
					info.stopTime = currentStopTime;
					info.state = currentEffectState;

					updatedInfoList[cellName].at(i) = info;
				}
			}
			ImGui::EndTable();
		}
	}

	Manager::GetSingleton()->setTimeToggleInfo(updatedInfoList);

	ImGui::SeparatorText("Add New");

	// Add new effect
	if (ImGui::Button("Add New Effect"))
	{
		ImGui::OpenPopup("Create Time Entry");
	}
	AddNewTime(updatedInfoList);
	ImGui::End();
}

void Menu::SpawnInteriorSettings(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Interior Settings", &m_showInteriorSettings, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Configure interior toggling settings here.");

	// Retrieve the current weather toggle info
	std::map<std::string, std::vector<InteriorToggleInformation>> infoList = Manager::GetSingleton()->getInteriorToggleInfo();
	std::map<std::string, std::vector<InteriorToggleInformation>> updatedInfoList = infoList; // Start with existing info
	static char inputBuffer[256] = "";
	ImGui::InputTextWithHint("##Search", "Search Interior Cell...", inputBuffer, sizeof(inputBuffer));

	int headerId = -1;
	int globalIndex = 0;
	for (const auto& [cellName, effects] : infoList)
	{
		if (strlen(inputBuffer) > 0 && cellName.find(inputBuffer) == std::string::npos)
			continue;

		headerId++;
		std::string headerUniqueId = cellName + std::to_string(headerId);

		if (ImGui::CollapsingHeader((cellName + "##" + headerUniqueId + "##Header").c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::BeginTable(("EffectsTable##" + headerUniqueId).c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
			ImGui::TableSetupColumn(("Effect##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("State##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("Actions##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("Cell##" + headerUniqueId).c_str());
			ImGui::TableHeadersRow();

			for (int i = 0; i < effects.size(); i++, globalIndex++)
			{
				InteriorToggleInformation info = effects[i];
				bool valueChanged = false;

				std::string effectComboId = "Effect##" + headerUniqueId + std::to_string(i);
				std::string effectStateId = "State##" + headerUniqueId + std::to_string(i);
				std::string removeId = "RemoveEffect##" + headerUniqueId + std::to_string(i);
				std::string editId = "EditEffect##" + headerUniqueId + std::to_string(i);

				std::string currentEffectName = info.effectName;
				bool currentEffectState = info.state;

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (CreateCombo(effectComboId.c_str(), currentEffectName, m_effects, ImGuiComboFlags_None)) { valueChanged = true; }
				ImGui::TableNextColumn();
				if (ImGui::Checkbox(effectStateId.c_str(), &currentEffectState)) { valueChanged = true; }
				ImGui::TableNextColumn();
				if (ImGui::Button(removeId.c_str()))
				{
					updatedInfoList[cellName].erase(
						std::remove_if(updatedInfoList[cellName].begin(), updatedInfoList[cellName].end(),
						[&info](const InteriorToggleInformation& interiorInfo) {
							return interiorInfo.effectName == info.effectName;
						}
					),
						updatedInfoList[cellName].end()
					);

					if (updatedInfoList[cellName].empty())
					{
						updatedInfoList.erase(cellName);
					}

					globalIndex--;
					continue;
				}
				if (ImGui::Button(editId.c_str())) { ImGui::Text("I do nothing"); }
				ImGui::TableNextColumn();
				ImGui::Text("%s", cellName.c_str());

				if (valueChanged)
				{
					info.effectName = currentEffectName;
					info.state = currentEffectState;
					updatedInfoList[cellName].at(i) = info;
				}
			}
			ImGui::EndTable();
		}

	}

	// Update the manager with the new list
	Manager::GetSingleton()->setInteriorToggleInfo(updatedInfoList);

	ImGui::SeparatorText("Add New");
	// Add new effect
	if (ImGui::Button("Add New Effect"))
	{
		ImGui::OpenPopup("Create Interior Entry");
	}
	AddNewInterior(updatedInfoList);

	ImGui::End();
}

void Menu::SpawnWeatherSettings(ImGuiID dockspace_id)
{
	ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
	ImGui::Begin("Weather Settings", &m_showWeatherSettings, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Configure weather toggling settings here.");
	ImGui::SeparatorText("Effects");

	// Retrieve the current weather toggle info
	std::map<std::string, std::vector<WeatherToggleInformation>> infoList = Manager::GetSingleton()->getWeatherToggleInfo();
	std::map<std::string, std::vector<WeatherToggleInformation>> updatedInfoList = infoList; // Start with existing info
	static char inputBuffer[256] = "";
	ImGui::InputTextWithHint("##Search", "Search Worldspaces...", inputBuffer, sizeof(inputBuffer));

	int headerId = -1;
	int globalIndex = 0;
	for (const auto& [worldSpaceName, effects] : infoList)
	{
		if (strlen(inputBuffer) > 0 && worldSpaceName.find(inputBuffer) == std::string::npos)
			continue;

		headerId++;
		std::string headerUniqueId = worldSpaceName + std::to_string(headerId);

		if (ImGui::CollapsingHeader((worldSpaceName + "##" + headerUniqueId + "##Header").c_str(), ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::BeginTable(("EffectsTable##" + headerUniqueId).c_str(), 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
			ImGui::TableSetupColumn(("Effect##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("State##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("Weather##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("Actions##" + headerUniqueId).c_str());
			ImGui::TableSetupColumn(("Worldspace##" + headerUniqueId).c_str());
			ImGui::TableHeadersRow();

			for (int i = 0; i < effects.size(); i++, globalIndex++)
			{
				WeatherToggleInformation info = effects[i];
				bool valueChanged = false;

				std::string effectComboId = "Effect##" + headerUniqueId + std::to_string(i);
				std::string effectStateId = "State##" + headerUniqueId + std::to_string(i);
				std::string weatherId = "Weather##" + headerUniqueId + std::to_string(i);
				std::string removeId = "RemoveEffect##" + headerUniqueId + std::to_string(i);
				std::string editId = "EditEffect##" + headerUniqueId + std::to_string(i);

				std::string currentEffectName = info.effectName;
				std::string currentWeather = info.weatherFlag;
				bool currentEffectState = info.state;

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (CreateCombo(effectComboId.c_str(), currentEffectName, m_effects, ImGuiComboFlags_None)) { valueChanged = true; }
				ImGui::TableNextColumn();
				if (ImGui::Checkbox(effectStateId.c_str(), &currentEffectState)) { valueChanged = true; }
				ImGui::TableNextColumn();
				if (CreateCombo(weatherId.c_str(), currentWeather, m_weatherFlags, ImGuiComboFlags_None)) { valueChanged = true; }
				ImGui::TableNextColumn();
				if (ImGui::Button(removeId.c_str()))
				{
					updatedInfoList[worldSpaceName].erase(
						std::remove_if(updatedInfoList[worldSpaceName].begin(), updatedInfoList[worldSpaceName].end(),
						[&info](const WeatherToggleInformation& weatherInfo) {
							return weatherInfo.effectName == info.effectName && weatherInfo.weatherFlag == info.weatherFlag;
						}
					),
						updatedInfoList[worldSpaceName].end()
					);

					if (updatedInfoList[worldSpaceName].empty())
					{
						updatedInfoList.erase(worldSpaceName);
					}

					globalIndex--;
					continue;
				}
				if (ImGui::Button(editId.c_str())) { ImGui::Text("I do nothing"); }
				ImGui::TableNextColumn();
				ImGui::Text("%s", worldSpaceName.c_str());

				if (valueChanged)
				{
					info.effectName = currentEffectName;
					info.weatherFlag = currentWeather;
					info.state = currentEffectState;
					updatedInfoList[worldSpaceName].at(i) = info;
				}
			}
			ImGui::EndTable();
		}

	}

	// Update the manager with the new list
	Manager::GetSingleton()->setWeatherToggleInfo(updatedInfoList);

	ImGui::SeparatorText("Add New");
	// Add new effect
	if (ImGui::Button("Add New Effect"))
	{
		ImGui::OpenPopup("Create Weather Entry");
	}
	AddNewWeather(updatedInfoList);

	ImGui::End();
}

void Menu::AddNewTime(std::map<std::string, std::vector<TimeToggleInformation>>& updatedInfoList)
{
	static std::string currentWorldSpace;
	static std::string currentEffect;
	static float currentStartTime;
	static float currentStopTime;
	static bool toggled = false;

	static char startHourStr[3] = "00", startMinuteStr[3] = "00";
	static char stopHourStr[3] = "00", stopMinuteStr[3] = "00";

	if (ImGui::BeginPopupModal("Create Time Entry", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Reset static variables for each popup
		if (ImGui::IsWindowAppearing())
		{
			currentWorldSpace.clear();
			currentEffect.clear();
			toggled = false;
			strcpy(startHourStr, "00");
			strcpy(startMinuteStr, "00");
			strcpy(stopHourStr, "00");
			strcpy(stopMinuteStr, "00");
		}

		ImGui::Text("Select a Worldspace");
		CreateCombo("Worldspace", currentWorldSpace, m_worldSpaces, ImGuiComboFlags_None);
		ImGui::Separator();

		// Start time
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(35);
		ImGui::InputText("##StartHours", startHourStr, sizeof(startHourStr), ImGuiInputTextFlags_CharsDecimal);
		ClampInputValue(startHourStr, 23);
		if (strcmp(startHourStr, "00") == 0 && strlen(startHourStr) == 0) strcpy(startHourStr, "0");
		ImGui::SameLine();
		ImGui::Text(":");
		ImGui::SameLine();
		ImGui::InputText("##StartMinutes", startMinuteStr, sizeof(startMinuteStr), ImGuiInputTextFlags_CharsDecimal);
		ClampInputValue(startMinuteStr, 59);
		if (strcmp(startMinuteStr, "00") == 0 && strlen(startMinuteStr) == 0) strcpy(startMinuteStr, "0");
		ImGui::PopItemWidth();

		// Stop Time
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(35);
		ImGui::InputText("##StopHours", stopHourStr, sizeof(stopHourStr), ImGuiInputTextFlags_CharsDecimal);
		ClampInputValue(stopHourStr, 23);
		if (strcmp(stopHourStr, "00") == 0 && strlen(stopHourStr) == 0) strcpy(stopHourStr, "0");
		ImGui::SameLine();
		ImGui::Text(":");
		ImGui::SameLine();
		ImGui::InputText("##StopMinutes", stopMinuteStr, sizeof(stopMinuteStr), ImGuiInputTextFlags_CharsDecimal);
		ClampInputValue(stopMinuteStr, 59);
		if (strcmp(stopMinuteStr, "00") == 0 && strlen(stopMinuteStr) == 0) strcpy(stopMinuteStr, "0");
		ImGui::PopItemWidth();

		ImGui::Separator();
		ImGui::Text("Select the Effect");
		CreateCombo("Effect", currentEffect, m_effects, ImGuiComboFlags_None);
		ImGui::SameLine();
		ImGui::Checkbox("Toggled On", &toggled);

		ImGui::Separator();
		if (ImGui::Button("Finish"))
		{
			// Convert the input text to integers
			int startHours = strlen(startHourStr) > 0 ? std::stoi(startHourStr) : 0;
			int startMinutes = strlen(startMinuteStr) > 0 ? std::stoi(startMinuteStr) : 0;
			int stopHours = strlen(stopHourStr) > 0 ? std::stoi(stopHourStr) : 0;
			int stopMinutes = strlen(stopMinuteStr) > 0 ? std::stoi(stopMinuteStr) : 0;

			// Convert hours and minutes into float (HH.MM) format
			currentStartTime = startHours + (startMinutes / 100.0f);
			currentStopTime = stopHours + (stopMinutes / 100.0f);

			TimeToggleInformation info;
			info.effectName = currentEffect;
			info.startTime = currentStartTime;
			info.stopTime = currentStopTime;
			info.state = toggled;

			updatedInfoList[currentWorldSpace].emplace_back(info);
			Manager::GetSingleton()->setTimeToggleInfo(updatedInfoList);

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Menu::ClampInputValue(char* inputStr, int maxVal)
{
	int val = strlen(inputStr) > 0 ? std::stoi(inputStr) : 0;
	if (val > maxVal)
	{
		val = maxVal;
		snprintf(inputStr, 3, "%02d", val);
	}
}

void Menu::EditValues(const std::string& effectName)
{
	if (ImGui::BeginPopupModal("Edit Effect Values", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
		ImGui::Text(("Editing " + effectName + ":").c_str());


		// Get the uniform information
		std::vector<UniformInfo> uniforms = Manager::GetSingleton()->enumerateUniformNames(effectName);

		// Loop through each uniform to display its name and type
		for (const auto& uniformInfo : uniforms)
		{
			// Get the type of the uniform variable
			std::string type = Manager::GetSingleton()->getUniformType(uniformInfo.uniformVariable);

			// Display the uniform name and its type
			ImGui::Text("%s: %s", uniformInfo.uniformName.c_str(), type.c_str());
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Menu::AddNewInterior(std::map<std::string, std::vector<InteriorToggleInformation>>& updatedInfoList)
{
	static std::string currentCell;
	static std::string currentEffect;
	static bool toggled = false;

	if (ImGui::BeginPopupModal("Create Interior Entry", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Reset static variables for each popup
		if (ImGui::IsWindowAppearing())
		{
			currentCell.clear();
			currentEffect.clear();
			toggled = false;
		}

		ImGui::Text("Select an Interior Cell");
		CreateCombo("Cell", currentCell, m_interiorCells, ImGuiComboFlags_None);
		ImGui::Separator();

		ImGui::Text("Select the Effect");
		CreateCombo("Effect", currentEffect, m_effects, ImGuiComboFlags_None);
		ImGui::SameLine();
		ImGui::Checkbox("Toggled On", &toggled);

		ImGui::Separator();
		if (ImGui::Button("Finish"))
		{
			InteriorToggleInformation info;
			info.effectName = currentEffect;
			info.state = toggled;

			updatedInfoList[currentCell].emplace_back(info);
			Manager::GetSingleton()->setInteriorToggleInfo(updatedInfoList);

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Menu::AddNewWeather(std::map<std::string, std::vector<WeatherToggleInformation>>& updatedInfoList)
{
	static std::string currentWorldSpace;
	static std::string currentWeatherFlag;
	static std::string currentEffect;
	static bool toggled = false;

	if (ImGui::BeginPopupModal("Create Weather Entry", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Reset static variables for each popup
		if (ImGui::IsWindowAppearing())
		{
			currentWorldSpace.clear();
			currentWeatherFlag.clear();
			currentEffect.clear();
			toggled = false;
		}

		ImGui::Text("Select a Worldspace");
		CreateCombo("Worldspace", currentWorldSpace, m_worldSpaces, ImGuiComboFlags_None);
		ImGui::Text("Select a Weather");
		CreateCombo("Weather", currentWeatherFlag, m_weatherFlags, ImGuiComboFlags_None);
		ImGui::Separator();

		ImGui::Text("Select the Effect");
		CreateCombo("Effect", currentEffect, m_effects, ImGuiComboFlags_None);
		ImGui::SameLine();
		ImGui::Checkbox("Toggled On", &toggled);

		ImGui::Separator();
		if (ImGui::Button("Finish"))
		{
			WeatherToggleInformation info;
			info.weatherFlag = currentWeatherFlag;
			info.effectName = currentEffect;
			info.state = toggled;

			updatedInfoList[currentWorldSpace].emplace_back(info);
			Manager::GetSingleton()->setWeatherToggleInfo(updatedInfoList);

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

bool Menu::CreateCombo(const char* label, std::string& currentItem, std::vector<std::string>& items, ImGuiComboFlags_ flags)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float w = 500.0f;
	float spacing = style.ItemInnerSpacing.x;
	float button_sz = ImGui::GetFrameHeight();
	ImGui::PushItemWidth(w - spacing - button_sz * 2.0f);

	bool itemChanged = false;

	static char searchBuffer[256] = "";

	if (ImGui::BeginCombo(label, currentItem.c_str(), flags))
	{
		ImGui::InputTextWithHint("##Search", "Search...", searchBuffer, sizeof(searchBuffer));

		std::vector<std::string> filteredItems;
		for (const auto& item : items)
		{
			if (strcasestr(item.c_str(), searchBuffer))
			{
				filteredItems.push_back(item);
			}
		}

		for (std::string& item : filteredItems)
		{
			bool isSelected = (currentItem == item);
			if (ImGui::Selectable(item.c_str(), isSelected))
			{
				currentItem = item;
				itemChanged = true;
				searchBuffer[0] = '\0';
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
				filename = filename + ".json";
				const auto start = std::chrono::high_resolution_clock::now();
				const bool success = Manager::GetSingleton()->serializeJSONPreset(filename);
				const auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> duration = end - start;

				// update preset list
				m_presets.clear();
				m_presets = Manager::GetSingleton()->enumeratePresets();

				if (!success)
				{
					m_lastMessage = "Failed to save preset '" + filename + "'.";
					m_lastMessageColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
				}
				else
				{
					m_selectedPreset = filename;
					Manager::GetSingleton()->setLastPreset(m_selectedPreset);
					Manager::GetSingleton()->serializeINI();

					m_lastMessage = "Successfully saved Preset: '" + filename + "'! Took: " + std::to_string(duration.count()) + "ms";
					m_lastMessageColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
				}

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
