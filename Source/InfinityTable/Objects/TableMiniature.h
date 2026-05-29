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

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString CharacterName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   OwnerPlayerIndex = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   HP        = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   MaxHP     = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   MoveSpeed = 6; // grid cells
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) bool    bIsEnemy  = false;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_SetHP(int32 NewHP);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_MoveToCell(FIntPoint Cell);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_SetCharacterName(const FString& Name);

    UFUNCTION(BlueprintCallable)
    float GetHPPercent() const;

    // Aura indicator (status effects)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
    TArray<FString> StatusEffects;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_AddStatus(const FString& Status);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_RemoveStatus(const FString& Status);
};
