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

	const std::string key = "Menu";
	std::vector<MenuToggleInformation> vec;
	if (json.contains(key))
	{
		SKSE::log::info("Found");

		auto& jsonArray = json[key].get_array();
		//vec.clear();
		//vec.reserve(jsonArray.size());

		SKSE::log::info("Array size: {}", jsonArray.size());
		//SKSE::log::info("first: {}", jsonArray[0].as<std::string>());


		/*
		for (auto it = jsonArray.begin(); it != jsonArray.end();)
		{
			const auto& member = *it;

			SKSE::log::info("Member: {}", member.as<std::string>());
			/*
			MenuToggleInformation type;

			glz::read_json(type, member.as<std::string>());
			vec.emplace_back(type);

	}
	*/


	/*
	for (const auto& item : jsonArray) {
		typename std::decay_t<decltype(vec)>::value_type value;
		auto readResult = glz::read_json(value, item);
		if (!readResult) {
			const std::string descriptive_error = glz::format_error(readResult);
			SKSE::log::error("Error deserializing vector item for key '{}': {}", key, descriptive_error);
		}
		else {
			vec.emplace_back(value);
		}
	}
	*/

	/*
	auto& jsonArray = json.at(key).get_array();

	vec.clear();
	vec.reserve(jsonArray.size());

	auto arrayResult = glz::read_json(vec, jsonArray);
	if (!arrayResult) {
		const std::string error = glz::format_error(arrayResult);
		SKSE::log::error("Error parsing JSON array: {}", error);
	}
	*/
	}
	else {
		SKSE::log::error("Key '{}' not found in JSON.", key);
	}

	/*
	* The idea
	deserializeArbitraryVector(buffer.str(),
		std::make_pair("Numbers", m_menuToggleInfo),
		std::make_pair("Menu", m_numbers));
	*/
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