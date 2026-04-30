// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeelKit.h"
#include "LevelEditor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"
#include "EditorViewportClient.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Scene.h"
#include "TimerManager.h"
#include "Sound/SoundWave.h"

#define LOCTEXT_NAMESPACE "FGameFeelKitModule"

IMPLEMENT_MODULE(FGameFeelKitModule, GameFeelKit)

UAudioComponent* UGameFeelPreviewLibrary::ActivePreviewBGM = nullptr;

// ------------
// -- MODULE --
// ------------

void FGameFeelKitModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGameFeelKitModule::RegisterMenus));
}

void FGameFeelKitModule::RegisterMenus()
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
				FNewToolMenuDelegate::CreateRaw(this, &FGameFeelKitModule::FillMenu)
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
					FGameFeelKitModule::FillMenu(Menu);
				}
			);
		}
	}
}

void FGameFeelKitModule::FillMenu(UToolMenu* Menu)
{
	// CATEGORY: "Game Feel"
	if (!Menu->ContainsSection("OliwiaDevTools_GameFeel"))
	{
		Menu->AddSection(
			"OliwiaDevTools_GameFeel",
			LOCTEXT("GameFeelSection_Label", "Game Feel")
		);
	}

	FToolMenuSection* Section = Menu->FindSection("OliwiaDevTools_GameFeel");
	if (!Section) return;

	Section->AddMenuEntry(
		"GameFeelKit",
		LOCTEXT("GameFeelBtn_Label", "Game Feel Kit"),
		LOCTEXT("GameFeelBtn_Tooltip", "Opens the Game Feel kit utility widget"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGameFeelKitModule::TriggerGameFeelKit))
	);
}

void FGameFeelKitModule::TriggerGameFeelKit()
{
	FString WidgetPath = TEXT("/GameFeelKit/UI/EUW_GameFeelKit.EUW_GameFeelKit");
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

void FGameFeelKitModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
}

// ----------------
// -- CORE LOGIC --
// ----------------

// ---------------------------
// -- COMPONENT CONSTRUCTOR --
// ---------------------------

UGameFeelComponent::UGameFeelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ActiveAudioComponent = nullptr;
	OriginalScale = FVector::OneVector;
}

// -------------------------------
// -- RUNTIME TRIGGER FUNCTIONS --
// -------------------------------

