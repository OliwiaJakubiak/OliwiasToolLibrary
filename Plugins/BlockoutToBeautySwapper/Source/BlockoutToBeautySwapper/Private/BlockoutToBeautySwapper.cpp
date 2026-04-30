// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlockoutToBeautySwapper.h"
#include "LevelEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "EditorActorFolders.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Engine/World.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#define LOCTEXT_NAMESPACE "FBlockoutToBeautySwapperModule"
IMPLEMENT_MODULE(FBlockoutToBeautySwapperModule, BlockoutToBeautySwapper)

// ------------
// -- MODULE --
// ------------

void FBlockoutToBeautySwapperModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBlockoutToBeautySwapperModule::RegisterMenus));
}
void FBlockoutToBeautySwapperModule::RegisterMenus()
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
                FNewToolMenuDelegate::CreateRaw(this, &FBlockoutToBeautySwapperModule::FillMenu)
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
                    FBlockoutToBeautySwapperModule::FillMenu(Menu);
                }
            );
        }
    }
}
void FBlockoutToBeautySwapperModule::FillMenu(UToolMenu* Menu)
{
    // CATEGORY: "Environment"
    if (!Menu->ContainsSection("OliwiaDevTools_Environment"))
    {
        Menu->AddSection(
            "OliwiaDevTools_Environment",
            LOCTEXT("EnvironmentSection_Label", "Environment")
        );
    }

    FToolMenuSection* Section = Menu->FindSection("OliwiaDevTools_Environment");
    if (!Section) return;

    Section->AddMenuEntry(
        "BlockoutToBeautySwapper",
        LOCTEXT("BlockoutBtn_Label", "Blockout to Beauty Swapper"),
        LOCTEXT("BlockoutBtn_Tooltip", "Opens the Blockout to Beauty Swapper utility widget"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FBlockoutToBeautySwapperModule::TriggerBlockoutToBeautySwapper))
    );
}
void FBlockoutToBeautySwapperModule::TriggerBlockoutToBeautySwapper()
{
    FString WidgetPath = TEXT("/BlockoutToBeautySwapper/UI/EUW_BlockoutToBeautySwapper.EUW_BlockoutToBeautySwapper");
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
void FBlockoutToBeautySwapperModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC --
// ----------------

// ----------------------------------------------------------------------
// -- REFRESH SELECTION --
// Reads level viewport selection via EditorActorSubsystem
// Reads content browser StaticMesh selection via EditorUtilitySubsystem
// Returns names for UI display and references for swap logic 
// Called directly from BTN_Refresh OnClicked
// ----------------------------------------------------------------------

void UBlockoutSwapperLibrary::RefreshSelection(
    AActor*& OutSourceActor,
    UStaticMesh*& OutReplacementMesh,
    FString& OutSourceActorName,
    FString& OutReplacementMeshName)
{
	OutSourceActor = nullptr;
	OutReplacementMesh = nullptr;
	OutSourceActorName = TEXT("No Actor Selected");
	OutReplacementMeshName = TEXT("No Replacement Mesh Selected");
	// -- Read level viewport selection --
	UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
    if (EditorActorSubsystem)
    {
        TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
        if (SelectedActors.Num() > 0 && SelectedActors[0])
        {
            OutSourceActor = SelectedActors[0];
			OutSourceActorName = SelectedActors[0]->GetActorLabel();
        }
	}
	// -- Read content browser selection --
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();
	TArray<FAssetData> SelectedAssets;
	ContentBrowser.GetSelectedAssets(SelectedAssets);
    for (const FAssetData& Asset : SelectedAssets)
    {
        if (Asset.AssetClassPath == FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("StaticMesh")))
        {
            OutReplacementMesh = Cast<UStaticMesh>(Asset.GetAsset());
            if (OutReplacementMesh)
            {
                OutReplacementMeshName = Asset.AssetName.ToString();
            }
            break;
        }
	}
}

// ---------------------------------------------------------------------
// -- PERFORM SWAP --
// Handles both single actor and all instances swap
// Preserves transform - position, rotation and scale
// Returns affected actor and original mesh references for undo storage
// Returns log entry for UI display
// ---------------------------------------------------------------------

void UBlockoutSwapperLibrary::PerformSwap(
    AActor* SourceActor,
    UStaticMesh* ReplacementMesh,
    bool bReplaceAll,
    TArray<AActor*>& OutAffectedActors,
    UStaticMesh*& OutOriginalMesh,
    FString& OutLogEntry)
{
    OutAffectedActors.Empty();
    OutOriginalMesh = nullptr;
    OutLogEntry = TEXT("");
    if (!SourceActor || !ReplacementMesh) return;
    // -- Get original mesh from source actor --
    UStaticMeshComponent* SourceComponent = SourceActor->FindComponentByClass < UStaticMeshComponent>();
    if (!SourceComponent)
    {
        OutLogEntry = TEXT("Selected actor does not have a Static Mesh Component.");
        return;
    }
    OutOriginalMesh = SourceComponent->GetStaticMesh();
    if (!OutOriginalMesh) return;
    FString OriginalMeshName = OutOriginalMesh->GetName();
    FString NewMeshName = ReplacementMesh->GetName();
    if (bReplaceAll)
    {
        // -- Replace all instances in the level -- 
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World) return;
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);
        for (AActor* Actor : AllActors)
        {
            UStaticMeshComponent* MeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (!MeshComponent) continue;
            if (MeshComponent->GetStaticMesh() == OutOriginalMesh)
            {
                MeshComponent->SetStaticMesh(ReplacementMesh);
                Actor->MarkPackageDirty();
                OutAffectedActors.Add(Actor);
            }
        }
        OutLogEntry = FString::Printf(TEXT("%s -> %s (%d instances)"),
            *OriginalMeshName, *NewMeshName, OutAffectedActors.Num());
    }
    else
    {
        // -- Replace selected actor only --
        SourceComponent->SetStaticMesh(ReplacementMesh);
        SourceActor->MarkPackageDirty();
        OutAffectedActors.Add(SourceActor);
        OutLogEntry = FString::Printf(TEXT("%s -> %s (1 instance)"),
            *OriginalMeshName, *NewMeshName);
    }
}

// ------------------------------------------------------
// -- UNDO SWAP --
// Restores OriginalMesh to all actors in provided array
// Returns log entry for UI display
// ------------------------------------------------------

void UBlockoutSwapperLibrary::UndoSwap(
    const TArray<AActor*>& AffectedActors,
    UStaticMesh* OriginalMesh,
    FString& OutLogEntry)
{
    OutLogEntry = TEXT("");
    if (!OriginalMesh || AffectedActors.Num() == 0)
    {
        OutLogEntry = TEXT("Nothing to undo");
        return;
    }
    for (AActor* Actor : AffectedActors)
    {
        if (!Actor) continue;
        UStaticMeshComponent* MeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (!MeshComponent) continue;
        MeshComponent->SetStaticMesh(OriginalMesh);
        Actor->MarkPackageDirty();
    }
    OutLogEntry = FString::Printf(TEXT("Undo complete — restored %s to %d actors"),
        *OriginalMesh->GetName(), AffectedActors.Num());
}
#undef LOCTEXT_NAMESPACE