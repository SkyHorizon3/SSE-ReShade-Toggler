#include "PCH.h"
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


	if (m_Conf.ToggleStateMenus.find("All") != std::string::npos)
	{
		for (const Info& menu : m_Conf.MenuList)
		{
			if (m_openMenus.find(menu.Name) != m_openMenus.end())
			{
				enableReshadeMenu = false;
			}
		}

		ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeMenu, m_Conf.ToggleAllStateMenus);
	}
	else if (m_Conf.ToggleStateMenus.find("Specific") != std::string::npos)
	{
		for (const TechniqueInfo& info : m_Conf.TechniqueMenuInfoList)
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
	if (m_Conf.ToggleStateTime.find("Specific") != std::string::npos)
	{
		for (TechniqueInfo& info : m_Conf.TechniqueTimeInfoList)
		{

#ifndef NDEBUG
			SKSE::log::debug("info.startTime: {} - info.stopTime: {}", info.StartTime, info.StopTime);
#endif
			info.Enable = !IsTimeWithinRange(TimecurrentTime, info.StartTime, info.StopTime);
		}
	}

	// All
	bool enableReshadeTime = true;
	if (m_Conf.ToggleStateTime.find("All") != std::string::npos)
	{
		for (auto& allInfo : m_Conf.TechniqueTimeInfoListAll)
		{
			enableReshadeTime = !IsTimeWithinRange(TimecurrentTime, allInfo.StartTime, allInfo.StopTime);

#ifndef NDEBUG
			SKSE::log::debug("State: {} for time: {} - {}. ReshadeBool: {}", allInfo.State, allInfo.StartTime, allInfo.StopTime, enableReshadeTime);
#endif
		}
	}


	if (m_Conf.ToggleStateTime.find("All") != std::string::npos)
	{
		ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeTime, m_Conf.ToggleAllStateTime);
	}
	else if (m_Conf.ToggleStateTime.find("Specific") != std::string::npos)
	{
		for (const TechniqueInfo& info : m_Conf.TechniqueTimeInfoList)
		{
			ReshadeIntegration::GetSingleton()->ApplyTechniqueState(info.Enable, info);
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

RE::BSEventNotifyControl Processor::ProcessInteriorBasedToggling() //RELOCATION_ID(13171, 13316) + 0x2E6, 0x46D; RELOCATION_ID(13172, 13317) + 0x2A, 0x1E
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
					m_Conf.IsInInteriorCell = true;
					return false;
				}
				else
				{
#ifndef NDEBUG
					SKSE::log::debug("Player is in exterior cell");
#endif
					m_Conf.IsInInteriorCell = false;
					return true;
				}

			}
		();


		if (m_Conf.ToggleStateInterior.find("All") != std::string::npos)
		{
			ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshade, m_Conf.ToggleAllStateInterior);
		}
		else if (m_Conf.ToggleStateInterior.find("Specific") != std::string::npos)
		{
			ReshadeIntegration::GetSingleton()->ApplySpecificReshadeStates(enableReshade, ReshadeIntegration::Categories::Interior);
		}

	}
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Processor::ProcessWeatherBasedToggling() // Weatherchange function: RELOCATION_ID(25684, 26231)+ 0x44F, 0x46C
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
			m_Conf.Weatherflags = "kNone";
			break;
		case RE::TESWeather::WeatherDataFlag::kRainy:
			m_Conf.Weatherflags = "kRainy";
			break;
		case RE::TESWeather::WeatherDataFlag::kPleasant:
			m_Conf.Weatherflags = "kPleasant";
			break;
		case RE::TESWeather::WeatherDataFlag::kCloudy:
			m_Conf.Weatherflags = "kCloudy";
			break;
		case RE::TESWeather::WeatherDataFlag::kSnow:
			m_Conf.Weatherflags = "kSnow";
			break;
		case RE::TESWeather::WeatherDataFlag::kPermAurora:
			m_Conf.Weatherflags = "kPermAurora";
			break;
		case RE::TESWeather::WeatherDataFlag::kAuroraFollowsSun:
			m_Conf.Weatherflags = "kAuroraFollowsSun";
			break;
		}

		//DEBUG_LOG(g_Logger, "weatherflag {}", weatherflags);

		bool enableReshadeWeather = true;


		if (m_Conf.ToggleStateWeather.find("All") != std::string::npos)
		{
			for (const Info& weather : m_Conf.WeatherList)
			{
				//DEBUG_LOG(g_Logger, "weatherToDisable {}", weatherToDisable);

				if (weather.Name == m_Conf.Weatherflags)
				{
					enableReshadeWeather = false;
				}

			}
			ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeWeather, m_Conf.ToggleAllStateWeather);

		}
		else if (m_Conf.ToggleStateWeather.find("Specific") != std::string::npos)
		{
			for (const TechniqueInfo& info : m_Conf.TechniqueWeatherInfoList)
			{
				if (info.Name == m_Conf.Weatherflags)
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