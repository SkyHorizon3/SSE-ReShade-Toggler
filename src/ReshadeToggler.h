#pragma once

#include "PCH.h"
#include <unordered_set>
#include <SimpleIni.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "../include/reshade.hpp"

HMODULE g_hModule = nullptr;
static reshade::api::effect_runtime* s_pRuntime = nullptr;
std::shared_ptr<spdlog::logger> g_Logger;
std::unordered_set<std::string> g_menuValue;