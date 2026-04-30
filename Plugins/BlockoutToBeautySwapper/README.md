# Blockout to Beauty Swapper 

> Part of [Oliwia's Tool Library](https://github.com/OliwiaJakubiak/OliwiasToolLibrary) | [Tool Library Site](http://oliwia-tool-library.webflow.io)

Swap any level asset for a replacement mesh while preserving transform. 

---

## What it Does 

Blockout to Beauty Swapper lets you replace placeholder or blockout geometry with final art assets directly in the level editor, without touching your scene's layout. Transform, scale and position are fully preserved on every swap, and undo support keeps the workflow non-destructive.

---

## Key Features

- Swap any placed Actor for a replacement Static Mesh
- Fully preserves transform, scale and position on swap
- Replace selected only or all instances at once
- Undo Last Swap support for non-destructive workflow
- Refresh Selection to confirm detected assets before committing 

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

1. Select any actor in the level viewport
2. Select a replacement Static Mesh in the Content Browser
3. Hit **Refresh Selection** to confirm both are detected
4. Choose **Replace Selected Only** or **Replace All Instances** then hit **Swap**
5. Use **Undo Last Swap** to revert if needed  

---

## More 

Full showcase and documentation at [oliwia-tool-library.webflow.io](http://oliwia-tool-library.webflow.io)
