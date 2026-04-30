// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoMaterialSetup.h"
#include "LevelEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "MaterialEditingLibrary.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "AssetRegistry/AssetData.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"

#define LOCTEXT_NAMESPACE "FAutoMaterialSetupModule"
IMPLEMENT_MODULE(FAutoMaterialSetupModule, AutoMaterialSetup)

// ------------
// -- MODULE --
// ------------

void FAutoMaterialSetupModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAutoMaterialSetupModule::RegisterMenus));
}
void FAutoMaterialSetupModule::RegisterMenus()
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
                FNewToolMenuDelegate::CreateRaw(this, &FAutoMaterialSetupModule::FillMenu)
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
                    FAutoMaterialSetupModule::FillMenu(Menu);
                }
            );
        }
    }
}
void FAutoMaterialSetupModule::FillMenu(UToolMenu* Menu)
{
    // CATEGORY: "3D Art"
    if (!Menu->ContainsSection("OliwiaDevTools_3DArt"))
    {
        Menu->AddSection(
            "OliwiaDevTools_3DArt",
            LOCTEXT("3DArtSection_Label", "3D Art")
        );
    }

    FToolMenuSection* Section = Menu->FindSection("OliwiaDevTools_3DArt");
    if (!Section) return;

    Section->AddMenuEntry(
        "AutoMaterialSetup",
        LOCTEXT("AutoMaterialBtn_Label", "Auto Material Setup"),
        LOCTEXT("AutoMaterialBtn_Tooltip", "Opens the Auto Material Setup utility widget"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FAutoMaterialSetupModule::TriggerAutoMaterialSetup))
    );
}
void FAutoMaterialSetupModule::TriggerAutoMaterialSetup()
{
    FString WidgetPath = TEXT("/AutoMaterialSetup/UI/EUW_AutoMaterialSetup.EUW_AutoMaterialSetup");
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
void FAutoMaterialSetupModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC --
// ----------------

// ----------------------------------------
// -- RESOLVE SELECTION DATA --
// Handles all four selection scenarios: 
// 1. Folder + Mesh selected 
// 2. Individual textures + Mesh selected
// 3. Folder selected, no mesh
// 4. Individual textures selected, no mesh
// -----------------------------------------

void UAutoMaterialSetupLibrary::ResolveSelectionData(
    const TArray<FAssetData>& SelectedAssets,
    FString& OutFolderPath,
    UStaticMesh*& OutStaticMesh,
    FString& OutMaterialName)
{
	OutFolderPath = FString();
	OutStaticMesh = nullptr;
	OutMaterialName = TEXT("M_Material"); // Default fallback name
    for (const FAssetData& Asset : SelectedAssets)
    {
        // -- Derive folder path from first Texture2D found --
		// This works whether textures are selected individually or as a folder, as long as at least one texture is selected
        if (Asset.AssetClassPath == FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("Texture2D")))
        {
            if (OutFolderPath.IsEmpty())
            {
                OutFolderPath = Asset.PackagePath.ToString();
			}
        }
        // -- Find Static Mesh for name and assignment --
        // If found, overrides default material name with M_MeshName
        if (Asset.AssetClassPath == FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("StaticMesh")))
        {
            OutStaticMesh = Cast<UStaticMesh>(Asset.GetAsset());
            if (OutStaticMesh)
            {
                OutMaterialName = TEXT("M_") + Asset.AssetName.ToString();
            }
        }
    }
    // -- Fallback: if no textures found in selection, try selected folder from content browser --
	// Handles case where user selects folder icon rather than individual textures
    if (OutFolderPath.IsEmpty())
    {
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();
		TArray<FString> SelectedFolders;
		ContentBrowser.GetSelectedPathViewFolders(SelectedFolders);
        if (SelectedFolders.Num() > 0)
        {
            FString Folder = SelectedFolders[0];
            // Strip /All/ prefix if present - added by UE internally for virtual paths
            if (Folder.StartsWith(TEXT("/All/")))
            {
                Folder = Folder.Mid(4);
            }
            OutFolderPath = Folder;
        }
	}
}

// ------------------------------------------------------------------------------
// -- SCAN FOLDER FOR TEXTURES --
// Searches content browser folder for textures and matches user-defined suffixes
// Returns a struct with all found textures assigned
// ------------------------------------------------------------------------------

