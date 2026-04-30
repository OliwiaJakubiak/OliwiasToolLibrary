// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CharacterKickstarter.generated.h"

// ----------------------------------------------------------------
// -- BASE CHARACTER CLASS --	
// Character base with all boilerplate wired up
// All movement values expose as EditAnywhere for easy tweaking
// Supports First Person, Third Person, Top Down, and Side Scroller
// ----------------------------------------------------------------

UCLASS(Blueprintable, BlueprintType)
class CHARACTERKICKSTARTER_API AKickstarterCharacterBase : public ACharacter
{
	GENERATED_BODY()
public:
	AKickstarterCharacterBase();
	// -- COMPONENTS --
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kickstarter|Camera")
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kickstarter|Camera")
	UCameraComponent* FollowCamera;
	// -- MOVEMENT --
	// These are all exposed so user can adjust in blueprint details panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kickstarter|Movement")
	float WalkSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kickstarter|Movement")
	float RunSpeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kickstarter|Movement")
	float SprintSpeed = 900.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kickstarter|Movement")
	float CrouchSpeed = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kickstarter|Movement")
	float JumpHeight = 420.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kickstarter|Movement")
	float GravityScale = 1.75f;
	// -- STATE --
	// Read only in blueprint - set internally by movement functions
	UPROPERTY(BlueprintReadOnly, Category = "Kickstarter|State")
	bool bIsSprinting = false;
	UPROPERTY(BlueprintReadOnly, Category = "Kickstarter|State")
	bool bIsCrouching = false;
	// -- MOVEMENT FUNCTIONS --
	// Called from blueprint  input events 
	// all logic handled in c++ - user never needs to rewrite these
	UFUNCTION(BlueprintCallable, Category = "Kickstarter|Movement")
	void StartSprinting();
	UFUNCTION(BlueprintCallable, Category = "Kickstarter|Movement")
	void StopSprinting();
	UFUNCTION(BlueprintCallable, Category = "Kickstarter|Movement")
	void StartCrouching();
	UFUNCTION(BlueprintCallable, Category = "Kickstarter|Movement")
	void StopCrouching();
	UFUNCTION(BlueprintCallable, Category = "Kickstarter|Movement")
	void MoveForward(float Value);
	UFUNCTION(BlueprintCallable, Category = "Kickstarter|Movement")
	void MoveRight(float Value);
	UFUNCTION(BlueprintCallable, Category = "Kickstarter|Movement")
	void LookUp(float Value);
	UFUNCTION(BlueprintCallable, Category = "Kickstarter|Movement")
	void Turn(float Value);
protected:
	virtual void BeginPlay() override;
private:
	void ApplyMovementSettings();
};

// -------------------------------------------------------------------------------------
// -- FUNCTION LIBRARY --
// EUW generation logic - called directly from button OnClicked
// duplicates pre-built templates into users project rather than generating procedurally
// -------------------------------------------------------------------------------------

UCLASS()
class CHARACTERKICKSTARTER_API UCharacterKickstarterLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Duplicates chosen character template into users output folder 
	// TemplatePath - path to template inside plugin /Content/Templates/
	// OutputPath - user defined or defaults to /Game/Characters/
	// CharacterName - name for generated Blueprint
	UFUNCTION(BlueprintCallable, Category = "CharacterKickstarter")
	static bool GenerateCharacterTemplate(
		const FString& TemplatePath,
		const FString& OutputPath,
		const FString& CharacterName,
		FString& OutGeneratedPath);
	// Generates a Player Controller Blueprint in the output folder
	UFUNCTION(BlueprintCallable, Category = "CharacterKickstarter")
	static bool GeneratePlayerController(
		const FString& OutputPath,
		const FString& ControllerName);
	// Generates a Game Mode Blueprint in the output folder and auto-sets it in World Settings
	UFUNCTION(BlueprintCallable, Category = "CharacterKickstarter")
	static bool GenerateAndSetGameMode(
		const FString& OutputPath,
		const FString& GameModeName,
		const FString& CharacterBlueprintPath,
		const FString& ControllerBlueprintPath);
};

// ------------
// -- MODULE --
// ------------ 


class FCharacterKickstarterModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerCharacterKickstarter();
};
