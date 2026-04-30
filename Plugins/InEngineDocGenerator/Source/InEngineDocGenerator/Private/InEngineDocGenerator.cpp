// Copyright Epic Games, Inc. All Rights Reserved.

#include "InEngineDocGenerator.h"
#include "LevelEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Json.h"

#define LOCTEXT_NAMESPACE "FInEngineDocGeneratorModule"
IMPLEMENT_MODULE(FInEngineDocGeneratorModule, InEngineDocGenerator)

// ------------
// -- MODULE --
// ------------

void FInEngineDocGeneratorModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FInEngineDocGeneratorModule::RegisterMenus));
	// -- Auto log session start if journal file exists --
	// Only logs if user has previously used the tool 
	// Avoids creating a journal file on first load without user intent 
	if (FPaths::FileExists(UDocGeneratorLibrary::GetJournalFilePath()))
	{
		UDocGeneratorLibrary::LogSessionEvent(TEXT("Session Started"));
	}
}
void FInEngineDocGeneratorModule::RegisterMenus()
{
	if (!UToolMenus::IsToolMenuUIEnabled()) return;
	TArray<FName> MenuTargets = {
		FName("LevelEditor.MainMenu"),
		FName("MainFrame.MainMenu")
	};
	for (const FName& MenuName : MenuTargets)
	{
		UToolMenu* MainMenu = UToolMenus::Get()->ExtendMenu(MenuName);
		if (!MainMenu) continue;
		if (!MainMenu->ContainsSection("OliwiasDevTools"))
		{
			FToolMenuSection& Section = MainMenu->AddSection(
				"OliwiasDevTools",
				TAttribute<FText>(),
				FToolMenuInsert("Help", EToolMenuInsertType::After)
			);
			Section.AddSubMenu(
				"OliwiaDevToolsMenu",
				LOCTEXT("MainBtn_Label", "Oliwia's DevTools"),
				LOCTEXT("MainBtn_Tooltip", "Custom pipeline and organisation tools"),
				FNewToolMenuDelegate::CreateRaw(this, &FInEngineDocGeneratorModule::FillMenu)
			);
		}
		else
		{
			FToolMenuSection* ExistingSection = MainMenu->FindSection("OliwiasDevTools");
			if (!ExistingSection) continue;
			FToolMenuEntry* ExistingEntry = ExistingSection->FindEntry("OliwiaDevToolsMenu");
			if (!ExistingEntry) continue;
			FNewToolMenuDelegate PreviousDelegate = ExistingEntry->SubMenuData.ConstructMenu.NewToolMenu;
			ExistingEntry->SubMenuData.ConstructMenu.NewToolMenu = FNewToolMenuDelegate::CreateLambda(
				[PreviousDelegate, this](UToolMenu* Menu)
				{
					if (PreviousDelegate.IsBound()) PreviousDelegate.Execute(Menu);
					FInEngineDocGeneratorModule::FillMenu(Menu);
				}
			);
		}
	}
}
void FInEngineDocGeneratorModule::FillMenu(UToolMenu* Menu)
{
	// CATEGORY: "Documentation"
	if (!Menu->ContainsSection("OliwiaDevTools_Documentation"))
	{
		Menu->AddSection(
			"OliwiaDevTools_Documentation",
			LOCTEXT("DocumentationSection_Label", "Documentation")
		);
	}
	FToolMenuSection* Section = Menu->FindSection("OliwiaDevTools_Documentation");
	if (!Section) return;
	Section->AddMenuEntry(
		"InEngineDocGenerator",
		LOCTEXT("DocGeneratorBtn_Label", "In-Engine Doc Generator"),
		LOCTEXT("DocGeneratorBtn_Tooltip", "Opens the In-Engine Doc Generator - your in-editor development journal"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FInEngineDocGeneratorModule::TriggerDocGenerator))
	);
}
void FInEngineDocGeneratorModule::TriggerDocGenerator()
{
	FString WidgetPath = TEXT("/InEngineDocGenerator/UI/EUW_DocGenerator.EUW_DocGenerator");
	UObject* WidgetObj = StaticLoadObject(UEditorUtilityWidgetBlueprint::StaticClass(), nullptr, *WidgetPath);
	if (WidgetObj != nullptr)
	{
		UEditorUtilityWidgetBlueprint* WidgetBP = Cast<UEditorUtilityWidgetBlueprint>(WidgetObj);
		if (WidgetBP != nullptr)
		{
			if (UEditorUtilitySubsystem* Subsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
			{
				Subsystem->SpawnAndRegisterTab(WidgetBP);
			}
		}
	}
}
void FInEngineDocGeneratorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	// -- Auto log session end --
	if (FPaths::FileExists(UDocGeneratorLibrary::GetJournalFilePath()))
	{
		UDocGeneratorLibrary::LogSessionEvent(TEXT("Session Ended"));
	}
}

