#pragma once
#include "Globals.h"
#include <string>
#include <vector>
#include <SimpleIni.h>

class ReshadeToggler
{
public:

    static ReshadeToggler GetSingleton()
    {
        ReshadeToggler toggler;
        return toggler;
    }

    void Setup();
    void SetupLog();
    void Load();
    void LoadINI();
};
