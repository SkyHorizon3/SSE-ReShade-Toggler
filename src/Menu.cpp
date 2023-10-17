#include "../include/Globals.h"
#include "../include/Menu.h"
#include "../include/ReshadeToggler.h"
#include "../include/ReshadeIntegration.h"
#include "../include/Processor.h"

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

void Menu::SettingsMenu()
{
	if (ImGui::Button("Save"))
	{
		saveConfigPopupOpen = true; // Open the Save Config popup
		inputBuffer[0] = '\0';     // Clear the input buffer
	}

	SaveConfig();

	CreateCombo("Select Preset", selectedPreset, g_Presets, ImGuiComboFlags_None);
	ImGui::SameLine();
	if (ImGui::Button("Load Preset"))
	{
		selectedPresetPath = "Data\\SKSE\\Plugins\\TogglerConfigs\\" + selectedPreset;
		// Load the selected preset (you can implement this logic)
		if (std::filesystem::exists(selectedPresetPath))
		{
			ReshadeToggler::GetSingleton()->LoadPreset(selectedPreset);
			CSimpleIniA ini;
			ini.SetUnicode(false);
			ini.SetValue("Presets", "PresetPath", selectedPresetPath.c_str());
			ini.SetValue("Presets", "PresetName", selectedPreset.c_str());
			ini.SaveFile("Data\\SKSE\\Plugins\\ReShadeToggler.ini");
		}
		else
		{
			g_Logger->info("Tried to load presetpath {}. Preset doesn't exist!", selectedPresetPath);
		}
	}

	if (ImGui::Button("Refresh"))
	{
		g_Presets.clear();
		ReshadeIntegration::EnumeratePresets();
	}

	if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		RenderInfoPage();
	}

	if (EnableMenus)
	{
		if (ImGui::CollapsingHeader("Menus", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderMenusPage();
		}

	}

	if (EnableTime)
	{
		if (ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderTimePage();
		}
	}

	if (EnableInterior)
	{
		if (ImGui::CollapsingHeader("Interior", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderInteriorPage();
		}
	}

	if (EnableWeather)
	{
		if (ImGui::CollapsingHeader("Weather", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			RenderWeatherPage();
		}
	}
}

// I LOVE THIS. ALL HAIL SimpleINI!!!!!!
void Menu::Save(const std::string& filename)
{
	// Define the path to your mod's TogglerConfigs directory
	std::string configDirectory = "Data\\SKSE\\Plugins\\TogglerConfigs"; // Update this path as needed

	// Ensure that the directory exists, create it if necessary
	std::filesystem::create_directories(configDirectory);

	// Combine the directory path and the provided filename
	std::string fullPath = configDirectory + "\\" + filename + ".ini";

	CSimpleIniA ini;
	ini.SetUnicode(false);

	// Save General section
	ini.SetBoolValue("General", "EnableMenus", EnableMenus);
	ini.SetBoolValue("General", "EnableTime", EnableTime);
	ini.SetBoolValue("General", "EnableInterior", EnableInterior);
	ini.SetBoolValue("General", "EnableWeather", EnableWeather);

	// MenusGeneral Section
	ini.SetValue("MenusGeneral", "MenuToggleOption", ToggleStateMenus.c_str());
	ini.SetValue("MenusGeneral", "MenuToggleAllState", ToggleAllStateMenus.c_str());

	for (size_t i = 0; i < techniqueMenuInfoList.size(); i++)
	{
		const auto& menuInfo = techniqueMenuInfoList[i];
		std::string effectFileKey = "MenuToggleSpecificFile" + std::to_string(i + 1);
		std::string effectStateKey = "MenuToggleSpecificState" + std::to_string(i + 1);
		std::string effectMenuKey = "MenuToggleSpecificMenu" + std::to_string(i + 1);

		ini.SetValue("MenusGeneral", effectFileKey.c_str(), menuInfo.filename.c_str());
		ini.SetValue("MenusGeneral", effectStateKey.c_str(), menuInfo.state.c_str());
		ini.SetValue("MenusGeneral", effectMenuKey.c_str(), menuInfo.Name.c_str());
	}

	// Save MenusProcess section
	for (size_t i = 0; i < menuList.size(); i++)
	{
		const auto& menuData = menuList[i];
		std::string menuKey = "Menu" + std::to_string(i + 1);
		ini.SetValue("MenusProcess", menuKey.c_str(), menuData.Name.c_str());
	}

	// Save Time Section
	ini.SetValue("Time", "TimeUpdateInterval", std::to_string(TimeUpdateIntervalTime).c_str());

	ini.SetValue("Time", "TimeToggleOption", ToggleStateTime.c_str());
	ini.SetValue("Time", "TimeToggleAllState", ToggleAllStateTime.c_str());

	for (const auto& info : techniqueTimeInfoListAll)
	{
		ini.SetDoubleValue("Time", "TimeToggleAllTimeStart", info.startTime);
		ini.SetDoubleValue("Time", "TimeToggleAllTimeStop", info.stopTime);
	}

	for (size_t i = 0; i < techniqueTimeInfoList.size(); i++)
	{
		const auto& timeInfo = techniqueTimeInfoList[i];
		std::string effectFileKey = "TimeToggleSpecificFile" + std::to_string(i + 1);
		std::string effectStateKey = "TimeToggleSpecificState" + std::to_string(i + 1);
		std::string effectStartTimeKey = "TimeToggleSpecificTimeStart" + std::to_string(i + 1);
		std::string effectStopTimeKey = "TimeToggleSpecificTimeStop" + std::to_string(i + 1);

		ini.SetValue("Time", effectFileKey.c_str(), timeInfo.filename.c_str());
		ini.SetValue("Time", effectStateKey.c_str(), timeInfo.state.c_str());
		ini.SetDoubleValue("Time", effectStartTimeKey.c_str(), timeInfo.startTime);
		ini.SetDoubleValue("Time", effectStopTimeKey.c_str(), timeInfo.stopTime);
	}

	// Save Interior section
	ini.SetValue("Interior", "InteriorUpdateInterval", std::to_string(TimeUpdateIntervalInterior).c_str());
	ini.SetValue("Interior", "InteriorToggleOption", ToggleStateInterior.c_str());
	ini.SetValue("Interior", "InteriorToggleAllState", ToggleAllStateInterior.c_str());

	for (size_t i = 0; i < techniqueInteriorInfoList.size(); i++)
	{
		const auto& interiorInfo = techniqueInteriorInfoList[i];
		std::string effectFileKey = "InteriorToggleSpecificFile" + std::to_string(i + 1);
		std::string effectStateKey = "InteriorToggleSpecificState" + std::to_string(i + 1);

		ini.SetValue("Interior", effectFileKey.c_str(), interiorInfo.filename.c_str());
		ini.SetValue("Interior", effectStateKey.c_str(), interiorInfo.state.c_str());
	}

	// Save Weather section
	ini.SetValue("Weather", "WeatherUpdateInterval", std::to_string(TimeUpdateIntervalWeather).c_str());
	ini.SetValue("Weather", "WeatherToggleOption", ToggleStateWeather.c_str());
	ini.SetValue("Weather", "WeatherToggleAllState", ToggleAllStateWeather.c_str());

	for (size_t i = 0; i < techniqueWeatherInfoList.size(); i++)
	{
		const auto& weatherInfo = techniqueWeatherInfoList[i];
		std::string effectFileKey = "WeatherToggleSpecificFile" + std::to_string(i + 1);
		std::string effectStateKey = "WeatherToggleSpecificState" + std::to_string(i + 1);
		std::string effectWeatherKey = "WeatherToggleSpecificWeather" + std::to_string(i + 1);

		ini.SetValue("Weather", effectFileKey.c_str(), weatherInfo.filename.c_str());
		ini.SetValue("Weather", effectStateKey.c_str(), weatherInfo.state.c_str());
		ini.SetValue("Weather", effectWeatherKey.c_str(), weatherInfo.Name.c_str());
	}

	// Save WeatherProcess section
	for (size_t i = 0; i < weatherList.size(); i++)
	{
		const auto& weatherData = weatherList[i];
		std::string weatherKey = "Weather" + std::to_string(i + 1);
		ini.SetValue("WeatherProcess", weatherKey.c_str(), weatherData.Name.c_str());
	}

	ini.SaveFile(fullPath.c_str());
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
				Save(filename);

				//Refresh
				g_Presets.clear();
				ReshadeIntegration::EnumeratePresets();

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
	if (ImGui::Checkbox("Enable Menu", &EnableMenus))
	{
		auto& eventProcessorMenu = Processor::GetSingleton();
		if (EnableMenus)
		{
			RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessorMenu);
		}
		else
		{
			RE::UI::GetSingleton()->RemoveEventSink<RE::MenuOpenCloseEvent>(&eventProcessorMenu);
		}
	}

	if (ImGui::Checkbox("Enable Time", &EnableTime))
	{
		if (EnableTime && isLoaded)
		{
			Processor::GetSingleton().ProcessTimeBasedToggling();
		}
	}

	if (ImGui::Checkbox("Enable Interior", &EnableInterior))
	{
		if (EnableInterior && isLoaded)
		{
			Processor::GetSingleton().ProcessInteriorBasedToggling();
		}
	}

	if (ImGui::Checkbox("Enable Weather", &EnableWeather))
	{
		if (EnableWeather && isLoaded && !IsInInteriorCell)
		{
			Processor::GetSingleton().ProcessWeatherBasedToggling();
		}
	}

	if (EnableTime || EnableInterior || EnableWeather)
		ImGui::SeparatorText("Update Intervals");
	if (EnableTime)
		ImGui::SliderInt("Time Update Interval", &TimeUpdateIntervalTime, 0, 120, "%d");
	if (EnableInterior)
		ImGui::SliderInt("Interior Update Interval", &TimeUpdateIntervalInterior, 0, 120, "%d");
	if (EnableWeather)
		ImGui::SliderInt("Weather Update Interval", &TimeUpdateIntervalWeather, 0, 120, "%d");
}

void Menu::RenderMenusPage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Menu Toggle State", ToggleStateMenus, g_ToggleState, ImGuiComboFlags_None);

	if (ToggleStateMenus.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("MenuAllState", ToggleAllStateMenus, g_EffectStateMenu, ImGuiComboFlags_None);

		// Display all the Menus
		ImGui::SeparatorText("Menus");
		if (!menuList.empty())
		{
			for (int i = 0; i < menuList.size(); i++)
			{
				auto& iniMenus = menuList[i];

				bool valueChanged = false;
				std::string menuIndexComboID = "Menu" + std::to_string(i + 1);
				std::string menuNameComboID = "##Menu" + std::to_string(i);
				std::string removeID = "Remove Menu##" + std::to_string(i);

				std::string currentMenuName = iniMenus.Name;

				ImGui::Text("%s:", menuIndexComboID.c_str());
				ImGui::SameLine();
				if (CreateCombo(menuNameComboID.c_str(), currentMenuName, g_MenuNames, ImGuiComboFlags_None)) { valueChanged = true; }

				// Add a button to remove the effect
				if (ImGui::Button(removeID.c_str()))
				{
					if (!menuList.empty())
					{
						menuList.erase(menuList.begin() + i);
						i--;  // Decrement i to stay at the current index after removing the element
					}
				}

				if (valueChanged)
				{
					iniMenus.Index = "Menu" + std::to_string(i);
					iniMenus.Name = currentMenuName;
					DEBUG_LOG(g_Logger, "New Menu Name:{} - {}", iniMenus.Index, iniMenus.Name);
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

			menuList.push_back(menu);
		}
	}

	if (ToggleStateMenus.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!techniqueMenuInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < techniqueMenuInfoList.size(); i++)
			{
				auto& menuInfo = techniqueMenuInfoList[i];

				if (menuInfo.filename != "" && menuInfo.state != "")
				{

					bool valueChanged = false;
					// Create IDs for every element in the vector
					std::string effectComboID = "Effect##Menu" + std::to_string(i);
					std::string effectStateID = "State##Menu" + std::to_string(i);
					std::string removeID = "Remove Effect##Menu" + std::to_string(i);
					std::string menuID = "Menu##Menu" + std::to_string(i);

					std::string currentEffectFileName = menuInfo.filename;
					std::string currentEffectState = menuInfo.state;
					std::string currentEffectMenu = menuInfo.Name;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, g_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, g_EffectStateMenu, ImGuiComboFlags_None)) { valueChanged = true; }
					if (CreateCombo(menuID.c_str(), currentEffectMenu, g_MenuNames, ImGuiComboFlags_None)) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!techniqueMenuInfoList.empty())
						{
							techniqueMenuInfoList.erase(techniqueMenuInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}

					if (valueChanged)
					{
						menuInfo.filename = currentEffectFileName;
						menuInfo.state = currentEffectState;
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
			info.filename = "Default.fx";
			info.state = "off";

			techniqueMenuInfoList.push_back(info);
		}

	}
}

void Menu::RenderTimePage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Time Toggle State", ToggleStateTime, g_ToggleState, ImGuiComboFlags_None);

	if (ToggleStateTime.find("All") != std::string::npos)
	{
		bool valueChanged = false;
		for (int i = 0; i < techniqueTimeInfoListAll.size(); i++)
		{
			auto& info = techniqueTimeInfoListAll[i];

			std::string currentEffectState = info.state;
			double currentStartTime = info.startTime;
			double currentStopTime = info.stopTime;

			ImGui::SameLine();
			if (CreateCombo("TimeAllState", ToggleAllStateTime, g_EffectStateTime, ImGuiComboFlags_None)) valueChanged = true;
			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::SliderScalar("Start Time", ImGuiDataType_Double, &currentStartTime, &minTime, &maxTime, "%.2f")) valueChanged = true;
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200.0f);
			if (ImGui::SliderScalar("Stop Time", ImGuiDataType_Double, &currentStopTime, &minTime, &maxTime, "%.2f")) valueChanged = true;

			if (valueChanged)
			{
				info.state = currentEffectState;
				info.startTime = currentStartTime;
				info.stopTime = currentStopTime;
			}
		}
	}

	// Wtf happened to this... holy crap this nesting!
	if (ToggleStateTime.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!techniqueTimeInfoList.empty())
		{
			for (int i = 0; i < techniqueTimeInfoList.size(); i++)
			{
				auto& timeInfo = techniqueTimeInfoList[i];

				if (timeInfo.filename != "" && timeInfo.state != "")
				{

					// Create IDs for every element in the vector that is to be rendered
					std::string effectComboID = "Effect##Time" + std::to_string(i);
					std::string effectStateID = "State##Time" + std::to_string(i);
					std::string startTimeID = "StartTime##Time" + std::to_string(i);
					std::string stopTimeID = "StopTime##Time" + std::to_string(i);
					std::string removeID = "Remove##Time" + std::to_string(i);

					std::string currentEffectFileName = timeInfo.filename;
					std::string currentEffectState = timeInfo.state;
					double currentStartTime = timeInfo.startTime;
					double currentStopTime = timeInfo.stopTime;

					bool valueChanged = false;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, g_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, g_EffectStateTime, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::SliderScalar(startTimeID.c_str(), ImGuiDataType_Double, &currentStartTime, &minTime, &maxTime, "%.2f")) { valueChanged = true; }
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::SliderScalar(stopTimeID.c_str(), ImGuiDataType_Double, &currentStopTime, &minTime, &maxTime, "%.2f")) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
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

		// Add new 
		if (ImGui::Button("Add new Effect"))
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

void Menu::RenderInteriorPage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Interior Toggle State", ToggleStateInterior, g_ToggleState, ImGuiComboFlags_None);

	if (ToggleStateInterior.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("InteriorAllState##", ToggleAllStateInterior, g_EffectStateInterior, ImGuiComboFlags_None);
	}

	bool valueChanged = false;
	if (ToggleStateInterior.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!techniqueInteriorInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < techniqueInteriorInfoList.size(); i++)
			{
				auto& interiorInfo = techniqueInteriorInfoList[i];

				if (interiorInfo.filename != "" && interiorInfo.state != "")
				{
					// Create IDs for every element in the vector
					std::string effectComboID = "Effect##Inter" + std::to_string(i);
					std::string effectStateID = "State##Inter" + std::to_string(i);
					std::string removeID = "Remove Effect##Inter" + std::to_string(i);

					std::string currentEffectFileName = interiorInfo.filename;
					std::string currentEffectState = interiorInfo.state;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, g_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, g_EffectStateInterior, ImGuiComboFlags_None)) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!techniqueInteriorInfoList.empty())
						{
							techniqueInteriorInfoList.erase(techniqueInteriorInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}

					if (valueChanged)
					{
						interiorInfo.filename = currentEffectFileName;
						interiorInfo.state = currentEffectState;
					}
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Effect"))
		{
			TechniqueInfo info;
			info.filename = "Default.fx";
			info.state = "off";

			techniqueInteriorInfoList.push_back(info);
		}
	}
}

