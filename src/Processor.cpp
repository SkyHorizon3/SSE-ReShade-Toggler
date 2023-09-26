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

    // Is this necessary? No. Are we still doing it, yes. Why? Idk, it looks fancy
    bool enableReshade = [this]() {
        for (const auto& menuToDisable : g_MenuValue)
        {
            if (m_OpenMenus.find(menuToDisable) != m_OpenMenus.end())
            {
                return false; // If any disabled menu is open, disable Reshade
            }
        }
        return true; // If no disabled menus are open, enable Reshade
    }();

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

    std::lock_guard<std::mutex> lock(timeMutexTime);

    DEBUG_LOG(g_Logger, "Started ProcessTimeBasedToggling", nullptr);

    const auto time = RE::Calendar::GetSingleton();

    double currentTime = time->GetHour();
    DEBUG_LOG(g_Logger, "currentTime: {} ", currentTime);

    bool enableReshade = [this, currentTime]()
        {
            for (const TechniqueInfo& info : techniqueTimeInfoList)
            {
                if (IsTimeWithinRange(currentTime, info.startTime, info.stopTime))
                {
                    return false;
                }
            }
            return true;
    }();

    if (s_pRuntime != nullptr)
    {
        if (ToggleStateTime.find("All") != std::string::npos)
        {
            ReshadeIntegration::ApplyReshadeState(enableReshade, ToggleAllStateTime);
        }
        else if (ToggleStateTime.find("Specific") != std::string::npos)
        {
            ReshadeIntegration::ApplySpecificReshadeStates(enableReshade, Categories::Time);
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
  
    DEBUG_LOG(g_Logger, "Got player Singleton: {} ", player->GetName());
      
    if(const auto cell = player->GetParentCell())
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

        }();

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