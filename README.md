# Oliwia's Tool Library 

A growing suite of production-ready Unreal Engine plugins built to streamline game development workflows, spanning a variety of categories designed to reduce friction and keep you in flow.

> Built and tested in **Unreal Engine 5.7.4**

🔗 [View full documentation and showcase at oliwia-tool-library.webflow.io](http://oliwia-tool-library.webflow.io)

---

## Tools 

| # | Plugin | Category | Description | 
|---|--------|----------|-------------|
| 01 | Smart Asset Organiser | Organisation | Automatically organise and rename project assets into industry standard folder structures and prefixes. | 
| 02 | Game Feel Kit | Game Feel | A component-based toolkit for adding juice, impact and polish to any actor with one-click effect triggers. | 
| 03 | UI Menu Forge | Game Feel | Generate pre-wired, production-ready UI screen templates with a central manager. Ready to style and ship. | 
| 04 | Mesh Utility Kit | 3D Art | One-click mesh utilities for resizing, origin resetting and collision generation on Static Mesh actors. |
| 05 | Auto Material Setup | 3D Art | Automatically detect, wire and assign a full PBR material from a texture folder in a single click. |
| 06 | Quick Lighting Kit | Environment | Apply professional scene lighting presets instantly with full control over sun, sky and fog values. |
| 07 | Blockout to Beauty Swapper | Environment | Swap any level asset for a replacement mesh while preserving transform. One click, all instances or selected only. | 
| 08 | Character Kickstarter | Character & Animation | Generate a fully configured, play-ready character Blueprint with movement, camera and input pre-wired. | 
| 09 | Universal Animation Template | Character & Animation | Generate a custom Animation Blueprint with the exact states you need, skeleton assigned and variables ready. | 
| 10 | In-Engine Doc Generator | Documentation | A dockable in-editor development journal. Log structured notes by category, timestamped and exportable. | 

---

## Requirements 

- **Unreal Engine 5.7.4** or later
- **C++ project** - all plugins contain C++ source and require a project with C++ enabled and Visual Studio (or equivalent) installed
- If your project is Blueprint-only, you will need to convert it to a C++ project first. Unreal Engine can do this automatically via *Tools -> New C++ Class*

---

## Installation 

1. Download the plugin `.zip` from the relevant release on this page
2. Extract the plugin folder into your project's `Plugins` directory (create it if it doesn't exist)
3. Right-click your `.uproject` file and select **Generate Visual Studio project files**
4. Open the project - Unreal will prompt you to build the plugin. Click **Yes**
5. Once built, enable the plugin via *Edit -> Plugins* if it isn't already active

---

## Oliwia's DevTools Menu

All tools in this library are accessible from a single **Oliwia's DevTools** menu added to the Unreal Engine toolbar. Once any plugin from the library is enabled in your project, the menu will appear in the editor. Serving you a central place to open any installed tool without hunting through the editor UI. 

---

## Releases 

Each plugin is versioned and released independently. Find individual plugin downloads under [Releases](../../releases)

---

## About 

Oliwia's Tool Library is a professional, developer-facing plugin suite designed around real game development workflows. Each tool is built to integrate cleanly into any UE5 project with minimal setup, focused on saving time, reducing friction, and maintaining industry-standard practices.

The library is actively maintained and continues to grow. For full showcases, feature breakdowns, and how-it-works documentation visit the [Tool Library Site](http://oliwia-tool-library.webflow.io)

For more about the development process and the person behind the tools, visit [My Portfolio](http://oliwiajakubiak.webflow.io)
