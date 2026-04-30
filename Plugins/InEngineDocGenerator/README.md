# In-Engine Doc Generator 

> Part of [Oliwia's Tool Library](https://github.com/OliwiaJakubiak/OliwiasToolLibrary) | [Tool Library Site](http://oliwia-tool-library.webflow.io)

A dockable in-editor development journal - log structured notes by category, timestamped and exportable. 

---

## What it Does 

In-Engine Doc Generator is a dockable editor panel that keeps your development journal inside Unreal Engine. Log structured notes by category, each automatically timestamped, and export your journal as TXT, CSV, or HTML whenever you need it outside the editor. Optional session logging automatically tracks  edit or sessions in the background.

---

## Key Features

- Dockable in-editor panel - stays accessible without interrupting your workflow
- Structured note logging by category with automatic timestamps
- Export journal as TXT, CSV, or HTML
- Project Name and Author configuration via Settings
- Optional session logging to automatically track editor sessions 

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

1. Open **Settings** and enter your Project Name and Author
2. Select a category from the dropdown and write your note
3. Hit **Add Entry** to log it with an automatic timestamp
4. Use **Export TXT**, **CSV** or **HTML** to save your journal outside the editor
5. Enable **Session Logging** in Settings to automatically track editor sessions 

---

## More 

Full showcase and documentation at [oliwia-tool-library.webflow.io](http://oliwia-tool-library.webflow.io)