// ----------------
// -- CORE LOGIC --
// ----------------

// --------------------------------------------
// -- GET JOURNAL FILE PATH --
// Returns full path to journal JSON file 
// Saved in project Saved/DevJournal/ folder 
// Not in Content/ so it doesn't affect builds 
// or appear in content browser
// --------------------------------------------

FString UDocGeneratorLibrary::GetJournalFilePath()
{
	return FPaths::ProjectSavedDir() / TEXT("DevJournal") / TEXT("DevJournal.json");
}

// ---------------------------------------------------------------------------------------------------------------------
// -- GET CURRENT TIMESTAMP --
// Returns formatted date and time string 
// Format: YYYY-MM-DD HH:MM
// ISO 8601 standard format, same as Git version control and Excel/Google Sheets both recognise it correctly through CSV
// ---------------------------------------------------------------------------------------------------------------------

FString UDocGeneratorLibrary::GetCurrentTimestamp()
{
	FDateTime Now = FDateTime::Now();
	return FString::Printf(TEXT("%04d-%02d-%02d %02d:%02d"),
		Now.GetYear(),
		Now.GetMonth(),
		Now.GetDay(),
		Now.GetHour(),
		Now.GetMinute()
	);
}

// --------------------------------------------------------------------------------
// -- GET FORMATTED LOG DISPLAY --
// Loads all entries and formats them as a single display string for TB_LogDisplay
// Called directly from Event Construct
// --------------------------------------------------------------------------------

FString UDocGeneratorLibrary::GetFormattedLogDisplay()
{
	TArray<FJournalEntry> Entries = LoadEntries();
	FString DisplayText;
	for (const FJournalEntry& Entry : Entries)
	{
		DisplayText += FString::Printf(TEXT("[%s] %s - %s\n"),
			*Entry.Category,
			*Entry.Timestamp,
			*Entry.Note
		);
	}
	return DisplayText;
}

// ----------------------------------------------
// -- ADD ENTRY --
// Creates a new journal entry with timestamp
// Loads existing entries, appends new one, saves
// Returns the created entry for UI display
// ----------------------------------------------

FJournalEntry UDocGeneratorLibrary::AddEntry(
	const FString& Category,
	const FString& Note,
	const FString& Author)
{
	FJournalEntry NewEntry;
	NewEntry.Category = Category;
	NewEntry.Note = Note;
	NewEntry.Author = Author;
	NewEntry.Timestamp = GetCurrentTimestamp();
	TArray<FJournalEntry> Entries = LoadEntries();
	Entries.Add(NewEntry);
	SaveEntries(Entries);
	return NewEntry;
}

// --------------------------------------------------
// -- LOAD ENTRIES --
// Reads journal JSON file from disk 
// Returns empty array if file doesnt exist yet 
// Parses each entry back into FJournalEntry structs
// --------------------------------------------------

TArray<FJournalEntry> UDocGeneratorLibrary::LoadEntries()
{
	TArray<FJournalEntry> Entries;
	FString FilePath = GetJournalFilePath();
	if (!FPaths::FileExists(FilePath))
	{
		return Entries;
	}
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("DocGenerator: Failed to load journal file"));
		return Entries;
	}
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, JsonArray))
	{
		UE_LOG(LogTemp, Warning, TEXT("DocGenerator: Failed to parse journal JSON"));
		return Entries;
	}
	for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
	{
		TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
		if (!JsonObject.IsValid()) continue;
		FJournalEntry Entry;
		Entry.Category = JsonObject->GetStringField(TEXT("Category"));
		Entry.Timestamp = JsonObject->GetStringField(TEXT("Timestamp"));
		Entry.Author = JsonObject->GetStringField(TEXT("Author"));
		Entry.Note = JsonObject->GetStringField(TEXT("Note"));
		Entries.Add(Entry);
	}
	return Entries;
}

// ----------------------------------------------------
// -- SAVE ENTRIES --
// Serialises all entries to JSON and writes to disk 
// Creates the DevJournal directory if it doesnt exist
// ----------------------------------------------------