// -- SCREEN SHAKE --
void UGameFeelComponent::TriggerScreenShake()
{
	if (!ScreenShakeData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: No ScreenShake Data Asset assigned"));
		return;
	}

	if (!ScreenShakeData->ShakeClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: ScreenShake Data Asset has no ShakeClass assigned"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	PC->ClientStartCameraShake(ScreenShakeData->ShakeClass, ScreenShakeData->Intensity);
}

// -- PARTICLE BURST --
void UGameFeelComponent::TriggerParticleBurst()
{
	if (!ParticleBurstData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: No ParticleBurst Data Asset assigned"));
		return;
	}

	if (!ParticleBurstData->NiagaraSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: ParticleBurst Data Asset has no NiagaraSystem assigned"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector SpawnLocation = Owner->GetActorLocation() + ParticleBurstData->SpawnOffset;
	FVector ScaleVector = FVector(ParticleBurstData->Scale);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		ParticleBurstData->NiagaraSystem,
		SpawnLocation,
		FRotator::ZeroRotator,
		ScaleVector,
		true,
		true
	);
}

// -- SOUND CUE --
void UGameFeelComponent::TriggerSoundCue()
{
	if (!SoundCueData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: No SoundCue Data Asset assigned"));
		return;
	}

	if (!SoundCueData->SoundAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: SoundCue Data Asset has no SoundAsset assigned"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	float FinalPitch = SoundCueData->PitchMultiplier;
	if (SoundCueData->bRandomisePitch)
	{
		FinalPitch = FMath::RandRange(
			SoundCueData->PitchVarianceRange.X,
			SoundCueData->PitchVarianceRange.Y
		);
	}

	FVector SpawnLocation = FVector::ZeroVector;
	if (SoundCueData->bSpawnAtActorLocation)
	{
		AActor* Owner = GetOwner();
		if (Owner) SpawnLocation = Owner->GetActorLocation();
	}

	UGameplayStatics::PlaySoundAtLocation(
		World,
		SoundCueData->SoundAsset,
		SpawnLocation,
		SoundCueData->VolumeMultiplier,
		FinalPitch
	);
}

// -- BGM --
void UGameFeelComponent::TriggerBGM()
{
	if (!BGMData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: No BGM Data Asset assigned"));
		return;
	}

	if (!BGMData->MusicTrack)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: BGM Data Asset has no MusicTrack assigned"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	if (ActiveAudioComponent)
	{
		ActiveAudioComponent->FadeOut(BGMData->FadeOutDuration, 0.0f);
	}

	ActiveAudioComponent = UGameplayStatics::SpawnSound2D(
		World,
		BGMData->MusicTrack,
		BGMData->VolumeMultiplier,
		1.0f,
		0.0f,
		nullptr,
		false,
		true
	);

	if (ActiveAudioComponent)
	{
		ActiveAudioComponent->bIsUISound = true;
		ActiveAudioComponent->SetVolumeMultiplier(0.0f);
		ActiveAudioComponent->FadeIn(BGMData->FadeInDuration, BGMData->VolumeMultiplier);

		if (BGMData->bLooping)
		{
			USoundWave* SoundWave = Cast<USoundWave>(BGMData->MusicTrack);
			if (SoundWave)
			{
				SoundWave->bLooping = true;
			}
		}
	}
}

// -- SCALE PULSE -- 
void UGameFeelComponent::TriggerScalePulse()
{
	if (!ScalePulseData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: No ScalePulse Data Asset assigned"));
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	OriginalScale = Owner->GetActorScale3D();
	Owner->SetActorScale3D(ScalePulseData->PeakScale);

	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().SetTimer(
		ScalePulseTimerHandle,
		this,
		&UGameFeelComponent::RestoreScale,
		ScalePulseData->Duration,
		false
	);
}

void UGameFeelComponent::RestoreScale()
{
	if (!ScalePulseData) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	if (ScalePulseData->bReturnToOriginal)
	{
		Owner->SetActorScale3D(OriginalScale);
	}
}

// -- TIME DILATION --
void UGameFeelComponent::TriggerTimeDilation()
{
	if (!TimeDilationData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: No TimeDilation Data Asset assigned"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SetGlobalTimeDilation(World, TimeDilationData->DilationAmount);

	if (TimeDilationData->bAffectsAudioPitch)
	{
		if (ActiveAudioComponent)
		{
			ActiveAudioComponent->SetPitchMultiplier(TimeDilationData->DilationAmount);
		}
	}

	World->GetTimerManager().SetTimer(
		TimeDilationTimerHandle,
		this,
		&UGameFeelComponent::RestoreTimeDilation,
		TimeDilationData->Duration,
		false
	);
}

void UGameFeelComponent::RestoreTimeDilation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);

	if (TimeDilationData && TimeDilationData->bAffectsAudioPitch)
	{
		if (ActiveAudioComponent)
		{
			ActiveAudioComponent->SetPitchMultiplier(1.0f);
		}
	}
}

// -- CHROMATIC ABERRATION --
void UGameFeelComponent::TriggerChromaticAberration()
{
	if (!ChromaticAberrationData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: No ChromaticAberration Data Asset assigned"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
	if (!CameraManager) return;

	CameraManager->ViewTarget.POV.PostProcessBlendWeight = 1.0f;
	CameraManager->ViewTarget.POV.PostProcessSettings.bOverride_SceneFringeIntensity = true;
	CameraManager->ViewTarget.POV.PostProcessSettings.SceneFringeIntensity = ChromaticAberrationData->Intensity;

	World->GetTimerManager().SetTimer(
		ChromaticAberrationTimerHandle,
		this,
		&UGameFeelComponent::RestoreChromaticAberration,
		ChromaticAberrationData->Duration,
		false
	);
}

void UGameFeelComponent::RestoreChromaticAberration()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager) return;

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
	CameraManager->ViewTarget.POV.PostProcessSettings.SceneFringeIntensity = 0.0f;
	CameraManager->ViewTarget.POV.PostProcessBlendWeight = 0.0f;
}

// -- VIGNETTE PULSE --
void UGameFeelComponent::TriggerVignettePulse()
{
	if (!VignettePulseData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel: No VignettePulse Data Asset assigned"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
	if (!CameraManager) return;

	CameraManager->ViewTarget.POV.PostProcessBlendWeight = 1.0f;
	CameraManager->ViewTarget.POV.PostProcessSettings.bOverride_VignetteIntensity = true;
	CameraManager->ViewTarget.POV.PostProcessSettings.VignetteIntensity = VignettePulseData->Intensity;

	World->GetTimerManager().SetTimer(
		VignettePulseTimerHandle,
		this,
		&UGameFeelComponent::RestoreVignettePulse,
		VignettePulseData->Duration,
		false
	);
}

void UGameFeelComponent::RestoreVignettePulse()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager) return;

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
	CameraManager->ViewTarget.POV.PostProcessSettings.VignetteIntensity = 0.0f;
	CameraManager->ViewTarget.POV.PostProcessBlendWeight = 0.0f;
}

