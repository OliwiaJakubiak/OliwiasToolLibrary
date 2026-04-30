// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraShakeBase.h"
#include "Curves/CurveFloat.h"
#include "Engine/DataAsset.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFeelKit.generated.h"

// -----------------
// -- DATA ASSETS --
// -----------------

UCLASS(BlueprintType)
class GAMEFEELKIT_API UDA_GameFeel_ScreenShake : public UPrimaryDataAsset
{
	GENERATED_BODY()
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ScreenShake")
	TSubclassOf<UCameraShakeBase> ShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ScreenShake")
	float Intensity = 1.0f;
};

UCLASS(BlueprintType)
class GAMEFEELKIT_API UDA_GameFeel_ScalePulse : public UPrimaryDataAsset
{
	GENERATED_BODY()
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ScalePulse")
	FVector PeakScale = FVector(1.2f, 1.2f, 1.2f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ScalePulse")
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ScalePulse")
	bool bReturnToOriginal = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ScalePulse")
	UCurveFloat* CurveShape = nullptr;
};


UCLASS(BlueprintType)
class GAMEFEELKIT_API UDA_GameFeel_ParticleBurst : public UPrimaryDataAsset
{
	GENERATED_BODY()
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ParticleBurst")
	UNiagaraSystem* NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ParticleBurst")
	FVector SpawnOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ParticleBurst")
	float Scale = 1.0f;
};

UCLASS(BlueprintType)
class GAMEFEELKIT_API UDA_GameFeel_SoundCue : public UPrimaryDataAsset
{
	GENERATED_BODY()
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|SoundCue")
	USoundBase* SoundAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|SoundCue")
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|SoundCue")
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|SoundCue")
	bool bRandomisePitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|SoundCue")
	FVector2D PitchVarianceRange = FVector2D(0.9f, 1.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|SoundCue")
	bool bSpawnAtActorLocation = true;
};

UCLASS(BlueprintType)
class GAMEFEELKIT_API UDA_GameFeel_BGM : public UPrimaryDataAsset
{
	GENERATED_BODY()
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|BGM")
	USoundBase* MusicTrack = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|BGM")
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|BGM")
	float FadeInDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|BGM")
	float FadeOutDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|BGM")
	bool bLooping = true;
};

UCLASS(BlueprintType)
class GAMEFEELKIT_API UDA_GameFeel_TimeDilation : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|TimeDilation")
	float DilationAmount = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|TimeDilation")
	float Duration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|TimeDilation")
	bool bAffectsAudioPitch = true;
};

UCLASS(BlueprintType)
class GAMEFEELKIT_API UDA_GameFeel_ChromaticAberration : public UPrimaryDataAsset
{
	GENERATED_BODY()
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ChromaticAberration")
	float Intensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|ChromaticAberration")
	float Duration = 0.2f;
};

UCLASS(BlueprintType)
class GAMEFEELKIT_API UDA_GameFeel_VignettePulse : public UPrimaryDataAsset
{
	GENERATED_BODY()
public: 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|VignettePulse")
	float Intensity = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|VignettePulse")
	float Duration = 0.3f;
};

// ---------------------
// -- ACTOR COMPONENT --
// ---------------------

UCLASS(ClassGroup=(GameFeel), meta=(BlueprintSpawnableComponent))
class GAMEFEELKIT_API UGameFeelComponent : public UActorComponent
{
	GENERATED_BODY()

public: 
	UGameFeelComponent();

	// Data Asset slots -- assign in Details panel after adding component to actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
	UDA_GameFeel_ScreenShake* ScreenShakeData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
	UDA_GameFeel_TimeDilation* TimeDilationData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|VFX")
	UDA_GameFeel_ScalePulse* ScalePulseData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|VFX")
	UDA_GameFeel_ParticleBurst* ParticleBurstData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|VFX")
	UDA_GameFeel_ChromaticAberration* ChromaticAberrationData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|VFX")
	UDA_GameFeel_VignettePulse* VignettePulseData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Audio")
	UDA_GameFeel_SoundCue* SoundCueData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Audio")
	UDA_GameFeel_BGM* BGMData;

	// Runtime trigger functions - call these from game blueprint logic 
	UFUNCTION(BlueprintCallable, Category = "GameFeel|Camera")
	void TriggerScreenShake();

	UFUNCTION(BlueprintCallable, Category = "GameFeel|Camera")
	void TriggerTimeDilation();

