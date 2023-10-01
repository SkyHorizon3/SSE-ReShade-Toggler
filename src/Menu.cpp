#include "../include/Globals.h"
#include "../include/Menu.h"
#include "../include/ReshadeToggler.h"
#include "../include/ReshadeIntegration.h"
#include "../include/Processor.h"

// Forward declare the threads
void TimeThread();
void InteriorThread();
void WeatherThread();

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
	//if (ImGui::Button("Save"))
	//{
	//}

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
			std::thread(TimeThread).detach();
		}
	}

	if (ImGui::Checkbox("Enable Interior", &EnableInterior))
	{
		if (EnableInterior && isLoaded)
		{
			Processor::GetSingleton().ProcessInteriorBasedToggling();
			std::thread(InteriorThread).detach();
		}
	}

	if (ImGui::Checkbox("Enable Weather", &EnableWeather))
	{
		if (EnableWeather && isLoaded)
		{
			Processor::GetSingleton().ProcessWeatherBasedToggling();
			std::thread(WeatherThread).detach();
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
		CreateCombo("State", ToggleAllStateMenus, g_EffectState, ImGuiComboFlags_None);
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
					std::string effectComboID = "Effect##" + std::to_string(i);
					std::string effectStateID = "State##" + std::to_string(i);
					std::string removeID = "Remove Effect##" + std::to_string(i);

					std::string currentEffectFileName = menuInfo.filename;
					std::string currentEffectState = menuInfo.state;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, g_MenuNames, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, g_EffectState, ImGuiComboFlags_None)) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str(), ImVec2(200, 20)))
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
					}
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Effect", ImVec2(200, 20)))
		{
			TechniqueInfo info;
			info.filename = "Default.fx";
			info.state = "off";

			techniqueMenuInfoList.push_back(info);
		}

	}

	// Display all the Menus
	ImGui::SeparatorText("Menus");
	if (!menuList.empty())
	{
		for (int i = 0; i < menuList.size(); i++)
		{
			auto& iniMenus = menuList[i];

			bool valueChanged = false;
			std::string menuIndexComboID = "Menu" + std::to_string(i + 1);
			std::string menuNameComboID = "##" + std::to_string(i);
			std::string removeID = "Remove Menu##" + std::to_string(i);

			std::string currentMenuName = iniMenus.Name;

			ImGui::Text("%s:", menuIndexComboID.c_str());
			ImGui::SameLine();
			if (CreateCombo(menuNameComboID.c_str(), currentMenuName, g_MenuNames, ImGuiComboFlags_None)) { valueChanged = true; }

			// Add a button to remove the effect
			if (ImGui::Button(removeID.c_str(), ImVec2(200, 20)))
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
			}
		}
	}

	ImGui::Separator();

	// Add new
	if (ImGui::Button("Add New Menu", ImVec2(150, 20)))
	{
		Info menu;
		menu.Index = "";
		menu.Name = "default";

		menuList.push_back(menu);
	}
}

void Menu::RenderTimePage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Time Toggle State", ToggleStateTime, g_ToggleState, ImGuiComboFlags_None);

	if (ToggleStateTime.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("State", ToggleAllStateTime, g_EffectState, ImGuiComboFlags_None);
	}

	// Wtf happened to this function... holy crap this nesting!
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
					if (CreateCombo(effectStateID.c_str(), currentEffectState, g_EffectState, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::SliderScalar(startTimeID.c_str(), ImGuiDataType_Double, &currentStartTime, &minTime, &maxTime, "%.2f")) { valueChanged = true; }
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::SliderScalar(stopTimeID.c_str(), ImGuiDataType_Double, &currentStopTime, &minTime, &maxTime, "%.2f")) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str(), ImVec2(50, 20)))
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

void Menu::RenderInteriorPage()
{
	ImGui::SeparatorText("Toggle State");
	CreateCombo("Interior Toggle State", ToggleStateInterior, g_ToggleState, ImGuiComboFlags_None);

	if (ToggleStateInterior.find("All") != std::string::npos)
	{
		ImGui::SameLine();
		CreateCombo("State", ToggleAllStateInterior, g_EffectState, ImGuiComboFlags_None);
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
					std::string effectComboID = "Effect##" + std::to_string(i);
					std::string effectStateID = "State##" + std::to_string(i);
					std::string removeID = "Remove Effect##" + std::to_string(i);

					std::string currentEffectFileName = interiorInfo.filename;
					std::string currentEffectState = interiorInfo.state;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, g_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, g_EffectState, ImGuiComboFlags_None)) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str(), ImVec2(200, 20)))
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
		if (ImGui::Button("Add New Effect", ImVec2(200, 20)))
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
		CreateCombo("State", ToggleAllStateWeather, g_EffectState, ImGuiComboFlags_None);
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
					std::string effectComboID = "Effect##" + std::to_string(i);
					std::string effectStateID = "State##" + std::to_string(i);
					std::string removeID = "Remove Effect##" + std::to_string(i);

					std::string currentEffectFileName = weatherInfo.filename;
					std::string currentEffectState = weatherInfo.state;
					std::string currentWeatherFlag = weatherflags;

					if (CreateCombo(effectComboID.c_str(), currentEffectFileName, g_Effects, ImGuiComboFlags_None)) { valueChanged = true; }
					ImGui::SameLine();
					if (CreateCombo(effectStateID.c_str(), currentEffectState, g_EffectState, ImGuiComboFlags_None)) { valueChanged = true; }

					// Add a button to remove the effect
					if (ImGui::Button(removeID.c_str(), ImVec2(200, 20)))
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
					}
				}
			}
		}

		ImGui::Separator();

		// Add new
		if (ImGui::Button("Add New Effect", ImVec2(200, 20)))
		{
			TechniqueInfo info;
			info.filename = "Default.fx";
			info.state = "off";

			techniqueWeatherInfoList.push_back(info);
		}
	}

	// Display all the Menus
	ImGui::SeparatorText("Weathers");
	if (!weatherList.empty())
	{
		for (int i = 0; i < weatherList.size(); i++)
		{
			auto& iniWeather = weatherList[i];

			bool valueChanged = false;
			std::string weatherIndexComboID = "Weather" + std::to_string(i + 1);
			std::string weatherNameComboID = "##" + std::to_string(i);
			std::string removeID = "Remove Weather##" + std::to_string(i);

			std::string currentWeatherName = iniWeather.Name;

			ImGui::Text("%s:", weatherIndexComboID.c_str());
			ImGui::SameLine();
			if (CreateCombo(weatherNameComboID.c_str(), currentWeatherName, g_WeatherFlags, ImGuiComboFlags_None)) { valueChanged = true; }

			// Add a button to remove the effect
			if (ImGui::Button(removeID.c_str(), ImVec2(200, 20)))
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
	if (ImGui::Button("Add New Weather", ImVec2(150, 20)))
	{
		Info weather;
		weather.Index = "";
		weather.Name = "default";

		weatherList.push_back(weather);
	}
}