// -------------------------------------------------------
// -- PREVIEW LIBRARY - INDIVIDUAL FUNCTIONS (internal) --
// -------------------------------------------------------

// -- SCREENSHAKE --
void UGameFeelPreviewLibrary::PreviewScreenShake(UObject* WorldContextObject, TSubclassOf<UCameraShakeBase> ShakeClass, float Intensity)
{
	if (!ShakeClass) return;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return;

	// Only works in PIE/runtime - editor world has no player controller 
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: ScreenShake requires Play In Editor (PIE). Press Play first, then trigger preview."));
		return;
	}

	PC->ClientStartCameraShake(ShakeClass, Intensity);
}

// -- PARTICLE BURST --
void UGameFeelPreviewLibrary::PreviewParticleBurst(UObject* WorldContextObject, UNiagaraSystem* System, FVector Location, float Scale)
{
	if (!System)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: No NiagaraSystem provided"));
		return;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		System,
		Location,
		FRotator::ZeroRotator,
		FVector(Scale),
		true,
		true
	);
}

// -- SOUND CUE --
void UGameFeelPreviewLibrary::PreviewSoundCue(UObject* WorldContextObject, USoundBase* Sound, float Volume, float Pitch)
{
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: No Sound provided"));
		return;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	UGameplayStatics::PlaySoundAtLocation(
		World,
		Sound,
		FVector::ZeroVector,
		Volume,
		Pitch
	);
}

// -- BGM --
void UGameFeelPreviewLibrary::PreviewBGM(UObject* WorldContextObject, USoundBase* Music, float Volume, float FadeIn)
{
	if (!Music)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: No Music provided"));
		return;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	// Stop any existing preview BGM first
	if (ActivePreviewBGM && ActivePreviewBGM->IsPlaying())
	{
		ActivePreviewBGM->Stop();
		ActivePreviewBGM = nullptr;
	}

	ActivePreviewBGM = UGameplayStatics::SpawnSoundAtLocation(
		World,
		Music,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Volume,
		1.0f
	);
}

void UGameFeelPreviewLibrary::StopBGMPreview()
{
	if (ActivePreviewBGM)
	{
		if (ActivePreviewBGM->IsPlaying())
		{
			ActivePreviewBGM->FadeOut(0.5f, 0.0f);
		}
		ActivePreviewBGM = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: No BGM currently playing to stop."));
	}
}

