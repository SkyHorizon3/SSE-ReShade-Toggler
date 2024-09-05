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


	std::unordered_map<std::string, std::vector<MenuToggleInformation>> menuData;
	menuData.emplace("Menu", m_menuToggleInfo);
	std::string buffer = glz::write_json(menuData).value_or("error");

	std::unordered_map<std::string, std::vector<int>> testData;
	testData.emplace("Numbers", test);

	buffer += glz::write_json(testData).value_or("error");

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

