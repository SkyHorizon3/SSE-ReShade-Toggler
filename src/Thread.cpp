#include "Thread.h"
#include "Config.h"

ankerl::unordered_dense::map<std::string, Thread::ProcessFunction> Thread::m_mainThreadQueue;
std::mutex Thread::m_mainThreadQueueMutex;

void Thread::SubmitToMainThread(const std::string& functionName, ProcessFunction function)
{
	std::scoped_lock<std::mutex> lock(m_mainThreadQueueMutex);
	m_mainThreadQueue[functionName] = function;
}

void Thread::ExecuteMainThreadQueue()
{
	std::scoped_lock<std::mutex> lock(m_mainThreadQueueMutex);
	const auto processor = Processor::GetSingleton();

	for (const auto& func : m_mainThreadQueue)
	{
		//g_Logger->info("Function: {}", func.first.c_str());
		(processor->*func.second)(); // Run function
	}
	m_mainThreadQueue.clear();

}

void Thread::Run()
{
	if (m_mainThreadQueue.find("Weather") != m_mainThreadQueue.end() || m_mainThreadQueue.find("Interior") != m_mainThreadQueue.end() || m_mainThreadQueue.find("Time") != m_mainThreadQueue.end())
	{
		ExecuteMainThreadQueue();
	}
}

void Thread::RuntimeThread()
{
	SKSE::log::info("Attaching RuntimeThread");

	while (isLoaded)
	{

		Config config;
		const auto& info = config.GetGeneralInformation();

		if (info.EnableTime)
		{
			//g_Logger->info("Adding Time to Mainqueue");
			std::this_thread::sleep_for(std::chrono::seconds(config.GetTimeInformation().TimeUpdateInterval));
			SubmitToMainThread("Time", &Processor::ProcessTimeBasedToggling);
		}

		if (info.EnableInterior)
		{
			//g_Logger->info("Adding Interior to Mainqueue");
			std::this_thread::sleep_for(std::chrono::seconds(config.GetInteriorInformation().InteriorUpdateInterval));
			SubmitToMainThread("Interior", &Processor::ProcessInteriorBasedToggling);
		}

		if (info.EnableWeather)//&& !m_Conf.IsInInteriorCell
		{
			//g_Logger->info("Adding Weather to Mainqueue");
			std::this_thread::sleep_for(std::chrono::seconds(config.GetWeatherInformation().WeatherUpdateInterval));
			SubmitToMainThread("Weather", &Processor::ProcessWeatherBasedToggling);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		Run();
	}
}