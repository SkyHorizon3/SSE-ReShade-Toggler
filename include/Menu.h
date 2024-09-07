#pragma once


namespace Colors
{
	namespace Theme
	{
		constexpr auto accent = IM_COL32(236, 158, 36, 255);
		constexpr auto highlight = IM_COL32(39, 185, 242, 255);
		constexpr auto niceBlue = IM_COL32(83, 232, 254, 255);
		constexpr auto compliment = IM_COL32(78, 151, 166, 255);
		constexpr auto background = IM_COL32(36, 36, 36, 255);
		constexpr auto backgroundDark = IM_COL32(26, 26, 26, 255);
		constexpr auto titlebar = IM_COL32(21, 21, 21, 255);
		constexpr auto propertyField = IM_COL32(15, 15, 15, 255);
		constexpr auto text = IM_COL32(192, 192, 192, 255);
		constexpr auto textBrighter = IM_COL32(210, 210, 210, 255);
		constexpr auto textDarker = IM_COL32(128, 128, 128, 255);
		constexpr auto textError = IM_COL32(230, 51, 51, 255);
		constexpr auto muted = IM_COL32(77, 77, 77, 255);
		constexpr auto groupHeader = IM_COL32(47, 47, 47, 255);
		constexpr auto selection = IM_COL32(237, 192, 119, 255);
		constexpr auto selectionMuted = IM_COL32(237, 201, 142, 23);
		constexpr auto backgroundPopup = IM_COL32(50, 50, 50, 255);
		constexpr auto validPrefab = IM_COL32(82, 179, 222, 255);
		constexpr auto invalidPrefab = IM_COL32(222, 43, 43, 255);
		constexpr auto missingMesh = IM_COL32(230, 102, 76, 255);
		constexpr auto meshNotSet = IM_COL32(250, 101, 23, 255);
	} // namespace Theme
} // namespace Colors


class Menu : public ISingleton<Menu>
{
public:
	void SettingsMenu();

private:
	void SpawnMainPage(ImGuiID dockspaceId);
	void SpawnMenuSettings(ImGuiID dockspaceId);
	void SpawnTimeSettings(ImGuiID dockspaceId);
	void SpawnInteriorSettings(ImGuiID dockspaceId);
	void SpawnWeatherSettings(ImGuiID dockspaceId);
	bool CreateCombo(const char* label, std::string& currentItem, std::vector<std::string>& items, ImGuiComboFlags_ flags);
private:
	void SaveFile();

	void SetColors();
	void RemoveColors();
private:

	char m_inputBuffer[256] = { 0 };
	std::string m_selectedPreset;
	std::vector<std::string> m_presets;

	bool m_saveConfigPopupOpen = false;
	bool m_openSettingsMenu = false;
	bool m_showMenuSettings = false;
	bool m_showTimeSettings = false;
	bool m_showInteriorSettings = false;
	bool m_showWeatherSettings = false;
};