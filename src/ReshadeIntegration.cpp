#include "ReshadeIntegration.h"
#include "Processor.h"

void ReshadeIntegration::ApplyTechniqueState(bool enableReshade, const TechniqueInfo& info)
{
	s_pRuntime->enumerate_techniques(info.Filename.c_str(), [&enableReshade, &info](reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique)
		{
			//DEBUG_LOG(g_Logger, "State: {} for: {}", info.state.c_str(), info.filename.c_str());
			if (info.State == "off")
			{
				runtime->set_technique_state(technique, enableReshade);
			}
			else if (info.State == "on")
			{
				runtime->set_technique_state(technique, !enableReshade);
			}
		});
}

void ReshadeIntegration::ApplySpecificReshadeStates(bool enableReshade, Categories ProcessState)
{
	//DEBUG_LOG(g_Logger, "Specific is enabled! - EnableReshade: {}", enableReshade);
	auto conf = Config::GetSingleton()->GetConfig();
	switch (ProcessState)
	{
	case Categories::Menu:
		for (const TechniqueInfo& info : conf.TechniqueMenuInfoList)
		{
			ApplyTechniqueState(enableReshade, info);
		}
		break;
		// Kind of redundant, but we'll keep it in here for now, might need later
	case Categories::Time:
		for (const TechniqueInfo& info : conf.TechniqueTimeInfoList)
		{
			ApplyTechniqueState(enableReshade, info);
		}
		break;
	case Categories::Interior:
		for (const TechniqueInfo& info : conf.TechniqueInteriorInfoList)
		{
			ApplyTechniqueState(enableReshade, info);
		}
		break;
	case Categories::Weather:
		for (const TechniqueInfo& info : conf.TechniqueWeatherInfoList)
		{
			ApplyTechniqueState(!enableReshade, info);
		}
		break;
	default:
		SKSE::log::error("Invalid option");
	}
}

void ReshadeIntegration::ApplyReshadeState(bool enableReshade, const std::string& toggleState)
{
	if (toggleState == "off")
	{
		s_pRuntime->set_effects_state(enableReshade);
	}
	else if (toggleState == "on")
	{
		s_pRuntime->set_effects_state(!enableReshade);
	}
}
