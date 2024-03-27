#include "Processor.h"
#include "ReshadeIntegration.h"
#include "Globals.h"

RE::BSEventNotifyControl Processor::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source)
{
	if (!a_event || !a_source)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

	const auto& menuName = a_event->menuName;
	auto& opening = a_event->opening;

	auto [it, inserted] = m_openMenus.emplace(menuName);

	if (!opening)
	{
		m_isMenuOpen = false;
		m_openMenus.erase(it); // Mark menu as closed using the iterator
	}
	else { m_isMenuOpen = true; }

	if (m_openMenus.empty())
	{
		return RE::BSEventNotifyControl::kContinue; // Skip if no open menus
	}

	bool enableReshadeMenu = true;


	if (ToggleStateMenus.find("All") != std::string::npos)
	{
		for (const Info& menu : menuList)
		{
			if (m_openMenus.find(menu.Name) != m_openMenus.end())
			{
				enableReshadeMenu = false;
			}
		}

		ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeMenu, ToggleAllStateMenus);
	}
	else if (ToggleStateMenus.find("Specific") != std::string::npos)
	{
		for (const TechniqueInfo& info : techniqueMenuInfoList)
		{
			if (m_openMenus.find(info.Name) != m_openMenus.end())
			{
				enableReshadeMenu = false;
			}
			else
			{
				enableReshadeMenu = true;
			}

			ReshadeIntegration::GetSingleton()->ApplyTechniqueState(enableReshadeMenu, info);
		}
	}

#ifndef NDEBUG
	SKSE::log::debug("Menu {} {}", menuName, opening ? "open" : "closed");
	SKSE::log::debug("Reshade {}", enableReshadeMenu ? "enabled" : "disabled");
#endif

	return RE::BSEventNotifyControl::kContinue;

}

RE::BSEventNotifyControl Processor::ProcessTimeBasedToggling()
{

	std::lock_guard<std::mutex> timeLock(m_timeMutexTime);

	if (m_isMenuOpen)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

#ifndef NDEBUG
	SKSE::log::debug("Started ProcessTimeBasedToggling");
#endif

	const auto time = RE::Calendar::GetSingleton();

	float TimecurrentTime = time->GetHour();

#ifndef NDEBUG
	SKSE::log::debug("currentTime: {} ", TimecurrentTime);
#endif

	// Specific
	std::vector<Bool> enableReshade;
	if (ToggleStateTime.find("Specific") != std::string::npos)
	{
		for (TechniqueInfo& info : techniqueTimeInfoList)
		{

#ifndef NDEBUG
			SKSE::log::debug("info.startTime: {} - info.stopTime: {}", info.startTime, info.stopTime);
#endif
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

#ifndef NDEBUG
			SKSE::log::debug("State: {} for time: {} - {}. ReshadeBool: {}", allInfo.state, allInfo.startTime, allInfo.stopTime, enableReshadeTime);
#endif
		}
	}


	if (ToggleStateTime.find("All") != std::string::npos)
	{
		ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeTime, ToggleAllStateTime);
	}
	else if (ToggleStateTime.find("Specific") != std::string::npos)
	{
		for (const TechniqueInfo& info : techniqueTimeInfoList)
		{
			ReshadeIntegration::GetSingleton()->ApplyTechniqueState(info.enable, info);
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
	std::lock_guard<std::mutex> lock(m_timeMutexInterior);

	if (m_isMenuOpen)
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
#ifndef NDEBUG
					SKSE::log::debug("Player is in interior cell");
#endif
					IsInInteriorCell = true;
					return false;
				}
				else
				{
#ifndef NDEBUG
					SKSE::log::debug("Player is in exterior cell");
#endif
					IsInInteriorCell = false;
					return true;
				}

			}
		();


		if (ToggleStateInterior.find("All") != std::string::npos)
		{
			ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshade, ToggleAllStateInterior);
		}
		else if (ToggleStateInterior.find("Specific") != std::string::npos)
		{
			ReshadeIntegration::GetSingleton()->ApplySpecificReshadeStates(enableReshade, ReshadeIntegration::Categories::Interior);
		}

	}
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Processor::ProcessWeatherBasedToggling()
{
	std::lock_guard<std::mutex> lock(m_timeMutexWeather);

	if (m_isMenuOpen)
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
			ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeWeather, ToggleAllStateWeather);

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

				ReshadeIntegration::GetSingleton()->ApplyTechniqueState(enableReshadeWeather, info);
			}
		}


	}

	return RE::BSEventNotifyControl::kContinue;
}