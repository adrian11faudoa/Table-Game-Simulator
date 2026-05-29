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

    UFUNCTION(BlueprintCallable, Category="Audio")
    void PlaySoundAtLocation(const FString& SoundID, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio")
    void PlaySound2D(const FString& SoundID, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category="Audio")
    void SetAmbientEnvironment(const FString& EnvironmentID);

    UFUNCTION(BlueprintCallable, Category="Audio")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category="Audio")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category="Audio")
    void StopAmbient();

    UFUNCTION(BlueprintCallable, Category="Audio")
    void RegisterSound(const FString& SoundID, USoundBase* Sound);

private:
    UPROPERTY()
    TMap<FString, USoundBase*> LoadedSounds;

    UPROPERTY()
    UAudioComponent* AmbientComponent = nullptr;

    float MusicVolume = 0.3f;
    float SFXVolume   = 1.0f;

    void LoadDefaultSounds();
    void LoadSound(const FString& SoundID, const FString& AssetPath);
};
