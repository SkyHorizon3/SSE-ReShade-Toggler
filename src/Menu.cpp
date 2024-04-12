#include "Menu.h"
#include "ReshadeIntegration.h"
#include "Processor.h"
#include "Config.h"

void Menu::EnumerateEffects()
{
	const std::filesystem::path shadersDirectory = L"reshade-shaders\\Shaders";

	if (std::filesystem::exists(shadersDirectory))
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(shadersDirectory))
		{
			if (entry.is_regular_file() && entry.path().filename().extension() == ".fx")
			{
				m_Effects.emplace_back(entry.path().filename().string());
			}
		}
		//sort files
		std::sort(m_Effects.begin(), m_Effects.end());
	}

}

void Menu::EnumeratePresets()
{
	const std::filesystem::path presetDirectory = L"Data\\SKSE\\Plugins\\TogglerConfigs\\";

	for (const auto& preset : std::filesystem::recursive_directory_iterator(presetDirectory))
	{
		if (preset.is_regular_file() && preset.path().filename().extension() == ".yaml")
		{
			m_Presets.emplace_back(preset.path().filename().string());
		}
	}
	//sort presets
	std::sort(m_Presets.begin(), m_Presets.end());
}

void Menu::EnumerateMenus()
{
	const auto& menuMap = RE::UI::GetSingleton()->menuMap;

	for (const auto& menu : menuMap)
	{
		const auto& menuName = menu.first.c_str();
		m_MenuNames.emplace_back(menuName);
	}

	std::sort(m_MenuNames.begin(), m_MenuNames.end());
}

void Menu::CreateCombo(const char* label, std::string& currentItem, std::vector<std::string>& items, ImGuiComboFlags_ flags)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float w = 250.0f;
	float spacing = style.ItemInnerSpacing.x;
	float button_sz = ImGui::GetFrameHeight();
	ImGui::PushItemWidth(w - spacing - button_sz * 2.0f);

	if (ImGui::BeginCombo(label, currentItem.c_str(), flags))
	{
		for (std::string& item : items)
		{
			bool isSelected = (currentItem == item);
			if (ImGui::Selectable(item.c_str(), isSelected))
			{
				currentItem = item;
			}
			if (isSelected) { ImGui::SetItemDefaultFocus(); }
		}
		ImGui::EndCombo();
	}

	ImGui::PopItemWidth();
}

