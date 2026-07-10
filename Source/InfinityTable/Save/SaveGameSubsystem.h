#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Objects/TableObject.h"
#include "SaveGameSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FTableSaveState
{
    GENERATED_BODY()
    UPROPERTY() FString Slot;
    UPROPERTY() FString Name;
    UPROPERTY() FString Environment;
    UPROPERTY() int64   Timestamp = 0;
    UPROPERTY() TArray<FTableObjectState> Objects;
};

USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString Slot;
    UPROPERTY(BlueprintReadOnly) FString Name;
    UPROPERTY(BlueprintReadOnly) FString Environment;
    UPROPERTY(BlueprintReadOnly) int64   Timestamp = 0;
};

UCLASS()
class INFINITYTABLE_API USaveGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    UFUNCTION(BlueprintCallable) bool SaveTable(const FString& Slot, const FString& DisplayName);
    UFUNCTION(BlueprintCallable) bool LoadTable(const FString& Slot);
    UFUNCTION(BlueprintCallable) bool DeleteSlot(const FString& Slot);
    UFUNCTION(BlueprintCallable) TArray<FSaveSlotInfo> GetSaveSlots() const;
    UPROPERTY(EditDefaultsOnly) float AutosaveIntervalSeconds = 300.0f;
private:
    void* DB = nullptr;
    FString DBPath;
    FTimerHandle AutosaveTimer;
    bool OpenDB();
    void CloseDB();
    bool ExecSQL(const FString& SQL);
    TArray<TMap<FString,FString>> QuerySQL(const FString& SQL) const;
    void AutoSave();
};
