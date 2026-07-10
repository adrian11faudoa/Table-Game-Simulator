#pragma once
#include "CoreMinimal.h"
#include "Objects/TableObject.h"
#include "TableMiniature.generated.h"

UCLASS()
class INFINITYTABLE_API ATableMiniature : public ATableObject
{
    GENERATED_BODY()
public:
    ATableMiniature();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString MiniatureName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   HealthPoints  = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   MaxHealthPoints = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) bool    bOnGrid       = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FIntPoint GridCell;

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
    void Server_SetHealth(int32 NewHP);

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
    void Server_MoveToCell(FIntPoint Cell);

    UFUNCTION(BlueprintCallable)
    float GetHealthPercent() const { return MaxHealthPoints > 0 ? (float)HealthPoints / MaxHealthPoints : 0.f; }
};
