#include "Events.h"
#include "Manager.h"

RE::BSEventNotifyControl Event::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source)
{
	if (!a_event || !a_source)
		return RE::BSEventNotifyControl::kContinue;

	const auto& menuName = a_event->menuName;
	const auto& opening = a_event->opening;

	auto [it, inserted] = m_openMenus.emplace(menuName);

	if (!opening)
		m_openMenus.erase(it);

	if (m_openMenus.empty())
		return RE::BSEventNotifyControl::kContinue;

	Manager::GetSingleton()->ToggleEffectMenu(m_openMenus);

	return RE::BSEventNotifyControl::kContinue;
}