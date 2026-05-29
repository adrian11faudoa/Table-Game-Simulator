#include "TableCard.h"
#include "Net/UnrealNetwork.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Audio/AudioManagerSubsystem.h"

ATableCard::ATableCard()
{
    ObjectTypeID = TEXT("Card");
    MeshComponent->SetLinearDamping(3.0f);
    MeshComponent->SetAngularDamping(4.0f);
}

void ATableCard::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATableCard, CardID);
    DOREPLIFETIME(ATableCard, CardName);
    DOREPLIFETIME(ATableCard, bFaceUp);
    DOREPLIFETIME(ATableCard, bInHand);
    DOREPLIFETIME(ATableCard, HandOwner);
}

void ATableCard::Server_FlipCard_Implementation()
{
    bFaceUp = !bFaceUp;
    // Rotate mesh 180° on Y axis
    FRotator R = GetActorRotation();
    R.Pitch += 180.0f;
    SetActorRotation(R);
    ApplyTextures();

    if (UAudioManagerSubsystem* AM = GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>())
        AM->PlaySoundAtLocation(TEXT("Card_Flip"), GetActorLocation());
}

void ATableCard::Server_SetInHand_Implementation(APlayerController* Owner, bool bHand)
{
    bInHand   = bHand;
    HandOwner = bHand ? Owner : nullptr;

    // When in hand: disable physics, move to player's hand zone
    MeshComponent->SetSimulatePhysics(!bHand);
    if (bHand)
        Ownership = EObjectOwnership::Player;
    else
        Ownership = EObjectOwnership::Public;
}

void ATableCard::Server_SetFaceUp_Implementation(bool bUp)
{
    bFaceUp = bUp;
    ApplyTextures();
}

bool ATableCard::CanPlayerSeeFace(APlayerController* PC) const
{
    if (bFaceUp) return true;
    if (bInHand && HandOwner == PC) return true;
    // Host can always see
    if (PC && PC->HasAuthority()) return true;
    return false;
}

void ATableCard::ApplyTextures()
{
    if (!DynMaterial) return;
    UTexture2D* Tex = bFaceUp ? FrontTexture : BackTexture;
    if (Tex)
        DynMaterial->SetTextureParameterValue(TEXT("CardTexture"), Tex);
}
