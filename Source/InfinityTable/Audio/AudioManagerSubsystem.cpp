#include "AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadDefaultSounds();
}

void UAudioManagerSubsystem::LoadDefaultSounds()
{
    struct { const TCHAR* ID; const TCHAR* Path; } SoundDefs[] = {
        { TEXT("Dice_Roll_Hard"),  TEXT("/Game/Audio/Dice/DA_DiceRollHard")  },
        { TEXT("Dice_Roll_Soft"),  TEXT("/Game/Audio/Dice/DA_DiceRollSoft")  },
        { TEXT("Dice_Settle"),     TEXT("/Game/Audio/Dice/DA_DiceSettle")     },
        { TEXT("Card_Pickup"),     TEXT("/Game/Audio/Cards/DA_CardPickup")    },
        { TEXT("Card_Place"),      TEXT("/Game/Audio/Cards/DA_CardPlace")     },
        { TEXT("Card_Flip"),       TEXT("/Game/Audio/Cards/DA_CardFlip")      },
        { TEXT("Card_Shuffle"),    TEXT("/Game/Audio/Cards/DA_CardShuffle")   },
        { TEXT("Object_Pickup"),   TEXT("/Game/Audio/Objects/DA_ObjPickup")   },
        { TEXT("Object_Place"),    TEXT("/Game/Audio/Objects/DA_ObjPlace")    },
        { TEXT("Object_Throw"),    TEXT("/Game/Audio/Objects/DA_ObjThrow")    },
        { TEXT("UI_Click"),        TEXT("/Game/Audio/UI/DA_UIClick")          },
        { TEXT("UI_Open"),         TEXT("/Game/Audio/UI/DA_UIOpen")           },
        { TEXT("Player_Join"),     TEXT("/Game/Audio/UI/DA_PlayerJoin")       },
    };

    for (const auto& Def : SoundDefs)
        LoadSound(Def.ID, Def.Path);
}

void UAudioManagerSubsystem::LoadSound(const FString& SoundID, const FString& AssetPath)
{
    USoundBase* Sound = LoadObject<USoundBase>(nullptr, *AssetPath);
    if (Sound) LoadedSounds.Add(SoundID, Sound);
}

void UAudioManagerSubsystem::RegisterSound(const FString& SoundID, USoundBase* Sound)
{
    if (Sound) LoadedSounds.Add(SoundID, Sound);
}

void UAudioManagerSubsystem::PlaySoundAtLocation(
    const FString& SoundID, FVector Location, float Volume, float Pitch)
{
    USoundBase** Sound = LoadedSounds.Find(SoundID);
    if (!Sound || !*Sound) return;

    UGameplayStatics::PlaySoundAtLocation(
        GetGameInstance()->GetWorld(), *Sound, Location,
        Volume * SFXVolume, Pitch);
}

void UAudioManagerSubsystem::PlaySound2D(const FString& SoundID, float Volume)
{
    USoundBase** Sound = LoadedSounds.Find(SoundID);
    if (!Sound || !*Sound) return;

    UGameplayStatics::PlaySound2D(
        GetGameInstance()->GetWorld(), *Sound, Volume * SFXVolume);
}

void UAudioManagerSubsystem::SetAmbientEnvironment(const FString& EnvironmentID)
{
    if (AmbientComponent && AmbientComponent->IsPlaying())
        AmbientComponent->FadeOut(2.0f, 0.0f);

    FString AmbientPath = FString::Printf(
        TEXT("/Game/Audio/Ambient/DA_Ambient_%s"), *EnvironmentID);

    USoundBase* Ambient = LoadObject<USoundBase>(nullptr, *AmbientPath);
    if (!Ambient) return;

    AmbientComponent = UGameplayStatics::SpawnSound2D(
        GetGameInstance()->GetWorld(), Ambient, MusicVolume,
        1.0f, 0.0f, nullptr, true);

    if (AmbientComponent) AmbientComponent->FadeIn(3.0f, MusicVolume);
}

void UAudioManagerSubsystem::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    if (AmbientComponent) AmbientComponent->SetVolumeMultiplier(MusicVolume);
}

void UAudioManagerSubsystem::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudioManagerSubsystem::StopAmbient()
{
    if (AmbientComponent) AmbientComponent->FadeOut(2.0f, 0.0f);
}