void Menu::SettingsMenu()
{
	if (ImGui::Button("Save"))
	{
		saveConfigPopupOpen = true; // Open the Save Config popup
		inputBuffer[0] = '\0';     // Clear the input buffer
	}

	SaveConfig();

	CreateCombo("Select Preset", selectedPreset, m_Presets, ImGuiComboFlags_None);
	ImGui::SameLine();
	if (ImGui::Button("Load Preset"))
	{
		selectedPresetPath = "Data\\SKSE\\Plugins\\TogglerConfigs\\" + selectedPreset;

		// Load the selected preset 
		if (std::filesystem::exists(selectedPresetPath))
		{
			Config::GetSingleton()->LoadPreset(selectedPreset);
			CSimpleIniA ini;
			ini.SetUnicode(false);
			ini.SetValue("Presets", "PresetPath", selectedPresetPath.c_str());
			ini.SetValue("Presets", "PresetName", selectedPreset.c_str());
			ini.SaveFile("Data\\SKSE\\Plugins\\ReShadeEffectToggler.ini");
		}
		else
		{
			SKSE::log::info("Tried to load presetpath {}. Preset doesn't exist!", selectedPresetPath);
		}
	}

	if (ImGui::Button("Refresh"))
	{
		m_Presets.clear();
		EnumeratePresets();
	}

	if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		RenderInfoPage();
	}

	if (m_General.EnableMenus)
	{
		if (ImGui::CollapsingHeader("Menus", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderMenusPage();
		}

	}

	if (m_General.EnableTime)
	{
		if (ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderTimePage();
		}
	}

	if (m_General.EnableInterior)
	{
		if (ImGui::CollapsingHeader("Interior", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderInteriorPage();
		}
	}

	if (m_General.EnableWeather)
	{
		if (ImGui::CollapsingHeader("Weather", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderWeatherPage();
		}
	}
}

void Menu::SaveConfig()
{
	if (saveConfigPopupOpen)
	{
		ImGui::OpenPopup("Save Config");

		// Check if the "Save Config" modal is open
		if (ImGui::BeginPopupModal("Save Config", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Enter the filename:");

			// Create an input field for the filename
			ImGui::InputText("##FileName", inputBuffer, sizeof(inputBuffer));

			if (ImGui::Button("Ok, Save!"))
			{
				// Use the provided filename or the default if empty
				std::string filename = (inputBuffer[0] != '\0') ? inputBuffer : m_SaveFilename;

				// Call the Save function with the chosen filename
				Config::GetSingleton()->SerializePreset(filename);

				//Refresh
				m_Presets.clear();
				EnumeratePresets();

				// Close the modal and reset the flag
				ImGui::CloseCurrentPopup();
				saveConfigPopupOpen = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				// Close the modal and reset the flag
				ImGui::CloseCurrentPopup();
				saveConfigPopupOpen = false;
			}

			ImGui::EndPopup();
		}
	}
}

void Menu::RenderInfoPage()
{
	if (ImGui::Checkbox("Enable Menu", &m_General.EnableMenus))
	{
		if (m_General.EnableMenus)
		{
			RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(Processor::GetSingleton());
		}
		else
		{
			RE::UI::GetSingleton()->RemoveEventSink<RE::MenuOpenCloseEvent>(Processor::GetSingleton());
		}
	}

	if (ImGui::Checkbox("Enable Time", &m_General.EnableTime))
	{
		if (m_General.EnableTime && isLoaded)
		{
			Processor::GetSingleton()->ProcessTimeBasedToggling();
		}
	}

	if (ImGui::Checkbox("Enable Interior", &m_General.EnableInterior))
	{
		if (m_General.EnableInterior && isLoaded)
		{
			Processor::GetSingleton()->ProcessInteriorBasedToggling();
		}
	}

	if (ImGui::Checkbox("Enable Weather", &m_General.EnableWeather))
	{
		if (m_General.EnableWeather && isLoaded) //&& !m_General.IsInInteriorCell
		{
			Processor::GetSingleton()->ProcessWeatherBasedToggling();
		}
	}

	if (m_General.EnableTime || m_General.EnableInterior || m_General.EnableWeather)
		ImGui::SeparatorText("Update Intervals");
	if (m_General.EnableTime)
		ImGui::SliderInt("Time Update Interval", &m_Time.TimeUpdateInterval, 0, 120, "%d");
	if (m_General.EnableInterior)
		ImGui::SliderInt("Interior Update Interval", &m_Interior.InteriorUpdateInterval, 0, 120, "%d");
	if (m_General.EnableWeather)
		ImGui::SliderInt("Weather Update Interval", &m_Weather.WeatherUpdateInterval, 0, 120, "%d");
}

void Menu::RenderMenusPage()
{
	ImGui::SeparatorText("Toggle Option");
	CreateCombo("Menu Toggle Option", m_Menu.MenuToggleOption, m_ToggleState, ImGuiComboFlags_None);

	if (m_Menu.MenuToggleOption.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("MenuAllState", m_Menu.MenuToggleAllState, m_EffectState, ImGuiComboFlags_None);

		// Display all the Menus
		ImGui::SeparatorText("Menus");
		if (!m_Menu.MenuList.empty())
		{
			for (int i = 0; i < m_Menu.MenuList.size(); i++)
			{
				const std::string& menuIndexComboID = "Menu" + std::to_string(i);
				const std::string& menuNameComboID = "##Menu" + std::to_string(i);
				const std::string& removeID = "Remove Menu##" + std::to_string(i);

				ImGui::Text("%s:", menuIndexComboID.c_str());
				ImGui::SameLine();

				CreateCombo(menuNameComboID.c_str(), m_Menu.MenuList[i], m_MenuNames, ImGuiComboFlags_None);

				// Add a button to remove the effect
				if (ImGui::Button(removeID.c_str()))
				{
					if (!m_Menu.MenuList.empty())
					{
						m_Menu.MenuList.erase(m_Menu.MenuList.begin() + i);
						i--;  // Decrement i to stay at the current index after removing the element
					}
				}

			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Menu"))
		{
			m_Menu.MenuList.emplace_back("default");
		}
	}
	else if (m_Menu.MenuToggleOption.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!m_Menu.TechniqueMenuInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < m_Menu.TechniqueMenuInfoList.size(); i++)
			{
				auto& menuInfo = m_Menu.TechniqueMenuInfoList[i];

				if (!menuInfo.Filename.empty() && !menuInfo.State.empty())
				{

					// Create IDs for every element in the vector
					const std::string& effectComboID = "Effect##Menu" + std::to_string(i);
					const std::string& effectStateID = "State##Menu" + std::to_string(i);
					const std::string& removeID = "Remove Effect##Menu" + std::to_string(i);
					const std::string& menuID = "Menu##Menu" + std::to_string(i);

					CreateCombo(effectComboID.c_str(), menuInfo.Filename, m_Effects, ImGuiComboFlags_None);
					ImGui::SameLine();
					CreateCombo(effectStateID.c_str(), menuInfo.State, m_EffectState, ImGuiComboFlags_None);
					CreateCombo(menuID.c_str(), menuInfo.Name, m_MenuNames, ImGuiComboFlags_None);

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!m_Menu.TechniqueMenuInfoList.empty())
						{
							m_Menu.TechniqueMenuInfoList.erase(m_Menu.TechniqueMenuInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Effect"))
		{
			TechniqueInfo info;
			info.Filename = "Default.fx";
			info.State = "off";
			m_Menu.TechniqueMenuInfoList.emplace_back(info);
		}

	}
}

void Menu::RenderTimePage()
{
	ImGui::SeparatorText("Toggle Option");
	CreateCombo("Time Toggle Option", m_Time.TimeToggleOption, m_ToggleState, ImGuiComboFlags_None);

	if (m_Time.TimeToggleOption.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("TimeAllState", m_Time.TimeToggleAllState, m_EffectState, ImGuiComboFlags_None);
		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderScalar("Start Time", ImGuiDataType_Double, &m_Time.TimeToggleAllTimeStart, &minTime, &maxTime, "%.2f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderScalar("Stop Time", ImGuiDataType_Double, &m_Time.TimeToggleAllTimeStop, &minTime, &maxTime, "%.2f");
	}
	else if (m_Time.TimeToggleOption.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!m_Time.TechniqueTimeInfoList.empty())
		{
			for (int i = 0; i < m_Time.TechniqueTimeInfoList.size(); i++)
			{
				auto& timeInfo = m_Time.TechniqueTimeInfoList[i];

				if (!timeInfo.Filename.empty() && !timeInfo.State.empty())
				{
					// IDs
					const std::string& effectComboID = "Effect##Time" + std::to_string(i);
					const std::string& effectStateID = "State##Time" + std::to_string(i);
					const std::string& startTimeID = "StartTime##Time" + std::to_string(i);
					const std::string& stopTimeID = "StopTime##Time" + std::to_string(i);
					const std::string& removeID = "Remove##Time" + std::to_string(i);

					CreateCombo(effectComboID.c_str(), timeInfo.Filename, m_Effects, ImGuiComboFlags_None);
					ImGui::SameLine();
					CreateCombo(effectStateID.c_str(), timeInfo.State, m_EffectState, ImGuiComboFlags_None);
					ImGui::SetNextItemWidth(200.0f);
					ImGui::SliderScalar(startTimeID.c_str(), ImGuiDataType_Double, &timeInfo.StartTime, &minTime, &maxTime, "%.2f");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					ImGui::SliderScalar(stopTimeID.c_str(), ImGuiDataType_Double, &timeInfo.StopTime, &minTime, &maxTime, "%.2f");

					if (ImGui::Button(removeID.c_str()))
					{
						if (!m_Time.TechniqueTimeInfoList.empty())
						{
							m_Time.TechniqueTimeInfoList.erase(m_Time.TechniqueTimeInfoList.begin() + i);
							i--;
						}
					}
					ImGui::Separator();
				}
			}
		}

		// Add new 
		if (ImGui::Button("Add new Effect"))
		{
			// Fill new info with default values
			TechniqueInfo info;
			info.Filename = "Default.fx";
			info.State = "off";
			info.StartTime = 0.0;
			info.StopTime = 0.0;

			m_Time.TechniqueTimeInfoList.emplace_back(info);
		}

	}
}

void Menu::RenderInteriorPage()
{
	ImGui::SeparatorText("Toggle Option");
	CreateCombo("Interior Toggle Option", m_Interior.InteriorToggleOption, m_ToggleState, ImGuiComboFlags_None);

	if (m_Interior.InteriorToggleOption.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("InteriorAllState##", m_Interior.InteriorToggleAllState, m_EffectState, ImGuiComboFlags_None);
	}
	else if (m_Interior.InteriorToggleOption.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!m_Interior.TechniqueInteriorInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < m_Interior.TechniqueInteriorInfoList.size(); i++)
			{
				auto& interiorInfo = m_Interior.TechniqueInteriorInfoList[i];

				if (!interiorInfo.Filename.empty() && !interiorInfo.State.empty())
				{
					// Create IDs for every element in the vector
					const std::string& effectComboID = "Effect##Inter" + std::to_string(i);
					const std::string& effectStateID = "State##Inter" + std::to_string(i);
					const std::string& removeID = "Remove Effect##Inter" + std::to_string(i);

					CreateCombo(effectComboID.c_str(), interiorInfo.Filename, m_Effects, ImGuiComboFlags_None);
					ImGui::SameLine();
					CreateCombo(effectStateID.c_str(), interiorInfo.State, m_EffectState, ImGuiComboFlags_None);

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!m_Interior.TechniqueInteriorInfoList.empty())
						{
							m_Interior.TechniqueInteriorInfoList.erase(m_Interior.TechniqueInteriorInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Effect"))
		{
			TechniqueInfo info;
			info.Filename = "Default.fx";
			info.State = "off";

			m_Interior.TechniqueInteriorInfoList.emplace_back(info);
		}
	}
}

void Menu::RenderWeatherPage()
{
	ImGui::SeparatorText("Toggle Option");
	CreateCombo("Weather Toggle Option", m_Weather.WeatherToggleOption, m_ToggleState, ImGuiComboFlags_None);

	if (m_Weather.WeatherToggleOption.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("WeatherAllState", m_Weather.WeatherToggleAllState, m_EffectState, ImGuiComboFlags_None);

		// Display all the Weathers
		ImGui::SeparatorText("Weathers");
		if (!m_Weather.WeatherList.empty())
		{
			for (int i = 0; i < m_Weather.WeatherList.size(); i++)
			{
				const std::string& weatherIndexComboID = "Weather" + std::to_string(i + 1);
				const std::string& weatherNameComboID = "##Weather" + std::to_string(i);
				const std::string& removeID = "Remove Weather##" + std::to_string(i);

				ImGui::Text("%s:", weatherIndexComboID.c_str());
				ImGui::SameLine();
				CreateCombo(weatherNameComboID.c_str(), m_Weather.WeatherList[i], m_WeatherFlags, ImGuiComboFlags_None);

				// Add a button to remove the effect
				if (ImGui::Button(removeID.c_str()))
				{
					if (!m_Weather.WeatherList.empty())
					{
						m_Weather.WeatherList.erase(m_Weather.WeatherList.begin() + i);
						i--;
					}
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Weather"))
		{
			m_Weather.WeatherList.emplace_back("default");
		}
	}
	else if (m_Weather.WeatherToggleOption.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!m_Weather.TechniqueWeatherInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < m_Weather.TechniqueWeatherInfoList.size(); i++)
			{
				auto& weatherInfo = m_Weather.TechniqueWeatherInfoList[i];

				if (!weatherInfo.Filename.empty() && !weatherInfo.State.empty())
				{
					// Create IDs for every element in the vector
					const std::string& effectComboID = "Effect##Weather" + std::to_string(i);
					const std::string& effectStateID = "State##Weather" + std::to_string(i);
					const std::string& removeID = "Remove Effect##Weather" + std::to_string(i);
					const std::string& weatherID = "Weather##Weather" + std::to_string(i);

					CreateCombo(effectComboID.c_str(), weatherInfo.Filename, m_Effects, ImGuiComboFlags_None);
					ImGui::SameLine();
					CreateCombo(effectStateID.c_str(), weatherInfo.State, m_EffectState, ImGuiComboFlags_None);
					CreateCombo(weatherID.c_str(), weatherInfo.Name, m_WeatherFlags, ImGuiComboFlags_None);
					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!m_Weather.TechniqueWeatherInfoList.empty())
						{
							m_Weather.TechniqueWeatherInfoList.erase(m_Weather.TechniqueWeatherInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Effect"))
		{
			TechniqueInfo info;
			info.Filename = "Default.fx";
			info.State = "off";

			m_Weather.TechniqueWeatherInfoList.emplace_back(info);
		}
	}
}