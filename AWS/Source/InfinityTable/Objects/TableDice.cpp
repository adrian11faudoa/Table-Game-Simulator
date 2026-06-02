#include "TableDice.h"
#include "Net/UnrealNetwork.h"
#include "Scripting/LuaSubsystem.h"

ATableDice::ATableDice()
{
    PrimaryActorTick.bCanEverTick = true;
    ObjectTypeID = TEXT("Dice");
    MeshComponent->SetLinearDamping(1.5f);
    MeshComponent->SetAngularDamping(2.0f);
}

void ATableDice::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(ATableDice, DiceType);
    DOREPLIFETIME(ATableDice, CustomSides);
    DOREPLIFETIME(ATableDice, LastResult);
}

void ATableDice::Server_Roll_Implementation(FVector Impulse)
{
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->AddImpulse(Impulse, NAME_None, true);
    FVector Spin(FMath::RandRange(-1200.f,1200.f),FMath::RandRange(-1200.f,1200.f),FMath::RandRange(-600.f,600.f));
    MeshComponent->AddAngularImpulseInDegrees(Spin, NAME_None, true);
    bWasMoving = true; RestingTimer = 0.f;
}

void ATableDice::Server_RollRandom_Implementation()
{
    LastResult = FMath::RandRange(1, GetMaxValue());
    OnDiceRolled.Broadcast(this, LastResult);
    if (ULuaSubsystem* L = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        L->FireEvent(TEXT("DiceRolled"));
}

void ATableDice::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!HasAuthority() || !bWasMoving) return;
    float Speed = MeshComponent->GetPhysicsLinearVelocity().Size()
                + MeshComponent->GetPhysicsAngularVelocityInDegrees().Size();
    if (Speed < 2.f) { RestingTimer += DeltaTime; if (RestingTimer >= 0.35f) { bWasMoving=false; DetectResult(); } }
    else RestingTimer = 0.f;
}

void ATableDice::DetectResult()
{
    LastResult = CalculateFaceUp();
    OnDiceRolled.Broadcast(this, LastResult);
    if (ULuaSubsystem* L = GetGameInstance()->GetSubsystem<ULuaSubsystem>())
        L->FireEvent(TEXT("DiceRolled"));
}

int32 ATableDice::CalculateFaceUp() const { return FMath::RandRange(1, GetMaxValue()); }

int32 ATableDice::GetMaxValue() const
{
    switch(DiceType)
    {
        case EDiceType::D4:  return 4;  case EDiceType::D6:  return 6;
        case EDiceType::D8:  return 8;  case EDiceType::D10: return 10;
        case EDiceType::D12: return 12; case EDiceType::D20: return 20;
        case EDiceType::DCustom: return CustomSides;
        default: return 6;
    }
}
