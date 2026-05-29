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

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString CardID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString CardName;
    UPROPERTY(BlueprintReadOnly, Replicated) bool bFaceUp    = false;
    UPROPERTY(BlueprintReadOnly, Replicated) bool bInHand    = false;
    UPROPERTY(BlueprintReadOnly, Replicated) APlayerController* HandOwner = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* FrontTexture;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* BackTexture;

    UFUNCTION(Server, Reliable) void Server_FlipCard();
    UFUNCTION(Server, Reliable) void Server_SetInHand(APlayerController* Owner, bool bHand);
    UFUNCTION(Server, Reliable) void Server_SetFaceUp(bool bUp);

    UFUNCTION(BlueprintCallable)
    bool CanPlayerSeeFace(APlayerController* PC) const;

    UFUNCTION(BlueprintCallable)
    void ApplyTextures();
};
