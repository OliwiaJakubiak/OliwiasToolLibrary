// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "AssetRegistry/AssetData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Engine/Texture2D.h"
#include "AutoMaterialSetup.generated.h"

// -----------------------------------------------------
// -- TEXTURE MAP STRUCT --
// Holds all detected textures for a single PBR material
// Passed from EUW to c++ for generation function 
// -----------------------------------------------------

USTRUCT(BlueprintType)
struct FAutoMaterialTextureSet
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMaterialSetup")
	UTexture2D* BaseColor = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMaterialSetup")
	UTexture2D* Normal = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMaterialSetup")
	UTexture2D* Roughness = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMaterialSetup")
	UTexture2D* Metallic = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMaterialSetup")
	UTexture2D* AO = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMaterialSetup")
	UTexture2D* Emissive = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMaterialSetup")
	UTexture2D* Opacity = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoMaterialSetup")
	UTexture2D* Height = nullptr;
};

// ----------------------------------------------
// -- FUNCTION LIBRARY --
// All core logic callable from the EUW Blueprint
// ----------------------------------------------

UCLASS()
class AUTOMATERIALSETUP_API UAutoMaterialSetupLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Accepts raw FAssetData array directly from Blueprints Get Selected Assets node
	// Handles both folder and file selection, returns a struct with all detected textures
	// Derives folder path from Texture2D PackagePath 
	// Returns mesh if found in selection, otherwise nullptr
	UFUNCTION(BlueprintCallable, Category = "AutoMaterialSetup")
	static void ResolveSelectionData(
		const TArray<FAssetData>& SelectedAssets,
		FString& OutFolderPath,
		UStaticMesh*& OutStaticMesh,
		FString& OutMaterialName
		);
	// Scans a content browser folder and matches textures to PBR slots
	// checks if asset names contain the provided suffix string
	UFUNCTION(BlueprintCallable, Category = "AutoMaterialSetup")
	static FAutoMaterialTextureSet ScanFolderForTextures(
		const FString& FolderPath,
		const FString& BaseColorSuffix,
		const FString& NormalSuffix,
		const FString& RoughnessSuffix,
		const FString& MetallicSuffix,
		const FString& AOSuffix,
		const FString& EmissiveSuffix,
		const FString& OpacitySuffix,
		const FString& HeightSuffix
	);
	// Creates a full PBR material with correctly configured nodes
	// Saves to OutputPath, assigns to TargetMesh if provided
	// Returns the created material or nullptr on failure
	UFUNCTION(BlueprintCallable, Category = "AutoMaterialSetup")
	static UMaterial* GeneratePBRMaterial(
		const FAutoMaterialTextureSet& TextureSet,
		const FString& MaterialName,
		const FString& OutputPath,
		UStaticMesh* TargetMesh
	);
};

// ------------
// -- MODULE --
// ------------

class FAutoMaterialSetupModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerAutoMaterialSetup();
};
