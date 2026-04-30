// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "MeshUtilityKit.generated.h"

// -----------------------------------------------------
// -- COLLISION TPYE ENUM --
// Passed from EUW to c++ collision generation function
// Defines which collision shape to apply to the mesh 
// -----------------------------------------------------

UENUM(BlueprintType)
enum class EMeshCollisionType : uint8
{
	Box				UMETA(DisplayName = "Box"),
	Sphere			UMETA(DisplayName = "Sphere"),
	Capsule			UMETA(DisplayName = "Capsule"),
	SimpleAsComplex UMETA(DisplayName = "Simple As Complex"),
	ComplexAsSimple UMETA(DisplayName = "Complex As Simple")
};

// ----------------------------------------------------
// -- FUNCTION LIBRARY -- 
// Core mesh utility logic callable from EUW Blueprint 
// accessible from any Blueprint context
// ----------------------------------------------------

UCLASS()
class MESHUTILITYKIT_API UMeshUtilityKitFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Resize selected static mesh actors in the viewport 
	// Permanently modifies the source asset scale
	UFUNCTION(BlueprintCallable, Category = "MeshUtilityKit")
	static int32 ResizeSelectedMeshes_CPP(
		const TArray<AActor*>& SelectedActors, 
		float TargetSize
	);
	// Resets the pivot point of selected static mesh actors to origin 
	// Permanently modifies vertex positions in the source asset
	// Optionally resets rotation as well 
	UFUNCTION(BlueprintCallable, Category = "MeshUtilityKit")
	static int32 ResetAssetToOrigin_CPP(
		const TArray<AActor*>& SelectedActors, 
		bool bResetRotation
	);
	// Generates collision on selected static mesh actors 
	// Applies the chosen collision type to the source asset
	UFUNCTION(BlueprintCallable, Category = "MeshUtilityKit")
	static int32 GenerateCollisionOnSelectedMeshes_CPP(
		const TArray<AActor*>& SelectedActors, EMeshCollisionType CollisionType
	);
};

// ------------
// -- MODULE --
// ------------

class FMeshUtilityKitModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private: 
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerMeshUtilityKit();
};
