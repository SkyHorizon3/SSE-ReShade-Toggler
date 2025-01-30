# SSE-Reshade-Toggler 4.0

Welcome to **SSE-Reshade-Toggler 4.0**! This update brings some exciting new features, an overhauled UI, and a revamped internal structure, making toggling your ReShade effects easier, more intuitive, and way more powerful.

## Features

- **Menu-Based Toggling**: Automatically toggle ReShade effects when entering or exiting specific menus like the Map Menu, Main Menu, and more.
  
- **Time-Based Toggling**: Set user-defined time intervals to toggle ReShade effects on or off at specific moments.

- **Interior-Based Toggling**: Automatically toggle ReShade effects when entering interior cells (perfect for dungeons and houses).

- **Weather-Based Toggling**: Toggle ReShade effects based on weather conditions in the game. No more turning off effects during a storm!

- **Custom Configuration**: The new configuration system allows you to fine-tune toggling preferences directly through ReShade’s ImGui menu. Save and share your presets with ease!

## Requirements

- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/30379)
- [ReShade 6.1.1+ with Full Add-On Support](https://reshade.me/)
- [Po3 Tweaks](https://www.nexusmods.com/skyrimspecialedition/mods/51073)

## Configuration

### The New Configuration System

Version 4.0 introduces a much easier and cleaner way to manage your settings: **JSON configuration files**. No more direct file editing or messy INI tweaks. You can configure the toggling behavior through the ReShade overlay’s menu.

To access the configuration:

1. Open the **ReShade overlay** in-game.
2. Navigate to the **Add-ons** tab.
3. Here you can toggle your preferences. By default, the system disables effects on the Map Menu, Main Menu, and Loading Menu.

### JSON Configuration

All configuration is done automatically and stored in **JSON format**. This format is much more user-friendly and can be easily modified, but don’t worry—you generally won’t need to edit it directly. Simply use the built-in menu to configure everything!

Your saved presets can be found in the following directory:

`Data\SKSE\Plugins\TogglerConfigs\PresetName.json`

You can create and load presets through the menu for quick and easy access.

## Compatibility

- **Works with**: Skyrim SE, AE, and VR.
- **Compatible with**: Everything that’s compatible with ReShade.
- **Not compatible with**: Skyrim-Upscaler-ENB-Test-Build by PureDark.

## Release Notes (v4.0)

### Major Changes

1. **Complete UI Overhaul**  
   The UI has been completely redesigned to be more intuitive and modern, making configuration easier and less cumbersome.

2. **Toggle by Records**  
   Toggling functionality now operates based on **weather records** instead of flags. This shift allows for more precise and accurate toggling based on individual weathers.

3. **Papyrus Integration**  
   We’ve added support for **Papyrus scripting**, allowing for dynamic behavior and compatibility with other mods that utilize Papyrus scripts.

4. **Adjustable Shader Values**  
   Users now have the ability to tweak individual shader values for more control over visual effects.

5. **Complete Internal Rewrite**  
   The internal code has been entirely rewritten to enhance performance, maintainability, and future-proofing. The new toggle-by-record system was introduced to replace the older, less flexible method.

## Credits

- **[crosire](https://www.patreon.com/bePatron?u=23312785&redirect_uri=https%3A%2F%2Freshade.me%2F)** for ReShade
- **[doodlum](https://www.nexusmods.com/skyrimspecialedition/users/28038035)** and **[PureDark](https://github.com/PureDark)** for skyrim-reshade-helper
- **[Skyrim Scripting](https://www.youtube.com/c/SkyrimScripting)** for providing essential tutorials and code snippets.
