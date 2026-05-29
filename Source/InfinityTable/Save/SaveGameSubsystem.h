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
    UPROPERTY(BlueprintReadOnly) int32   ObjectCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTableSaved,  bool, bSuccess, const FString&, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTableLoaded, bool, bSuccess, const FString&, Slot);

UCLASS()
class INFINITYTABLE_API USaveGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="Save")
    bool SaveTable(const FString& Slot, const FString& DisplayName);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool LoadTable(const FString& Slot);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool DeleteSlot(const FString& Slot);

    UFUNCTION(BlueprintCallable, Category="Save")
    TArray<FSaveSlotInfo> GetSaveSlots() const;

    UFUNCTION(BlueprintCallable, Category="Save")
    bool ExportToJSON(const FString& Slot, const FString& FilePath) const;

    UFUNCTION(BlueprintCallable, Category="Save")
    bool ImportFromJSON(const FString& FilePath, const FString& NewSlot);

    UPROPERTY(BlueprintAssignable) FOnTableSaved  OnTableSaved;
    UPROPERTY(BlueprintAssignable) FOnTableLoaded OnTableLoaded;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float AutosaveIntervalSeconds = 300.0f;

private:
    // SQLite DB handle (opaque pointer)
    void* DB = nullptr;
    FString DBPath;
    FTimerHandle AutosaveTimer;

    bool  OpenDB();
    void  CloseDB();
    bool  ExecSQL(const FString& SQL);
    TArray<TMap<FString,FString>> QuerySQL(const FString& SQL) const;

    void  AutoSave();
    FString EscapeSQL(const FString& Input) const;
};
