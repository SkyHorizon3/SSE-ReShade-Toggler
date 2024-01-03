#pragma once
#include "PCH.h"
#include "Globals.h"
#include "ReShadeToggler.h"

class Processor : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
	static Processor& GetSingleton()
	{
		static Processor singleton;
		return singleton;
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

	std::unordered_set<std::string> m_OpenMenus;
	bool m_IsMenuOpen = false;
};