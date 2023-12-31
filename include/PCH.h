#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <unordered_set>
#include <chrono>
#include <vector>
#include <memory>
#include <Windows.h>
#include <cstdint>
#include <filesystem>

#include <spdlog/sinks/basic_file_sink.h>
#include <SimpleIni.h>

using namespace std::literals;

#if _DEBUG
#define DEBUG_LOG(logger, msg, ...) logger->info(msg, __VA_ARGS__)
#else
#define DEBUG_LOG(logger, msg, ...)
#endif

#include "Plugin.h"
#include "Utils.h"

#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include <ImGui/imgui.h>
#include <reshade/reshade.hpp>