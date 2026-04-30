// Copyright Epic Games, Inc. All Rights Reserved.

#include "MeshUtilityKit.h"
#include "LevelEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ToolMenus.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshAttributes.h"
#include "MeshDescription.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FMeshUtilityKitModule"
IMPLEMENT_MODULE(FMeshUtilityKitModule, MeshUtilityKit)

// ------------
// -- MODULE --
// ------------

void FMeshUtilityKitModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMeshUtilityKitModule::RegisterMenus));
}
void FMeshUtilityKitModule::RegisterMenus()
{
	if (!UToolMenus::IsToolMenuUIEnabled()) return;

	TArray < FName> MenuTargets = {
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
				FNewToolMenuDelegate::CreateRaw(this, &FMeshUtilityKitModule::FillMenu)
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
					FMeshUtilityKitModule::FillMenu(Menu);
				}
			);
		}
	}
}
void FMeshUtilityKitModule::FillMenu(UToolMenu* Menu)
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
		"MeshUtilityKit",
		LOCTEXT("MeshUtilityKitBtn_Label", "Mesh Utility Kit"),
		LOCTEXT("MeshUtilityKitBtn_Tooltip", "Opens the mesh utility widget"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FMeshUtilityKitModule::TriggerMeshUtilityKit))
	);
}
void FMeshUtilityKitModule::TriggerMeshUtilityKit()
{
	FString WidgetPath = TEXT("/MeshUtilityKit/UI/EUW_MeshUtilityKit.EUW_MeshUtilityKit");
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
void FMeshUtilityKitModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC -- 
// ----------------

// ---------------------------------------------------------
// -- RESIZE --
// Resizes selected static mesh actors to a target size 
// Applies uniform scale based on largest bounding box axis 
// Permanently modifies the source asset
// ---------------------------------------------------------

int32 UMeshUtilityKitFunctionLibrary::ResizeSelectedMeshes_CPP(
	const TArray<AActor*>& SelectedActors, 
	float TargetSize)
{
	int32 SuccessCount = 0;
	for (AActor* Actor : SelectedActors)
	{
		if (!Actor)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: Null actor found, skipping"));
			continue;
		}
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
		if (!StaticMeshActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: Actor %s is not a StaticMeshActor, skipping"), *Actor->GetName());
			continue;
		}
		UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
		if (!MeshComp)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: No StaticMeshComponent found on actor %s"), *Actor->GetName());
			continue;
		}
		UStaticMesh* StaticMesh = MeshComp->GetStaticMesh();
		if (!StaticMesh) continue;
		// -- Calculate scale factor from bounding box -- 
		FBox MeshBox = MeshComp->CalcBounds(FTransform::Identity).GetBox();
		float CurrentSize = MeshBox.GetSize().GetMax();
		if (CurrentSize <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: Invalid bounds on actor %s, skipping"), *Actor->GetName());
			continue;
		}
		float ScaleFactor = TargetSize / CurrentSize;
		if (ScaleFactor <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: Invalid scale factor on actor %s, skipping"), *Actor->GetName());
			continue;
		}
		Actor->SetActorScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor));
		StaticMesh->Modify();
		StaticMesh->MarkPackageDirty();
		SuccessCount++;
	}
	UE_LOG(LogTemp, Log, TEXT("MeshUtilityKit: Resize %d meshes"), SuccessCount);
	return SuccessCount;
}

// ---------------------------------------------------------
// -- RESET ASSET TO ORIGIN --
// Recentres mesh vertex data around world origin 
// Permanently modifies vertex positions in source asset 
// Compensates actor position so it srays visually in place 
// Optionally resets rotation 
// ---------------------------------------------------------

