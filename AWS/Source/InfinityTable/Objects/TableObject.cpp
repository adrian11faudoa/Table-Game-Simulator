#include "TableObject.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
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

void ATableObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(ATableObject, Ownership);
    DOREPLIFETIME(ATableObject, ObjectTypeID);
    DOREPLIFETIME(ATableObject, bIsGrabbed);
    DOREPLIFETIME(ATableObject, GrabbingPlayer);
    DOREPLIFETIME(ATableObject, bPhysicsLocked);
    DOREPLIFETIME(ATableObject, ObjectColor);
}

bool ATableObject::Server_PickUp_Validate(APlayerController* Picker)
{ return IsValid(Picker) && !bIsGrabbed && CanBeGrabbedBy(Picker); }
void ATableObject::Server_PickUp_Implementation(APlayerController* Picker)
{
    bIsGrabbed = true; GrabbingPlayer = Picker;
    MeshComponent->SetSimulatePhysics(false);
    Multicast_PlayPickupSound();
}

bool ATableObject::Server_Release_Validate(FVector Vel) { return Vel.Size() < 10000.f; }
void ATableObject::Server_Release_Implementation(FVector Velocity)
{
    bIsGrabbed = false; GrabbingPlayer = nullptr;
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetPhysicsLinearVelocity(Velocity);
    Multicast_PlayReleaseSound();
}

bool ATableObject::Server_SetTransform_Validate(FVector P, FRotator R)
{ return FVector::Dist(P, GetActorLocation()) < 600.f; }
void ATableObject::Server_SetTransform_Implementation(FVector Pos, FRotator Rot)
{ SetActorLocationAndRotation(Pos, Rot, false, nullptr, ETeleportType::TeleportPhysics); }

void ATableObject::Server_Flip_Implementation()
{
    FRotator R = GetActorRotation(); R.Roll += 180.f;
    SetActorRotation(R);
}

void ATableObject::Server_Lock_Implementation(bool bLock)
{ bPhysicsLocked = bLock; MeshComponent->SetSimulatePhysics(!bLock); }

void ATableObject::Server_SetColor_Implementation(FLinearColor C)
{
    ObjectColor = C;
    if (DynMaterial) DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), C);
}

void ATableObject::Server_Scale_Implementation(FVector S) { SetActorScale3D(S); }

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
    SetActorLocationAndRotation(State.Position, State.Rotation, false, nullptr, ETeleportType::TeleportPhysics);
    if (HasAuthority()) { Server_Lock(!State.bPhysicsEnabled); Server_SetColor(State.ObjectColor); }
}

void ATableObject::SnapToNearestPoint(float R) {}
bool ATableObject::CanBeGrabbedBy(APlayerController* PC) const
{
    if (Ownership == EObjectOwnership::Locked) return false;
    if (Ownership == EObjectOwnership::Host)   return PC->HasAuthority();
    return !bIsGrabbed;
}
void ATableObject::Multicast_PlayPickupSound_Implementation()  {}
void ATableObject::Multicast_PlayReleaseSound_Implementation() {}
