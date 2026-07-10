#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TableObject.generated.h"

UENUM(BlueprintType)
enum class EObjectOwnership : uint8
{
    Public UMETA(DisplayName="Public"),
    Player UMETA(DisplayName="Player Only"),
    Host   UMETA(DisplayName="Host Only"),
    Locked UMETA(DisplayName="Locked")
};

USTRUCT(BlueprintType)
struct FTableObjectState
{
    GENERATED_BODY()
    UPROPERTY() FVector   Position;
    UPROPERTY() FRotator  Rotation;
    UPROPERTY() FVector   LinearVelocity;
    UPROPERTY() FVector   AngularVelocity;
    UPROPERTY() bool      bPhysicsEnabled = true;
    UPROPERTY() FString   ObjectTypeID;
    UPROPERTY() FLinearColor ObjectColor = FLinearColor::White;
    UPROPERTY() FString   CustomDataJSON;
};

UCLASS(Abstract)
class INFINITYTABLE_API ATableObject : public AActor
{
    GENERATED_BODY()
public:
    ATableObject();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const override;

    UFUNCTION(Server, Reliable, WithValidation) void Server_PickUp(APlayerController* Picker);
    UFUNCTION(Server, Reliable, WithValidation) void Server_Release(FVector Velocity);
    UFUNCTION(Server, Reliable, WithValidation) void Server_SetTransform(FVector Pos, FRotator Rot);
    UFUNCTION(Server, Reliable) void Server_Flip();
    UFUNCTION(Server, Reliable) void Server_Lock(bool bLock);
    UFUNCTION(Server, Reliable) void Server_SetColor(FLinearColor NewColor);
    UFUNCTION(Server, Reliable) void Server_Scale(FVector NewScale);
    UFUNCTION(Server, Reliable) void Server_Duplicate();
    UFUNCTION(NetMulticast, Reliable) void Multicast_PlayPickupSound();
    UFUNCTION(NetMulticast, Reliable) void Multicast_PlayReleaseSound();

    UFUNCTION(BlueprintCallable) FTableObjectState GetObjectState() const;
    UFUNCTION(BlueprintCallable) void ApplyObjectState(const FTableObjectState& State);
    UFUNCTION(BlueprintCallable) void SnapToNearestPoint(float SnapRadius = 10.0f);
    UFUNCTION(BlueprintCallable) FString GetGUID() const { return ObjectGUID; }
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) EObjectOwnership Ownership = EObjectOwnership::Public;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) FString ObjectTypeID;
    UPROPERTY(Replicated, BlueprintReadOnly) bool bIsGrabbed = false;
    UPROPERTY(Replicated, BlueprintReadOnly) APlayerController* GrabbingPlayer = nullptr;
    UPROPERTY(Replicated, BlueprintReadOnly) bool bPhysicsLocked = false;
    UPROPERTY(Replicated, BlueprintReadOnly) FLinearColor ObjectColor = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FString> Tags;
    UPROPERTY(Replicated, BlueprintReadOnly) FString GroupID;

    // Grouping: objects sharing a non-empty GroupID move/lock/delete as a
    // unit when these are called on any member. This is deliberately a
    // lightweight tag-based grouping rather than a physics-welded
    // attachment (see ATableHingeJoint::EJointType::Fixed for true rigid
    // welding) — group members remain independently simulated bodies, just
    // coordinated by these server actions, which keeps grouping cheap for
    // large object counts (per PERFORMANCE OPTIMIZATION) and avoids the
    // physics-constraint overhead of treating every grouped card/token as
    // a joint. Continuous click-and-drag of an entire group together
    // (as opposed to the discrete actions below) is not yet implemented —
    // UGrabComponent currently tracks a single HeldObject; extending it to
    // drag a whole group is the natural next step if that interaction is
    // needed beyond these discrete group operations.
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_SetGroup(const FString& NewGroupID);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_MoveGroupBy(FVector Delta);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_LockGroup(bool bLock);
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_DestroyGroup();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UStaticMeshComponent* MeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UAudioComponent* AudioComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UMaterialInstanceDynamic* DynMaterial;
    FString ObjectGUID;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool CanBeGrabbedBy(APlayerController* PC) const;

private:
    // Settled-object net throttling (PERFORMANCE OPTIMIZATION — large object
    // counts): once an object's physics velocity has been near-zero for a
    // sustained period, its NetUpdateFrequency is dropped from the default
    // (driven by net.MaxNetUpdateFrequency, currently 60) down to a much
    // lower idle rate, since a stationary die or card doesn't need 60
    // transform updates/sec to clients. Grabbing, releasing with velocity,
    // or any server-side transform/physics change restores the full rate
    // immediately via WakeNetUpdateFrequency() — there's no perceptible
    // delay from the throttle because the object wasn't moving anyway.
    UPROPERTY(EditDefaultsOnly) float SettledNetUpdateFrequency = 2.0f;
    UPROPERTY(EditDefaultsOnly) float ActiveNetUpdateFrequency  = 60.0f;
    UPROPERTY(EditDefaultsOnly) float SettledVelocityThreshold  = 2.0f;
    UPROPERTY(EditDefaultsOnly) float SettledTimeBeforeThrottle = 1.0f;
    float SettledTimer = 0.f;
    bool  bNetThrottled = false;
    void WakeNetUpdateFrequency();
};
