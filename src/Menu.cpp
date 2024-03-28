#include "Globals.h"
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
		if (preset.is_regular_file() && preset.path().filename().extension() == ".ini")
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
		const auto& menuName = menu.first;
		std::string menuNameStr(menuName);

		m_MenuNames.emplace_back(menuNameStr);
	}

	std::sort(m_MenuNames.begin(), m_MenuNames.end());
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

	if (m_Conf.EnableMenus)
	{
		if (ImGui::CollapsingHeader("Menus", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderMenusPage();
		}

	}

	if (m_Conf.EnableTime)
	{
		if (ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderTimePage();
		}
	}

	if (m_Conf.EnableInterior)
	{
		if (ImGui::CollapsingHeader("Interior", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderInteriorPage();
		}
	}

	if (m_Conf.EnableWeather)
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

				auto c = Config::GetSingleton();
				// Call the Save function with the chosen filename
				c->SerializePreset(filename);

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
	if (ImGui::Checkbox("Enable Menu", &m_Conf.EnableMenus))
	{
		if (m_Conf.EnableMenus)
		{
			RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(Processor::GetSingleton());
		}
		else
		{
			RE::UI::GetSingleton()->RemoveEventSink<RE::MenuOpenCloseEvent>(Processor::GetSingleton());
		}
	}

	if (ImGui::Checkbox("Enable Time", &m_Conf.EnableTime))
	{
		if (m_Conf.EnableTime && isLoaded)
		{
			Processor::GetSingleton()->ProcessTimeBasedToggling();
		}
	}

	if (ImGui::Checkbox("Enable Interior", &m_Conf.EnableInterior))
	{
		if (m_Conf.EnableInterior && isLoaded)
		{
			Processor::GetSingleton()->ProcessInteriorBasedToggling();
		}
	}

	if (ImGui::Checkbox("Enable Weather", &m_Conf.EnableWeather))
	{
		if (m_Conf.EnableWeather && isLoaded && !m_Conf.IsInInteriorCell)
		{
			Processor::GetSingleton()->ProcessWeatherBasedToggling();
		}
	}

	if (m_Conf.EnableTime || m_Conf.EnableInterior || m_Conf.EnableWeather)
		ImGui::SeparatorText("Update Intervals");
	if (m_Conf.EnableTime)
		ImGui::SliderInt("Time Update Interval", &m_Conf.TimeUpdateIntervalTime, 0, 120, "%d");
	if (m_Conf.EnableInterior)
		ImGui::SliderInt("Interior Update Interval", &m_Conf.TimeUpdateIntervalInterior, 0, 120, "%d");
	if (m_Conf.EnableWeather)
		ImGui::SliderInt("Weather Update Interval", &m_Conf.TimeUpdateIntervalWeather, 0, 120, "%d");
}

void Menu::RenderMenusPage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Menu Toggle State", m_Conf.ToggleStateMenus, m_ToggleState, ImGuiComboFlags_None);

	if (m_Conf.ToggleStateMenus.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("MenuAllState", m_Conf.ToggleAllStateMenus, m_EffectState, ImGuiComboFlags_None);

		// Display all the Menus
		ImGui::SeparatorText("Menus");
		if (!m_Conf.MenuList.empty())
		{
			for (int i = 0; i < m_Conf.MenuList.size(); i++)
			{
				auto& iniMenus = m_Conf.MenuList[i];

				bool valueChanged = false;
				std::string menuIndexComboID = "Menu" + std::to_string(i + 1);
				std::string menuNameComboID = "##Menu" + std::to_string(i);
				std::string removeID = "Remove Menu##" + std::to_string(i);

				std::string currentMenuName = iniMenus.Name;

				ImGui::Text("%s:", menuIndexComboID.c_str());
				ImGui::SameLine();
				if (CreateCombo(menuNameComboID.c_str(), currentMenuName, m_MenuNames, ImGuiComboFlags_None)) { valueChanged = true; }

				// Add a button to remove the effect
				if (ImGui::Button(removeID.c_str()))
				{
					if (!m_Conf.MenuList.empty())
					{
						m_Conf.MenuList.erase(m_Conf.MenuList.begin() + i);
						i--;  // Decrement i to stay at the current index after removing the element
					}
				}

				if (valueChanged)
				{
					iniMenus.Index = "Menu" + std::to_string(i);
					iniMenus.Name = currentMenuName;
#ifndef NDEBUG
					SKSE::log::info("New Menu Name:{} - {}", iniMenus.Index, iniMenus.Name);
#endif
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Menu"))
		{
			Info menu;
			menu.Index = "";
			menu.Name = "default";

			m_Conf.MenuList.push_back(menu);
		}
	}

	if (m_Conf.ToggleStateMenus.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!m_Conf.TechniqueMenuInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < m_Conf.TechniqueMenuInfoList.size(); i++)
			{
				auto& menuInfo = m_Conf.TechniqueMenuInfoList[i];

				if (menuInfo.Filename != "" && menuInfo.State != "")
				{

					bool valueChanged = false;
					// Create IDs for every element in the vector
					std::string effectComboID = "Effect##Menu" + std::to_string(i);
					std::string effectStateID = "State##Menu" + std::to_string(i);
					std::string removeID = "Remove Effect##Menu" + std::to_string(i);
					std::string menuID = "Menu##Menu" + std::to_string(i);

					std::string currentEffectFileName = menuInfo.Filename;
					std::string currentEffectState = menuInfo.State;
					std::string currentEffectMenu = menuInfo.Name;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, m_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, m_EffectState, ImGuiComboFlags_None)) { valueChanged = true; }
					if (CreateCombo(menuID.c_str(), currentEffectMenu, m_MenuNames, ImGuiComboFlags_None)) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!m_Conf.TechniqueMenuInfoList.empty())
						{
							m_Conf.TechniqueMenuInfoList.erase(m_Conf.TechniqueMenuInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}

					if (valueChanged)
					{
						menuInfo.Filename = currentEffectFileName;
						menuInfo.State = currentEffectState;
						menuInfo.Name = currentEffectMenu;
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

			m_Conf.TechniqueMenuInfoList.push_back(info);
		}

	}
}

void Menu::RenderTimePage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Time Toggle State", m_Conf.ToggleStateTime, m_ToggleState, ImGuiComboFlags_None);

	if (m_Conf.ToggleStateTime.find("All") != std::string::npos)
	{
		bool valueChanged = false;
		for (int i = 0; i < m_Conf.TechniqueTimeInfoListAll.size(); i++)
		{
			auto& info = m_Conf.TechniqueTimeInfoListAll[i];

			std::string currentEffectState = info.State;
			double currentStartTime = info.StartTime;
			double currentStopTime = info.StopTime;

			ImGui::SameLine();
			if (CreateCombo("TimeAllState", m_Conf.ToggleAllStateTime, m_EffectState, ImGuiComboFlags_None)) valueChanged = true;
			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::SliderScalar("Start Time", ImGuiDataType_Double, &currentStartTime, &minTime, &maxTime, "%.2f")) valueChanged = true;
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::SliderScalar("Stop Time", ImGuiDataType_Double, &currentStopTime, &minTime, &maxTime, "%.2f")) valueChanged = true;

			if (valueChanged)
			{
				info.State = currentEffectState;
				info.StartTime = currentStartTime;
				info.StopTime = currentStopTime;
			}
		}
	}

	// Wtf happened to this... holy crap this nesting!
	if (m_Conf.ToggleStateTime.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!m_Conf.TechniqueTimeInfoList.empty())
		{
			for (int i = 0; i < m_Conf.TechniqueTimeInfoList.size(); i++)
			{
				auto& timeInfo = m_Conf.TechniqueTimeInfoList[i];

				if (timeInfo.Filename != "" && timeInfo.State != "")
				{

					// Create IDs for every element in the vector that is to be rendered
					std::string effectComboID = "Effect##Time" + std::to_string(i);
					std::string effectStateID = "State##Time" + std::to_string(i);
					std::string startTimeID = "StartTime##Time" + std::to_string(i);
					std::string stopTimeID = "StopTime##Time" + std::to_string(i);
					std::string removeID = "Remove##Time" + std::to_string(i);

					std::string currentEffectFileName = timeInfo.Filename;
					std::string currentEffectState = timeInfo.State;
					double currentStartTime = timeInfo.StartTime;
					double currentStopTime = timeInfo.StopTime;

					bool valueChanged = false;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, m_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, m_EffectState, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::SliderScalar(startTimeID.c_str(), ImGuiDataType_Double, &currentStartTime, &minTime, &maxTime, "%.2f")) { valueChanged = true; }
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::SliderScalar(stopTimeID.c_str(), ImGuiDataType_Double, &currentStopTime, &minTime, &maxTime, "%.2f")) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!m_Conf.TechniqueTimeInfoList.empty())
						{
							m_Conf.TechniqueTimeInfoList.erase(m_Conf.TechniqueTimeInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}

					ImGui::Separator();

					if (valueChanged)
					{
						// Update new values
						timeInfo.Filename = currentEffectFileName;
						timeInfo.State = currentEffectState;
						timeInfo.StartTime = currentStartTime;
						timeInfo.StopTime = currentStopTime;

						//ImGui::Text("New Values for %i: Effect: %s - State: %s - Start: %.2f - Stop: %.2f", i, timeInfo.filename.c_str(), timeInfo.state.c_str(), timeInfo.startTime, timeInfo.stopTime);
					}
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

			m_Conf.TechniqueTimeInfoList.push_back(info);
		}

	}
}

