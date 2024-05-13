#include "Processor.h"
#include "ReshadeIntegration.h"

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


	if (m_Menu.MenuToggleOption.find("All") != std::string::npos)
	{
		for (const auto& menu : m_Menu.MenuList)
		{
			if (m_openMenus.find(menu) != m_openMenus.end())
			{
				enableReshadeMenu = false;
			}
		}

		ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeMenu, m_Menu.MenuToggleAllState);
	}
	else if (m_Menu.MenuToggleOption.find("Specific") != std::string::npos)
	{
		for (const TechniqueInfo& info : m_Menu.TechniqueMenuInfoList)
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

	SKSE::log::debug("Menu {} {}", menuName, opening ? "open" : "closed");
	SKSE::log::debug("Reshade {}", enableReshadeMenu ? "enabled" : "disabled");

	return RE::BSEventNotifyControl::kContinue;

}

RE::BSEventNotifyControl Processor::ProcessTimeBasedToggling()
{

	std::lock_guard<std::mutex> timeLock(m_timeMutexTime);

	if (m_isMenuOpen)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

	SKSE::log::debug("Started ProcessTimeBasedToggling");

	const auto time = RE::Calendar::GetSingleton();

	float TimecurrentTime = time->GetHour();

	SKSE::log::debug("currentTime: {} ", TimecurrentTime);

	// Specific
	std::vector<Bool> enableReshade;
	if (m_Time.TimeToggleOption.find("Specific") != std::string::npos)
	{
		for (TechniqueInfo& info : m_Time.TechniqueTimeInfoList)
		{
			SKSE::log::debug("info.startTime: {} - info.stopTime: {}", info.StartTime, info.StopTime);

			info.Enable = !IsTimeWithinRange(TimecurrentTime, info.StartTime, info.StopTime);
		}
	}

	// All
	bool enableReshadeTime = true;
	if (m_Time.TimeToggleOption.find("All") != std::string::npos)
	{
		enableReshadeTime = !IsTimeWithinRange(TimecurrentTime, m_Time.TimeToggleAllTimeStart, m_Time.TimeToggleAllTimeStop);

		SKSE::log::debug("State: {} for time: {} - {}. ReshadeBool: {}", m_Time.TimeToggleAllState, m_Time.TimeToggleAllTimeStart, m_Time.TimeToggleAllTimeStop, enableReshadeTime);
	}


	if (m_Time.TimeToggleOption.find("All") != std::string::npos)
	{
		ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeTime, m_Time.TimeToggleAllState);
	}
	else if (m_Time.TimeToggleOption.find("Specific") != std::string::npos)
	{
		for (const TechniqueInfo& info : m_Time.TechniqueTimeInfoList)
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
					SKSE::log::debug("Player is in interior cell");
					//m_Conf.IsInInteriorCell = true;
					return false;
				}
				else
				{
					SKSE::log::debug("Player is in exterior cell");
					//m_Conf.IsInInteriorCell = false;
					return true;
				}

			}
		();


		if (m_Interior.InteriorToggleOption.find("All") != std::string::npos)
		{
			ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshade, m_Interior.InteriorToggleAllState);
		}
		else if (m_Interior.InteriorToggleOption.find("Specific") != std::string::npos)
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
		std::string weatherFlag = "";

		switch (flags.get())
		{
		case RE::TESWeather::WeatherDataFlag::kNone:
			weatherFlag = "kNone";
			break;
		case RE::TESWeather::WeatherDataFlag::kRainy:
			weatherFlag = "kRainy";
			break;
		case RE::TESWeather::WeatherDataFlag::kPleasant:
			weatherFlag = "kPleasant";
			break;
		case RE::TESWeather::WeatherDataFlag::kCloudy:
			weatherFlag = "kCloudy";
			break;
		case RE::TESWeather::WeatherDataFlag::kSnow:
			weatherFlag = "kSnow";
			break;
		case RE::TESWeather::WeatherDataFlag::kPermAurora:
			weatherFlag = "kPermAurora";
			break;
		case RE::TESWeather::WeatherDataFlag::kAuroraFollowsSun:
			weatherFlag = "kAuroraFollowsSun";
			break;
		}

		//DEBUG_LOG(g_Logger, "weatherflag {}", weatherflags);

		bool enableReshadeWeather = true;


		if (m_Weather.WeatherToggleOption.find("All") != std::string::npos)
		{
			for (const auto& weather : m_Weather.WeatherList)
			{
				//DEBUG_LOG(g_Logger, "weatherToDisable {}", weatherToDisable);

				if (weather == weatherFlag)
				{
					enableReshadeWeather = false;
				}

			}
			ReshadeIntegration::GetSingleton()->ApplyReshadeState(enableReshadeWeather, m_Weather.WeatherToggleAllState);

		}
		else if (m_Weather.WeatherToggleOption.find("Specific") != std::string::npos)
		{
			for (const TechniqueInfo& info : m_Weather.TechniqueWeatherInfoList)
			{
				if (info.Name == weatherFlag)
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