#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InitiativeTracker.generated.h"

USTRUCT(BlueprintType)
struct FInitiativeEntry
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite) FString Name;
    UPROPERTY(BlueprintReadWrite) int32   Initiative = 0;
    UPROPERTY(BlueprintReadWrite) int32   HP         = 10;
    UPROPERTY(BlueprintReadWrite) int32   MaxHP      = 10;
    UPROPERTY(BlueprintReadWrite) bool    bIsPlayer  = true;
    UPROPERTY(BlueprintReadWrite) FLinearColor Color = FLinearColor::White;
    UPROPERTY(BlueprintReadWrite) class ATableMiniature* Token = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnChanged, const FInitiativeEntry&, CurrentCombatant);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatEnded);

UCLASS()
class INFINITYTABLE_API UInitiativeTracker : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void SetOrder(const TArray<FInitiativeEntry>& Entries);
    UFUNCTION(BlueprintCallable) void AddCombatant(const FInitiativeEntry& Entry);
    UFUNCTION(BlueprintCallable) void RemoveCombatant(const FString& Name);
    UFUNCTION(BlueprintCallable) void NextTurn();
    UFUNCTION(BlueprintCallable) void PreviousTurn();
    UFUNCTION(BlueprintCallable) void EndCombat();
    UFUNCTION(BlueprintCallable) void StartCombat();
    UFUNCTION(BlueprintCallable) FInitiativeEntry GetCurrentCombatant() const;
    UFUNCTION(BlueprintCallable) int32            GetRoundNumber() const { return Round; }
    UFUNCTION(BlueprintCallable) TArray<FInitiativeEntry> GetOrder() const { return Order; }

    UPROPERTY(BlueprintAssignable) FOnTurnChanged OnTurnChanged;
    UPROPERTY(BlueprintAssignable) FOnCombatEnded OnCombatEnded;

private:
    TArray<FInitiativeEntry> Order;
    int32 CurrentIndex = 0;
    int32 Round        = 0;
    bool  bInCombat    = false;

    void SortByInitiative();
};
