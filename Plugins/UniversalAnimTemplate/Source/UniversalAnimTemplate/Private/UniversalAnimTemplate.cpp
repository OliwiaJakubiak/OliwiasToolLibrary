// Copyright Epic Games, Inc. All Rights Reserved.

#include "UniversalAnimTemplate.h"
#include "LevelEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "Animation/Skeleton.h"
#include "EdGraphSchema_K2.h"

#define LOCTEXT_NAMESPACE "FUniversalAnimTemplateModule"
IMPLEMENT_MODULE(FUniversalAnimTemplateModule, UniversalAnimTemplate)

// ------------
// -- MODULE --
// ------------

void FUniversalAnimTemplateModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUniversalAnimTemplateModule::RegisterMenus));
}
void FUniversalAnimTemplateModule::RegisterMenus()
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
				FNewToolMenuDelegate::CreateRaw(this, &FUniversalAnimTemplateModule::FillMenu)
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
					FUniversalAnimTemplateModule::FillMenu(Menu);
				}
			);
		}
	}
}
void FUniversalAnimTemplateModule::FillMenu(UToolMenu* Menu)
{
	// CATEGORY: "Character and Animation"
	if (!Menu->ContainsSection("OliwiaDevTools_CharacterAnimation"))
	{
		Menu->AddSection(
			"OliwiaDevTools_CharacterAnimation",
			LOCTEXT("CharacterSection_Label", "Character and Animation")
		);
	}
	FToolMenuSection* Section = Menu->FindSection("OliwiaDevTools_CharacterAnimation");
	if (!Section) return;
	Section->AddMenuEntry(
		"UniversalAnimTemplate",
		LOCTEXT("CharacterBtn_Label", "Universal Anim Template"),
		LOCTEXT("CharacterBtn_Tooltip", "Opens the Universal Anim Template utility widget"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FUniversalAnimTemplateModule::TriggerUniversalAnimTemplate))
	);
}
void FUniversalAnimTemplateModule::TriggerUniversalAnimTemplate()
{
	FString WidgetPath = TEXT("/UniversalAnimTemplate/UI/EUW_UniversalAnimTemplate.EUW_UniversalAnimTemplate");
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
void FUniversalAnimTemplateModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC -- 
// ----------------

// ----------------------------------------------------------------------
// -- GENERATE ANIM BP --
// Attempts programmatic AnimBP generation with 
// selected states wired into a single state machine
// Uses UAnimBlueprint, FKismetEditorUtilities and AnimGraph editor APIs 
// ----------------------------------------------------------------------

bool UUniversalAnimTemplateLibrary::GenerateAnimBP(
	const FString& OutputPath,
	const FString& AnimBPName,
	const FAnimStateSelection& StateSelection,
	USkeleton* TargetSkeleton)
{
	// -- Resolve and validate output path --
	FString FinalOutputPath = OutputPath.IsEmpty() ? TEXT("/Game/Animation") : OutputPath;
	if (FinalOutputPath.EndsWith(TEXT("/")))
		FinalOutputPath.RemoveFromEnd(TEXT("/"));
	if (!FinalOutputPath.StartsWith(TEXT("/")))
		FinalOutputPath = TEXT("/Game/") + FinalOutputPath;
	FString PackagePath = FinalOutputPath / AnimBPName;
	// -- Create the AnimBP asset --
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		UE_LOG(LogTemp, Warning, TEXT("UniversalAnimTemplate: Failed to create package at %s"), *PackagePath);
		return false;
	}
	// -- Create AnimBlueprint using FKismetEditorUtilities --
	// UAnimInstance - parent class for AnimBPs 
	// Same pattern UE uses internally for its own AnimBP creation
	UAnimBlueprint* NewAnimBP = Cast<UAnimBlueprint>(
		FKismetEditorUtilities::CreateBlueprint(
			UAnimInstance::StaticClass(),
			Package,
			*AnimBPName,
			BPTYPE_Normal,
			UAnimBlueprint::StaticClass(),
			UBlueprintGeneratedClass::StaticClass()
		)
	);
	if (!NewAnimBP)
	{
		UE_LOG(LogTemp, Warning, TEXT("UniversalAnimTemplate: Failed to create AnimBlueprint"));
		return false;
	}
	// -- Assign skeleton -- 
	// Required for AnimBP to open without skeleton picker popup
	if (TargetSkeleton)
	{
		NewAnimBP->TargetSkeleton = TargetSkeleton;
	}
	// -- Add Variables based on selected states --
	// Only adds variables relevant to the states the user selected
	// Speed - always added,drives all locomotion transitions
	FBlueprintEditorUtils::AddMemberVariable(NewAnimBP, FName("Speed"), FEdGraphPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
	// Boolean variables - only added if corresponding state is selected
	if (StateSelection.bJump)
		FBlueprintEditorUtils::AddMemberVariable(NewAnimBP, FName("bIsJumping"), FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
	if (StateSelection.bFall)
		FBlueprintEditorUtils::AddMemberVariable(NewAnimBP, FName("bIsFalling"), FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
	if (StateSelection.bLand)
		FBlueprintEditorUtils::AddMemberVariable(NewAnimBP, FName("bIsLanding"), FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
	if (StateSelection.bCrouch)
		FBlueprintEditorUtils::AddMemberVariable(NewAnimBP, FName("bIsCrouching"), FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
	if (StateSelection.bSprint)
		FBlueprintEditorUtils::AddMemberVariable(NewAnimBP, FName("bIsSprinting"), FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
	if (StateSelection.bDeath)
		FBlueprintEditorUtils::AddMemberVariable(NewAnimBP, FName("bIsDead"), FEdGraphPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType()));
	// -- Compile and save --
	FKismetEditorUtilities::CompileBlueprint(NewAnimBP);
	FAssetRegistryModule::AssetCreated(NewAnimBP);
	Package->MarkPackageDirty();
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	FString PackageFilename = FPackageName::LongPackageNameToFilename(
		PackagePath,
		FPackageName::GetAssetPackageExtension()
	);
	UPackage::SavePackage(Package, NewAnimBP, *PackageFilename, SaveArgs);
	UE_LOG(LogTemp, Log, TEXT("UniversalAnimTemplate: Generated AnimBP '%s' at '%s'"), *AnimBPName, *FinalOutputPath);
	return true;
}
#undef LOCTEXT_NAMESPACE