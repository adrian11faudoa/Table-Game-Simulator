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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString CharacterName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   HP     = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   MaxHP  = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   MoveSpeed = 6;
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_SetHP(int32 NewHP);
    UFUNCTION(BlueprintCallable) float GetHPPercent() const;
};
