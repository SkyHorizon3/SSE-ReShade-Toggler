#include "../include/Processor.h"
#include "../include/ReshadeIntegration.h"


RE::BSEventNotifyControl Processor::ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{

	const auto& menuName = event->menuName;
	auto& opening = event->opening;

	auto [it, inserted] = m_OpenMenus.emplace(menuName);

	if (!opening)
	{
		m_OpenMenus.erase(it); // Mark menu as closed using the iterator
	}

	if (m_OpenMenus.empty())
	{
		return RE::BSEventNotifyControl::kContinue; // Skip if no open menus
	}

	std::unordered_map<std::string, std::unordered_set<std::string>> menuEffectsMap;
	bool enableReshadeMenu = true;

	if (ToggleStateMenus.find("All") != std::string::npos)
	{
		for (const Info& menu : menuList)
		{
			if (m_OpenMenus.find(menu.Name) != m_OpenMenus.end())
			{
				enableReshadeMenu = false;
			}
		}
	}
	else if (ToggleStateMenus.find("Specific") != std::string::npos)
	{
		for (const TechniqueInfo& info : techniqueMenuInfoList)
		{
			menuEffectsMap[info.Name] = { info.filename };
		}

		// Loop through menus and toggle effects accordingly
		for (const auto& menuEntry : menuEffectsMap)
		{
			const std::string& menuName_ = menuEntry.first;
			const std::unordered_set<std::string>& effectsToToggle = menuEntry.second;

			// Check if the menu is open and if any of the associated effects should be toggled
			if (m_OpenMenus.find(menuName_) != m_OpenMenus.end())
			{
				for (const TechniqueInfo& info : techniqueMenuInfoList)
				{
					if (effectsToToggle.find(info.Name) != effectsToToggle.end())
					{
						enableReshadeMenu = true;
						break; // At least one effect should be toggled
					}
				}
			}
		}
	}

	if (s_pRuntime != nullptr)
	{
		if (ToggleStateMenus.find("All") != std::string::npos)
		{
			ReshadeIntegration::ApplyReshadeState(enableReshadeMenu, ToggleAllStateMenus);
		}
		else if (ToggleStateMenus.find("Specific") != std::string::npos)
		{
			ReshadeIntegration::ApplySpecificReshadeStates(enableReshadeMenu, Categories::Menu);
		}

		DEBUG_LOG(g_Logger, "Menu {} {}", menuName, opening ? "open" : "closed");
		DEBUG_LOG(g_Logger, "Reshade {}", enableReshadeMenu ? "enabled" : "disabled");
	}
	else
	{
		g_Logger->critical("Uhm, what? How? s_pRuntime was null. How the fuck did this happen");
	}

	return RE::BSEventNotifyControl::kContinue;

}

RE::BSEventNotifyControl Processor::ProcessTimeBasedToggling()
{

	std::lock_guard<std::mutex> timeLock(timeMutexTime);

	DEBUG_LOG(g_Logger, "Started ProcessTimeBasedToggling", nullptr);

	const auto time = RE::Calendar::GetSingleton();

	float TimecurrentTime = time->GetHour();
	DEBUG_LOG(g_Logger, "currentTime: {} ", TimecurrentTime);

	// Specific
	std::vector<Bool> enableReshade;
	if (ToggleStateTime.find("Specific") != std::string::npos)
	{
		for (TechniqueInfo& info : techniqueTimeInfoList)
		{
			DEBUG_LOG(g_Logger, "info.startTime: {} - info.stopTime: {}", info.startTime, info.stopTime);
			info.enable = !IsTimeWithinRange(TimecurrentTime, info.startTime, info.stopTime);
		}
	}

	// All
	bool enableReshadeTime = true;
	if (ToggleStateTime.find("All") != std::string::npos)
	{
		for (auto& allInfo : techniqueTimeInfoListAll)
		{
			enableReshadeTime = !IsTimeWithinRange(TimecurrentTime, allInfo.startTime, allInfo.stopTime);
			DEBUG_LOG(g_Logger, "State: {} for time: {} - {}. ReshadeBool: {}", allInfo.state, allInfo.startTime, allInfo.stopTime, enableReshadeTime);
		}
	}

	if (s_pRuntime != nullptr)
	{
		if (ToggleStateTime.find("All") != std::string::npos)
		{
			ReshadeIntegration::ApplyReshadeState(enableReshadeTime, ToggleAllStateTime);
		}
		else if (ToggleStateTime.find("Specific") != std::string::npos)
		{
			for (TechniqueInfo& info : techniqueTimeInfoList)
			{
				ReshadeIntegration::ApplyTechniqueState(info.enable, info);
			}
		}
	}

	return RE::BSEventNotifyControl::kContinue;
}

