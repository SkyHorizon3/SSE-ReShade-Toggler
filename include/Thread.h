#pragma once
#include "Processor.h"

class Thread
{

public:
	static void RuntimeThread();

private:
	using ProcessFunction = RE::BSEventNotifyControl(Processor::*)();

	static void SubmitToMainThread(const std::string& functionName, ProcessFunction function);
	static void ExecuteMainThreadQueue();
	static void Run();

	static ankerl::unordered_dense::map<std::string, ProcessFunction> m_mainThreadQueue;
	static std::mutex m_mainThreadQueueMutex;

};