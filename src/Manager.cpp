#include "Manager.h"
#include "Utils.h"
#include "glaze/glaze.hpp"

bool Manager::parseJSONPreset(const std::string& presetName)
{
	const std::string fullPath = getPresetPath(presetName);

	std::ifstream openFile(fullPath);
	if (!openFile.is_open())
	{
		SKSE::log::error("Couldn't load preset {}!", fullPath);
		return false;
	}

	std::stringstream buffer;
	buffer << openFile.rdbuf();

	const auto menuPair = std::make_pair("Menu", std::ref(m_menuToggleInfo));
	const auto timePair = std::make_pair("Time", std::ref(m_timeToggleInfo));
	const auto weatherPair = std::make_pair("Weather", std::ref(m_weatherToggleInfo));

	return deserializeArbitraryData(buffer.str(), menuPair, timePair, weatherPair);
}

bool Manager::serializeJSONPreset(const std::string& presetName)
{
	const std::string fullPath = getPresetPath(presetName);

	std::ofstream outFile(fullPath);
	if (!outFile.is_open())
	{
		SKSE::log::error("Couldn't save preset {}!", presetName);
		return false;
	}

	std::string buffer;
	if (!serializeArbitraryData(buffer, 
		std::make_pair("Menu", m_menuToggleInfo), 
		std::make_pair("Time", m_timeToggleInfo),
		std::make_pair("Weather", m_weatherToggleInfo)))
	{
		SKSE::log::error("Failed to serialize preset {}!", presetName);
		return false;
	}

	// TODO: prettify
	outFile << buffer;
	outFile.close();

	return true;
}

void Manager::toggleEffectMenu(const std::set<std::string>& openMenus)
{
	for (const auto& menuInfo : m_menuToggleInfo)
	{
		if (openMenus.find(menuInfo.menuName) != openMenus.end())
		{
			toggleEffect(menuInfo.effectName.c_str(), menuInfo.state);
		}
		else
		{
			toggleEffect(menuInfo.effectName.c_str(), !menuInfo.state);
		}
	}
}

std::vector<std::string> Manager::enumeratePresets()
{
	std::vector<std::string> presets;
	constexpr auto presetDirectory = L"Data\\SKSE\\Plugins\\ReShadeEffectTogglerPresets";
	if (!std::filesystem::exists(presetDirectory))
		std::filesystem::create_directories(presetDirectory);

	for (const auto& preset : std::filesystem::recursive_directory_iterator(presetDirectory))
	{
		if (preset.is_regular_file() && preset.path().filename().extension() == ".json")
		{
			presets.emplace_back(preset.path().filename().string());
		}
	}

	std::sort(presets.begin(), presets.end());
	return presets;
}

std::vector<std::string> Manager::enumerateEffects()
{
	constexpr const char* shadersDirectory = "reshade-shaders\\Shaders";
	std::vector<std::string> effects;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(shadersDirectory))
	{
		if (entry.is_regular_file() && entry.path().filename().extension() == ".fx")
		{
			effects.emplace_back(entry.path().filename().string());
		}
	}
	//sort files
	std::sort(effects.begin(), effects.end());
	return effects;
}

std::vector<std::string> Manager::enumerateMenus()
{
	const auto ui = RE::UI::GetSingleton();
	const auto& menuMap = ui->menuMap;
	std::vector<std::string> menuNames;

	for (const auto& menu : menuMap)
	{
		const auto& menuName = menu.first;
		menuNames.emplace_back(std::string(menuName));
	}
	std::sort(menuNames.begin(), menuNames.end());
	return menuNames;
}

std::vector<std::string> Manager::enumerateWorldSpaces()
{
	const auto& ws = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESWorldSpace>();
	std::vector<std::string> worldSpaces;
	for (const auto& space : ws)
	{
		const std::string s = std::to_string(Utils::getTrimmedFormID(space)) + "~" + Utils::getModName(space);
		worldSpaces.emplace_back(s);
	}

	return worldSpaces;
}

std::string Manager::getPresetPath(const std::string& presetName)
{
	constexpr const char* configDirectory = "Data\\SKSE\\Plugins\\ReShadeEffectTogglerPresets";

	if (!std::filesystem::exists(configDirectory))
		std::filesystem::create_directories(configDirectory);

	return std::string(configDirectory) + "\\" + presetName;
}

