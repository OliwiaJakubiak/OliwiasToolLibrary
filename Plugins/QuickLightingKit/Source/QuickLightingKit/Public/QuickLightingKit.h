// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h" 
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuickLightingKit.generated.h"

// ----------------------------------------------------------------
// -- FUNCTION LIBRARY --
// Core lighting logic callable directly from EUW button OnClicked 
// Buttons call ApplyLightingPreset with row name 
// ----------------------------------------------------------------

UCLASS()
class QUICKLIGHTINGKIT_API UQuickLightingKit : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Core lighting update - finds and updates all relevant actors in the world 
	// Called internally by ApplyLightingPreset
	// Also exposed to Blueprint for custom presets outside the Data Table 
	UFUNCTION(BlueprintCallable, Category = "QuickLightingKit", meta = (WorldContext = "WorldContextObject"))
	static void ExecuteLightingUpdate(
		const UObject* WorldContextObject,
		float SunPitch,
		FLinearColor SunColor,
		float SunIntensity,
		float SkyIntensity,
		float FogDensity);
};

// ------------
// -- MODULE --
// ------------
class FQuickLightingKitModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private: 
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerQuickLightingKit();
};