void Menu::RenderWeatherPage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Weather Toggle State", ToggleStateWeather, g_ToggleState, ImGuiComboFlags_None);

	if (ToggleStateWeather.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("WeatherAllState", ToggleAllStateWeather, g_EffectStateWeather, ImGuiComboFlags_None);

		// Display all the Weathers
		ImGui::SeparatorText("Weathers");
		if (!weatherList.empty())
		{
			for (int i = 0; i < weatherList.size(); i++)
			{
				auto& iniWeather = weatherList[i];

				bool valueChanged = false;
				std::string weatherIndexComboID = "Weather" + std::to_string(i + 1);
				std::string weatherNameComboID = "##Weather" + std::to_string(i);
				std::string removeID = "Remove Weather##" + std::to_string(i);

				std::string currentWeatherName = iniWeather.Name;

				ImGui::Text("%s:", weatherIndexComboID.c_str());
				ImGui::SameLine();
				if (CreateCombo(weatherNameComboID.c_str(), currentWeatherName, g_WeatherFlags, ImGuiComboFlags_None)) { valueChanged = true; }

				// Add a button to remove the effect
				if (ImGui::Button(removeID.c_str()))
				{
					if (!weatherList.empty())
					{
						weatherList.erase(weatherList.begin() + i);
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

			weatherList.push_back(weather);
		}
	}

	if (ToggleStateWeather.find("Specific") != std::string::npos)
	{
		ImGui::SeparatorText("Effects");
		if (!techniqueWeatherInfoList.empty())
		{
			// For Specific
			for (int i = 0; i < techniqueWeatherInfoList.size(); i++)
			{
				bool valueChanged = false;
				auto& weatherInfo = techniqueWeatherInfoList[i];

				if (weatherInfo.filename != "" && weatherInfo.state != "")
				{
					// Create IDs for every element in the vector
					std::string effectComboID = "Effect##Weather" + std::to_string(i);
					std::string effectStateID = "State##Weather" + std::to_string(i);
					std::string removeID = "Remove Effect##Weather" + std::to_string(i);
					std::string weatherID = "Weather##Weather" + std::to_string(i);

					std::string currentEffectFileName = weatherInfo.filename;
					std::string currentEffectState = weatherInfo.state;
					std::string currentWeatherFlag = weatherInfo.Name;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, g_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, g_EffectStateWeather, ImGuiComboFlags_None)) { valueChanged = true; }
					if (CreateCombo(weatherID.c_str(), currentWeatherFlag, g_WeatherFlags, ImGuiComboFlags_None)) { valueChanged = true; }
					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str()))
					{
						if (!techniqueWeatherInfoList.empty())
						{
							techniqueWeatherInfoList.erase(techniqueWeatherInfoList.begin() + i);
							i--;  // Decrement i to stay at the current index after removing the element
						}
					}

					if (valueChanged)
					{
						weatherInfo.filename = currentEffectFileName;
						weatherInfo.state = currentEffectState;
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
			info.filename = "Default.fx";
			info.state = "off";

			techniqueWeatherInfoList.push_back(info);
		}
	}
}