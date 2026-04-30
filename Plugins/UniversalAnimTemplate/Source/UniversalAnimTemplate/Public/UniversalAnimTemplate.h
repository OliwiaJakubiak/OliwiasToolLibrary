// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UniversalAnimTemplate.generated.h"

// -----------------------------------------------------
// -- ANIM STATE FLAGS STRUCT --
// Passed from EUW to c++ generation function 
// Each bool represents a state the user wants included
// Clean single parameter instead of 9 seperate bools
// -----------------------------------------------------

USTRUCT(BlueprintType)
struct FAnimStateSelection
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Locomotion")
	bool bIdle = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Locomotion")
	bool bWalk = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Locomotion")
	bool bRun = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Locomotion")
	bool bSprint = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Action")
	bool bJump = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Action")
	bool bFall = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Action")
	bool bLand = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Action")
	bool bCrouch = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UniversalAnimTemplate|Action")
	bool bDeath = false;
};

// --------------------------------------------------------
// -- FUNCTION LIBRARY -- 
// AnimBP generation logic called directly from EUW button 
// Attempts programmatic state machine generation via 
// UAnimBlueprint and UAnimStateMachineGraph APIs
// --------------------------------------------------------

UCLASS()
class UNIVERSALANIMTEMPLATE_API UUniversalAnimTemplateLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Generates a new AnimBP with only the selected states 
	// wired into a single state machine 
	// OutputPath - user defined or defaults to /Game/Animation/
	// AnimBPName - name for the generated AnimBP asset
	// StateSelection - which states to include 
	// TargetSkeleton - skeleton asset to bind the AnimBP to
	// Returns true if generation succeeded 
	UFUNCTION(BlueprintCallable, Category = "UniversalAnimTemplate")
	static bool GenerateAnimBP(
		const FString& OutputPath,
		const FString& AnimBPName,
		const FAnimStateSelection& StateSelection,
		USkeleton* TargetSkeleton
	);
};

// ------------
// -- MODULE --
// ------------

class FUniversalAnimTemplateModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private: 
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerUniversalAnimTemplate();
};
