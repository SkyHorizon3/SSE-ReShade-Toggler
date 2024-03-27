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

#include "Plugin.h"
#include "Utils.h"

#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include <ImGui/imgui.h>
#include <reshade/reshade.hpp>