int32 UMeshUtilityKitFunctionLibrary::ResetAssetToOrigin_CPP(
	const TArray<AActor*>& SelectedActors, 
	bool bResetRotation)
{
	int32 SuccessCount = 0;
	for (AActor* Actor : SelectedActors)
	{
		if (!Actor)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: Null actor found, skipping"));
			continue;
		}
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
		if (!StaticMeshActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: Actor %s is not a StaticMeshActor, skipping"), *Actor->GetName());
			continue;
		}
		UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
		if (!MeshComp)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: No StaticMeshComponent found on actor %s"), *Actor->GetName());
			continue;
		}
		UStaticMesh* StaticMesh = MeshComp->GetStaticMesh();
		if (!StaticMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: No StaticMesh found on actor %s"), *Actor->GetName());
			continue;
		}
		FMeshDescription* MeshDescription = StaticMesh->GetMeshDescription(0);
		if (!MeshDescription)
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: No MeshDescription found on actor %s"), *Actor->GetName());
			continue;
		}
		// -- Calculate bounding box centre in local space -- 
		FBox LocalBox = MeshComp->CalcBounds(FTransform::Identity).GetBox();
		FVector Centre = LocalBox.GetCenter();
		if (Centre.IsNearlyZero(0.1f))
		{
			UE_LOG(LogTemp, Warning, TEXT("MeshUtilityKit: Actor %s is already at origin, skipping"), *Actor->GetName());
			continue;
		}
		// -- Shifts all vertices by negative centre offset --
		// Recentres mesh data around its own origin permanently 
		TVertexAttributesRef<FVector3f> VertexPositions =
			MeshDescription->VertexAttributes().GetAttributesRef<FVector3f>(MeshAttribute::Vertex::Position);
		FVector3f Offset = FVector3f(-Centre.X, -Centre.Y, -Centre.Z);
		for (FVertexID VertexID : MeshDescription->Vertices().GetElementIDs())
		{
			VertexPositions[VertexID] += Offset;
		}
		StaticMesh->Modify();
		StaticMesh->CommitMeshDescription(0);
		StaticMesh->PostEditChange();
		// -- Compensate actor position so it stays visually in place -- 
		Actor->SetActorLocation(Actor->GetActorLocation() + Centre);
		if (bResetRotation)
		{
			Actor->SetActorRotation(FRotator::ZeroRotator);
		}
		Actor->ReregisterAllComponents();
		GEditor->RedrawAllViewports();
		StaticMesh->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("MeshUtilityKit: Reset asset %s to origin"), *Actor->GetName());
		SuccessCount++;
	}
	UE_LOG(LogTemp, Log, TEXT("MeshUtilityKit: Reset %d assets to origin"), SuccessCount);
	return SuccessCount;
}

// ----------------------------------------------------------------
// -- GENERATE COLLISION ON SELECTED MESHES --
// Applies chosen collision type to selected static mesh actors 
// Clears existing collision before applying new type 
// Supports Box, Sphere, Capsule, SimpleAsComplex, ComplexAsSimple
// ----------------------------------------------------------------

int32 UMeshUtilityKitFunctionLibrary::GenerateCollisionOnSelectedMeshes_CPP(
	const TArray<AActor*>& SelectedActors, 
	EMeshCollisionType CollisionType)
{
	int32 SuccessCount = 0;
	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
		if (!StaticMeshActor) continue;
		UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
		if (!MeshComp) continue;
		UStaticMesh* StaticMesh = MeshComp->GetStaticMesh();
		if (!StaticMesh) continue;
		UBodySetup* BodySetup = StaticMesh->GetBodySetup();
		if (!BodySetup) continue;
		StaticMesh->Modify();
		// -- Clear existing collision before applying new type --
		BodySetup->Modify();
		BodySetup->RemoveSimpleCollision();
		if (CollisionType == EMeshCollisionType::SimpleAsComplex)
		{
			BodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
		}
		else if (CollisionType == EMeshCollisionType::ComplexAsSimple)
		{
			BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
		}
		else
		{
			BodySetup->CollisionTraceFlag = CTF_UseDefault;
			FKAggregateGeom& AggGeom = BodySetup->AggGeom;
			if (CollisionType == EMeshCollisionType::Box)
			{
				FKBoxElem BoxElem;
				FBox MeshBox = StaticMesh->GetBoundingBox();
				FVector Extent = MeshBox.GetExtent();
				FVector Centre = MeshBox.GetCenter();
				BoxElem.X = Extent.X * 2.0f;
				BoxElem.Y = Extent.Y * 2.0f;
				BoxElem.Z = Extent.Z * 2.0f;
				BoxElem.Center = Centre;
				AggGeom.BoxElems.Add(BoxElem);
			}
			else if (CollisionType == EMeshCollisionType::Sphere)
			{
				FKSphereElem SphereElem;
				FBox MeshBox = StaticMesh->GetBoundingBox();
				SphereElem.Radius = MeshBox.GetExtent().GetMax();
				SphereElem.Center = MeshBox.GetCenter();
				AggGeom.SphereElems.Add(SphereElem);
			}
			else if (CollisionType == EMeshCollisionType::Capsule)
			{
				FKSphylElem CapsuleElem;
				FBox MeshBox = StaticMesh->GetBoundingBox();
				FVector Extent = MeshBox.GetExtent();
				CapsuleElem.Radius = FMath::Max(Extent.X, Extent.Y);
				CapsuleElem.Length = Extent.Z * 2.0f;
				CapsuleElem.Center = MeshBox.GetCenter();
				AggGeom.SphylElems.Add(CapsuleElem);
			}
		}
		// -- Rebuild and refresh --
		StaticMesh->CreateBodySetup();
		StaticMesh->PostEditChange();
		MeshComp->RecreatePhysicsState();
		StaticMesh->MarkPackageDirty();
		SuccessCount++;
	}
	UE_LOG(LogTemp, Log, TEXT("MeshUtilityKit: Generated collision on %d meshes"), SuccessCount);
	return SuccessCount;
}
#undef LOCTEXT_NAMESPACE