// -- SCALE PULSE --
void UGameFeelPreviewLibrary::PreviewScalePulse(AActor* TargetActor, FVector PeakScale, float Duration)
{
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: No actor selected for Scale Pulse preview"));
		return;
	}

	// Store original scale, apply peak scale 
	FVector OriginalScale = TargetActor->GetActorScale3D();
	TargetActor->SetActorScale3D(PeakScale);

	// Use a timer to restore - store original scale in a lambda capture 
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([TargetActor, OriginalScale]()
		{
			if (IsValid(TargetActor))
			{
				TargetActor->SetActorScale3D(OriginalScale);
			}
		});

	UWorld* World = TargetActor->GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
	}
}

// -- TIME DILATION --
void UGameFeelPreviewLibrary::PreviewTimeDilation(UObject* WorldContextObject, float Amount, float Duration)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	UGameplayStatics::SetGlobalTimeDilation(World, Amount);

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([World]()
		{
			if (IsValid(World))
			{
				UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
			}
		});

	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
}

// -- CHROMATIC ABERRATION -- 
void UGameFeelPreviewLibrary::PreviewChromaticAberration(UObject* WorldContextObject, float Intensity, float Duration)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	// Spawns a temporary post process volume
	APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>();
	if (!PPVolume) return;

	PPVolume->bUnbound = true;
	PPVolume->BlendWeight = 1.0f;
	PPVolume->Settings.bOverride_SceneFringeIntensity = true;
	PPVolume->Settings.SceneFringeIntensity = Intensity;

	// Destroy after duration using a lambda timer
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([PPVolume]()
		{
			if (IsValid(PPVolume))
			{
				PPVolume->Destroy();
			}
		});

	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
}

// -- VIGNETTE PULSE --
void UGameFeelPreviewLibrary::PreviewVignettePulse(UObject* WorldContextObject, float Intensity, float Duration)
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	// Spawns a temporary post process volume 
	APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>();
	if (!PPVolume) return;

	PPVolume->bUnbound = true;
	PPVolume->BlendWeight = 1.0f;
	PPVolume->Settings.bOverride_VignetteIntensity = true;
	PPVolume->Settings.VignetteIntensity = Intensity;

	// Destroy after duration using a lambda timer
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([PPVolume]()
		{
			if (IsValid(PPVolume))
			{
				PPVolume->Destroy();
			}
		});

	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
}

// --------------------------------------------------------
// -- PREVIEW EFFECTS - MASTER DISPATCHER --
// Reads data asset values from the component automatically 
// This is the only function the EUW preview buttons call 
// --------------------------------------------------------

