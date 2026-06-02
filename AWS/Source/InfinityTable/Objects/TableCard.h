#pragma once
#include "CoreMinimal.h"
#include "Objects/TableObject.h"
#include "TableCard.generated.h"

UCLASS()
class INFINITYTABLE_API ATableCard : public ATableObject
{
    GENERATED_BODY()
public:
    ATableCard();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString CardID;
    UPROPERTY(BlueprintReadOnly, Replicated) bool bFaceUp = false;
    UPROPERTY(BlueprintReadOnly, Replicated) bool bInHand = false;
    UPROPERTY(BlueprintReadOnly, Replicated) APlayerController* HandOwner = nullptr;
    UPROPERTY(EditAnywhere) UTexture2D* FrontTexture;
    UPROPERTY(EditAnywhere) UTexture2D* BackTexture;
    UFUNCTION(Server, Reliable) void Server_FlipCard();
    UFUNCTION(Server, Reliable) void Server_SetFaceUp(bool bUp);
    UFUNCTION(Server, Reliable) void Server_SetInHand(APlayerController* Owner, bool bHand);
    UFUNCTION(BlueprintCallable) bool CanPlayerSeeFace(APlayerController* PC) const;
};
