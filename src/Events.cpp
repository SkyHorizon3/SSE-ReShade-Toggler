#include "Events.h"

RE::BSEventNotifyControl Menu::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source)
{
	if (!a_event || !a_source)
		return RE::BSEventNotifyControl::kContinue;


	// Run Process function

	return RE::BSEventNotifyControl::kContinue;
}