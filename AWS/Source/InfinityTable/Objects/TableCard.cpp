#include "TableCard.h"
#include "Net/UnrealNetwork.h"

ATableCard::ATableCard() { ObjectTypeID = TEXT("Card"); }
void ATableCard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(ATableCard, CardID);
    DOREPLIFETIME(ATableCard, bFaceUp);
    DOREPLIFETIME(ATableCard, bInHand);
    DOREPLIFETIME(ATableCard, HandOwner);
}
void ATableCard::Server_FlipCard_Implementation() { bFaceUp = !bFaceUp; }
void ATableCard::Server_SetFaceUp_Implementation(bool bUp) { bFaceUp = bUp; }
void ATableCard::Server_SetInHand_Implementation(APlayerController* Owner, bool bHand)
{
    bInHand = bHand; HandOwner = bHand ? Owner : nullptr;
    MeshComponent->SetSimulatePhysics(!bHand);
}
bool ATableCard::CanPlayerSeeFace(APlayerController* PC) const
{
    if (bFaceUp) return true;
    if (bInHand && HandOwner == PC) return true;
    if (PC && PC->HasAuthority()) return true;
    return false;
}
