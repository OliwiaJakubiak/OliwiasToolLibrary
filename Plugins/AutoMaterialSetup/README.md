# Game Feel Kit

> Part of [Oliwia's Tool Library](https://github.com/OliwiaJakubiak/OliwiasToolLibrary) | [Tool Library Site](http://oliwia-tool-library.webflow.io)

A component-based toolkit for adding juice, impact and polish to any Actor with one-click effect triggers.

---

## What it Does 

Game Feel Kit is a modular, component-based plugin that lets you attach polished feedback effects to any Actor in your scene: screen shake, time dilation, hit stop, visual and audio triggers. Effects are configured via Data Assets and triggered from your own Blueprint logic or game events, with an in-editor preview buttons so you can test without entering Play mode. 

---

## Key Features

- Component-based - attach to any Actor with no project restructuring
- Data Asset driven - configure effect parameters cleanly per slot
- One-click effect triggers callable from Blueprint or C++
- In-editor EUW preview buttons for testing without Play mode
- Modular - use only the effects you need 

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

1. Add the **Game Feel Component** to any Actor in your scene
2. Assign Data Assets to each effect slot in the Details panel (or use the defaults) 
3. Call the trigger functions from your own Blueprint input or game logic
4. Use the EUW preview buttons to test effects without entering Play mode 

---

## More 

Full showcase and documentation at [oliwia-tool-library.webflow.io](http://oliwia-tool-library.webflow.io)