FAutoMaterialTextureSet UAutoMaterialSetupLibrary::ScanFolderForTextures(
    const FString& FolderPath,
    const FString& BaseColorSuffix,
    const FString& NormalSuffix,
    const FString& RoughnessSuffix,
    const FString& MetallicSuffix,
    const FString& AOSuffix,
    const FString& EmissiveSuffix,
    const FString& OpacitySuffix,
    const FString& HeightSuffix)
{
    FAutoMaterialTextureSet TextureSet;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssetsByPath(FName(*FolderPath), AssetList, true);
    for (const FAssetData& Asset : AssetList)
    {
        if (Asset.AssetClassPath != FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("Texture2D"))) continue;
            FString AssetName = Asset.AssetName.ToString();
            UE_LOG(LogTemp, Warning, TEXT("AutoMaterialSetup: Checking texture %s against BaseColor suffix %s"), *AssetName, *BaseColorSuffix);
            if (!BaseColorSuffix.IsEmpty() && AssetName.Contains(BaseColorSuffix))
				TextureSet.BaseColor = Cast<UTexture2D>(Asset.GetAsset());
			else if (!NormalSuffix.IsEmpty() && AssetName.Contains(NormalSuffix))
                TextureSet.Normal = Cast<UTexture2D>(Asset.GetAsset());
            else if (!RoughnessSuffix.IsEmpty() && AssetName.Contains(RoughnessSuffix))
                TextureSet.Roughness = Cast<UTexture2D>(Asset.GetAsset());
            else if (!MetallicSuffix.IsEmpty() && AssetName.Contains(MetallicSuffix))
                TextureSet.Metallic = Cast<UTexture2D>(Asset.GetAsset());
            else if (!AOSuffix.IsEmpty() && AssetName.Contains(AOSuffix))
                TextureSet.AO = Cast<UTexture2D>(Asset.GetAsset());
            else if (!EmissiveSuffix.IsEmpty() && AssetName.Contains(EmissiveSuffix))
                TextureSet.Emissive = Cast<UTexture2D>(Asset.GetAsset());
            else if (!OpacitySuffix.IsEmpty() && AssetName.Contains(OpacitySuffix))
                TextureSet.Opacity = Cast<UTexture2D>(Asset.GetAsset());
            else if (!HeightSuffix.IsEmpty() && AssetName.Contains(HeightSuffix))
				TextureSet.Height = Cast<UTexture2D>(Asset.GetAsset());
    }
    return TextureSet;
}

// ----------------------------------
// -- GENERATE PBR MATERIAL --
// Target Mesh assignment is optional
// ----------------------------------

