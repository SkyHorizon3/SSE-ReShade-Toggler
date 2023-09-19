#pragma once
#include "Globals.h"

class EffectRuntime : public reshade::api::effect_runtime
{
public:
    void set_effects_state(bool enabled) override
    {
        reshade::api::effect_runtime::set_effects_state(enabled);
    }
};


class TechniqueRuntime : public reshade::api::effect_runtime
{
public:
    void set_technique_state(reshade::api::effect_technique technique, bool enabled) override
    {
        reshade::api::effect_runtime::set_technique_state(technique, enabled);
    }
};


class ReshadeIntegration
{
public:

	static void ApplyTechniqueState(bool enableReshade, const TechniqueInfo& info);
	static void ApplySpecificReshadeStates(bool enableReshade, Categories ProcessState);
	static void ApplyReshadeState(bool enableReshade, const std::string& toggleState);
};