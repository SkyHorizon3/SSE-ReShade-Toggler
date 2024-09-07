#include "Manager.h"
#include "glaze/glaze.hpp"

void Manager::parseJSONPreset(const std::string& path)
{

	std::ifstream openFile(path);
	if (!openFile.is_open())
	{
		SKSE::log::error("Couldn't load preset {}!", path);
		return;
	}

	std::stringstream buffer;
	buffer << openFile.rdbuf();

	glz::json_t json{};
	SKSE::log::info("Buffer: {}", buffer.str());
	std::ignore = glz::read_json(json, buffer.str());

	const std::string key = "Numbers";
	std::vector<int> vec;
	if (json.contains(key))
	{
		SKSE::log::info("Found");

		auto& jsonArray = json[key].get_array();
		vec.clear();
		vec.reserve(jsonArray.size());

		std::string newBuffer;
		std::ignore = glz::write_json(jsonArray, newBuffer); // maybe dont ignore all of them 

		SKSE::log::info("newBuffer: {}", newBuffer);
		std::ignore = glz::read_json(vec, newBuffer);

		for (const auto& member : vec)
		{
			SKSE::log::info("Number: {}", member);
		}

	}
	else
	{
		SKSE::log::error("Key '{}' not found in JSON.", key);
	}
}

void Manager::serializeJSONPreset(const std::string& presetName)
{
	constexpr const char* configDirectory = "Data\\SKSE\\Plugins\\ReShadeEffectTogglerPresets";
	std::filesystem::create_directories(configDirectory);
	const std::string fullPath = std::string(configDirectory) + "\\" + presetName + ".json";

	std::ofstream outFile(fullPath);
	if (!outFile.is_open())
	{
		SKSE::log::error("Couldn't save preset {}!", presetName);
		return;
	}

	std::vector<int> test = { 1,2,2,2,2,2,2 };
	std::string buffer = serializeArbitraryVector(
		std::make_pair(std::string("Menu"), m_menuToggleInfo),
		std::make_pair(std::string("Numbers"), test)
	);

	// TODO: investigate why no good looking json. This is kinda irrelevant tho
	//buffer = glz::prettify_json(buffer);
	outFile << buffer;
	outFile.close();
}

void Manager::toggleEffectMenu(const std::set<std::string>& openMenus)
{
	for (const auto& menuInfo : m_menuToggleInfo)
	{
		if (openMenus.find(menuInfo.menuName) != openMenus.end())
		{
			toggleEffect(menuInfo.effectName.c_str(), menuInfo.state);
		}
	}
}

std::vector<std::string> Manager::EnumeratePresets()
{
	std::vector<std::string> presets;
	constexpr auto presetDirectory = L"Data\\SKSE\\Plugins\\ReShadeEffectTogglerPresets";
	if (!std::filesystem::exists(presetDirectory))
		std::filesystem::create_directories(presetDirectory);

	for (const auto& preset : std::filesystem::recursive_directory_iterator(presetDirectory))
	{
		if (preset.is_regular_file() && preset.path().filename().extension() == ".json")
		{
			presets.push_back(preset.path().filename().string());
		}
	}

	std::sort(presets.begin(), presets.end());
	return presets;
}

void Manager::toggleEffect(const char* effect, const bool state) const
{
	s_pRuntime->enumerate_techniques(effect, [&state](reshade::api::effect_runtime* runtime, reshade::api::effect_technique technique)
		{
			runtime->set_technique_state(technique, state); // True = enabled; False = disabled
		});
}

template <typename T>
std::string Manager::serializeVector(const std::string& key, const std::vector<T>& vec)
{
	std::string vecJson;
	const auto result = glz::write_json(vec, vecJson);  // Serialize the vector
	if (result) {
		const std::string descriptive_error = glz::format_error(result, vecJson);
		SKSE::log::error("Error serializing vector: {}", descriptive_error);
	}

	const std::string jsonStr = "\"" + key + "\": " + vecJson;
	return jsonStr;
}

template <typename... Args>
std::string Manager::serializeArbitraryVector(const Args&... args)
{
	std::stringstream jsonStream;
	jsonStream << "{ ";

	bool first = true;
	((jsonStream << (first ? (first = false, "") : ", ") << serializeVector(args.first, args.second)), ...);


	jsonStream << " }";

	return jsonStream.str();
}