#pragma once


namespace Colors
{
	namespace Theme
	{
		static auto accent = IM_COL32(236, 158, 36, 255);
		static auto highlight = IM_COL32(40, 44, 47, 56);
		static auto niceBlue = IM_COL32(83, 232, 254, 255);
		static auto compliment = IM_COL32(78, 151, 166, 255);
		static auto background = IM_COL32(19, 19, 19, 255);
		static auto backgroundDark = IM_COL32(30, 30, 30, 255);
		static auto titlebar = IM_COL32(20, 20, 20, 255);
		static auto titlebarEdit = IM_COL32(130, 90, 21, 95);
		static auto titlebarPlay = IM_COL32(91, 21, 130, 95);
		static auto titlebarPause = IM_COL32(130, 21, 21, 95);
		static auto propertyField = IM_COL32(5, 5, 5, 115);
		static auto text = IM_COL32(205, 205, 205, 255);
		static auto textBrighter = IM_COL32(210, 210, 210, 255);
		static auto textDarker = IM_COL32(128, 128, 128, 255);
		static auto textError = IM_COL32(230, 51, 51, 255);
		static auto muted = IM_COL32(77, 77, 77, 255);
		static auto groupHeader = IM_COL32(0, 0, 0, 115);
		static auto selection = IM_COL32(237, 192, 119, 255);
		static auto selectionMuted = IM_COL32(237, 201, 142, 23);
		static auto backgroundPopup = IM_COL32(19, 19, 19, 255);
		static auto validPrefab = IM_COL32(82, 179, 222, 255);
		static auto invalidPrefab = IM_COL32(222, 43, 43, 255);
		static auto missingMesh = IM_COL32(230, 102, 76, 255);
		static auto meshNotSet = IM_COL32(250, 101, 23, 255);
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