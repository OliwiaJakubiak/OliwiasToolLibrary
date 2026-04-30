// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuickLightingKit.h" 
#include "EngineUtils.h" 
#include "Engine/DirectionalLight.h" 
#include "Engine/SkyLight.h" 
#include "Components/LightComponent.h" 
#include "Components/SkyLightComponent.h" 
#include "Engine/ExponentialHeightFog.h" 
#include "Components/ExponentialHeightFogComponent.h" 
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FQuickLightingKitModule"
IMPLEMENT_MODULE(FQuickLightingKitModule, QuickLightingKit)

// ------------
// -- MODULE --
// ------------
void FQuickLightingKitModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("Oliwia's DevTools: QuickLightingKit module started"));
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FQuickLightingKitModule::RegisterMenus)
	);
}
void FQuickLightingKitModule::RegisterMenus()
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
				FNewToolMenuDelegate::CreateRaw(this, &FQuickLightingKitModule::FillMenu)
			);
			UE_LOG(LogTemp, Warning, TEXT("Oliwia's DevTools: Menu created by QuickLightingKit"));
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
					FQuickLightingKitModule::FillMenu(Menu);
				}
			);
			UE_LOG(LogTemp, Warning, TEXT("Oliwia's DevTools: QuickLightingKit added onto existing menu"));
		}
	}
}

void FQuickLightingKitModule::FillMenu(UToolMenu* Menu)
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
		"QuickLightingKitEntry",
		LOCTEXT("QuickLightinKitEntry_Label", "Quick Lighting Kit"),
		LOCTEXT("QuickLightingKitEntry_Tooltip", "Opens the Quick Lighting Kit widget"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FQuickLightingKitModule::TriggerQuickLightingKit))
	);
	UE_LOG(LogTemp, Warning, TEXT("Oliwia's DevTools: QuickLightingKit entry added"));
}
void FQuickLightingKitModule::TriggerQuickLightingKit()
{
	FString WidgetPath = TEXT("/QuickLightingKit/UI/EUW_LightingKit.EUW_LightingKit");
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
void FQuickLightingKitModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC -- 
// ----------------

// ----------------------------------------------------------------------------------------
// -- EXECUTE LIGHTING UPDATE --
// Finds all Directional lights, sky lights and exponential height fog actors in the level
// Updates their properties with the provided values
// Also exposed to blueprint for custom presets 
// ----------------------------------------------------------------------------------------

void UQuickLightingKit::ExecuteLightingUpdate(
	const UObject* WorldContextObject, 
	float SunPitch, 
	FLinearColor SunColor, 
	float SunIntensity, 
	float SkyIntensity, 
	float FogDensity)
{
	if (!WorldContextObject) return;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return;
	// -- Update all Directional lights -- 
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		It->SetActorRotation(FRotator(SunPitch, 0.0f, 0.0f));
		if (ULightComponent* LightComp = It->GetLightComponent())
		{
			LightComp->SetLightColor(SunColor);
			LightComp->SetIntensity(SunIntensity);
		}
	}
	// -- Update all Sky lights -- 
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		if (USkyLightComponent* SkyComp = It->GetComponentByClass<USkyLightComponent>())
		{
			SkyComp->SetIntensity(SkyIntensity);
			SkyComp->RecaptureSky(); 
		}
	}
	// -- Update all Exponential height fog actors -- 
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		if (UExponentialHeightFogComponent* FogComp = It->GetComponentByClass<UExponentialHeightFogComponent>())
		{
			FogComp->SetFogDensity(FogDensity);
		}
	}
}
#undef LOCTEXT_NAMESPACE