UMaterial* UAutoMaterialSetupLibrary::GeneratePBRMaterial(
    const FAutoMaterialTextureSet& TextureSet,
    const FString& MaterialName,
    const FString& OutputPath,
    UStaticMesh* TargetMesh)
{
    // -- Validate and resolve output path --
    FString FinalOutputPath = OutputPath.IsEmpty() ? TEXT("/Game/Materials") : OutputPath;
    // Strip trailing slash if present
	if (FinalOutputPath.EndsWith(TEXT("/")))
    {
        FinalOutputPath.RemoveFromEnd(TEXT("/"));
	}
	// Ensure path starts with /Game/ if it doesn't start with /
    if (!FinalOutputPath.StartsWith(TEXT("/")))
    {
        FinalOutputPath = TEXT("/Game/") + FinalOutputPath;
    }
	FString PackagePath = FinalOutputPath / MaterialName;
    // -- Create material package --
    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package) return nullptr;
    // -- Create material asset --
    UMaterial* NewMaterial = NewObject<UMaterial>(Package, *MaterialName, RF_Public | RF_Standalone);
    if (!NewMaterial) return nullptr;
    // -- UV Tiling scalar parameter --
    UMaterialExpressionScalarParameter* UVTiling = Cast<UMaterialExpressionScalarParameter>(UMaterialEditingLibrary::CreateMaterialExpression(NewMaterial, UMaterialExpressionScalarParameter::StaticClass()));
    UVTiling->ParameterName = FName("UV Tiling");
    UVTiling->DefaultValue = 1.0f;
    UVTiling->MaterialExpressionEditorX = -800;
    UVTiling->MaterialExpressionEditorY = 0;
    // -- Texture Coordinate node --
    UMaterialExpressionTextureCoordinate* TexCoord = Cast<UMaterialExpressionTextureCoordinate>(UMaterialEditingLibrary::CreateMaterialExpression(NewMaterial, UMaterialExpressionTextureCoordinate::StaticClass()));
    TexCoord->MaterialExpressionEditorX = -600;
    TexCoord->MaterialExpressionEditorY = 0;
    // -- Helper lambda to create texture sample nodes --
    auto AddTextureSample = [&](UTexture2D* Texture, int32 posY, bool bIsNormal) -> UMaterialExpressionTextureSample*
        {
            if (!Texture) return nullptr;
            UMaterialExpressionTextureSample* TexSample = Cast<UMaterialExpressionTextureSample>(UMaterialEditingLibrary::CreateMaterialExpression(NewMaterial, UMaterialExpressionTextureSample::StaticClass()));
            TexSample->Texture = Texture;
            TexSample->MaterialExpressionEditorX = -400;
            TexSample->MaterialExpressionEditorY = posY;
            // Set color space 
            if (bIsNormal)
            {
                Texture->CompressionSettings = TC_Normalmap;
                Texture->SRGB = false;
                Texture->UpdateResource();
                TexSample->SamplerType = SAMPLERTYPE_Normal;
            }
            else if (Texture != TextureSet.BaseColor)
            {
                // All non-BaseColor, non-Normal textures are linear 
                Texture->SRGB = false;
                Texture->UpdateResource();
                TexSample->SamplerType = SAMPLERTYPE_LinearColor;
            }
            // Wire TexCoord into UV input 
            TexSample->Coordinates.Expression = TexCoord;
            return TexSample;
        };
    // -- Create texture sample nodes --
    UMaterialExpressionTextureSample* BaseColorNode = AddTextureSample(TextureSet.BaseColor, 0, false);
    UMaterialExpressionTextureSample* NormalNode = AddTextureSample(TextureSet.Normal, 200, true);
    UMaterialExpressionTextureSample* RoughnessNode = AddTextureSample(TextureSet.Roughness, 400, false);
    UMaterialExpressionTextureSample* MetallicNode = AddTextureSample(TextureSet.Metallic, 600, false);
    UMaterialExpressionTextureSample* AONode = AddTextureSample(TextureSet.AO, 800, false);
    UMaterialExpressionTextureSample* EmissiveNode = AddTextureSample(TextureSet.Emissive, 1000, false);
    UMaterialExpressionTextureSample* OpacityNode = AddTextureSample(TextureSet.Opacity, 1200, false);
    UMaterialExpressionTextureSample* HeightNode = AddTextureSample(TextureSet.Height, 1400, false);
    // -- Wire nodes into material outputs --
    if (BaseColorNode)
        NewMaterial->GetExpressionInputForProperty(MP_BaseColor)->Expression = BaseColorNode;
    if (NormalNode)
        NewMaterial->GetExpressionInputForProperty(MP_Normal)->Expression = NormalNode;
    if (RoughnessNode)
    {
        FExpressionInput* Input = NewMaterial->GetExpressionInputForProperty(MP_Roughness);
        Input->Expression = RoughnessNode;
        Input->OutputIndex = 3; // R channel
    }
    if (MetallicNode)
    {
        FExpressionInput* Input = NewMaterial->GetExpressionInputForProperty(MP_Metallic);
        Input->Expression = MetallicNode;
        Input->OutputIndex = 3; // R channel
    }
    if (AONode)
    {
        FExpressionInput* Input = NewMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion);
        Input->Expression = AONode;
        Input->OutputIndex = 3; // R channel
    }
    if (EmissiveNode)
        NewMaterial->GetExpressionInputForProperty(MP_EmissiveColor)->Expression = EmissiveNode;
    if (OpacityNode)
    {
        FExpressionInput* Input = NewMaterial->GetExpressionInputForProperty(MP_Opacity);
        Input->Expression = OpacityNode;
        Input->OutputIndex = 3; // R channel
    }
    if (HeightNode)
    {
        FExpressionInput* Input = NewMaterial->GetExpressionInputForProperty(MP_WorldPositionOffset);
        Input->Expression = HeightNode;
        Input->OutputIndex = 3; // R channel
    }
    // -- Compile and save material asset --
    NewMaterial->PreEditChange(nullptr);
    NewMaterial->PostEditChange();
    UMaterialEditingLibrary::RecompileMaterial(NewMaterial);
    FAssetRegistryModule::AssetCreated(NewMaterial);
    Package->MarkPackageDirty();
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    UPackage::SavePackage(Package, NewMaterial, *PackageFileName, SaveArgs);
    // -- Assign material to target mesh if provided --
    // Material is always created regardless
    if (TargetMesh)
    {
        TargetMesh->SetMaterial(0, NewMaterial);
        TargetMesh->MarkPackageDirty();
    }
    return NewMaterial;
}

#undef LOCTEXT_NAMESPACE