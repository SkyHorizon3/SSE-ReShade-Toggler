#pragma once


class Manager : public ISingleton<Manager>
{
	// class for main functions used for all features

public:
	void ParseJSONPreset(const std::string& path);

	void SerializeJSONPreset(const std::string& path);

	void ToggleEffectMenu(const std::set<std::string>& openMenus);


private:
	void ToggleEffect(const char* technique, bool state) const;

	struct MenuToggleInformation
	{
		std::string effectName{};
		std::string menuName{};
		bool state = true;
	};

	std::vector<MenuToggleInformation> m_menuToggleInfo = { {"hi","bye", true},{"one","two", false}, {"hamster","rabbit", false} };
};

extern reshade::api::effect_runtime* s_pRuntime;