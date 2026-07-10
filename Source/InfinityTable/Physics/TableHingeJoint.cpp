#include "Physics/TableHingeJoint.h"
#include "Objects/TableObject.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ATableHingeJoint::ATableHingeJoint()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Constraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Constraint"));
    Constraint->SetupAttachment(Root);
}

void ATableHingeJoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(ATableHingeJoint, JointType);
    DOREPLIFETIME(ATableHingeJoint, ObjectA);
    DOREPLIFETIME(ATableHingeJoint, ObjectB);
}

void ATableHingeJoint::BeginPlay()
{
    Super::BeginPlay();

    // The constraint is only meaningful on the server, which owns physics
    // authority for the whole table (see ATableObject's bReplicatePhysics
    // model). Clients receive the resulting motion via normal physics
    // replication on ObjectA/ObjectB's mesh components, not via this joint
    // actor directly, so there's no need to construct the constraint
    // client-side — doing so would just create a redundant, unauthoritative
    // local simulation that could visually fight the replicated transform.
    if (HasAuthority())
    {
        ConfigureConstraint();
    }
}

void ATableHingeJoint::ConfigureConstraint()
{
    if (!Constraint || !ObjectA || !ObjectA->MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("TableHingeJoint: ObjectA (or its mesh) is missing — joint not configured."));
        return;
    }

    switch (JointType)
    {
        case EJointType::Hinge:
            Constraint->SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
            Constraint->SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
            Constraint->SetAngularTwistMotion(EAngularConstraintMotion::ACM_Limited);
            Constraint->SetAngularTwistLimit(FMath::Max(FMath::Abs(HingeMinAngle), FMath::Abs(HingeMaxAngle)));
            Constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            Constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            break;

        case EJointType::Fixed:
            Constraint->SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
            Constraint->SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
            Constraint->SetAngularTwistMotion(EAngularConstraintMotion::ACM_Locked);
            Constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            Constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            break;

        case EJointType::BallSocket:
            Constraint->SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Limited);
            Constraint->SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Limited);
            Constraint->SetAngularTwistMotion(EAngularConstraintMotion::ACM_Limited);
            Constraint->SetAngularSwing1Limit(SwingLimit);
            Constraint->SetAngularSwing2Limit(SwingLimit);
            Constraint->SetAngularTwistLimit(TwistLimit);
            Constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            Constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
            break;
    }

    if (ObjectB && ObjectB->MeshComponent)
    {
        // Two movable objects: standard A/B constraint.
        Constraint->SetConstrainedComponents(
            ObjectA->MeshComponent, NAME_None,
            ObjectB->MeshComponent, NAME_None);
    }
    else
    {
        // No ObjectB: anchor ObjectA to this joint actor's own (static,
        // unsimulated) root component instead — e.g. a door hinged to a
        // fixed wall rather than to another movable prop.
        Constraint->SetConstrainedComponents(
            ObjectA->MeshComponent, NAME_None,
            Root, NAME_None);
    }

    if (bEnableMotor && JointType == EJointType::Hinge)
    {
        Constraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
        Constraint->SetAngularVelocityDrive(false, true);
        Constraint->SetAngularDriveParams(MotorMaxTorque, MotorMaxTorque, 0.f);
        Constraint->SetAngularOrientationTarget(FRotator::ZeroRotator);
    }
}

void ATableHingeJoint::Server_BreakJoint_Implementation()
{
    if (Constraint)
    {
        Constraint->BreakConstraint();
    }
}

void ATableHingeJoint::Server_SetMotorEnabled_Implementation(bool bEnabled, float TargetVelocityDegPerSec)
{
    bEnableMotor = bEnabled;
    MotorTargetVelocityDegPerSec = TargetVelocityDegPerSec;

    if (!Constraint) return;

    if (bEnabled)
    {
        Constraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
        Constraint->SetAngularVelocityDrive(false, true);
        Constraint->SetAngularDriveParams(MotorMaxTorque, MotorMaxTorque, 0.f);
        // Target velocity is applied via the twist/swing target rotation
        // rate; UE5's constraint API drives toward an orientation rather
        // than a raw angular velocity directly, so callers wanting a
        // continuously-spinning motor (vs. a damped swing to a target
        // angle) should tick a Blueprint timeline updating
        // SetAngularOrientationTarget incrementally — this function sets
        // up the drive parameters but does not itself animate the target.
    }
    else
    {
        Constraint->SetAngularVelocityDrive(false, false);
    }
}

float ATableHingeJoint::GetCurrentAngle() const
{
    if (!ObjectA || !ObjectB || !ObjectA->MeshComponent || !ObjectB->MeshComponent)
        return 0.f;

    FRotator RelativeRot = (ObjectA->MeshComponent->GetComponentRotation()
                           - ObjectB->MeshComponent->GetComponentRotation());
    return RelativeRot.Yaw;
}
