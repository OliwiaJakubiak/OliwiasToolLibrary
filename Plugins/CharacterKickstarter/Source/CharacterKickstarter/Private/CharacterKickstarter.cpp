// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterKickstarter.h"
#include "LevelEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "FCharacterKickstarterModule"
IMPLEMENT_MODULE(FCharacterKickstarterModule, CharacterKickstarter)

// ------------
// -- MODULE --
// ------------

void FCharacterKickstarterModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCharacterKickstarterModule::RegisterMenus));
}
void FCharacterKickstarterModule::RegisterMenus()
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
				FNewToolMenuDelegate::CreateRaw(this, &FCharacterKickstarterModule::FillMenu)
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
					FCharacterKickstarterModule::FillMenu(Menu);
				}
			);
		}
	}
}
void FCharacterKickstarterModule::FillMenu(UToolMenu* Menu)
{
	// CATEGORY: "Character and Animation"
	if (!Menu->ContainsSection("OliwiaDevTools_CharacterAnimation"))
	{
		Menu->AddSection(
			"OliwiaDevTools_CharacterAnimation",
			LOCTEXT("CharacterSection_Label", "Character and Animation")
		);
	}
	FToolMenuSection* Section = Menu->FindSection("OliwiaDevTools_CharacterAnimation");
	if (!Section) return;
	Section->AddMenuEntry(
		"CharacterKickstarter",
		LOCTEXT("CharacterBtn_Label", "Character Kickstarter"),
		LOCTEXT("CharacterBtn_Tooltip", "Opens the Character Kickstarter utility widget"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FCharacterKickstarterModule::TriggerCharacterKickstarter))
	);
}
void FCharacterKickstarterModule::TriggerCharacterKickstarter()
{
	FString WidgetPath = TEXT("/CharacterKickstarter/UI/EUW_CharacterKickstarter.EUW_CharacterKickstarter");
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
void FCharacterKickstarterModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC --
// ----------------

// -------------------------------------------------------------
// -- BASE CHARACTER CONSTRUCTOR --
// Sets up all components with defaults
// Templates override camera and movement for their perspective
// -------------------------------------------------------------

AKickstarterCharacterBase::AKickstarterCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	// -- SPRING ARM --
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	// -- CAMERA --
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	// -- MOVEMENT DEFAULTS --
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = JumpHeight;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}
void AKickstarterCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyMovementSettings();
}

// ---------------------------------------------------------
// -- APPLY MOVEMENT SETTINGS --
// Syncs movement component with blueprint expose variables 
// Called on BeginPlay so user tweaks take effect at runtime
// ---------------------------------------------------------

void AKickstarterCharacterBase::ApplyMovementSettings()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpHeight;
	GetCharacterMovement()->GravityScale = GravityScale;
}

// ------------------------------------------------------------
// -- MOVEMENT FUNCTIONS --
// All called from blueprint input events 
// logic lives in c++ - user calls these from their input graph
// ------------------------------------------------------------

void AKickstarterCharacterBase::StartSprinting()
{
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}
void AKickstarterCharacterBase::StopSprinting()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
void AKickstarterCharacterBase::StartCrouching()
{
	bIsCrouching = true;
	Crouch();
	GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
}
void AKickstarterCharacterBase::StopCrouching()
{
	bIsCrouching = false;
	UnCrouch();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
void AKickstarterCharacterBase::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}
void AKickstarterCharacterBase::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}
void AKickstarterCharacterBase::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}
void AKickstarterCharacterBase::Turn(float Value)
{
	AddControllerYawInput(Value);
}

// ------------------------------------------------------------
// -- GENERATE CHARACTER TEMPLATE --
// Duplicates pre-built template Blueprint into user's project 
// ------------------------------------------------------------

bool UCharacterKickstarterLibrary::GenerateCharacterTemplate(
	const FString& TemplatePath,
	const FString& OutputPath,
	const FString& CharacterName,
	FString& OutGeneratedPath)
{
	FString FinalOutputPath = OutputPath.IsEmpty() ? TEXT("/Game/Characters") : OutputPath;
	if (FinalOutputPath.EndsWith(TEXT("/")))
		FinalOutputPath.RemoveFromEnd(TEXT("/"));
	FString DestinationPath = FinalOutputPath / CharacterName;
	UObject* DuplicateAsset = UEditorAssetLibrary::DuplicateAsset(TemplatePath, DestinationPath);
	bool bSuccess = (DuplicateAsset != nullptr);
	if (bSuccess)
	{
		UEditorAssetLibrary::SaveAsset(DestinationPath, false);
		UE_LOG(LogTemp, Log, TEXT("CharacterKickstarter: Generated character template at %s"), *DestinationPath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterKickstarter: Failed to generate template at %s"), *DestinationPath);
	}
	OutGeneratedPath = DestinationPath;
	return bSuccess;
}

// ----------------------------------------------------------------------
// -- GENERATE PLAYER CONTROLLER --
// Creates a blank player controller blueprint in the users output folder
// ----------------------------------------------------------------------

bool UCharacterKickstarterLibrary::GeneratePlayerController(
	const FString& OutputPath,
	const FString& ControllerName)
{
	FString FinalOutputPath = OutputPath.IsEmpty() ? TEXT("/Game/Characters") : OutputPath;
	if (FinalOutputPath.EndsWith(TEXT("/")))
		FinalOutputPath.RemoveFromEnd(TEXT("/"));
	FString PackagePath = FinalOutputPath / ControllerName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package) return false;
	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
		APlayerController::StaticClass(),
		Package,
		*ControllerName,
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass()
	);
	if (!NewBP) return false;
	FAssetRegistryModule::AssetCreated(NewBP);
	Package->MarkPackageDirty();
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	FString PackageFilename = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	UPackage::SavePackage(Package, NewBP, *PackageFilename, SaveArgs);
	UE_LOG(LogTemp, Log, TEXT("CharacterKickstarter: Generated Player Controller at %s"), *PackagePath);
	return true;
}

// -------------------------------------------------------------
// -- GENERATE AND SET GAME MODE --
// Creates a Game Mode blueprint in the output folder
// Autosets it in world settings so it takes effect immediately 
// Character and controller paths wired into game mode defaults
// -------------------------------------------------------------

bool UCharacterKickstarterLibrary::GenerateAndSetGameMode(
	const FString& OutputPath,
	const FString& GameModeName,
	const FString& CharacterBlueprintPath,
	const FString& ControllerBlueprintPath)
{
	FString FinalOutputPath = OutputPath.IsEmpty() ? TEXT("/Game/Characters") : OutputPath;
	if (FinalOutputPath.EndsWith(TEXT("/")))
		FinalOutputPath.RemoveFromEnd(TEXT("/"));
	FString PackagePath = FinalOutputPath / GameModeName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package) return false;
	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
		AGameModeBase::StaticClass(),
		Package,
		*GameModeName,
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass()
	);
	if (!NewBP) return false;
	// -- AUTO SET IT WORLD SETTINGS --
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World)
	{
		AWorldSettings* WorldSettings = World->GetWorldSettings();
		if (WorldSettings)
		{
			WorldSettings->DefaultGameMode = NewBP->GeneratedClass;
			WorldSettings->MarkPackageDirty();
		}
	}
	FAssetRegistryModule::AssetCreated(NewBP);
	Package->MarkPackageDirty();
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	FString PackageFilename = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	UPackage::SavePackage(Package, NewBP, *PackageFilename, SaveArgs);
	UE_LOG(LogTemp, Log, TEXT("CharacterKickstarter: Generated Game Mode at %s"), *PackagePath);
	return true;
}
#undef LOCTEXT_NAMESPACE