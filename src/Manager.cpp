#include "Manager.h"
#include "glaze/glaze.hpp"

void Manager::ParseJSONPreset(const std::string& path)
{


}

void Manager::SerializeJSONPreset(const std::string& presetName)
{
	constexpr const char* configDirectory = "Data\\SKSE\\Plugins\\ReShadeEffectTogglerPresets";
	std::filesystem::create_directories(configDirectory);
	const std::string fullPath = std::string(configDirectory) + "\\" + presetName + ".json";

	std::ofstream outFile(fullPath);
	if (!outFile.is_open())
	{
		SKSE::log::info("Couldn't save preset!");
		return;
	}

	std::vector<int> test = { 1,2,2,2,2,2,2 };
	std::string buffer = serializeArbitraryVector(
		std::make_pair(std::string("Menu"), m_menuToggleInfo),
		std::make_pair(std::string("Numbers"), test)
	);

	// TODO: investigate why no good looking json. This is kinda irrelevant tho
	// std::string beautifulJson = glz::prettify_json(buffer);
	outFile << buffer;
	outFile.close();
}

void Manager::ToggleEffectMenu(const std::set<std::string>& openMenus)
{
	for (const auto& menuInfo : m_menuToggleInfo)
	{
		if (openMenus.find(menuInfo.menuName) != openMenus.end())
		{
			ToggleEffect(menuInfo.effectName.c_str(), menuInfo.state);
		}
	}
}

void Manager::ToggleEffect(const char* effect, const bool state) const
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
	auto result = glz::write_json(vec, vecJson);  // Serialize the vector
	if (result) {
		std::string descriptive_error = glz::format_error(result, vecJson);
		SKSE::log::error("Error serializing vector: {}", descriptive_error);
	}

	std::string jsonStr = "\"" + key + "\": " + vecJson;
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