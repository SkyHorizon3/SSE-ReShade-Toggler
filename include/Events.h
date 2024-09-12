#pragma once

class Event : public RE::BSTEventSink<RE::MenuOpenCloseEvent>, public ISingleton<Event>
{
public:
	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override;


private:

	std::unordered_set<std::string> m_openMenus;

};