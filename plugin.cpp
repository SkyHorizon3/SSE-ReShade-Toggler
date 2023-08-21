#include <spdlog/sinks/basic_file_sink.h>
#include <SimpleIni.h>
#include "PCH.h"
#include "include/reshade.hpp"

namespace logger = SKSE::log;

HMODULE g_hModule = nullptr;
static reshade::api::effect_runtime* s_pRuntime = nullptr;
std::shared_ptr<spdlog::logger> g_Logger;

class MyEffectRuntime : public reshade::api::effect_runtime 
{
public:
    void set_effects_state(bool enabled) override 
    { 
        reshade::api::effect_runtime::set_effects_state(enabled); 
    }
};

class EventProcessor : public RE::BSTEventSink<RE::MenuOpenCloseEvent> 
{
public:
    static EventProcessor& GetSingleton() 
    {
        static EventProcessor singleton;
        return singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override 
    {
        const std::string_view menuName = event->menuName;
        if (menuName == "MapMenu") 
        {
            const bool opening = event->opening;
            logger::info("Menu {} {}", menuName, opening ? "open" : "closed");

            // Toggle Reshade effects based on menuState
            if (s_pRuntime != nullptr) 
            {
                s_pRuntime->set_effects_state(!opening);
                logger::info("Reshade {}", opening ? "disabled" : "enabled");
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

private:
    EventProcessor() = default;
    ~EventProcessor() = default;
    EventProcessor(const EventProcessor&) = delete;
    EventProcessor(EventProcessor&&) = delete;
    EventProcessor& operator=(const EventProcessor&) = delete;
    EventProcessor& operator=(EventProcessor&&) = delete;
};

static void on_reshade_begin_effects(reshade::api::effect_runtime* runtime) 
{ 
    s_pRuntime = runtime; 
}

void register_addon_events() 
{
    reshade::register_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
}

void unregister_addon_events() 
{
    reshade::unregister_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
}

void Load() 
{
    if (reshade::register_addon(g_hModule)) {
        g_Logger->info("Registered addon");
        register_addon_events();
    } else {
        g_Logger->info("ReShade not present.");
    }
}

void SetupLog() 
{
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) 
    {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    }

    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);

    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    g_Logger = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(g_Logger);
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}


// Fun Fact: 'BOOL' ist eigentlich ein int und 'TRUE' is 1, 'FALSE' dementsprechend 0. Warum Microsoft ihre eigenen Typen gemacht hat, die eigentlich nur int sind, keine Ahnung. Es gibt noch viel mehr von sowas
// 'APIENTRY' ist einfach nur __stdcall, aber einfacher zu verstehen gj Microsoft!
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID) 
{
    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
        g_hModule = hModule;
    } 
    else if (fdwReason == DLL_PROCESS_DETACH) 
    {
        unregister_addon_events();
        reshade::unregister_addon(hModule);
    }

    return TRUE;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    SetupLog();
    g_Logger->info("Loaded plugin");

    auto& eventProcessor = EventProcessor::GetSingleton();
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&eventProcessor);
    Load();

    return true;
}