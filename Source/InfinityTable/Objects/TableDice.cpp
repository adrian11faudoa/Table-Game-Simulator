#include "TableDice.h"
#include "Net/UnrealNetwork.h"
#include "Scripting/LuaSubsystem.h"
#include "Audio/AudioManagerSubsystem.h"

ATableDice::ATableDice()
{
    PrimaryActorTick.bCanEverTick = true;
    ObjectTypeID = TEXT("Dice");

    // Dice have higher friction + lower restitution for realistic rolling
    MeshComponent->SetLinearDamping(1.5f);
    MeshComponent->SetAngularDamping(2.0f);
    if (UPhysicsMaterial* PM = NewObject<UPhysicsMaterial>())
    {
        PM->Friction    = 0.8f;
        PM->Restitution = 0.3f;
        MeshComponent->SetPhysMaterialOverride(PM);
    }
}

void ATableDice::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATableDice, DiceType);
    DOREPLIFETIME(ATableDice, CustomSides);
    DOREPLIFETIME(ATableDice, LastResult);
}

void ATableDice::Server_Roll_Implementation(FVector Impulse)
{
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

    // Main impulse
    MeshComponent->AddImpulse(Impulse, NAME_None, true);

    // Random spin for unpredictability
    FVector Spin = FVector(
        FMath::RandRange(-1200.f, 1200.f),
        FMath::RandRange(-1200.f, 1200.f),
        FMath::RandRange(-600.f,   600.f)
    );
    MeshComponent->AddAngularImpulseInDegrees(Spin, NAME_None, true);

    bWasMoving   = true;
    RestingTimer = 0.0f;

    if (UAudioManagerSubsystem* AM = GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>())
        AM->PlaySoundAtLocation(TEXT("Dice_Roll_Hard"), GetActorLocation());
}

void ATableDice::Server_RollRandom_Implementation()
{
    int32 Result = FMath::RandRange(1, GetMaxValue());
    BroadcastResult(Result);
}

void ATableDice::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!HasAuthority() || !bWasMoving) return;

    float LinearSpeed  = MeshComponent->GetPhysicsLinearVelocity().Size();
    float AngularSpeed = MeshComponent->GetPhysicsAngularVelocityInDegrees().Size();

    if (LinearSpeed < 1.5f && AngularSpeed < 2.0f)
    {
        RestingTimer += DeltaTime;
        if (RestingTimer >= RestThreshold)
        {
            bWasMoving = false;
            DetectResult();
        }
    }
    else
    {
        RestingTimer = 0.0f;
    }
}

void ATableDice::DetectResult()
{
    int32 Result = CalculateFaceUp();
    BroadcastResult(Result);
}

void ATableDice::BroadcastResult(int32 Result)
{
    LastResult = Result;
    OnDiceRolled.Broadcast(this, Result);

    if (UAudioManagerSubsystem* AM = GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>())
        AM->PlaySoundAtLocation(TEXT("Dice_Settle"), GetActorLocation());

    if (ULuaSubsystem* Lua = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        Lua->FireEvent(TEXT("DiceRolled"), this, Result);

    UE_LOG(LogTemp, Log, TEXT("Dice %s rolled: %d (max %d)"),
        *ObjectTypeID, Result, GetMaxValue());
}

int32 ATableDice::CalculateFaceUp() const
{
    TArray<FVector> FaceNormals = GetFaceNormals(DiceType);
    if (FaceNormals.IsEmpty()) return FMath::RandRange(1, GetMaxValue());

    FVector WorldUp = FVector::UpVector;
    const FTransform T = GetActorTransform();

    int32 BestFace = 1;
    float BestDot  = -2.0f;

    for (int32 i = 0; i < FaceNormals.Num(); ++i)
    {
        FVector WorldNormal = T.TransformVectorNoScale(FaceNormals[i]);
        float   Dot         = FVector::DotProduct(WorldNormal, WorldUp);
        if (Dot > BestDot)
        {
            BestDot  = Dot;
            BestFace = i + 1;
        }
    }
    return BestFace;
}

int32 ATableDice::GetCurrentFaceUp() const { return LastResult; }

int32 ATableDice::GetMaxValue() const
{
    switch (DiceType)
    {
        case EDiceType::D4:     return 4;
        case EDiceType::D6:     return 6;
        case EDiceType::D8:     return 8;
        case EDiceType::D10:    return 10;
        case EDiceType::D10P:   return 10;
        case EDiceType::D12:    return 12;
        case EDiceType::D20:    return 20;
        case EDiceType::DCustom: return CustomSides;
        default: return 6;
    }
}

TArray<FVector> ATableDice::GetFaceNormals(EDiceType Type)
{
    // D6 face normals (cube): +X,-X,+Y,-Y,+Z,-Z
    if (Type == EDiceType::D6)
    {
        return {
            FVector( 1, 0, 0), FVector(-1, 0, 0),
            FVector( 0, 1, 0), FVector( 0,-1, 0),
            FVector( 0, 0, 1), FVector( 0, 0,-1)
        };
    }

    // For other dice types: distribute normals uniformly on sphere
    int32 Sides = 4;
    switch (Type)
    {
        case EDiceType::D4:  Sides = 4;  break;
        case EDiceType::D8:  Sides = 8;  break;
        case EDiceType::D12: Sides = 12; break;
        case EDiceType::D20: Sides = 20; break;
        default: Sides = 6; break;
    }

    TArray<FVector> Normals;
    Normals.Reserve(Sides);
    for (int32 i = 0; i < Sides; ++i)
    {
        float Theta = 2.0f * PI * i / Sides;
        float Phi   = FMath::Acos(1.0f - 2.0f * (i + 0.5f) / Sides);
        Normals.Add(FVector(
            FMath::Sin(Phi) * FMath::Cos(Theta),
            FMath::Sin(Phi) * FMath::Sin(Theta),
            FMath::Cos(Phi)
        ));
    }
    return Normals;
}
