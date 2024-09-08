#pragma once
#include <glaze/glaze.hpp>


struct MenuToggleInformation
{
	std::string effectName{};
	std::string menuName{};
	bool state = true;
};

class Manager : public ISingleton<Manager>
{
	// class for main functions used for all features

public:
	void parseJSONPreset(const std::string& presetName);

	void serializeJSONPreset(const std::string& presetName);

	void toggleEffectMenu(const std::set<std::string>& openMenus);

	std::vector<std::string> enumeratePresets();
	std::vector<std::string> enumerateEffects();
	std::vector<std::string> enumerateMenus();

	std::vector<MenuToggleInformation> getMenuToggleInfo() { return m_menuToggleInfo; }
	void setMenuToggleInfo(const std::vector<MenuToggleInformation>& info) { m_menuToggleInfo = info; }

private:
	std::string getPresetPath(std::string presetName);

	void toggleEffect(const char* technique, bool state) const;

	std::vector<MenuToggleInformation> m_menuToggleInfo; // = { {"hi","bye", true},{"one","two", false}, {"hamster","rabbit", false} };
	std::vector<int> m_test; //= { 1,2,2,2,2,2,2 };

private:
	template<typename T>
	std::string serializeVector(const std::string& key, const std::vector<T>& vec);

	template<typename... Args>
	std::string serializeArbitraryVector(const Args&... args);

	template<typename... Args>
	void deserializeArbitraryVector(const std::string& buf, Args&... args);

};

extern reshade::api::effect_runtime* s_pRuntime;