	UFUNCTION(BlueprintCallable, Category = "GameFeel|VFX")
	void TriggerScalePulse();

	UFUNCTION(BlueprintCallable, Category = "GameFeel|VFX")
	void TriggerParticleBurst();

	UFUNCTION(BlueprintCallable, Category = "GameFeel|VFX")
	void TriggerChromaticAberration();

	UFUNCTION(BlueprintCallable, Category = "GameFeel|VFX")
	void TriggerVignettePulse();

	UFUNCTION(BlueprintCallable, Category = "GameFeel|Audio")
	void TriggerSoundCue();

	UFUNCTION(BlueprintCallable, Category = "GameFeel|Audio")
	void TriggerBGM();

private:
	UPROPERTY()
	UAudioComponent* ActiveAudioComponent;

	FVector  OriginalScale;

	FTimerHandle ScalePulseTimerHandle;
	FTimerHandle TimeDilationTimerHandle;
	FTimerHandle ChromaticAberrationTimerHandle;
	FTimerHandle VignettePulseTimerHandle;

	void RestoreScale();
	void RestoreTimeDilation();
	void RestoreChromaticAberration();
	void RestoreVignettePulse();
};
// ----------------------------------------------------
// -- PREVIEW EFFECT ENUM --
// Maps to the 8 data asset slots on UGameFeelComponent
// ----------------------------------------------------

UENUM(BlueprintType)
enum class EGameFeelEffectType : uint8
{
	ScreenShake			UMETA(DisplayName = "ScreenShake"),
	ScalePulse			UMETA(DisplayName = "Scale Pulse"),
	ParticleBurst		UMETA(DisplayName = "ParticleBurst"),
	SoundCue			UMETA(DisplayName = "SoundCue"),
	BGM					UMETA(DisplayName = "BGM"),
	TimeDilation		UMETA(DisplayName = "TimeDilation"),
	ChromaticAberration UMETA(DisplayName = "ChromaticAberration"),
	VignettePulse		UMETA(DisplayName = "VignettePulse")
};

// -------------------------------------------------------------------
// -- PREVIEW FUNCTION LIBRARY --
// Editor-only preview functions called by the EUW preview tab
// Screenshake and TimeDilation require PIE - all other work in editor 
// -------------------------------------------------------------------

UCLASS()
class GAMEFEELKIT_API UGameFeelPreviewLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public: 

	// Tracks active preview BGM audio component so StopBGMPreview can stop it
	static UAudioComponent* ActivePreviewBGM;

	// MASTER PREVIEW DISPATCHER - the only node EUW preview buttons need to call 
	// Reads data asset values from the component automatically
	// Requires actor selection for: ScalePulse, ParticleBurst
	// Works without actor selection for: SoundCue, BGM, ChromaticAberration, VignettePulse 
	// Requires PIE for: ScreenShake, TimeDilation
	UFUNCTION(BlueprintCallable, Category = "GameFeel|Preview", meta = (WorldContext = "WorldContextObject"))
	static void PreviewEffect(UObject* WorldContextObject, UGameFeelComponent* Component, EGameFeelEffectType EffectType, AActor* SelectedActor = nullptr);

	// Stops the currently playing BGM preview
	UFUNCTION(BlueprintCallable, Category = "GameFeel|Preview")
	static void StopBGMPreview();

private:
	static void PreviewScreenShake(UObject* WorldContextObject, TSubclassOf<UCameraShakeBase> ShakeClass, float Intensity);
	static void PreviewTimeDilation(UObject* WorldContextObject, float Amount, float Duration);
	static void PreviewScalePulse(AActor* TargetActor, FVector PeakScale, float Duration);
	static void PreviewParticleBurst(UObject* WorldContextObject, UNiagaraSystem*, FVector Location, float Scale);
	static void PreviewSoundCue(UObject* WorldContextObject, USoundBase* Sound, float Volume, float Pitch);
	static void PreviewBGM(UObject* WorldContextObject, USoundBase* Music, float Volume, float FadeIn);
	static void PreviewChromaticAberration(UObject* WorldContextObject, float Intensity, float Duration);
	static void PreviewVignettePulse(UObject* WorldContextObject, float Intensity, float Duration);
};

// ------------
// -- MODULE --
// ------------

class FGameFeelKitModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private: 
	void RegisterMenus();
	void FillMenu(UToolMenu* Menu);
	void TriggerGameFeelKit();
};
