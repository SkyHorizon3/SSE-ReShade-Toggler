
# Description
With some reshade presets, some menus like the map are too bright or it just doesn't look good when enabled in certain menus. This brought up the idea for this SKSE plugin. It toggles reshade off/on if you enter/leave the menus that are written down in the INI file. By default, the reshade effects are disabled in the main menu, loading screen, and map. Which and how many menus are affected can be configured as desired in the ReShadeToggler.ini.

Now it's also possible to toggle (all) effects during user-defined time intervals.

# Features
Menu-Based Toggling: Toggle ReShade effects when entering or leaving specific menus.     

﻿﻿Time-Based Toggling: Enable or disable ReShade effects during user-defined time intervals.
  
Custom Configuration: Fine-tune your ReShade toggling preferences via the INI file.


# Requirements
Address Library for SKSE Plugins

SKSE﻿

ReShade 5.9+ with full add-on support﻿﻿

# Configuration

[General]

EnableMenus: Set to true to enable menu-based toggling, false to disable.

EnableTime: Set to true to enable time-based toggling, false to disable.

[MenusGeneral]

MenuToggleOption: Choose 'Specific' to toggle specific effects, or 'All' to toggle all effects.

MenuToggleAllState: If 'All' is chosen, set to 'on' to enable all effects, 'off' to disable.

MenuToggleSpecificStateX: Set state ('on' or 'off') for specific effects, where X is the effect number.

MenuToggleSpecificFileX: Specify the full effect file name for specific effects.

[MenusProcess]

MenuX: List of menus to trigger effect toggling.

[Time]

TimeUpdateInterval: Set time-based toggling interval in seconds.

TimeToggleOption: Choose 'Specific' or 'All' for time-based toggling.

TimeToggleAllState: If 'All' is chosen, set to 'on' to enable all effects, 'off' to disable.

TimeToggleSpecificTimeStartX: Specify start time (e.g., 8.00) for specific effects.

TimeToggleSpecificTimeStopX: Specify stop time for specific effects.

TimeToggleSpecificStateX: Set state ('on' or 'off') for specific time-based effects.

TimeToggleSpecificFileX: Specify the full effect file name for time-based effects.

# Compatibility
Compatible with everything thats compatible with Reshade.
Works on SE and AE. No idea about VR, but it should work.

# Credits
crosire﻿ for Reshade

doodlum﻿ and PureDark﻿ for skyrim-reshade-helper

Skyrim Scripting for providing skse tutorials and code snippets
