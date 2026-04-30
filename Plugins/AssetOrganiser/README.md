# Smart Asset Organiser 

> Part of [Oliwia's Tool Library](https://github.com/OliwiaJakubiak/OliwiasToolLibrary) | [Tool Library Site](http://oliwia-tool-library.webflow.io)

Automatically organise and rename project assets into industry standard structures and prefixes.

---

## What it Does 

Smart Asset Ograniser scans your selected assets and sorts them into a clean, structured folder hierarchy with correctly applied prefixes based on asset type. A Dry Run mode lets you preview all changes before anything is moved, keeping the proces fully non-destructive.

---

## Key Features

- Automatic asset sorting into categorised industry-standard folder structures
- Prefix assignment based on asset type following UE5 naming conventions
- Customisable prefix rules to match your team or project conventions
- Dry Run mode - preview all changed without moving anything
- Works on any selection of assets in the Content Browser

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

1. Select assets in the Content Browser
2. Enable **Dry Run** to preview changes without moving anything
3. Hit **Organise Selected Assets** to sort and rename
4. Prefixes and folders follow infustry standard naming conventions

---

## More 

Full showcase and documentation at [oliwia-tool-library.webflow.io](http://oliwia-tool-library.webflow.io)
