#pragma once
#include "Globals.h"

class ReshadeToggler
{
public:

	static ReshadeToggler* GetSingleton()
	{
		static ReshadeToggler toggler;
		return &toggler;
	}

	using FunctionToExecute = RE::BSEventNotifyControl(*)();

	void Setup();
	void SetupLog();
	void Load();
	void LoadINI(const std::string& presetPath);
	void LoadPreset(const std::string& Preset);
	void SubmitToMainThread(const std::string& functionName, FunctionToExecute);
	void ExecuteMainThreadQueue();
	void Run();

private:
	std::unordered_map <std::string, FunctionToExecute> m_MainThreadQueue;
	std::mutex m_MainThreadQueueMutex;
};