bool UDocGeneratorLibrary::SaveEntries(const TArray<FJournalEntry>& Entries)
{
	FString FilePath = GetJournalFilePath();
	// -- Create directory if it doesnt exist --
	FString Directory = FPaths::GetPath(FilePath);
	IFileManager::Get().MakeDirectory(*Directory, true);
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const FJournalEntry& Entry : Entries)
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		JsonObject->SetStringField(TEXT("Category"), Entry.Category);
		JsonObject->SetStringField(TEXT("Timestamp"), Entry.Timestamp);
		JsonObject->SetStringField(TEXT("Author"), Entry.Author);
		JsonObject->SetStringField(TEXT("Note"), Entry.Note);
		JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
	}
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonArray, Writer);
	if (!FFileHelper::SaveStringToFile(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("DocGenerator: Failed to save journal file"));
		return false;
	}
	return true;
}

// -------------------------------------
// -- CLEAR ENTRIES --
// Deletes all entries from the journal
// Saves empty array to disk
// -------------------------------------

bool UDocGeneratorLibrary::ClearEntries()
{
	return SaveEntries(TArray<FJournalEntry>());
}

// -----------------------------------------------------
// -- LOG SESSION EVENT --
// Automatically called on editor startup and shutdown 
// Creates a session category entry with the event type 
// Only fires if journal file already exists
// -----------------------------------------------------

void UDocGeneratorLibrary::LogSessionEvent(const FString& EventType)
{
	FJournalEntry SessionEntry;
	SessionEntry.Category = TEXT("Session");
	SessionEntry.Timestamp = GetCurrentTimestamp();
	SessionEntry.Author = TEXT("System");
	SessionEntry.Note = EventType;
	TArray<FJournalEntry> Entries = LoadEntries();
	Entries.Add(SessionEntry);
	SaveEntries(Entries);
}

// ------------------------------------------------
// -- EXPORT TXT --
// Exports all entries as plain text
// One entry per line: [Category] Timestamp - Note
// Saved to project Saved/DevJournal/ folder 
// ------------------------------------------------

