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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString Label;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32   Counter = 0;
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_SetCounter(int32 NewValue);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_IncrementCounter(int32 Delta = 1);
};
