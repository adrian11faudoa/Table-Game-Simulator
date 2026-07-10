#include "TableObject.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/Guid.h"
#include "Audio/AudioManagerSubsystem.h"
#include "Objects/TableSpawnManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"

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
    SetNetUpdateFrequency(ActiveNetUpdateFrequency);
}

void ATableObject::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority() || !MeshComponent || bIsGrabbed) return;

    float Speed = MeshComponent->GetPhysicsLinearVelocity().Size()
                + MeshComponent->GetPhysicsAngularVelocityInDegrees().Size();

    if (Speed < SettledVelocityThreshold)
    {
        SettledTimer += DeltaTime;
        if (!bNetThrottled && SettledTimer >= SettledTimeBeforeThrottle)
        {
            bNetThrottled = true;
            SetNetUpdateFrequency(SettledNetUpdateFrequency);
        }
    }
    else
    {
        SettledTimer = 0.f;
        WakeNetUpdateFrequency();
    }
}

void ATableObject::WakeNetUpdateFrequency()
{
    if (bNetThrottled)
    {
        bNetThrottled = false;
        SetNetUpdateFrequency(ActiveNetUpdateFrequency);
    }
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
    DOREPLIFETIME(ATableObject, GroupID);
}

bool ATableObject::Server_PickUp_Validate(APlayerController* Picker)
{ return IsValid(Picker) && !bIsGrabbed && CanBeGrabbedBy(Picker); }
void ATableObject::Server_PickUp_Implementation(APlayerController* Picker)
{
    bIsGrabbed = true; GrabbingPlayer = Picker;
    MeshComponent->SetSimulatePhysics(false);
    Multicast_PlayPickupSound();
    WakeNetUpdateFrequency();
}

bool ATableObject::Server_Release_Validate(FVector Vel) { return Vel.Size() < 10000.f; }
void ATableObject::Server_Release_Implementation(FVector Velocity)
{
    bIsGrabbed = false; GrabbingPlayer = nullptr;
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetPhysicsLinearVelocity(Velocity);
    Multicast_PlayReleaseSound();
    WakeNetUpdateFrequency();
}

bool ATableObject::Server_SetTransform_Validate(FVector P, FRotator R)
{ return FVector::Dist(P, GetActorLocation()) < 600.f; }
void ATableObject::Server_SetTransform_Implementation(FVector Pos, FRotator Rot)
{
    SetActorLocationAndRotation(Pos, Rot, false, nullptr, ETeleportType::TeleportPhysics);
    WakeNetUpdateFrequency();
}

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

void ATableObject::Server_SetGroup_Implementation(const FString& NewGroupID)
{
    GroupID = NewGroupID;
}

void ATableObject::Server_MoveGroupBy_Implementation(FVector Delta)
{
    if (GroupID.IsEmpty())
    {
        Server_SetTransform(GetActorLocation() + Delta, GetActorRotation());
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ATableObject> It(World); It; ++It)
    {
        if (It->GroupID == GroupID && !It->bPhysicsLocked)
        {
            It->Server_SetTransform(It->GetActorLocation() + Delta, It->GetActorRotation());
        }
    }
}

void ATableObject::Server_LockGroup_Implementation(bool bLock)
{
    if (GroupID.IsEmpty())
    {
        Server_Lock(bLock);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ATableObject> It(World); It; ++It)
    {
        if (It->GroupID == GroupID)
        {
            It->Server_Lock(bLock);
        }
    }
}

void ATableObject::Server_DestroyGroup_Implementation()
{
    if (GroupID.IsEmpty())
    {
        Destroy();
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // Collect first — destroying actors while iterating the world's actor
    // list with TActorIterator is unsafe (the iterator doesn't tolerate the
    // underlying array changing mid-iteration).
    TArray<ATableObject*> ToDestroy;
    for (TActorIterator<ATableObject> It(World); It; ++It)
    {
        if (It->GroupID == GroupID) ToDestroy.Add(*It);
    }
    for (ATableObject* Obj : ToDestroy)
    {
        if (IsValid(Obj)) Obj->Destroy();
    }
}
    UWorld* World = GetWorld();
    if (!World) return;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI) return;

    UTableSpawnManager* Spawner = GI->GetSubsystem<UTableSpawnManager>();
    if (!Spawner || ObjectTypeID.IsEmpty()) return;

    // Offset slightly so the duplicate doesn't spawn perfectly overlapping
    // the original (which would otherwise immediately physics-push them
    // apart in an unpredictable direction).
    FVector SpawnPos = GetActorLocation() + GetActorRightVector() * 15.f + FVector(0, 0, 5.f);
    ATableObject* Copy = Spawner->SpawnObject(ObjectTypeID, SpawnPos, GetActorRotation());
    if (Copy)
    {
        Copy->SetActorScale3D(GetActorScale3D());
        Copy->Server_SetColor(ObjectColor);
        // Deliberately does not copy Ownership/Tags/CustomDataJSON-style
        // per-instance state (e.g. a card's specific face, a deck's shuffle
        // order) — duplication produces a fresh instance of the same base
        // type, not a deep clone of arbitrary subclass state. Mods needing
        // deep-clone semantics for their own object types should implement
        // it via their own Lua logic using Objects.spawn + explicit field
        // copying instead of relying on this generic duplicate.
    }
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
    SetActorLocationAndRotation(State.Position, State.Rotation, false, nullptr, ETeleportType::TeleportPhysics);
    if (HasAuthority()) { Server_Lock(!State.bPhysicsEnabled); Server_SetColor(State.ObjectColor); }
}

void ATableObject::SnapToNearestPoint(float SnapRadius)
{
    FVector Loc = GetActorLocation();
    FVector Snapped(
        FMath::RoundToFloat(Loc.X / SnapRadius) * SnapRadius,
        FMath::RoundToFloat(Loc.Y / SnapRadius) * SnapRadius,
        Loc.Z
    );
    if (FVector::DistXY(Loc, Snapped) <= SnapRadius)
    {
        SetActorLocation(Snapped, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

bool ATableObject::CanBeGrabbedBy(APlayerController* PC) const
{
    if (Ownership == EObjectOwnership::Locked) return false;
    if (Ownership == EObjectOwnership::Host)   return PC->HasAuthority();
    return !bIsGrabbed;
}

void ATableObject::Multicast_PlayPickupSound_Implementation()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAudioManagerSubsystem* Audio = GI->GetSubsystem<UAudioManagerSubsystem>())
        {
            Audio->PlaySoundAtLocation(TEXT("object_pick"), GetActorLocation());
        }
    }
}

void ATableObject::Multicast_PlayReleaseSound_Implementation()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAudioManagerSubsystem* Audio = GI->GetSubsystem<UAudioManagerSubsystem>())
        {
            Audio->PlaySoundAtLocation(TEXT("object_drop"), GetActorLocation());
        }
    }
}
