#pragma once

class MenuManager
{
public:
	bool CreateCombo(const char* label, std::string& currentItem, std::vector<std::string>& items, ImGuiComboFlags_ flags);
	bool CreateTreeNode(const char* label, std::vector<std::string>& selectedItems, std::vector<std::string>& items, char* searchBuffer, size_t bufferSize, bool entireReShadeToggleOn);

	ImVec2 GetNativeViewportSizeScaled(float scale);

	void SetColors();
	void RemoveColors();
};