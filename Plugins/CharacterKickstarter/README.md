# Character Kickstarter 

> Part of [Oliwia's Tool Library](https://github.com/OliwiaJakubiak/OliwiasToolLibrary) | [Tool Library Site](http://oliwia-tool-library.webflow.io)

Generate a fully configured, play-ready character Blueprint with movement, camera and input pre-wired. 

---

## What it Does 

Character Kickstarter generates a fully configured character Blueprint based on your chosen template: First Person, Third Person, Top Down, Side Scroller or Blank. Movement, camera and input are pre-wired and ready to play. Optionally generate a matching Player Controller and Game Mode at the same time. 

---

## Key Features

- Five character templates: First Person, Third Person, Top Down, Side Scroller, Blank
- Movement, camera and input pre-wried out of the box
- Optional Player Controller and Game Mode generation
- Configurable output folder
- Open and adjust movement values and camera settings directly in the Details panel 

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

1. Choose a character template - First Person, Thirs Person, Top Down, Side Scroller or Blank
2. Optionally choose to also generate a Player Controller and Game Mode
3. Set an output folder or leave blank for the default
4. Hit **Generate** - open the Blueprint to adjust movement values and camera settings in the Details panel 
---

## More 

Full showcase and documentation at [oliwia-tool-library.webflow.io](http://oliwia-tool-library.webflow.io)
