#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

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

#include <ankerl/unordered_dense.h>
template <>
struct ankerl::unordered_dense::hash<std::string>
{
	using is_transparent = void;  // enable heterogeneous overloads
	using is_avalanching = void;  // mark class as high quality avalanching hash

	[[nodiscard]] auto operator()(std::string_view str) const noexcept -> uint64_t
	{
		return ankerl::unordered_dense::hash<std::string_view>{}(str);
	}
};

#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include <ImGui/imgui.h>
#include <reshade/reshade.hpp>
#include <yaml-cpp/yaml.h>