// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetRegistry/AssetData.h"
#include "AssetOrganiser.generated.h"

// ------------------------------------------
// -- ORGANISER RULE STRUCT --
// Passed from EUW to c++ core logic 
// Defines folder and prefix per asset class
// ------------------------------------------

USTRUCT(BlueprintType)
struct FAssetOrganiserRule
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FolderName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Prefix;
};

// ----------------------------------------------------
// -- FUNCTION LIBRARY --
// Core organisation logic callable from EUW Blueprint 
// accessible from any Blueprint context
// ----------------------------------------------------

UCLASS()
class ASSETORGANISER_API UAssetOrganiserFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Core organisation function 
	// Takes selected assets and rules, organise and/or renames based on flags
	// bOrganise - move assets into correct fodlers 
	// bRename - apply prefix to asset names 
	// bIsDryRun - log changes without performing them 
	// Returns number of successfully processed assets
	UFUNCTION(BlueprintCallable, Category = "AssetOrganiser")
	static int32 BatchOrganiseAssets_CPP(
		const TArray<FAssetData>& SelectedAssets, 
		const TMap<UClass*, FAssetOrganiserRule>& OrganiserRules, 
		bool bIsDryRun,
		bool bOrganise,
		bool bRename
	);
};

// ------------
// -- MODULE --
// ------------

class FAssetOrganiserModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void RegisterMenus(); 
	void FillMenu(UToolMenu* Menu); 
	void TriggerAssetOrganiser(); 
};