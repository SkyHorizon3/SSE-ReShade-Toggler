#include "../include/Processor.h"
#include "../include/ReshadeIntegration.h"


RE::BSEventNotifyControl Processor::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source)
{
	if (!a_event || !a_source)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

	const auto& menuName = a_event->menuName;
	auto& opening = a_event->opening;

	auto [it, inserted] = m_OpenMenus.emplace(menuName);

	if (!opening)
	{
		m_IsMenuOpen = false;
		m_OpenMenus.erase(it); // Mark menu as closed using the iterator
	}
	else { m_IsMenuOpen = true; }

	if (m_OpenMenus.empty())
	{
		return RE::BSEventNotifyControl::kContinue; // Skip if no open menus
	}

	bool enableReshadeMenu = true;

	if (s_pRuntime != nullptr)
	{
		if (ToggleStateMenus.find("All") != std::string::npos)
		{
			for (const Info& menu : menuList)
			{
				if (m_OpenMenus.find(menu.Name) != m_OpenMenus.end())
				{
					enableReshadeMenu = false;
				}
			}

			ReshadeIntegration::ApplyReshadeState(enableReshadeMenu, ToggleAllStateMenus);
		}
		else if (ToggleStateMenus.find("Specific") != std::string::npos)
		{
			for (const TechniqueInfo& info : techniqueMenuInfoList)
			{
				if (m_OpenMenus.find(info.Name) != m_OpenMenus.end())
				{
					enableReshadeMenu = false;
				}
				else
				{
					enableReshadeMenu = true;
				}

				ReshadeIntegration::ApplyTechniqueState(enableReshadeMenu, info);
			}
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

	if (m_IsMenuOpen)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

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

	if (m_IsMenuOpen)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

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

	if (m_IsMenuOpen)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

	const auto sky = RE::Sky::GetSingleton();

	if (const auto currentWeather = sky->currentWeather)
	{

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
					}
					else
					{
						enableReshadeWeather = true;
					}

					ReshadeIntegration::ApplyTechniqueState(enableReshadeWeather, info);
				}
			}
		}

	}

	return RE::BSEventNotifyControl::kContinue;
}