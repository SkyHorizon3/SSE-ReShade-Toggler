#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "Reshade/reshade.hpp"

#include <unordered_set>
#include <chrono>
#include <memory>
#include <Windows.h>
#include <cstdint>
#include "ImGui/imgui.h"


using namespace std::literals;

#if _DEBUG
	#define DEBUG_LOG(logger, msg, ...) logger->info(msg, __VA_ARGS__)
#else
	#define DEBUG_LOG(logger, msg, ...)
#endif