void Manager::toggleEffectWeather()
{
	const auto sky = RE::Sky::GetSingleton();
	const auto player = RE::PlayerCharacter::GetSingleton();
	const auto ui = RE::UI::GetSingleton();

	if (m_weatherToggleInfo.empty() || !player || !sky || !sky->currentWeather || !ui || ui->GameIsPaused())
		return;

	const auto flags = sky->currentWeather->data.flags;
	std::string weatherFlag{};

	switch (flags.get())
	{
	case RE::TESWeather::WeatherDataFlag::kNone:
		weatherFlag = "kNone";
		break;
	case RE::TESWeather::WeatherDataFlag::kRainy:
		weatherFlag = "kRainy";
		break;
	case RE::TESWeather::WeatherDataFlag::kPleasant:
		weatherFlag = "kPleasant";
		break;
	case RE::TESWeather::WeatherDataFlag::kCloudy:
		weatherFlag = "kCloudy";
		break;
	case RE::TESWeather::WeatherDataFlag::kSnow:
		weatherFlag = "kSnow";
		break;
	case RE::TESWeather::WeatherDataFlag::kPermAurora:
		weatherFlag = "kPermAurora";
		break;
	case RE::TESWeather::WeatherDataFlag::kAuroraFollowsSun:
		weatherFlag = "kAuroraFollowsSun";
		break;
	}

	const auto ws = player->GetWorldspace();
	if (!ws || m_lastWs.first && m_lastWs.first->formID != ws->formID) // player is in interior or changed worldspace
	{
		if (m_lastWs.first)
		{
			for (const auto& info : m_lastWs.second)
			{
				if (info.weatherFlag != weatherFlag) // change effect state back to original if it was toggled before
				{
					toggleEffect(info.effectName.c_str(), !info.state);
				}
			}
			m_lastWs.first = nullptr;
			m_lastWs.second.clear();
		}
		return;
	}

	const auto it = m_weatherToggleInfo.find(std::to_string(Utils::getTrimmedFormID(ws)) + "~" + Utils::getModName(ws));
	if (it == m_weatherToggleInfo.end()) // no info for ws in unordered map
		return;

	const auto& weatherInfo = it->second;
	for (const auto& info : weatherInfo)
	{
		if (info.weatherFlag == weatherFlag)
		{
			toggleEffect(info.effectName.c_str(), info.state);
			m_lastWs.first = ws;
			m_lastWs.second.emplace_back(info);
		}
		else
		{
			toggleEffect(info.effectName.c_str(), !info.state);
		}
	}
}

float Manager::getCurrentGameTime()
{
	using func_t = decltype(&Manager::getCurrentGameTime);
	static REL::Relocation<func_t> func{ REL::VariantID(56475, 56832, 0x9F3290) };
	return func();
}

void Manager::toggleEffect(const char* effect, const bool state) const
{
	s_pRuntime->enumerate_techniques(effect, [&state](reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique)
		{
			runtime->set_technique_state(technique, state); // True = enabled; False = disabled
		});
}

#pragma region TemplateTomfoolery
template <typename T>
bool Manager::serializeVector(const std::string& key, const std::vector<T>& vec, std::string& output)
{
	std::string vecJson;
	const auto result = glz::write_json(vec, vecJson);  // Serialize the vector
	if (result) {
		const std::string descriptive_error = glz::format_error(result, vecJson);
		SKSE::log::error("Error serializing vector: {}", descriptive_error);
		return false;
	}

	output = "\"" + key + "\": " + vecJson;
	return true;
}

template<typename T>
bool Manager::serializeMap(const std::string& key, const std::unordered_map<std::string, std::vector<T>>& map, std::string& output)
{
	std::stringstream mapJson;
	mapJson << "{ ";

	bool first = true;
	for (const auto& pair : map) {
		std::string vecJson;
		const auto result = glz::write_json(pair.second, vecJson);
		if (result) {
			const std::string descriptive_error = glz::format_error(result, vecJson);
			SKSE::log::error("Error serializing vector for key {}: {}", pair.first, descriptive_error);
			return false;
		}

		mapJson << (first ? (first = false, "") : ", ") << "\"" << pair.first << "\": " << vecJson;
	}

	mapJson << " }";
	output = "\"" + key + "\": " + mapJson.str();
	return true;
}

