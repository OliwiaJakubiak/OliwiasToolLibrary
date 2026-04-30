// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "BlockoutToBeautySwapper.generated.h"

// ----------------------
// -- FUNCTION LIBRARY --
// Core swap logic callable directly from EUW button OnClicked events
// All functions return output parameters for UI state updates 
// ----------------------

UCLASS()
class BLOCKOUTTOBEAUTYSWAPPER_API UBlockoutSwapperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Returns first selected actor in level viewport 
	// Returns nullptr if no actor is selected
	// Called directly from BTN_Refresh OnClicked
	UFUNCTION(BlueprintCallable, Category = "BlockoutToBeautySwapper")
	static void RefreshSelection(
		AActor*& OutSourceActor,
		UStaticMesh*& OutReplacementMesh,
		FString& OutSourceActorName,
		FString& OutReplacementMeshName
	);
	// Swaps Static Mesh on a single actor 
	// Preserves transform - position, rotation and scale
	// Returns original mesh so it can be stored for undo
	// Called directly from BTN_Swap OnClicked
	UFUNCTION(BlueprintCallable, Category = "BlockoutToBeautySwapper")
	static void PerformSwap(
		AActor* SourceActor,
		UStaticMesh* ReplacementMesh,
		bool bReplaceAll,
		TArray<AActor*>& OutAffectedActors,
		UStaticMesh*& OutOriginalMesh,
		FString& OutLogEntry
	);
	// Restores OriginalMesh to all actors in provided array
	// Called directly from BTN_Undo OnClicked
	UFUNCTION(BlueprintCallable, Category = "BlockoutToBeautySwapper")
	static void UndoSwap(
		const TArray<AActor*>& AffectedActors,
		UStaticMesh* OriginalMesh,
		FString& OutLogEntry
	);
};

// ------------
// -- MODULE --
// ------------

class FBlockoutToBeautySwapperModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerBlockoutToBeautySwapper();
};
