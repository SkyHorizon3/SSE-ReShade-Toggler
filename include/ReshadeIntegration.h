#pragma once
#include "Globals.h"

extern reshade::api::effect_runtime* s_pRuntime;

class ReshadeIntegration
{
public:

	static ReshadeIntegration* GetSingleton()
	{
		static ReshadeIntegration singleton;
		return &singleton;
	}

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