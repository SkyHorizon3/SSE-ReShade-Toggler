#include "../include/ReshadeIntegration.h"
#include "../include/ReShadeToggler.h"

void ReshadeIntegration::ApplyTechniqueState(bool enableReshade, const TechniqueInfo& info)
{
    s_pRuntime->enumerate_techniques(info.filename.c_str(), [&enableReshade, &info](reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique)
    {
        g_Logger->info("State: {} for: {}", info.state.c_str(), info.filename.c_str());
        if (info.state == "off")
        {
            runtime->set_technique_state(technique, enableReshade);
        }
        else if (info.state == "on")
        {
            runtime->set_technique_state(technique, !enableReshade);
        }
        else
        {
            g_Logger->error("Wrong input: MenuToggleSpecificState has to be on/off, input was: {} for: {}", info.state.c_str(), info.filename.c_str());
        }
    });
}

void ReshadeIntegration::ApplySpecificReshadeStates(bool enableReshade, Categories ProcessState)
{
    DEBUG_LOG(g_Logger, "Specific is enabled! - EnableReshade: {}", enableReshade);

    switch (ProcessState)
    {
    case Categories::Menu:
        for (const TechniqueInfo& info : techniqueMenuInfoList)
        {
            ApplyTechniqueState(enableReshade, info);
        }
        break;
    case Categories::Time:
        for (const TechniqueInfo& info : techniqueTimeInfoList)
        {
            ApplyTechniqueState(enableReshade, info);
        }
        break;
    default:
        g_Logger->info("Invalid option");
    }
}

void ReshadeIntegration::ApplyReshadeState(bool enableReshade, const std::string& toggleState)
{
    DEBUG_LOG(g_Logger, "{} is enabled! - EnableReshade: {}", toggleState, enableReshade);

    if (toggleState == "off")
    {
        s_pRuntime->set_effects_state(enableReshade);
    }
    else if (toggleState == "on")
    {
        s_pRuntime->set_effects_state(!enableReshade);
    }
}
