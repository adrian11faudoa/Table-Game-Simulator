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

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UStaticMeshComponent* MeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UAudioComponent* AudioComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UMaterialInstanceDynamic* DynMaterial;
    FString ObjectGUID;
    virtual void BeginPlay() override;
    virtual bool CanBeGrabbedBy(APlayerController* PC) const;
};
