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

void Manager::toggleEffectMenu(const std::unordered_set<std::string>& openMenus)
{
	for (auto& menuInfo : m_menuToggleInfo)
	{
		if (openMenus.find(menuInfo.menuName) != openMenus.end())
		{
			if (!menuInfo.isToggled)
			{
				toggleEffect(menuInfo.effectName.c_str(), menuInfo.state);
				menuInfo.isToggled = true;
			}
		}
		else if (menuInfo.isToggled)
		{
			toggleEffect(menuInfo.effectName.c_str(), !menuInfo.state);
			menuInfo.isToggled = false;
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
	std::vector<std::string> effects;

	s_pRuntime->enumerate_techniques(nullptr, [&](reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique)
		{
			char nameBuffer[30];

			runtime->get_technique_effect_name(technique, nameBuffer);

			if (std::find(effects.begin(), effects.end(), nameBuffer) == effects.end())
			{
				effects.emplace_back(nameBuffer);
			}

		});

	std::sort(effects.begin(), effects.end());
	return effects;
}

std::vector<std::string> Manager::enumerateMenus()
{
	const auto ui = RE::UI::GetSingleton();
	std::vector<std::string> menuNames;

	for (const auto& menu : ui->menuMap)
	{
		menuNames.emplace_back(std::string(menu.first));
	}
	std::sort(menuNames.begin(), menuNames.end());
	return menuNames;
}

std::vector<std::string> Manager::enumerateWorldSpaces()
{
	const auto& ws = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESWorldSpace>();
	std::vector<std::string> worldSpaces;

	if (ws.empty())
		return worldSpaces;

	for (const auto& space : ws)
	{
		const std::string s = std::format("{:08X}~{}", Utils::getTrimmedFormID(space), Utils::getModName(space));
		worldSpaces.emplace_back(s);
	}

	return worldSpaces;
}

// TODO: Maybe make this also account for exterior cells, returning a pair of interior and exterior cells?
std::vector<std::string> Manager::enumerateInteriorCells()
{
	const auto& cells = RE::TESDataHandler::GetSingleton()->interiorCells;

	std::vector<std::string> interiorCells;
	interiorCells.reserve(cells.size());

	for (const auto& cell : cells)
	{
		if (cell)
		{
			const auto& fullname = (cell->GetFullNameLength() > 0) ? cell->GetFullName() : cell->GetFormEditorID();

			interiorCells.emplace_back(std::format("{}~{}", fullname, Utils::getModName(cell)));
		}
	}

	return interiorCells;
}


std::string Manager::getPresetPath(const std::string& presetName)
{
	constexpr const char* configDirectory = "Data\\SKSE\\Plugins\\ReShadeEffectTogglerPresets";

	if (!std::filesystem::exists(configDirectory))
		std::filesystem::create_directories(configDirectory);

	return std::string(configDirectory) + "\\" + presetName;
}

bool Manager::allowtoggleEffectWeather(const WeatherToggleInformation& cachedweather, const std::unordered_map<std::string, std::vector<WeatherToggleInformation>>::iterator& it) const
{
	if (it == m_weatherToggleInfo.end())
		return true;

	for (const auto& newInfo : it->second)
	{
		if (cachedweather.effectName == newInfo.effectName &&
			cachedweather.state == newInfo.state &&
			cachedweather.weatherFlag == newInfo.weatherFlag)
		{
			return false;
		}
	}

	return true;
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
	const auto it = m_weatherToggleInfo.find(std::format("{:08X}~{}", Utils::getTrimmedFormID(ws), Utils::getModName(ws)));
	const auto cachedWorldspace = m_lastWs.first;

	if (!ws || cachedWorldspace && cachedWorldspace->formID != ws->formID) // player is in interior or changed worldspace
	{
		if (cachedWorldspace)
		{
			for (const auto& info : m_lastWs.second)
			{
				if (!ws || allowtoggleEffectWeather(info, it)) // change effect state back to original if it was toggled before
				{
					toggleEffect(info.effectName.c_str(), !info.state);
				}
			}
			m_lastWs.first = nullptr;
			m_lastWs.second.clear();
		}
		return;
	}

	if (it == m_weatherToggleInfo.end()) // no info for ws in unordered map
		return;

	for (auto& info : it->second)
	{
		if (info.weatherFlag == weatherFlag)
		{
			toggleEffect(info.effectName.c_str(), info.state);
			info.isToggled = true;
			m_lastWs.first = ws;
			m_lastWs.second.emplace_back(info);
		}
		else if (info.isToggled)
		{
			toggleEffect(info.effectName.c_str(), !info.state);
			info.isToggled = false;
		}
	}
}

void Manager::toggleEffectTime()
{
	const auto ui = RE::UI::GetSingleton();
	const auto calendar = RE::Calendar::GetSingleton();
	if (m_timeToggleInfo.empty() || !calendar || !ui || ui->GameIsPaused())
		return;

	for (auto& timeInfo : m_timeToggleInfo)
	{
		const std::uint32_t currentHour = static_cast<std::uint32_t>(calendar->GetHour());
		const float currentTime = currentHour + (calendar->GetMinutes() / 100.f);

		const bool inRange = timeWithinRange(currentTime, timeInfo.startTime, timeInfo.stopTime);

		if (inRange)
		{
			if (!timeInfo.isToggled)
			{
				toggleEffect(timeInfo.effectName.c_str(), timeInfo.state);
				timeInfo.isToggled = true;
			}
		}
		else if (!inRange && timeInfo.isToggled)
		{
			toggleEffect(timeInfo.effectName.c_str(), !timeInfo.state);
			timeInfo.isToggled = false;
		}
	}

}

bool Manager::timeWithinRange(const float& currentTime, const float& startTime, const float& stopTime) const
{
	return currentTime >= startTime && currentTime <= stopTime;
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
		if constexpr (std::is_same_v<std::decay_t<decltype(pair.second)>, std::vector<typename std::decay_t<decltype(pair.second)>::value_type>>) {
			return deserializeVector(pair.first, json, pair.second);
		}
		else if constexpr (std::is_same_v<std::decay_t<decltype(pair.second)>, std::unordered_map<std::string, std::vector<typename std::decay_t<decltype(pair.second)>::mapped_type::value_type>>>) {
			return deserializeMapOfVectors(pair.first, json, pair.second);
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