void Menu::RenderInteriorPage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Interior Toggle State", m_Conf.ToggleStateInterior, m_ToggleState, ImGuiComboFlags_None);

	if (m_Conf.ToggleStateInterior.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("InteriorAllState##", m_Conf.ToggleAllStateInterior, m_EffectState, ImGuiComboFlags_None);
	}

	bool valueChanged = false;
	if (m_Conf.ToggleStateInterior.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!m_Conf.TechniqueInteriorInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < m_Conf.TechniqueInteriorInfoList.size(); i++)
			{
				auto& interiorInfo = m_Conf.TechniqueInteriorInfoList[i];

				if (interiorInfo.Filename != "" && interiorInfo.State != "")
				{
					// Create IDs for every element in the vector
					std::string effectComboID = "Effect##Inter" + std::to_string(i);
					std::string effectStateID = "State##Inter" + std::to_string(i);
					std::string removeID = "Remove Effect##Inter" + std::to_string(i);

					std::string currentEffectFileName = interiorInfo.Filename;
					std::string currentEffectState = interiorInfo.State;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, m_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, m_EffectState, ImGuiComboFlags_None)) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!m_Conf.TechniqueInteriorInfoList.empty())
						{
							m_Conf.TechniqueInteriorInfoList.erase(m_Conf.TechniqueInteriorInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}

					if (valueChanged)
					{
						interiorInfo.Filename = currentEffectFileName;
						interiorInfo.State = currentEffectState;
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

			m_Conf.TechniqueInteriorInfoList.push_back(info);
		}
	}
}

