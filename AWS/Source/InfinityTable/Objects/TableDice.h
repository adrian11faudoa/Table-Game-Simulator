#pragma once
#include "CoreMinimal.h"
#include "Objects/TableObject.h"
#include "TableDice.generated.h"

UENUM(BlueprintType)
enum class EDiceType : uint8 { D4,D6,D8,D10,D12,D20,DCustom };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiceRolled, ATableDice*, Dice, int32, Result);

UCLASS()
class INFINITYTABLE_API ATableDice : public ATableObject
{
    GENERATED_BODY()
public:
    ATableDice();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) EDiceType DiceType    = EDiceType::D6;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32     CustomSides = 6;
    UPROPERTY(BlueprintReadOnly, Replicated) int32 LastResult = 0;
    UPROPERTY(BlueprintAssignable) FOnDiceRolled OnDiceRolled;

    UFUNCTION(Server, Reliable) void Server_Roll(FVector Impulse);
    UFUNCTION(Server, Reliable) void Server_RollRandom();
    UFUNCTION(BlueprintCallable) int32 GetCurrentFaceUp() const { return LastResult; }
    UFUNCTION(BlueprintCallable) int32 GetMaxValue() const;
private:
    bool  bWasMoving   = false;
    float RestingTimer = 0.f;
    void  DetectResult();
    int32 CalculateFaceUp() const;
};
