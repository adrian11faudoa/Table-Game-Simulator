#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundBase.h"
#include "AudioManagerSubsystem.generated.h"

UCLASS()
class INFINITYTABLE_API UAudioManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    UFUNCTION(BlueprintCallable) void PlaySoundAtLocation(const FString& SoundID, FVector Location, float Volume=1.f);
    UFUNCTION(BlueprintCallable) void PlaySound2D(const FString& SoundID, float Volume=1.f);
    UFUNCTION(BlueprintCallable) void SetAmbientEnvironment(const FString& EnvironmentID);
    UFUNCTION(BlueprintCallable) void SetMusicVolume(float Volume);
    UFUNCTION(BlueprintCallable) void SetSFXVolume(float Volume);
private:
    UPROPERTY() TMap<FString, USoundBase*> LoadedSounds;
    UPROPERTY() UAudioComponent* AmbientComponent = nullptr;
    float MusicVolume = 0.3f;
    float SFXVolume   = 1.0f;
    void LoadSound(const FString& ID, const FString& AssetPath);
};
