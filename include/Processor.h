#pragma once

class Processor : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
	static Processor* GetSingleton()
	{
		static Processor singleton;
		return &singleton;
	}

	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override;
	RE::BSEventNotifyControl ProcessTimeBasedToggling();
	RE::BSEventNotifyControl ProcessInteriorBasedToggling();
	RE::BSEventNotifyControl ProcessWeatherBasedToggling();

private:
	bool IsTimeWithinRange(double currentTime, double startTime, double endTime);


	Processor() = default;
	~Processor() = default;
	Processor(const Processor&) = delete;
	Processor(Processor&&) = delete;
	Processor& operator=(const Processor&) = delete;
	Processor& operator=(Processor&&) = delete;

	ankerl::unordered_dense::set<std::string> m_openMenus;
	bool m_isMenuOpen = false;

	std::mutex m_timeMutexTime;
	std::mutex m_vectorMutexTime;
	std::mutex m_timeMutexInterior;
	std::mutex m_timeMutexWeather;
};