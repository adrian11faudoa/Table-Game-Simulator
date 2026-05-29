#include "TableSpawnManager.h"
#include "Objects/TableDice.h"
#include "Objects/TableCard.h"
#include "Objects/TableDeck.h"
#include "Objects/TableToken.h"
#include "Objects/TableMiniature.h"
#include "Engine/World.h"

void UTableSpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterBuiltInTypes();
}

void UTableSpawnManager::RegisterBuiltInTypes()
{
    // Dice
    for (const FString& DiceID : { FString("d4"), FString("d6"), FString("d8"),
                                    FString("d10"), FString("d12"), FString("d20") })
    {
        FObjectDefinition Def;
        Def.TypeID = DiceID;
        Def.Class  = ATableDice::StaticClass();
        Def.Mass   = 0.05f; // 50g dice
        ObjectRegistry.Add(DiceID, Def);
    }

    // Card
    {
        FObjectDefinition Def;
        Def.TypeID        = TEXT("card");
        Def.Class         = ATableCard::StaticClass();
        Def.Mass          = 0.002f; // 2g card
        Def.Friction      = 0.3f;
        Def.Restitution   = 0.1f;
        Def.DefaultScale  = FVector(0.63f, 0.88f, 0.02f); // poker card proportions
        ObjectRegistry.Add(TEXT("card"), Def);
    }

    // Deck
    {
        FObjectDefinition Def;
        Def.TypeID = TEXT("deck");
        Def.Class  = ATableDeck::StaticClass();
        ObjectRegistry.Add(TEXT("deck"), Def);
    }

    // Token
    {
        FObjectDefinition Def;
        Def.TypeID       = TEXT("token");
        Def.Class        = ATableToken::StaticClass();
        Def.Mass         = 0.01f;
        Def.DefaultScale = FVector(0.3f, 0.3f, 0.05f);
        ObjectRegistry.Add(TEXT("token"), Def);
    }

    // Miniature
    {
        FObjectDefinition Def;
        Def.TypeID       = TEXT("miniature");
        Def.Class        = ATableMiniature::StaticClass();
        Def.Mass         = 0.03f;
        Def.DefaultScale = FVector(0.25f, 0.25f, 0.5f);
        ObjectRegistry.Add(TEXT("miniature"), Def);
    }
}

ATableObject* UTableSpawnManager::SpawnObject(const FString& TypeID,
                                               FVector Position,
                                               FRotator Rotation)
{
    FObjectDefinition* Def = ObjectRegistry.Find(TypeID);
    if (!Def)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SpawnManager: Unknown TypeID '%s'"), *TypeID);
        return nullptr;
    }
    return SpawnObjectWithState(*Def, Position, Rotation);
}

ATableObject* UTableSpawnManager::SpawnObjectWithState(const FObjectDefinition& Def,
                                                        FVector Position,
                                                        FRotator Rotation)
{
    UWorld* World = GetGameInstance()->GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ATableObject* Obj = World->SpawnActor<ATableObject>(
        Def.Class, Position, Rotation, Params);

    if (!Obj) return nullptr;

    Obj->ObjectTypeID = Def.TypeID;
    Obj->SetActorScale3D(Def.DefaultScale);

    if (Def.Mesh)
        Obj->MeshComponent->SetStaticMesh(Def.Mesh);

    if (Def.Material)
        Obj->MeshComponent->SetMaterial(0, Def.Material);

    // Apply physics properties
    Obj->MeshComponent->SetMassOverrideInKg(NAME_None, Def.Mass, true);

    if (UPhysicsMaterial* PM = NewObject<UPhysicsMaterial>(Obj))
    {
        PM->Friction    = Def.Friction;
        PM->Restitution = Def.Restitution;
        Obj->MeshComponent->SetPhysMaterialOverride(PM);
    }

    return Obj;
}

void UTableSpawnManager::RegisterObjectType(const FObjectDefinition& Definition)
{
    ObjectRegistry.Add(Definition.TypeID, Definition);
}

TArray<FString> UTableSpawnManager::GetRegisteredTypeIDs() const
{
    TArray<FString> Keys;
    ObjectRegistry.GetKeys(Keys);
    return Keys;
}

bool UTableSpawnManager::IsTypeRegistered(const FString& TypeID) const
{
    return ObjectRegistry.Contains(TypeID);
}
