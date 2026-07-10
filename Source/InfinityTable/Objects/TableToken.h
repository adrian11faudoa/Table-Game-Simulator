#pragma once
#include "CoreMinimal.h"
#include "Objects/TableObject.h"
#include "TableToken.generated.h"

UCLASS()
class INFINITYTABLE_API ATableToken : public ATableObject
{
    GENERATED_BODY()
public:
    ATableToken();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString TokenLabel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   CounterValue = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) bool    bFaceUp      = true;

    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable) void Server_SetCounter(int32 Value);
    UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable) void Server_FlipToken();
};
