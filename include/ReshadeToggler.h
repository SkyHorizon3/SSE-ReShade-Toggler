#pragma once
#include "Globals.h"
#include <string>
#include <vector>
#include <SimpleIni.h>

class ReshadeToggler
{
public:
    void Setup();
    void SetupLog();
    void Load();
    void LoadINI();

private:
    std::vector<std::string> m_SpecificMenu;
    std::vector<std::string> m_SpecificTime;
    std::vector<std::string> m_INImenus;
};
