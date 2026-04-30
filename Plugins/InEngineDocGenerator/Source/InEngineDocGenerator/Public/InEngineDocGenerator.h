// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InEngineDocGenerator.generated.h"

// ------------------------------------------------
// -- JOURNAL ENTRY STRUCT --
// Represents a single log entry in the dev journal 
// Stored as JSON, displayed in the EUW scroll box 
// ------------------------------------------------

USTRUCT(BlueprintType)
struct FJournalEntry
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DocGenerator")
	FString Category;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DocGenerator")
	FString Timestamp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DocGenerator")
	FString Author;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DocGenerator")
	FString Note;
};

// --------------------------------------------------
// -- FUNCTION LIBRARY --
// All journal logic called from EUW buttons directly 
// File I/O, JSON persistence, export functions 
// Session logging hooks into editor startup/shutdown 
// --------------------------------------------------

UCLASS()
class INENGINEDOCGENERATOR_API UDocGeneratorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public: 
	// Adds a new entry to the journal and saves to disk 
	// Called directly from BTN_AddEntry OnClicked 
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static FJournalEntry AddEntry(
		const FString& Category,
		const FString& Note,
		const FString& Author
	);
	// Loads all entries from the journal file on disk
	// Called on EUW open to populate the log display
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static TArray<FJournalEntry> LoadEntries();
	// Saves all entires to the journal file on disk 
	// Called after any modification to the journal
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static bool SaveEntries(const TArray<FJournalEntry>& Entries);
	// Clears all entries from the journal
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static bool ClearEntries();
	// Exports journal to a plain text file
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static bool ExportTXT(
		const TArray<FJournalEntry>& Entries,
		const FString& ProjectName,
		const FString& Author
	);
	// Exports journal to a CSV file -	Excel compatible
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static bool ExportCSV(
		const TArray<FJournalEntry>& Entries,
		const FString& ProjectName,
		const FString& Author
	);
	// Exports journal to a formatted HTML report
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static bool ExportHTML(
		const TArray<FJournalEntry>& Entries,
		const FString& ProjectName,
		const FString& Author
	);
	// Logs a session start or end entry automatically
	// Called from module StartupModule and ShutdownModule
	// Only fires if session logging is enabled
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static void LogSessionEvent(const FString& EventType);
	// Returns the full path to the journal file
	// Saved/DevJournal/DevJournal.json in the project directory
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static FString GetJournalFilePath();
	// Returns current date and time as formatted string
	// Used for entry timestamps
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static FString GetCurrentTimestamp();
	// Loads all entries and returns them as a single formatted display string
	// Called from Event Construct to populate TB_LogDisplay directly 
	UFUNCTION(BlueprintCallable, Category = "DocGenerator")
	static FString GetFormattedLogDisplay();
};

// ------------
// -- MODULE --
// ------------

class FInEngineDocGeneratorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private: 
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerDocGenerator();
};
