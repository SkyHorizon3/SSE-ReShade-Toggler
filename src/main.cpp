#include "ReshadeIntegration.h"
#include "Processor.h"
#include "Menu.h"
#include "Config.h"
#include "Thread.h"

#define DLLEXPORT __declspec(dllexport)
extern "C" DLLEXPORT const char* NAME = "SSE ReShade Effect Toggler";
extern "C" DLLEXPORT const char* DESCRIPTION = "ReShade Effect Toggler by SkyHorizon and PhilikusHD.";
reshade::api::effect_runtime* s_pRuntime = nullptr;
HMODULE g_hModule = nullptr;

// Callback when Reshade begins effects
static void on_reshade_begin_effects(reshade::api::effect_runtime* runtime)
{
	s_pRuntime = runtime;
}

static void DrawMenu(reshade::api::effect_runtime*)
{
	Menu::GetSingleton()->SettingsMenu();
}

// Register and unregister addon events
void register_addon_events()
{
	reshade::register_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
	reshade::register_overlay(nullptr, &DrawMenu);
}

void unregister_addon_events()
{
	reshade::unregister_event<reshade::addon_event::init_effect_runtime>(on_reshade_begin_effects);
	reshade::unregister_overlay(nullptr, &DrawMenu);
}

// Setup logger for plugin
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
	auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));


#ifndef NDEBUG
	loggerPtr->set_level(spdlog::level::trace);
	loggerPtr->flush_on(spdlog::level::trace);
#else
	loggerPtr->set_level(spdlog::level::info);
	loggerPtr->flush_on(spdlog::level::info);
#endif


	spdlog::set_default_logger(std::move(loggerPtr));
}

// Load Reshade and register events
bool Load()
{
	if (reshade::register_addon(g_hModule))
	{
		SKSE::log::info("Registered addon");
		register_addon_events();
		return true;
	}
	else
	{
		SKSE::log::info("ReShade not present.");
		return false;
	}
}

void MessageListener(SKSE::MessagingInterface::Message* message)
{
	// https://github.com/ianpatt/skse64/blob/09f520a2433747f33ae7d7c15b1164ca198932c3/skse64/PluginAPI.h#L193-L212
	switch (message->type)
	{

	case SKSE::MessagingInterface::kDataLoaded:
		isLoaded = true;
		if (isLoaded)
		{
			std::thread(Thread::RuntimeThread).detach();
		}
		break;


	default:
		break;

	}
}

int __stdcall DllMain(HMODULE hModule, uint32_t fdwReason, void*)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		g_hModule = hModule;
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		if (isLoaded)
		{
			std::thread(Thread::RuntimeThread).join();
		}
		unregister_addon_events();
		reshade::unregister_addon(hModule);
	}

	return 1;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
	SKSE::Init(skse);

	SetupLog();

	if (!Load())
	{
		return false;
	}

	const auto menu = Menu::GetSingleton();
	menu->EnumeratePresets();
	menu->EnumerateEffects();
	menu->EnumerateMenus();

	CSimpleIniA ini;
	ini.SetUnicode(false);
	ini.LoadFile("Data\\SKSE\\Plugins\\ReShadeEffectToggler.ini");
	menu->selectedPresetPath = ini.GetValue("Presets", "PresetPath");
	menu->selectedPreset = ini.GetValue("Presets", "PresetName");

	if (std::filesystem::exists(menu->selectedPresetPath))
	{
		Config::GetSingleton()->DeserializePreset(menu->selectedPresetPath);
	}
	else
	{
		Config::GetSingleton()->DeserializePreset("Data\\SKSE\\Plugins\\TogglerConfigs\\Default.yaml");
	}

	SKSE::GetMessagingInterface()->RegisterListener(MessageListener);
	SKSE::log::info("{} v{} loaded", Plugin::NAME, Plugin::VERSION);

	if (Config::GetSingleton()->GetGeneralInformation().EnableMenus)
	{
		RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(Processor::GetSingleton());
	}

	return true;
}

extern "C" DLLEXPORT const auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
	}
();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo * pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}