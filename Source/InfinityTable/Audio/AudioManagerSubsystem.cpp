#include "Audio/AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // Pre-load common sounds
    LoadSound(TEXT("dice_roll"),   TEXT("/Game/Audio/SFX/DiceRoll"));
    LoadSound(TEXT("card_flip"),   TEXT("/Game/Audio/SFX/CardFlip"));
    LoadSound(TEXT("object_pick"), TEXT("/Game/Audio/SFX/ObjectPickup"));
    LoadSound(TEXT("object_drop"), TEXT("/Game/Audio/SFX/ObjectDrop"));
    LoadSound(TEXT("ambient_tavern"), TEXT("/Game/Audio/Ambient/Tavern"));
    LoadSound(TEXT("ambient_scifi"),  TEXT("/Game/Audio/Ambient/SciFi"));
}

void UAudioManagerSubsystem::PlaySoundAtLocation(const FString& SoundID, FVector Location, float Volume)
{
    if (USoundBase** Found = LoadedSounds.Find(SoundID))
    {
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::PlaySoundAtLocation(World, *Found, Location, Volume);
        }
    }
}

void UAudioManagerSubsystem::PlaySound2D(const FString& SoundID, float Volume)
{
    if (USoundBase** Found = LoadedSounds.Find(SoundID))
    {
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::PlaySound2D(World, *Found, Volume);
        }
    }
}

void UAudioManagerSubsystem::SetAmbientEnvironment(const FString& EnvironmentID)
{
    FString AmbientID = FString::Printf(TEXT("ambient_%s"), *EnvironmentID.ToLower());
    if (AmbientComponent) { AmbientComponent->Stop(); }
    if (USoundBase** Found = LoadedSounds.Find(AmbientID))
    {
        if (UWorld* World = GetWorld())
        {
            AmbientComponent = UGameplayStatics::SpawnSound2D(World, *Found, MusicVolume, 1.f, 0.f, nullptr, false, true);
        }
    }
}

void UAudioManagerSubsystem::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.f, 1.f);
    if (AmbientComponent) { AmbientComponent->SetVolumeMultiplier(MusicVolume); }
}

void UAudioManagerSubsystem::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UAudioManagerSubsystem::LoadSound(const FString& ID, const FString& AssetPath)
{
    if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, *AssetPath))
    {
        LoadedSounds.Add(ID, Sound);
    }
}
