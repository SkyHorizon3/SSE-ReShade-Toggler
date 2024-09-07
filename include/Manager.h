#pragma once
#include <glaze/glaze.hpp>


class Manager : public ISingleton<Manager>
{
	// class for main functions used for all features

public:
	void parseJSONPreset(const std::string& path);

	void serializeJSONPreset(const std::string& path);

	void toggleEffectMenu(const std::set<std::string>& openMenus);

	std::vector<std::string> EnumeratePresets();

	struct MenuToggleInformation
	{
		std::string effectName{};
		std::string menuName{};
		bool state = true;
	};

private:
	void toggleEffect(const char* technique, bool state) const;


	std::vector<MenuToggleInformation> m_menuToggleInfo = { {"hi","bye", true},{"one","two", false}, {"hamster","rabbit", false} };;

private:
	template<typename T>
	std::string serializeVector(const std::string& key, const std::vector<T>& vec);

	template<typename... Args>
	std::string serializeArbitraryVector(const Args&... args);

};

extern reshade::api::effect_runtime* s_pRuntime;