#pragma once

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
	void SubmitToMainThread(const std::string& functionName, FunctionToExecute);
	void ExecuteMainThreadQueue();
	void Run();

private:
	std::unordered_map <std::string, FunctionToExecute> m_mainThreadQueue;
	std::mutex m_mainThreadQueueMutex;
};