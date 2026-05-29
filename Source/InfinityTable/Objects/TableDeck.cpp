#include "TableDeck.h"
#include "Net/UnrealNetwork.h"
#include "Audio/AudioManagerSubsystem.h"
#include "Scripting/LuaSubsystem.h"
#include "Core/ITGameMode.h"
#include "Kismet/GameplayStatics.h"

const float ATableDeck::CardThickness = 0.35f; // cm per card

ATableDeck::ATableDeck()
{
    ObjectTypeID = TEXT("Deck");
    MeshComponent->SetSimulatePhysics(false); // Deck is static
}

void ATableDeck::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATableDeck, Cards);
}

void ATableDeck::Server_Shuffle_Implementation()
{
    // Fisher-Yates shuffle
    for (int32 i = Cards.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Cards.Swap(i, j);
    }

    RepositionCards();
    OnDeckShuffled.Broadcast(this);

    if (UAudioManagerSubsystem* AM = GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>())
        AM->PlaySoundAtLocation(TEXT("Card_Shuffle"), GetActorLocation());

    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        Lua->FireEvent(TEXT("DeckShuffled"), this);
}

void ATableDeck::Server_AddCard_Implementation(ATableCard* Card)
{
    if (!IsValid(Card)) return;
    Cards.Add(Card);
    Card->Server_SetInHand(nullptr, false);
    RepositionCards();
}

void ATableDeck::Server_AddCardToBottom_Implementation(ATableCard* Card)
{
    if (!IsValid(Card)) return;
    Cards.Insert(Card, 0);
    RepositionCards();
}

void ATableDeck::Server_InsertCard_Implementation(ATableCard* Card, int32 Index)
{
    if (!IsValid(Card)) return;
    int32 SafeIndex = FMath::Clamp(Index, 0, Cards.Num());
    Cards.Insert(Card, SafeIndex);
    RepositionCards();
}

ATableCard* ATableDeck::Server_DrawTop_Implementation(bool bFaceUp)
{
    if (Cards.IsEmpty()) return nullptr;

    ATableCard* Card = Cards.Last();
    Cards.RemoveAt(Cards.Num() - 1);

    // Place drawn card slightly above the deck
    FVector DrawPos = GetActorLocation() + FVector(20.0f, 0.0f, 20.0f);
    Card->Server_SetTransform(DrawPos, GetActorRotation());
    Card->Server_SetFaceUp(bFaceUp);
    Card->MeshComponent->SetSimulatePhysics(true);

    RepositionCards();
    OnCardDrawn.Broadcast(this, Card);

    if (UAudioManagerSubsystem* AM = GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>())
        AM->PlaySoundAtLocation(TEXT("Card_Pickup"), GetActorLocation());

    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        Lua->FireEvent(TEXT("CardDrawn"), this, Card);

    return Card;
}

ATableCard* ATableDeck::Server_DrawBottom_Implementation(bool bFaceUp)
{
    if (Cards.IsEmpty()) return nullptr;
    ATableCard* Card = Cards[0];
    Cards.RemoveAt(0);
    Card->Server_SetFaceUp(bFaceUp);
    Card->MeshComponent->SetSimulatePhysics(true);
    RepositionCards();
    OnCardDrawn.Broadcast(this, Card);
    return Card;
}

void ATableDeck::Server_DealToPlayers_Implementation(int32 CardsPerPlayer, bool bFaceDown)
{
    TArray<APlayerController*> Players;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        Players.Add(It->Get());

    for (int32 Round = 0; Round < CardsPerPlayer; ++Round)
    {
        for (APlayerController* PC : Players)
        {
            if (Cards.IsEmpty()) return;
            ATableCard* Card = Server_DrawTop(!bFaceDown);
            Card->Server_SetInHand(PC, true);
        }
    }
}

ATableCard* ATableDeck::PeekTop() const
{
    return Cards.IsEmpty() ? nullptr : Cards.Last();
}

void ATableDeck::RepositionCards()
{
    FVector BasePos = GetActorLocation();
    for (int32 i = 0; i < Cards.Num(); ++i)
    {
        if (!IsValid(Cards[i])) continue;
        FVector CardPos = BasePos + FVector(0, 0, i * CardThickness);
        Cards[i]->SetActorLocation(CardPos);
        Cards[i]->MeshComponent->SetSimulatePhysics(false);
    }

    // Update deck mesh height to reflect card count
    FVector Scale = GetActorScale3D();
    Scale.Z = FMath::Max(0.1f, Cards.Num() * CardThickness / 10.0f);
    SetActorScale3D(Scale);
}
