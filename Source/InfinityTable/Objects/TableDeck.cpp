#include "Objects/TableDeck.h"
#include "Objects/TableCard.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Audio/AudioManagerSubsystem.h"

ATableDeck::ATableDeck()
{
    ObjectTypeID = TEXT("deck");
}

void ATableDeck::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(ATableDeck, DeckName);
    DOREPLIFETIME(ATableDeck, CardsRemaining);
}

void ATableDeck::RebuildOrderFromDefinition()
{
    ShuffledOrder = DeckDefinition;
    CardsRemaining = ShuffledOrder.Num();
}

void ATableDeck::Server_Shuffle_Implementation()
{
    if (ShuffledOrder.Num() == 0)
        RebuildOrderFromDefinition();

    // Fisher-Yates shuffle
    for (int32 i = ShuffledOrder.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        ShuffledOrder.Swap(i, j);
    }
    CardsRemaining = ShuffledOrder.Num();
    Multicast_PlayPickupSound();
}

ATableCard* ATableDeck::SpawnTopCardActor()
{
    if (ShuffledOrder.Num() == 0) return nullptr;

    FDeckCardEntry Top = ShuffledOrder.Pop();
    CardsRemaining = ShuffledOrder.Num();

    UWorld* World = GetWorld();
    if (!World || !CardClass) return nullptr;

    FVector SpawnLoc = GetActorLocation() + FVector(0, 0, 5.f);
    ATableCard* Card = World->SpawnActor<ATableCard>(CardClass, SpawnLoc, GetActorRotation());
    if (Card)
    {
        Card->CardID = Top.CardID;
        Card->bFaceUp = false;
    }
    return Card;
}

void ATableDeck::Server_DrawTopCard_Implementation(APlayerController* Drawer)
{
    ATableCard* Card = SpawnTopCardActor();
    if (Card && Drawer)
    {
        Card->Server_SetInHand(Drawer, true);
    }
    if (Card)
    {
        if (UAudioManagerSubsystem* Audio = GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>())
            Audio->PlaySoundAtLocation(TEXT("card_flip"), GetActorLocation());
    }
}

void ATableDeck::Server_DrawToHand_Implementation(APlayerController* Drawer, int32 Count)
{
    for (int32 i = 0; i < Count; ++i)
    {
        Server_DrawTopCard_Implementation(Drawer);
    }
}

void ATableDeck::Server_DiscardCard_Implementation(ATableCard* Card)
{
    if (!Card) return;
    Card->Server_SetInHand(nullptr, false);
    Card->Server_SetFaceUp(true);
    if (bIsDiscardPile)
    {
        Card->SetActorLocation(GetActorLocation() + FVector(0, 0, 5.f * (DeckDefinition.Num() - CardsRemaining)));
    }
}

void ATableDeck::Server_ResetDeck_Implementation()
{
    RebuildOrderFromDefinition();
}

void ATableDeck::Server_DealToAllPlayers_Implementation(int32 CardsEach)
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<APlayerController*> Controllers;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
            Controllers.Add(PC);
    }

    for (int32 Round = 0; Round < CardsEach; ++Round)
    {
        for (APlayerController* PC : Controllers)
        {
            Server_DrawTopCard_Implementation(PC);
        }
    }
}