bool Processor::IsTimeWithinRange(double currentTime, double startTime, double endTime)
{
	if (currentTime >= startTime && currentTime <= endTime)
	{
		return true;
	}
	else
	{
		return false;
	}
}

RE::BSEventNotifyControl Processor::ProcessInteriorBasedToggling()
{
	std::lock_guard<std::mutex> lock(timeMutexInterior);

	const auto player = RE::PlayerCharacter::GetSingleton();

	// DEBUG_LOG(g_Logger, "Got player Singleton: {} ", player->GetName());

	if (const auto cell = player->GetParentCell())
	{
		bool enableReshade = [this, cell]()
			{
				if (cell->IsInteriorCell())
				{
					DEBUG_LOG(g_Logger, "Player is in interior cell", nullptr);
					IsInInteriorCell = true;
					return false;
				}
				else
				{
					DEBUG_LOG(g_Logger, "Player is in exterior cell", nullptr);
					IsInInteriorCell = false;
					return true;
				}

			}
		();

		if (s_pRuntime != nullptr)
		{
			if (ToggleStateInterior.find("All") != std::string::npos)
			{
				ReshadeIntegration::ApplyReshadeState(enableReshade, ToggleAllStateInterior);
			}
			else if (ToggleStateInterior.find("Specific") != std::string::npos)
			{
				ReshadeIntegration::ApplySpecificReshadeStates(enableReshade, Categories::Interior);
			}
		}
	}
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Processor::ProcessWeatherBasedToggling()
{
	std::lock_guard<std::mutex> lock(timeMutexWeather);

	const auto sky = RE::Sky::GetSingleton();

	const auto currentWeather = sky->currentWeather;

	const auto flags = currentWeather->data.flags;

	switch (flags.get())
	{
	case RE::TESWeather::WeatherDataFlag::kNone:
		weatherflags = "kNone";
		break;
	case RE::TESWeather::WeatherDataFlag::kRainy:
		weatherflags = "kRainy";
		break;
	case RE::TESWeather::WeatherDataFlag::kPleasant:
		weatherflags = "kPleasant";
		break;
	case RE::TESWeather::WeatherDataFlag::kCloudy:
		weatherflags = "kCloudy";
		break;
	case RE::TESWeather::WeatherDataFlag::kSnow:
		weatherflags = "kSnow";
		break;
	case RE::TESWeather::WeatherDataFlag::kPermAurora:
		weatherflags = "kPermAurora";
		break;
	case RE::TESWeather::WeatherDataFlag::kAuroraFollowsSun:
		weatherflags = "kAuroraFollowsSun";
		break;
	}

	//DEBUG_LOG(g_Logger, "weatherflag {}", weatherflags);

	bool enableReshadeWeather = true;

	if (s_pRuntime != nullptr)
	{
		if (ToggleStateWeather.find("All") != std::string::npos)
		{
			for (const Info& weather : weatherList)
			{
				//DEBUG_LOG(g_Logger, "weatherToDisable {}", weatherToDisable);

				if (weather.Name == weatherflags)
				{
					enableReshadeWeather = false;
					break;
				}
			}

			ReshadeIntegration::ApplyReshadeState(enableReshadeWeather, ToggleAllStateWeather);
		}
		else if (ToggleStateWeather.find("Specific") != std::string::npos)
		{
			for (const TechniqueInfo& info : techniqueWeatherInfoList)
			{
				if (info.Name == weatherflags)
				{
					enableReshadeWeather = false;
					break;
				}
			}

			ReshadeIntegration::ApplySpecificReshadeStates(enableReshadeWeather, Categories::Weather);
		}
	}



	return RE::BSEventNotifyControl::kContinue;
}