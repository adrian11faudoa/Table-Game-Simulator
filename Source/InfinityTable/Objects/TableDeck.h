#pragma once
#include "CoreMinimal.h"
#include "Objects/TableObject.h"
#include "TableCard.h"
#include "TableDeck.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCardDrawn, ATableDeck*, Deck, ATableCard*, Card);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeckShuffled, ATableDeck*, Deck);

UCLASS()
class INFINITYTABLE_API ATableDeck : public ATableObject
{
    GENERATED_BODY()
public:
    ATableDeck();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintReadOnly, Replicated)
    TArray<ATableCard*> Cards;

    UPROPERTY(BlueprintAssignable) FOnCardDrawn   OnCardDrawn;
    UPROPERTY(BlueprintAssignable) FOnDeckShuffled OnDeckShuffled;

    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_Shuffle();
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_AddCard(ATableCard* Card);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_AddCardToBottom(ATableCard* Card);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_InsertCard(ATableCard* Card, int32 Index);
    UFUNCTION(Server, Reliable, BlueprintCallable) ATableCard* Server_DrawTop(bool bFaceUp = false);
    UFUNCTION(Server, Reliable, BlueprintCallable) ATableCard* Server_DrawBottom(bool bFaceUp = false);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_DealToPlayers(int32 CardsPerPlayer, bool bFaceDown = true);

    UFUNCTION(BlueprintCallable) ATableCard* PeekTop() const;
    UFUNCTION(BlueprintCallable) int32  GetCount() const { return Cards.Num(); }
    UFUNCTION(BlueprintCallable) bool   IsEmpty()  const { return Cards.IsEmpty(); }

private:
    void RepositionCards();
    static const float CardThickness; // cm
};
