// Copyright Epic Games, Inc. All Rights Reserved.

#include "UIMenuForge.h"
#include "LevelEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "GameFramework/GameUserSettings.h"

#define LOCTEXT_NAMESPACE "FUIMenuForgeModule"

IMPLEMENT_MODULE(FUIMenuForgeModule, UIMenuForge)

// ------------
// -- MODULE -- 
// ------------

void FUIMenuForgeModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUIMenuForgeModule::RegisterMenus));
}

void FUIMenuForgeModule::RegisterMenus()
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
                FNewToolMenuDelegate::CreateRaw(this, &FUIMenuForgeModule::FillMenu)
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
                    FUIMenuForgeModule::FillMenu(Menu);
                }
            );
        }
    }
}

void FUIMenuForgeModule::FillMenu(UToolMenu* Menu)
{
    // CATEGORY: "Game Feel"
    if (!Menu->ContainsSection("OliwiaDevTools_GameFeel"))
    {
        Menu->AddSection(
            "OliwiaDevTools_GameFeel",
            LOCTEXT("GameFeelSection_Label", "Game Feel")
        );
    }

    FToolMenuSection* Section = Menu->FindSection("OliwiaDevTools_GameFeel");
    if (!Section) return;

    Section->AddMenuEntry(
        "UIMenuForge",
        LOCTEXT("UIMenuForgeBtn_Label", "UI Menu Forge"),
        LOCTEXT("UIMenuForgeBtn_Tooltip", "Opens the UI Menu Forge utility widget"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FUIMenuForgeModule::TriggerUIMenuForge))
    );
}

void FUIMenuForgeModule::TriggerUIMenuForge()
{
    FString WidgetPath = TEXT("/UIMenuForge/UI/EUW/EUW_UIMenuForge.EUW_UIMenuForge");
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

void FUIMenuForgeModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC --
// ----------------

// --------------------------
// -- UI MANAGER SUBSYSTEM --
// --------------------------

void UUIMenuForge_Manager::ShowScreen(EScreenType ScreenType, TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass) return;
    if (ActiveScreens.Contains(ScreenType)) return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!NewWidget) return;

    NewWidget->AddToViewport();
    ActiveScreens.Add(ScreenType, NewWidget);
    UpdateInputMode();
}

void UUIMenuForge_Manager::HideScreen(EScreenType ScreenType)
{
    if (!ActiveScreens.Contains(ScreenType)) return;

    UUserWidget* Widget = ActiveScreens[ScreenType];
    if (Widget)
    {
        Widget->RemoveFromParent();
    }

    ActiveScreens.Remove(ScreenType);
    UpdateInputMode();
}

void UUIMenuForge_Manager::HideAllScreens()
{
    for (auto& Pair : ActiveScreens)
    {
        if (Pair.Value)
        {
            Pair.Value->RemoveFromParent();
        }
    }

    ActiveScreens.Empty();
    UpdateInputMode();
}

void UUIMenuForge_Manager::TogglePause(bool bPause)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    UGameplayStatics::SetGamePaused(World, bPause);
}

void UUIMenuForge_Manager::UpdateInputMode()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    bool bHasHUD = ActiveScreens.Contains(EScreenType::HUD);
    int32 ScreenCount = ActiveScreens.Num();

    if (ScreenCount == 0 || (ScreenCount == 1 && bHasHUD))
    {
        // Game only - no UI screens active
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
    else if (ScreenCount > 1 && bHasHUD)
    {
        // Game and UI - HUD plus another screen
        FInputModeGameAndUI InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
    else
    {
        // UI only - no HUD, just menu screens
        FInputModeUIOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}

// -------------------------
// -- BASE WIDGET CLASSES --
// -------------------------

void UUIMenuForge_MainMenu::NativeConstruct()
{
    Super::NativeConstruct();
}
void UUIMenuForge_PauseMenu::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUIMenuForge_SettingsMenu::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUIMenuForge_SettingsMenu::SetMasterVolume(float Value)
{
    if (!MasterSoundMix || !MasterSoundClass) return;
    UGameplayStatics::SetSoundMixClassOverride(
        this,
        MasterSoundMix,
        MasterSoundClass,
        Value,
        1.0f,
        0.0f
    );
    UGameplayStatics::PushSoundMixModifier(this, MasterSoundMix);
}

void UUIMenuForge_SettingsMenu::SetMusicVolume(float Value)
{
    if (!MasterSoundMix || !MusicSoundClass) return;
    UGameplayStatics::SetSoundMixClassOverride(
        this,
        MasterSoundMix,
        MusicSoundClass,
        Value,
        1.0f,
        0.0f
    );
    UGameplayStatics::PushSoundMixModifier(this, MasterSoundMix);
}

void UUIMenuForge_SettingsMenu::SetSFXVolume(float Value)
{
    if (!MasterSoundMix || !SFXSoundClass) return;
    UGameplayStatics::SetSoundMixClassOverride(
        this,
        MasterSoundMix,
        SFXSoundClass,
        Value,
        1.0f,
        0.0f
    );
    UGameplayStatics::PushSoundMixModifier(this, MasterSoundMix);
}

void UUIMenuForge_SettingsMenu::SetFullscreen(bool bFullscreen)
{
    UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
    if (!Settings) return;

    Settings->SetFullscreenMode(bFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
    Settings->ApplySettings(false);
}

void UUIMenuForge_HUD::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUIMenuForge_LoadingScreen::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUIMenuForge_Credits::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUIMenuForge_DeathScreen::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUIMenuForge_CustomScreen::NativeConstruct()
{
    Super::NativeConstruct();
}

#undef LOCTEXT_NAMESPACE