template <typename... Args>
bool Manager::serializeArbitraryData(std::string& output, const Args&... args)
{
	std::stringstream jsonStream;
	jsonStream << "{ ";

	bool first = true;
	bool success = true;

	auto processArg = [&](const auto& pair) {
		std::string serialized;
		if constexpr (std::is_same_v<std::decay_t<decltype(pair.second)>, std::vector<typename std::decay_t<decltype(pair.second)>::value_type>>) {
			// Handle vector case
			if (serializeVector(pair.first, pair.second, serialized)) {
				jsonStream << (first ? (first = false, "") : ", ") << serialized;
			}
			else {
				success = false;
			}
		}
		else if constexpr (std::is_same_v<std::decay_t<decltype(pair.second)>, std::unordered_map<std::string, std::vector<typename std::decay_t<decltype(pair.second)>::mapped_type::value_type>>>) {
			// Handle unordered_map case
			if (serializeMap(pair.first, pair.second, serialized)) {
				jsonStream << (first ? (first = false, "") : ", ") << serialized;
			}
			else {
				success = false;
			}
		}
		else {
			SKSE::log::error("Unsupported type for serialization");
			success = false;
		}
		};

	// Process each argument pair
	(processArg(args), ...);

	jsonStream << " }";
	output = jsonStream.str();

	return success;
}

template <typename T>
bool Manager::deserializeVector(const std::string& key, const glz::json_t& json, std::vector<T>& vec)
{
	if (json.contains(key)) {
		std::string buffer;
		const auto write_result = glz::write_json(json[key], buffer);
		if (write_result) {
			SKSE::log::error("Error serializing JSON for key '{}'.", key);
			return false;
		}

		const auto read_result = glz::read_json(vec, buffer);
		if (read_result) {
			SKSE::log::error("Error deserializing vector for key '{}'.", key);
			return false;
		}
	}
	else {
		SKSE::log::error("Key '{}' not found in JSON.", key);
		return false;
	}
	return true;
}

template <typename T>
bool Manager::deserializeMapOfVectors(const std::string& key, const glz::json_t& json, std::unordered_map<std::string, std::vector<T>>& map)
{
	if (json.contains(key)) {
		const auto& mapData = json[key].get_object();
		map.clear();
		for (const auto& [subKey, subValue] : mapData) {
			std::vector<T> vec;

			// Serialize the subValue to a string and then deserialize into the vector
			std::string buffer;
			const auto write_result = glz::write_json(subValue, buffer);
			if (write_result) {
				SKSE::log::error("Error serializing JSON for subKey '{}'.", subKey);
				return false;
			}

			const auto read_result = glz::read_json(vec, buffer);
			if (read_result) {
				SKSE::log::error("Error deserializing vector for subKey '{}'.", subKey);
				return false;
			}
			map[subKey] = std::move(vec);
		}
	}
	else {
		SKSE::log::error("Key '{}' not found in JSON.", key);
		return false;
	}
	return true;
}

template <typename... Args>
bool Manager::deserializeArbitraryData(const std::string& buf, Args&... args)
{
	glz::json_t json{};
	const auto result = glz::read_json(json, buf);
	if (result) {
		const std::string descriptive_error = glz::format_error(result, buf);
		SKSE::log::error("Error parsing JSON: {}", descriptive_error);
		return false;
	}
	bool success = true;

	auto process_pair = [&](auto& pair) {
		const auto& key = pair.first;
		auto& data = pair.second;

		if constexpr (std::is_same_v<std::decay_t<decltype(data)>, std::vector<typename std::decay_t<decltype(data)>::value_type>>) {
			return deserializeVector(key, json, data);
		}
		else if constexpr (std::is_same_v<std::decay_t<decltype(pair.second)>, std::unordered_map<std::string, std::vector<typename std::decay_t<decltype(pair.second)>::mapped_type::value_type>>>) {
			return deserializeMapOfVectors(key, json, pair.second);
		}
		else
		{
			return false;
		}
	};

	((success &= process_pair(args)), ...);
	return success;
}

#pragma endregion