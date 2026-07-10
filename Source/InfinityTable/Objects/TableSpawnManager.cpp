#include "Objects/TableSpawnManager.h"
#include "Objects/TableCard.h"
#include "Objects/TableDeck.h"
#include "Objects/TableDice.h"
#include "Objects/TableMiniature.h"
#include "Objects/TableToken.h"
#include "Engine/World.h"

void UTableSpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterBuiltInTypes();
}

void UTableSpawnManager::RegisterBuiltInTypes()
{
    auto Reg = [this](FString ID, TSubclassOf<ATableObject> Class, float Mass, float Fric, float Rest, FVector Scale)
    {
        FObjectDefinition Def;
        Def.TypeID       = ID;
        Def.Class        = Class;
        Def.Mass         = Mass;
        Def.Friction     = Fric;
        Def.Restitution  = Rest;
        Def.DefaultScale = Scale;
        RegisterObjectType(Def);
    };

    Reg(TEXT("card"),       ATableCard::StaticClass(),      0.005f, 0.4f, 0.1f, FVector(0.7f, 1.0f, 0.01f));
    Reg(TEXT("deck"),       ATableDeck::StaticClass(),      0.05f,  0.4f, 0.1f, FVector(0.7f, 1.0f, 0.1f));
    Reg(TEXT("discard_pile"), ATableDeck::StaticClass(),    0.05f,  0.4f, 0.1f, FVector(0.7f, 1.0f, 0.1f));
    Reg(TEXT("d6"),         ATableDice::StaticClass(),      0.015f, 0.5f, 0.5f, FVector::OneVector);
    Reg(TEXT("d20"),        ATableDice::StaticClass(),      0.015f, 0.4f, 0.4f, FVector::OneVector);
    Reg(TEXT("miniature"),  ATableMiniature::StaticClass(), 0.05f,  0.7f, 0.1f, FVector::OneVector);
    Reg(TEXT("token"),      ATableToken::StaticClass(),     0.005f, 0.6f, 0.05f, FVector(1.f, 1.f, 0.1f));
}

void UTableSpawnManager::RegisterObjectType(const FObjectDefinition& Def)
{
    ObjectRegistry.Add(Def.TypeID, Def);
}

ATableObject* UTableSpawnManager::SpawnObject(const FString& TypeID, FVector Position, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FObjectDefinition* Def = ObjectRegistry.Find(TypeID);
    if (!Def || !Def->Class) return nullptr;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ATableObject* Obj = World->SpawnActor<ATableObject>(Def->Class, Position, Rotation, Params);
    if (Obj)
    {
        Obj->SetActorScale3D(Def->DefaultScale);
        if (UStaticMeshComponent* Mesh = Obj->FindComponentByClass<UStaticMeshComponent>())
        {
            Mesh->SetMassOverrideInKg(NAME_None, Def->Mass * 1000.f);
            Mesh->SetScalarParameterValueOnMaterials(TEXT("Friction"), Def->Friction);
        }
    }
    return Obj;
}

TArray<FString> UTableSpawnManager::GetRegisteredTypeIDs() const
{
    TArray<FString> Out;
    ObjectRegistry.GetKeys(Out);
    return Out;
}
