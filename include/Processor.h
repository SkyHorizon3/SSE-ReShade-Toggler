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
	
	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* source) override;
	RE::BSEventNotifyControl ProcessTimeBasedToggling();
	
	bool IsTimeWithinRange(double currentTime, double startTime, double endTime);


private:
	Processor() = default;
	~Processor() = default;
	Processor(const Processor&) = delete;
	Processor(Processor&&) = delete;
	Processor& operator=(const Processor&) = delete;
	Processor& operator=(Processor&&) = delete;

	std::unordered_set<std::string> m_OpenMenus;
};