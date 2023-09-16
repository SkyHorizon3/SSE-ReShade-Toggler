#include "../include/Processor.h"
#include "../include/ReshadeIntegration.h"

RE::BSEventNotifyControl Processor::ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* source)
{
    
    if (!EnableMenus)
    {
        g_Logger->info("EnableMenus is set to false, no menus will be processed."); // Skip execution if EnableMenus is false
        return RE::BSEventNotifyControl::kContinue;
    }

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
            ReshadeIntegration::ApplyReshadeState(enableReshade, ToggleStateMenus);
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
    std::lock_guard<std::mutex> lock(timeMutex);


    if (!EnableTime)
    {
        g_Logger->info("Time-based toggling is disabled!");
        return RE::BSEventNotifyControl::kContinue;
    }

    g_Logger->info("Started ProcessTimeBasedToggling");

   // std::this_thread::sleep_for(std::chrono::seconds(TimeUpdateIntervall));

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
            ReshadeIntegration::ApplyReshadeState(enableReshade, ToggleStateTime);
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
