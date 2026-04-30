# Auto Material Setup 

> Part of [Oliwia's Tool Library](https://github.com/OliwiaJakubiak/OliwiasToolLibrary) | [Tool Library Site](http://oliwia-tool-library.webflow.io)

Automatically detect, wire and assign a full PBR material from a texture folder in a single click. 

---

## What it Does 

Auto Material Setup scans a selected texture folder, automatically detects PBR texture maps based on configurable naming convention suffixes, generates a fully wired material , and optionally assigns it directly to a selected Static Mesh - all without leaving the editor.

---

## Key Features

- Automatic PBR texture detection via configurable naming convention suffixes
- Full material generation with correctly wired texture inputs
- Direct mesh assignment from within the tool
- Configurable output folder
- Material creation handled in C++ via UMaterialEditingLibrary for full reliability 

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

1. Select your texture folder (or individual textures) and optionally a Static Mesh in the Content Browser
2. Adjust naming convention suffixes to match your texture naming if needed
3. Set an output folder or leave blank for the default
4. Hit **Generate Material** to auto-build and assign your PBR material 
---

## More 

Full showcase and documentation at [oliwia-tool-library.webflow.io](http://oliwia-tool-library.webflow.io)
