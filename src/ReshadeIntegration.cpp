#include "../include/ReshadeIntegration.h"
#include "../include/ReShadeToggler.h"

void ReshadeIntegration::ApplyTechniqueState(bool enableReshade, const TechniqueInfo& info)
{
	s_pRuntime->enumerate_techniques(info.filename.c_str(), [&enableReshade, &info](reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique)
		{
			//DEBUG_LOG(g_Logger, "State: {} for: {}", info.state.c_str(), info.filename.c_str());
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
	//DEBUG_LOG(g_Logger, "Specific is enabled! - EnableReshade: {}", enableReshade);

	switch (ProcessState)
	{
	case Categories::Menu:
		for (const TechniqueInfo& info : techniqueMenuInfoList)
		{
			ApplyTechniqueState(enableReshade, info);
		}
		break;
		// Kind of redundant, but we'll keep it in here for now, might need later
	case Categories::Time:
		for (const TechniqueInfo& info : techniqueTimeInfoList)
		{
			ApplyTechniqueState(enableReshade, info);
		}
		break;
	case Categories::Interior:
		for (const TechniqueInfo& info : techniqueInteriorInfoList)
		{
			ApplyTechniqueState(enableReshade, info);
		}
		break;
	case Categories::Weather:
		for (const TechniqueInfo& info : techniqueWeatherInfoList)
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
	//DEBUG_LOG(g_Logger, "All is enabled! - EnableReshade: {}", enableReshade);

	if (toggleState == "off")
	{
		s_pRuntime->set_effects_state(enableReshade);
	}
	else if (toggleState == "on")
	{
		s_pRuntime->set_effects_state(!enableReshade);
	}
}

void ReshadeIntegration::EnumerateEffects()
{
	const std::filesystem::path shadersDirectory = L"reshade-shaders\\Shaders";

	for (const auto& entry : std::filesystem::recursive_directory_iterator(shadersDirectory))
	{
		if (entry.is_regular_file() && entry.path().filename().extension() == ".fx")
		{
			g_Effects.push_back(entry.path().filename().string());
		}
	}
	//sort Files
	std::sort(g_Effects.begin(), g_Effects.end());
}

void ReshadeIntegration::EnumeratePresets()
{
	const std::filesystem::path presetDirectory = L"Data\\SKSE\\Plugins\\TogglerConfigs\\";

	for (const auto& preset : std::filesystem::recursive_directory_iterator(presetDirectory))
	{
		if (preset.is_regular_file() && preset.path().filename().extension() == ".ini")
		{
			g_Presets.push_back(preset.path().filename().string());
		}
	}
	//sort Presets
	std::sort(g_Presets.begin(), g_Presets.end());
}
