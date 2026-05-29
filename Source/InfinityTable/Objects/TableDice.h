#pragma once
#include "CoreMinimal.h"
#include "Objects/TableObject.h"
#include "TableDice.generated.h"

UENUM(BlueprintType)
enum class EDiceType : uint8
{
    D4   UMETA(DisplayName="d4"),
    D6   UMETA(DisplayName="d6"),
    D8   UMETA(DisplayName="d8"),
    D10  UMETA(DisplayName="d10"),
    D10P UMETA(DisplayName="d10 Percentile"),
    D12  UMETA(DisplayName="d12"),
    D20  UMETA(DisplayName="d20"),
    DCustom UMETA(DisplayName="Custom")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnDiceRolled, ATableDice*, Dice, int32, Result);

UCLASS()
class INFINITYTABLE_API ATableDice : public ATableObject
{
    GENERATED_BODY()

public:
    ATableDice();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    EDiceType DiceType = EDiceType::D6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    int32 CustomSides = 6;

    UPROPERTY(BlueprintAssignable)
    FOnDiceRolled OnDiceRolled;

    UPROPERTY(BlueprintReadOnly, Replicated)
    int32 LastResult = 0;

    UFUNCTION(Server, Reliable)
    void Server_Roll(FVector Impulse);

    UFUNCTION(Server, Reliable)
    void Server_RollRandom();          // No physics — instant result

    UFUNCTION(BlueprintCallable)
    int32 GetCurrentFaceUp() const;

    UFUNCTION(BlueprintCallable)
    int32 GetMaxValue() const;

    UFUNCTION(BlueprintCallable)
    EDiceType GetDiceType() const { return DiceType; }

private:
    bool  bWasMoving      = false;
    float RestingTimer    = 0.0f;
    const float RestThreshold = 0.35f;

    void  DetectResult();
    int32 CalculateFaceUp() const;
    void  BroadcastResult(int32 Result);

    // Face normal lookup tables
    static TArray<FVector> GetFaceNormals(EDiceType Type);
};
