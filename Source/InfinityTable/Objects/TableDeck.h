#pragma once
#include "CoreMinimal.h"
#include "Objects/TableObject.h"
#include "TableDeck.generated.h"

class ATableCard;

USTRUCT(BlueprintType)
struct FDeckCardEntry
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite) FString CardID;
    UPROPERTY(BlueprintReadWrite) FSoftObjectPath FrontTexturePath;
    UPROPERTY(BlueprintReadWrite) FSoftObjectPath BackTexturePath;
};

UCLASS()
class INFINITYTABLE_API ATableDeck : public ATableObject
{
    GENERATED_BODY()
public:
    ATableDeck();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FDeckCardEntry> DeckDefinition;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString DeckName;
    UPROPERTY(BlueprintReadOnly, Replicated) int32 CardsRemaining = 0;
    UPROPERTY(EditAnywhere) TSubclassOf<ATableCard> CardClass;
    UPROPERTY(EditAnywhere) bool bIsDiscardPile = false;

    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_Shuffle();
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_DrawTopCard(APlayerController* Drawer);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_DrawToHand(APlayerController* Drawer, int32 Count);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_DiscardCard(ATableCard* Card);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_ResetDeck();
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_DealToAllPlayers(int32 CardsEach);

    UFUNCTION(BlueprintCallable) int32 GetCardsRemaining() const { return CardsRemaining; }

private:
    TArray<FDeckCardEntry> ShuffledOrder;
    void RebuildOrderFromDefinition();
    ATableCard* SpawnTopCardActor();
};
