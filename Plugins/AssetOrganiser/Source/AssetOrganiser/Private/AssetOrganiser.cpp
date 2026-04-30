// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetOrganiser.h"
#include "LevelEditor.h" 
#include "EditorUtilityWidgetBlueprint.h" 
#include "EditorUtilitySubsystem.h" 
#include "EditorAssetLibrary.h" 
#include "AssetRegistry/AssetRegistryModule.h"
#include "ToolMenus.h" 
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FAssetOrganiserModule"
IMPLEMENT_MODULE(FAssetOrganiserModule, AssetOrganiser)

// ------------
// -- MODULE --
// ------------

void FAssetOrganiserModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("Oliwia's DevTools: Module started!"));
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAssetOrganiserModule::RegisterMenus));
}
void FAssetOrganiserModule::RegisterMenus()
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
				FNewToolMenuDelegate::CreateRaw(this, &FAssetOrganiserModule::FillMenu)
			);
			UE_LOG(LogTemp, Warning, TEXT("Oliwia's DevTools: Menu created by AssetOrganiser"));
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
					FAssetOrganiserModule::FillMenu(Menu);
				}
			);
			UE_LOG(LogTemp, Warning, TEXT("Oliwia's DevTools: AssetOrganiser added onto existing menu"));
		}
	}
}
void FAssetOrganiserModule::FillMenu(UToolMenu* Menu)
{
	// CATEGORY: "Organisation"
	if (!Menu->ContainsSection("OliwiaDevTools_Organisation"))
	{
		Menu->AddSection(
			"OliwiaDevTools_Organisation",
			LOCTEXT("OrganisationSection_Label", "Organisation")
		);
	}
		FToolMenuSection* Section = Menu->FindSection("OliwiaDevTools_Organisation");
		if (!Section) return;

		Section->AddMenuEntry(
			"SmartAssetOrganiser",
			LOCTEXT("OrganiserBtn_Label", "Smart Asset Organiser"),
			LOCTEXT("OrganiserBtn_Tooltip", "Opens the organisation utility widget"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FAssetOrganiserModule::TriggerAssetOrganiser))
		);
		UE_LOG(LogTemp, Warning, TEXT("Oliwia's DevTools: AssetOrganiser entry added"));
}
void FAssetOrganiserModule::TriggerAssetOrganiser()
{
	FString WidgetPath = TEXT("/AssetOrganiser/UI/EUW_AssetOrganiser.EUW_AssetOrganiser"); // Path to the widget blueprint, change as needed)
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
void FAssetOrganiserModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC --
// ----------------

// -----------------------------------------------------------------------------------------------------------------------------------------
// -- BATCH ORGANISE ASSETS -- 
// Processes selected assets against organiser rules
// bOrganise - moves assets into correct subfolders 
// bRename - applies prefix to asset names 
// Bot hcan be used independently or together 
// Folder existence is handled by RenameAsset which create folders automatically if they don't exist and places assets in existing folders
// No duplicate folders ever created 
// -----------------------------------------------------------------------------------------------------------------------------------------

int32 UAssetOrganiserFunctionLibrary::BatchOrganiseAssets_CPP(
	const TArray<FAssetData>& SelectedAssets, 
	const TMap<UClass*, FAssetOrganiserRule>& OrganiserRules, 
	bool bIsDryRun,
	bool bOrganise,
	bool bRename)
{
	int32 SuccessCount = 0;
	for (const FAssetData& Asset : SelectedAssets)
	{
		UClass* AssetClass = Asset.GetClass();
		if (!OrganiserRules.Contains(AssetClass)) continue;
		const FAssetOrganiserRule& Rule = OrganiserRules[AssetClass];
		FString CurrentPath = Asset.PackagePath.ToString();
		FString AssetName = Asset.AssetName.ToString();
		// -- Build new folder path --
		// Only append subfolder if bOrganise is true 
		FString NewFolder = bOrganise
			? FString::Printf(TEXT("%s/%s"), *CurrentPath, *Rule.FolderName) : CurrentPath;
		// -- Build new asset name --
		// Only apply prefix if bRename is true and prefix not already applied
		FString NewName = bRename && !AssetName.StartsWith(Rule.Prefix)
			? Rule.Prefix + AssetName : AssetName;
		FString NewPath = NewFolder / NewName;
		if (bIsDryRun)
		{
			UE_LOG(LogTemp, Warning, TEXT("DRY RUN: '%s' -> '%s'"),
				*Asset.PackageName.ToString(), *NewPath);
			continue;
		}
		// -- RenameAsset handles folder creation automatically --
		// Creates folder if it doesn't exist 
		// Places asset in existing folder if it does 
		// No duplicate folders created 
		if (UEditorAssetLibrary::RenameAsset(Asset.PackageName.ToString(), NewPath))
		{
			SuccessCount++;
		}
	}
	return SuccessCount;
}
#undef LOCTEXT_NAMESPACE