void Menu::RenderWeatherPage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Weather Toggle State", m_Conf.ToggleStateWeather, m_ToggleState, ImGuiComboFlags_None);

	if (m_Conf.ToggleStateWeather.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("WeatherAllState", m_Conf.ToggleAllStateWeather, m_EffectState, ImGuiComboFlags_None);

		// Display all the Weathers
		ImGui::SeparatorText("Weathers");
		if (!m_Conf.WeatherList.empty())
		{
			for (int i = 0; i < m_Conf.WeatherList.size(); i++)
			{
				auto& iniWeather = m_Conf.WeatherList[i];

				bool valueChanged = false;
				std::string weatherIndexComboID = "Weather" + std::to_string(i + 1);
				std::string weatherNameComboID = "##Weather" + std::to_string(i);
				std::string removeID = "Remove Weather##" + std::to_string(i);

				std::string currentWeatherName = iniWeather.Name;

				ImGui::Text("%s:", weatherIndexComboID.c_str());
				ImGui::SameLine();
				if (CreateCombo(weatherNameComboID.c_str(), currentWeatherName, m_WeatherFlags, ImGuiComboFlags_None)) { valueChanged = true; }

				// Add a button to remove the effect
				if (ImGui::Button(removeID.c_str()))
				{
					if (!m_Conf.WeatherList.empty())
					{
						m_Conf.WeatherList.erase(m_Conf.WeatherList.begin() + i);
						i--;  // Decrement i to stay at the current index after removing the element
					}
				}

				if (valueChanged)
				{
					iniWeather.Index = "Weather" + std::to_string(i);
					iniWeather.Name = currentWeatherName;
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Weather"))
		{
			Info weather;
			weather.Index = "";
			weather.Name = "default";

			m_Conf.WeatherList.push_back(weather);
		}
	}

	if (m_Conf.ToggleStateWeather.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!m_Conf.TechniqueWeatherInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < m_Conf.TechniqueWeatherInfoList.size(); i++)
			{
				bool valueChanged = false;
				auto& weatherInfo = m_Conf.TechniqueWeatherInfoList[i];

				if (weatherInfo.Filename != "" && weatherInfo.State != "")
				{
					// Create IDs for every element in the vector
					std::string effectComboID = "Effect##Weather" + std::to_string(i);
					std::string effectStateID = "State##Weather" + std::to_string(i);
					std::string removeID = "Remove Effect##Weather" + std::to_string(i);
					std::string weatherID = "Weather##Weather" + std::to_string(i);

					std::string currentEffectFileName = weatherInfo.Filename;
					std::string currentEffectState = weatherInfo.State;
					std::string currentWeatherFlag = weatherInfo.Name;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, m_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, m_EffectState, ImGuiComboFlags_None)) { valueChanged = true; }
					if (CreateCombo(weatherID.c_str(), currentWeatherFlag, m_WeatherFlags, ImGuiComboFlags_None)) { valueChanged = true; }
					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!m_Conf.TechniqueWeatherInfoList.empty())
						{
							m_Conf.TechniqueWeatherInfoList.erase(m_Conf.TechniqueWeatherInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}

					if (valueChanged)
					{
						weatherInfo.Filename = currentEffectFileName;
						weatherInfo.State = currentEffectState;
						weatherInfo.Name = currentWeatherFlag;
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

			m_Conf.TechniqueWeatherInfoList.push_back(info);
		}
	}
}