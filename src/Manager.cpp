#include "Manager.h"
#include "glaze/glaze.hpp"

void Manager::parseJSONPreset(const std::string& presetName)
{
	const std::string fullPath = getPresetPath(presetName);

	std::ifstream openFile(fullPath);
	if (!openFile.is_open())
	{
		SKSE::log::error("Couldn't load preset {}!", fullPath);
		return;
	}

	std::stringstream buffer;
	buffer << openFile.rdbuf();

	const auto menuPair = std::make_pair("Menu", std::ref(m_menuToggleInfo));
	const auto numbersPair = std::make_pair("Numbers", std::ref(m_test));

	deserializeArbitraryVector(buffer.str(),
		menuPair,
		numbersPair
	);

	SKSE::log::info("Menu:");
	for (const auto& member : m_menuToggleInfo)
	{
		SKSE::log::info("\t{}, {}, {}", member.effectName, member.menuName, member.state);
	}

	SKSE::log::info("Numbers:");
	for (const auto& member : m_test)
	{
		SKSE::log::info("\t{}", member);
	}
}

void Manager::serializeJSONPreset(const std::string& presetName)
{
	const std::string fullPath = getPresetPath(presetName);

	std::ofstream outFile(fullPath);
	if (!outFile.is_open())
	{
		SKSE::log::error("Couldn't save preset {}!", presetName);
		return;
	}

	std::string buffer = serializeArbitraryVector(
		std::make_pair(std::string("Menu"), m_menuToggleInfo),
		std::make_pair(std::string("Numbers"), m_test)
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
			presets.emplace_back(preset.path().filename().string());
		}
	}

	std::sort(presets.begin(), presets.end());
	return presets;
}

std::string Manager::getPresetPath(std::string presetName)
{
	constexpr const char* configDirectory = "Data\\SKSE\\Plugins\\ReShadeEffectTogglerPresets";

	if (!std::filesystem::exists(configDirectory))
		std::filesystem::create_directories(configDirectory);

	return std::string(configDirectory) + "\\" + presetName;
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

template<typename ...Args>
void Manager::deserializeArbitraryVector(const std::string& buf, Args& ...args)
{
	glz::json_t json{};
	std::ignore = glz::read_json(json, buf);

	auto process_pair = [&](const auto& pair) {
		const auto& key = pair.first;
		auto& vec = pair.second;

		if (json.contains(key))
		{
			//SKSE::log::info("Found");

			auto& jsonArray = json[key].get_array();
			vec.clear();
			vec.reserve(jsonArray.size());

			// Serialize the array to a string and then deserialize into the vector
			std::string newBuffer;
			std::ignore = glz::write_json(jsonArray, newBuffer);
			std::ignore = glz::read_json(vec, newBuffer);
		}
		else
		{
			SKSE::log::error("Key '{}' not found in JSON.", key);
		}
		};

	// Apply the lambda function to each argument pair
	(process_pair(args), ...);
}