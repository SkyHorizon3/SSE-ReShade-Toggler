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


	bool enableReshade = [this]()
		{
			for (const Info& menu : menuList)
			{
				if (m_OpenMenus.find(menu.Name) != m_OpenMenus.end())
				{
					return false;
				}
			}
			return true;
		}
	();


	if (s_pRuntime != nullptr)
	{
		if (ToggleStateMenus.find("All") != std::string::npos)
		{
			ReshadeIntegration::ApplyReshadeState(enableReshade, ToggleAllStateMenus);
		}
		else if (ToggleStateMenus.find("Specific") != std::string::npos)
		{
			ReshadeIntegration::ApplySpecificReshadeStates(enableReshade, Categories::Menu);
		}

		DEBUG_LOG(g_Logger, "Menu {} {}", menuName, opening ? "open" : "closed");
		DEBUG_LOG(g_Logger, "Reshade {}", enableReshade ? "enabled" : "disabled");
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
					DEBUG_LOG(g_Logger, "Player is in interior cell {}", cell->GetName());
					return false;
				}
				else
				{
					DEBUG_LOG(g_Logger, "Player is in exterior cell", nullptr);
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

	if (const auto currentWeather = sky->currentWeather)
	{

		const auto flags = currentWeather->data.flags;

		if (flags.any(RE::TESWeather::WeatherDataFlag::kNone))
		{
			weatherflags = "kNone";
		}
		else if (flags.any(RE::TESWeather::WeatherDataFlag::kRainy))
		{
			weatherflags = "kRainy";
		}
		else if (flags.any(RE::TESWeather::WeatherDataFlag::kPleasant))
		{
			weatherflags = "kPleasant";
		}
		else if (flags.any(RE::TESWeather::WeatherDataFlag::kCloudy))
		{
			weatherflags = "kCloudy";
		}
		else if (flags.any(RE::TESWeather::WeatherDataFlag::kSnow))
		{
			weatherflags = "kSnow";
		}
		else if (flags.any(RE::TESWeather::WeatherDataFlag::kPermAurora))
		{
			weatherflags = "kPermAurora";
		}
		else if (flags.any(RE::TESWeather::WeatherDataFlag::kAuroraFollowsSun))
		{
			weatherflags = "kAuroraFollowsSun";
		}

		//DEBUG_LOG(g_Logger, "weatherflag {}", weatherflags);

		bool enableReshadeWeather = true;
		for (const auto& weatherToDisable : g_WeatherValue)
		{
			//DEBUG_LOG(g_Logger, "weatherToDisable {}", weatherToDisable);

			if (weatherToDisable == weatherflags)
			{
				enableReshadeWeather = false;
				break;
			}
		}

		if (s_pRuntime != nullptr)
		{
			if (ToggleStateWeather.find("All") != std::string::npos)
			{
				ReshadeIntegration::ApplyReshadeState(enableReshadeWeather, ToggleAllStateWeather);
			}
			else if (ToggleStateWeather.find("Specific") != std::string::npos)
			{
				ReshadeIntegration::ApplySpecificReshadeStates(enableReshadeWeather, Categories::Weather);
			}
		}


	}
	return RE::BSEventNotifyControl::kContinue;
}
