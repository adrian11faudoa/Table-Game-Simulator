#include "TableObject.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Physics/SnapPointComponent.h"
#include "Misc/Guid.h"

ATableObject::ATableObject()
{
    bReplicates       = true;
    bReplicatePhysics = true;
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    MeshComponent->SetIsReplicated(true);
    MeshComponent->SetLinearDamping(0.5f);
    MeshComponent->SetAngularDamping(1.0f);
    RootComponent = MeshComponent;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
}

void ATableObject::BeginPlay()
{
    Super::BeginPlay();
    ObjectGUID = FGuid::NewGuid().ToString();
    DynMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
}

void ATableObject::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATableObject, Ownership);
    DOREPLIFETIME(ATableObject, ObjectTypeID);
    DOREPLIFETIME(ATableObject, bIsGrabbed);
    DOREPLIFETIME(ATableObject, GrabbingPlayer);
    DOREPLIFETIME(ATableObject, bPhysicsLocked);
    DOREPLIFETIME(ATableObject, ObjectColor);
}

bool ATableObject::Server_PickUp_Validate(APlayerController* Picker)
{
    return IsValid(Picker) && !bIsGrabbed && CanBeGrabbedBy(Picker);
}

void ATableObject::Server_PickUp_Implementation(APlayerController* Picker)
{
    bIsGrabbed     = true;
    GrabbingPlayer = Picker;
    MeshComponent->SetSimulatePhysics(false);
    Multicast_PlayPickupSound();
}

bool ATableObject::Server_Release_Validate(FVector Velocity)
{
    return Velocity.Size() < 10000.0f;
}

void ATableObject::Server_Release_Implementation(FVector Velocity)
{
    bIsGrabbed     = false;
    GrabbingPlayer = nullptr;
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetPhysicsLinearVelocity(Velocity);
    Multicast_PlayReleaseSound();
}

bool ATableObject::Server_SetTransform_Validate(FVector Pos, FRotator Rot)
{
    return FVector::Dist(Pos, GetActorLocation()) < 600.0f;
}

void ATableObject::Server_SetTransform_Implementation(FVector Pos, FRotator Rot)
{
    SetActorLocationAndRotation(Pos, Rot,
        false, nullptr, ETeleportType::TeleportPhysics);
}

void ATableObject::Server_Flip_Implementation()
{
    FRotator R = GetActorRotation();
    R.Roll += 180.0f;
    SetActorRotation(R);
    Multicast_PlayFlipSound();
}

void ATableObject::Server_Lock_Implementation(bool bLock)
{
    bPhysicsLocked = bLock;
    MeshComponent->SetSimulatePhysics(!bLock);
}

void ATableObject::Server_SetColor_Implementation(FLinearColor NewColor)
{
    ObjectColor = NewColor;
    ApplyColorToMesh();
}

void ATableObject::Server_Scale_Implementation(FVector NewScale)
{
    SetActorScale3D(NewScale);
}

void ATableObject::ApplyColorToMesh()
{
    if (DynMaterial)
        DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), ObjectColor);
}

FTableObjectState ATableObject::GetObjectState() const
{
    FTableObjectState S;
    S.Position        = GetActorLocation();
    S.Rotation        = GetActorRotation();
    S.LinearVelocity  = MeshComponent->GetPhysicsLinearVelocity();
    S.AngularVelocity = MeshComponent->GetPhysicsAngularVelocityInDegrees();
    S.bPhysicsEnabled = !bPhysicsLocked;
    S.ObjectTypeID    = ObjectTypeID;
    S.ObjectColor     = ObjectColor;
    return S;
}

void ATableObject::ApplyObjectState(const FTableObjectState& State)
{
    SetActorLocationAndRotation(State.Position, State.Rotation,
        false, nullptr, ETeleportType::TeleportPhysics);
    MeshComponent->SetPhysicsLinearVelocity(State.LinearVelocity);
    MeshComponent->SetPhysicsAngularVelocityInDegrees(State.AngularVelocity);
    if (HasAuthority()) Server_Lock(!State.bPhysicsEnabled);
    if (HasAuthority()) Server_SetColor(State.ObjectColor);
}

void ATableObject::SnapToNearestPoint(float SnapRadius)
{
    TArray<AActor*> Overlapping;
    GetOverlappingActors(Overlapping);

    for (AActor* A : Overlapping)
    {
        if (USnapPointComponent* SP = A->FindComponentByClass<USnapPointComponent>())
        {
            float Dist = FVector::Dist(GetActorLocation(), SP->GetComponentLocation());
            if (Dist < SnapRadius)
            {
                Server_SetTransform(SP->GetComponentLocation(),
                                    SP->GetComponentRotation());
                return;
            }
        }
    }
}

bool ATableObject::CanBeGrabbedBy(APlayerController* PC) const
{
    switch (Ownership)
    {
        case EObjectOwnership::Locked: return false;
        case EObjectOwnership::Host:   return PC->HasAuthority();
        case EObjectOwnership::Player:
            return GrabbingPlayer == nullptr || GrabbingPlayer == PC;
        default:
            return !bIsGrabbed;
    }
}

void ATableObject::Multicast_PlayPickupSound_Implementation()  { /* Play SFX via UAudioManagerSubsystem */ }
void ATableObject::Multicast_PlayReleaseSound_Implementation() { /* Play SFX via UAudioManagerSubsystem */ }
void ATableObject::Multicast_PlayFlipSound_Implementation()    { /* Play SFX via UAudioManagerSubsystem */ }
