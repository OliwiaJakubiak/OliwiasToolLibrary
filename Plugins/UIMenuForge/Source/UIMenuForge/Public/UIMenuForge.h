// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIMenuForge.generated.h"

// --------------------------
// -- UI MANAGER SUBSYSTEM -- 
// --------------------------

UENUM(BlueprintType)
enum class EScreenType : uint8
{
	MainMenu		UMETA(DisplayName = "Main Menu"),
	PauseMenu		UMETA(DisplayName = "Pause Menu"),
	SettingsMenu	UMETA(DisplayName = "Settings Menu"),
	HUD				UMETA(DisplayName = "HUD"),
	LoadingScreen	UMETA(DisplayName = "Loading Screen"),
	Credits			UMETA(DisplayName = "Credits"),
	DeathScreen		UMETA(DisplayName = "Death Screen"),
	Custom			UMETA(DisplayName = "Custom")
};

UCLASS()
class UIMENUFORGE_API UUIMenuForge_Manager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public: 
	
	UFUNCTION(BlueprintCallable, Category = "UIMenuForge|Manager")
	void ShowScreen(EScreenType ScreenType, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "UIMenuForge|Manager")
	void HideScreen(EScreenType ScreenType);

	UFUNCTION(BlueprintCallable, Category = "UIMenuForge|Manager")
	void HideAllScreens();

	UFUNCTION(BlueprintCallable, Category = "UIMenuForge|Manager")
	void TogglePause(bool bPause);

private:

	UPROPERTY()
	TMap<EScreenType, UUserWidget*> ActiveScreens;

	void UpdateInputMode();
};

// ----------------------------------------------------------------
// -- BASE WIDGET CLASSES --
// All marked abstract - prevents direct instantation 
// User creates blueprint children form these base classes 
// Templates in Content/UI/Screens/Templates/ use these as parents 
// ----------------------------------------------------------------

UCLASS(Abstract)
class UIMENUFORGE_API UUIMenuForge_MainMenu : public UUserWidget
{
	GENERATED_BODY()
public: 
	virtual void NativeConstruct() override;
};

UCLASS(Abstract)
class UIMENUFORGE_API UUIMenuForge_PauseMenu : public UUserWidget
{
	GENERATED_BODY()
public: 
	virtual void NativeConstruct() override;
};

UCLASS(Abstract)
class UIMENUFORGE_API UUIMenuForge_SettingsMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	// Audio 
	UFUNCTION(BlueprintCallable, Category = "UIMenuForge|Settings")
	void SetMasterVolume(float Value);

	UFUNCTION(BlueprintCallable, Category = "UIMenuForge|Settings")
	void SetMusicVolume(float Value);

	UFUNCTION(BlueprintCallable, Category = "UIMenuForge|Settings")
	void SetSFXVolume(float Value);

	// Display
	UFUNCTION(BlueprintCallable, Category = "UIMenuForge|Settings")
	void SetFullscreen(bool bFullscreen);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIMenuForge|Settings")
	USoundMix* MasterSoundMix = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIMenuForge|Settings")
	USoundClass* MasterSoundClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIMenuForge|Settings")
	USoundClass* MusicSoundClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIMenuForge|Settings")
	USoundClass* SFXSoundClass = nullptr;
};

UCLASS(Abstract)
class UIMENUFORGE_API UUIMenuForge_HUD : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, Category = "UIMenuForge|HUD")
	float HealthPercent = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "UIMenuForge|HUD")
	float StaminaPercent = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "UIMenuForge|HUD")
	int32 Score = 0;

	UPROPERTY(BlueprintReadWrite, Category = "UIMenuForge|HUD")
	float ElapsedTime = 0.0f;
};

UCLASS(Abstract)
class UIMENUFORGE_API UUIMenuForge_LoadingScreen : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, Category = "UIMenuForge|LoadingScreen")
	float LoadingPercent = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "UIMenuForge|LoadingScreen")
	FText TipText = FText::FromString("Loading...");
};
 
UCLASS(Abstract)
class UIMENUFORGE_API UUIMenuForge_Credits : public UUserWidget
{
	GENERATED_BODY()
public: 
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, Category = "UIMenuForge|Credits")
	FText CreditsText = FText::FromString("Your credits here...");
};

UCLASS(Abstract)
class UIMENUFORGE_API UUIMenuForge_DeathScreen : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
};

UCLASS(Abstract)
class UIMENUFORGE_API UUIMenuForge_CustomScreen : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
};

// ------------
// -- MODULE --
// ------------

class FUIMenuForgeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private: 
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerUIMenuForge();
};
