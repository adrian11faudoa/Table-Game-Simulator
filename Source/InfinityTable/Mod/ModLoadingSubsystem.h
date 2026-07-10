#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Mod/ModManifest.h"
#include "ModLoadingSubsystem.generated.h"

UCLASS()
class INFINITYTABLE_API UModLoadingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable) TArray<FString> DiscoverMods() const;
    UFUNCTION(BlueprintCallable) bool ValidateMod(const FString& ModFolder, FString& OutError) const;
    UFUNCTION(BlueprintCallable) bool LoadMod(const FString& ModFolder);
    UFUNCTION(BlueprintCallable) bool UnloadMod(const FString& ModID);
    UFUNCTION(BlueprintCallable) TArray<FITModManifest> GetLoadedMods() const;
    UFUNCTION(BlueprintCallable) bool HasPermission(const FString& ModID, const FString& Permission) const;

    UPROPERTY(EditDefaultsOnly) FString ModsDirectoryName = TEXT("Mods");
    UPROPERTY(EditDefaultsOnly) TArray<FString> BlockedScriptPatterns = {
        TEXT("os."), TEXT("io."), TEXT("require("), TEXT("loadstring("), TEXT("dofile(")
    };
    UPROPERTY(EditDefaultsOnly) TArray<FString> BlockedFileExtensions = {
        TEXT(".exe"), TEXT(".dll"), TEXT(".so"), TEXT(".bat"), TEXT(".sh")
    };

private:
    UPROPERTY() TMap<FString, FITModManifest> LoadedMods;
    FString GetModsRootPath() const;
    bool ParseManifest(const FString& JsonPath, FITModManifest& OutManifest) const;
    bool ScanLuaForBlockedPatterns(const FString& ScriptPath, FString& OutError) const;
};
