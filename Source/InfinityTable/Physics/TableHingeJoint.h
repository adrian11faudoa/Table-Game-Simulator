#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TableHingeJoint.generated.h"

class ATableObject;
class UPhysicsConstraintComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class EJointType : uint8
{
    Hinge       UMETA(DisplayName="Hinge (door/lid, 1-axis rotation)"),
    Fixed       UMETA(DisplayName="Fixed (rigidly welds two objects)"),
    BallSocket  UMETA(DisplayName="Ball & Socket (free rotation, e.g. articulated minis)")
};

/**
 * Connects two ATableObjects (or one object to a fixed point in the world)
 * via a UE5 physics constraint, covering the prompt's "Joint/hinge systems"
 * requirement under PHYSICS SYSTEM. Typical uses: a chest lid that swings
 * open, a door in RPG terrain, an articulated miniature's limb.
 *
 * This is a server-authoritative actor like ATableObject — the constraint
 * itself is simulated by Chaos Physics on the server and the constrained
 * objects' transforms replicate down to clients the same way any other
 * ATableObject does (via bReplicatePhysics on the underlying mesh
 * components), so no extra replication logic is needed here beyond
 * ensuring the joint is created/destroyed on the server.
 */
UCLASS()
class INFINITYTABLE_API ATableHingeJoint : public AActor
{
    GENERATED_BODY()
public:
    ATableHingeJoint();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) EJointType JointType = EJointType::Hinge;

    // The two objects this joint connects. ObjectB may be left null to
    // instead anchor ObjectA to this actor's fixed world location (e.g. a
    // door hinged to a static wall rather than to another movable object).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) ATableObject* ObjectA = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) ATableObject* ObjectB = nullptr;

    // Hinge-specific limits (degrees). Ignored for Fixed/BallSocket.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HingeMinAngle = -90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HingeMaxAngle = 90.0f;

    // Ball & socket swing/twist limits (degrees). Ignored for Hinge/Fixed.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SwingLimit = 45.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TwistLimit = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnableMotor = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MotorTargetVelocityDegPerSec = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MotorMaxTorque = 5000.0f;

    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_BreakJoint();
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_SetMotorEnabled(bool bEnabled, float TargetVelocityDegPerSec);
    UFUNCTION(BlueprintCallable) float GetCurrentAngle() const;

protected:
    UPROPERTY(VisibleAnywhere) USceneComponent* Root;
    UPROPERTY(VisibleAnywhere) UPhysicsConstraintComponent* Constraint;

private:
    void ConfigureConstraint();
};
