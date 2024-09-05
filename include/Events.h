#pragma once

class Menu : public RE::BSTEventSink<RE::MenuOpenCloseEvent>, public ISingleton<Menu>
{
public:
	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override;


private:

	std::set<std::string> m_openMenus;

};