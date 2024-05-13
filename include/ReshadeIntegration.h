#pragma once
#include "Config.h"

extern reshade::api::effect_runtime* s_pRuntime;

class ReshadeIntegration : public ISingleton<ReshadeIntegration>
{
public:

	enum class Categories
	{
		Menu,
		Time,
		Weather,
		Interior
	};

	void ApplyTechniqueState(bool enableReshade, const TechniqueInfo& info);
	void ApplySpecificReshadeStates(bool enableReshade, Categories ProcessState);
	void ApplyReshadeState(bool enableReshade, const std::string& toggleState);
};