bool UDocGeneratorLibrary::ExportTXT(
	const TArray<FJournalEntry>& Entries,
	const FString& ProjectName,
	const FString& Author)
{
	FString Output;
	Output += FString::Printf(TEXT("Dev Journal - %s\n"), *ProjectName);
	Output += FString::Printf(TEXT("Author: %s\n"), *Author);
	Output += FString::Printf(TEXT("Exported: %s\n"), *GetCurrentTimestamp());
	Output += TEXT("----------------------------------------\n\n");
	for (const FJournalEntry& Entry : Entries)
	{
		Output += FString::Printf(TEXT("[%s] %s - %s\n"),
			*Entry.Category,
			*Entry.Timestamp,
			*Entry.Note
		);
	}
	FString ExportPath = FPaths::ProjectSavedDir() / TEXT("DevJournal") / TEXT("DevJournal_Export.txt");
	if (!FFileHelper::SaveStringToFile(Output, *ExportPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("DocGenerator: Failed to export TXT"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("DocGenerator: Exported TXT to %s"), *ExportPath);
	return true;
}

// -----------------------------------------------
// -- EXPORT CSV --
// Exports all entries as CSV - Excel compatible 
// Saved to project Saved/DevJournal/ folder 
// -----------------------------------------------

bool UDocGeneratorLibrary::ExportCSV(
	const TArray<FJournalEntry>& Entries,
	const FString& ProjectName,
	const FString& Author)
{
	FString Output;
	Output += TEXT("Category,Timestamp,Author,Note\n");
	for (const FJournalEntry& Entry : Entries)
	{
		// -- Wrap note in quotes to handle commas in text --
		Output += FString::Printf(TEXT("%s,%s,%s,\"%s\"\n"),
			*Entry.Category,
			*Entry.Timestamp,
			*Entry.Author,
			*Entry.Note
		);
	}
	FString ExportPath = FPaths::ProjectSavedDir() / TEXT("DevJournal") / TEXT("DevJournal_Export.csv");
	if (!FFileHelper::SaveStringToFile(Output, *ExportPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("DocGenerator: Failed to export CSV"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("DocGenerator: Exported CSV to %s"), *ExportPath);
	return true;
}

// -----------------------------------------------
// -- EXPORT HTML --
// Exports all entries as a formatted HTML report
// Colour coded by category
// Saved to project Saved/DevJournal/ folder 
// -----------------------------------------------

bool UDocGeneratorLibrary::ExportHTML(
	const TArray<FJournalEntry>& Entries,
	const FString& ProjectName,
	const FString& Author)
{
	// -- Category colour map --
	TMap<FString, FString> CategoryColours;
	CategoryColours.Add(TEXT("Bug"), TEXT("#e74c3c"));
	CategoryColours.Add(TEXT("Fixed"), TEXT("#2ecc71"));
	CategoryColours.Add(TEXT("Decision"), TEXT("#3498db"));
	CategoryColours.Add(TEXT("ToDo"), TEXT("#f1c40f"));
	CategoryColours.Add(TEXT("Added"), TEXT("#1abc9c"));
	CategoryColours.Add(TEXT("Changed"), TEXT("#e67e22"));
	CategoryColours.Add(TEXT("Removed"), TEXT("#95a5a6"));
	CategoryColours.Add(TEXT("Note"), TEXT("#ecf0f1"));
	CategoryColours.Add(TEXT("Session"), TEXT("#9b59b6"));
	FString Output;
	Output += TEXT("<!DOCTYPE html><html><head>");
	Output += TEXT("<meta charset='UTF-8'>");
	Output += TEXT("<title>Dev Journal</title>");
	Output += TEXT("<style>");
	Output += TEXT("body { font-family: 'Arial', sans-serif; background: #1e1e1e; color: #ffffff; padding: 40px; margin: 0; }");
	Output += TEXT("h1 { font-size: 48px; font-weight: 900; color: #c1d0d8; text-transform: uppercase; letter-spacing: -1px; margin-bottom: 4px; }");
	Output += TEXT(".meta { color: #747474; font-size: 13px; margin-bottom: 40px; border-bottom: 1px solid #2a2a2a; padding-bottom: 20px; }");
	Output += TEXT(".entry { padding: 14px 16px; margin: 8px 0; border-radius: 4px; background: #252525; border-left: 3px solid #c1d0d8; }");
	Output += TEXT(".tag { display: inline-block; padding: 2px 10px; border-radius: 3px; font-size: 11px; font-weight: 700; margin-right: 10px; color: #1e1e1e; text-transform: uppercase; letter-spacing: 0.5px; }");
	Output += TEXT(".timestamp { color: #747474; font-size: 12px; }");
	Output += TEXT(".note { margin-top: 6px; color: #ffffff; font-size: 13px; line-height: 1.5; }");
	Output += TEXT(".header-bar { border-bottom: 2px solid #c1d0d8; margin-bottom: 30px; padding-bottom: 10px; }");
	Output += TEXT(".count { color: #c1d0d8; font-size: 13px; margin-top: 4px; }");
	Output += TEXT("footer { margin-top: 40px; color: #747474; font-size: 12px; border-top: 1px solid #2a2a2a; padding-top: 16px; }");
	Output += TEXT("</style></head><body>");
	Output += TEXT("<div class='header-bar'>");
	Output += FString::Printf(TEXT("<h1>%s</h1>"), *ProjectName);
	Output += FString::Printf(TEXT("<div class='meta'>Author: %s &nbsp;|&nbsp; Exported: %s</div>"),
		*Author, *GetCurrentTimestamp());
	Output += FString::Printf(TEXT("<div class='count'>%d entries</div>"), Entries.Num());
	Output += TEXT("</div>");
	for (const FJournalEntry& Entry : Entries)
	{
		FString Colour = CategoryColours.Contains(Entry.Category)
			? CategoryColours[Entry.Category] : TEXT("#ecf0f1");
		Output += TEXT("<div class='entry'>");
		Output += FString::Printf(TEXT("<span class='tag' style='background:%s'>%s</span>"),
			*Colour, *Entry.Category);
		Output += FString::Printf(TEXT("<span class='timestamp'>%s</span>"), *Entry.Timestamp);
		Output += FString::Printf(TEXT("<div class='note'>%s</div>"), *Entry.Note);
		Output += TEXT("</div>");
	}
	Output += TEXT("<footer>Generated by Oliwia's Tool Library - In-Engine Doc Generator &nbsp;|&nbsp; Unreal Engine 5</footer>");
	Output += TEXT("</body></html>");
	FString ExportPath = FPaths::ProjectSavedDir() / TEXT("DevJournal") / TEXT("DevJournal_Export.html");
	if (!FFileHelper::SaveStringToFile(Output, *ExportPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("DocGenerator: Failed to export HTML"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("DocGenerator: Exported HTML to %s"), *ExportPath);
	return true;
}
#undef LOCTEXT_NAMESPACE
	