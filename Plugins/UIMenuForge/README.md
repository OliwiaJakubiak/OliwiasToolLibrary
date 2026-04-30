# UI Menu Forge 

> Part of [Oliwia's Tool Library](https://github.com/OliwiaJakubiak/OliwiasToolLibrary) | [Tool Library Site](http://oliwia-tool-library.webflow.io)

Generate pre-wired, production-ready UI screen templates with a central manager - ready to style and ship. 

---

## What it Does 

UI Menu Forge generates a set of pre-wired UMG screen templates: Main Menu, Pause Menu, HUD, Loading Screen, Credits, Death Screen or Custom, which are all managed by a central UI Manager subsystem. Select only the screens you need, generate them into your project, and get straight to styling rather than building UI infrastructure from scratch. 

---

## Key Features

- Generate any combination of: Main Menu, Pause Menu, Settings, HUD, Loading Screen, Credits, Death Screen, Custom
- Central UI Manager (GameInstanceSubsystem) handles all screen switching logic
- Pre-wried navigation between screens our of the box
- Fully customisable generated templates - they're yours to style
- No manual Project Settings configuraiton required 

---

## Requirements 

- Unreal Engine 5.7.4 or later
- C++ project with Visual Studio (or equivalent) installed
- If your project is Blueprint-only, convert it first via *Tools -> New C++ Class*

---

## Installation 

1. Download the plugin `.zip` from the [latest release](../../releases)
2. Extract the plugin folder into your project's `Plugins` directory
3. Right-click your `.uproject` and select **Generate Visual Studio project files**
4. Open the project and click **Yes** when prompted to build
5. Enable the plugin via *Edit -> Plugins* if not already active

---

## How To Use 

1. Open the tool and select which screen templates you need
2. Hit **Generate Selected Screens** to create them in your project
3. Open each generated Blueprint to add your own styling and layout
4. Use **Get Game Instance Subsystem** with **UIMenuForge Manager** to show and hide screens from your game logic
---

## More 

Full showcase and documentation at [oliwia-tool-library.webflow.io](http://oliwia-tool-library.webflow.io)
