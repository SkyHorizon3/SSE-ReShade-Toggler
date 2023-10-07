#pragma once
#include "Globals.h"
#include <string>
#include <vector>
#include <SimpleIni.h>

class ReshadeToggler
{
public:

	static ReshadeToggler GetSingleton()
	{
		ReshadeToggler toggler;
		return toggler;
	}

	void Setup();
	void SetupLog();
	void Load();
	void LoadINI(const std::string& presetPath);
	void LoadPreset(const std::string& Preset);
};

class CentralControl
{
public:

	static CentralControl* GetSingleton()
	{
		static CentralControl control;
		return &control;
	}

	void Update();
	void Prioritize();

private:
	std::mutex m_mutex;
	std::unordered_map<Categories, bool> m_systemStates;

	void FinalReshadeUpdate(bool enable);

};