void UGameFeelPreviewLibrary::PreviewEffect(
	UObject* WorldContextObject,
	UGameFeelComponent* Component,
	EGameFeelEffectType EffectType,
	AActor* SelectedActor)
{
	if (!Component)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: No GameFeelComponent found. Add a GameFeel Component to the selected actor first."));
		return;
	}

	switch (EffectType)
	{
	// -- SCREEN SHAKE --
	case EGameFeelEffectType::ScreenShake:
	{
		if (!Component->ScreenShakeData)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: ScreenShake - no Data Asset assigned on component."));
			return;
		}
		if (!Component->ScreenShakeData->ShakeClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: ScreenShake - Data Asset has no ShakeClass set."));
			return;
		}
		PreviewScreenShake(
			WorldContextObject,
			Component->ScreenShakeData->ShakeClass,
			Component->ScreenShakeData->Intensity);
		break;
	}
	// -- SCALE PULSE --
	case EGameFeelEffectType::ScalePulse:
	{
		if (!Component->ScalePulseData)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: ScalePulse - no Data Asset assigned on component."));
			return;
		}
		// Use explicitly passed actor first, fall back to component owner
		AActor* Target = SelectedActor ? SelectedActor : Component->GetOwner();
		if (!Target)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: ScalePulse - no actor to scale. Select an actor in the viewport."));
			return;
		}
		PreviewScalePulse(
			Target,
			Component->ScalePulseData->PeakScale,
			Component->ScalePulseData->Duration);
		break;
	}
	// -- PARTICLE BURST --
	case EGameFeelEffectType::ParticleBurst:
	{
		if (!Component->ParticleBurstData)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: ParticleBurst - no Data Asset assigned on component."));
			return;
		}
		if (!Component->ParticleBurstData->NiagaraSystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: ParticleBurst - Data Asset has no NiagaraSystem set."));
			return;
		}
		// Resolve spawn location from selected actor or component owner + offset
		AActor* Target = SelectedActor ? SelectedActor : Component->GetOwner();
		FVector SpawnLoc = Target ? Target->GetActorLocation() + Component->ParticleBurstData->SpawnOffset : Component->ParticleBurstData->SpawnOffset;
		PreviewParticleBurst(
			WorldContextObject,
			Component->ParticleBurstData->NiagaraSystem,
			SpawnLoc,
			Component->ParticleBurstData->Scale);
		break;
	}
	// -- SOUND CUE -- 
	case EGameFeelEffectType::SoundCue:
	{
		if (!Component->SoundCueData)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: SoundCue - no Data Asset assigned on component."));
			return;
		}
		if (!Component->SoundCueData->SoundAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: SoundCue - Data Asset has no SoundAsset set."));
			return;
		}
		// Mirror the pitch randomisation logic from TriggerSoundCue
		float FinalPitch = Component->SoundCueData->PitchMultiplier;
		if (Component->SoundCueData->bRandomisePitch)
		{
			FinalPitch = FMath::RandRange(
				Component->SoundCueData->PitchVarianceRange.X,
				Component->SoundCueData->PitchVarianceRange.Y);
		}
		PreviewSoundCue(
			WorldContextObject,
			Component->SoundCueData->SoundAsset,
			Component->SoundCueData->VolumeMultiplier,
			FinalPitch);
		break;
	}
	// -- BGM --
	case EGameFeelEffectType::BGM:
	{
		if (!Component->BGMData)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: BGM - no Data Asset assigned on component."));
			return;
		}
		if (!Component->BGMData->MusicTrack)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: BGM - Data Asset has no MusicTrack set."));
			return;
		}
		PreviewBGM(
			WorldContextObject,
			Component->BGMData->MusicTrack,
			Component->BGMData->VolumeMultiplier,
			Component->BGMData->FadeInDuration);
		break;
	}
	// -- TIME DILATION --
	case EGameFeelEffectType::TimeDilation:
	{
		if (!Component->TimeDilationData)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: TimeDilation - no Data Asset assigned on component."));
			return;
		}
		PreviewTimeDilation(
			WorldContextObject,
			Component->TimeDilationData->DilationAmount,
			Component->TimeDilationData->Duration);
		break;
	}
	// -- CHROMATIC ABERRATION --
	case EGameFeelEffectType::ChromaticAberration:
	{
		if (!Component->ChromaticAberrationData)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: ChromaticAberration - no Data Asset assigned on component."));
			return;
		}
		PreviewChromaticAberration(
			WorldContextObject,
			Component->ChromaticAberrationData->Intensity,
			Component->ChromaticAberrationData->Duration);
		break;
	}
	// -- VIGNETTE PULSE --
	case EGameFeelEffectType::VignettePulse:
	{
		if (!Component->VignettePulseData)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameFeel Preview: VignettePulse - no Data Asset assigned on component."));
			return;
		}
		PreviewVignettePulse(
			WorldContextObject,
			Component->VignettePulseData->Intensity,
			Component->VignettePulseData->Duration);
		break;
	}

	}
}

#undef LOCTEXT_NAMESPACE