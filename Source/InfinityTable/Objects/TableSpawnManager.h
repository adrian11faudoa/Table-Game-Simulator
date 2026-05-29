#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Objects/TableObject.h"
#include "TableSpawnManager.generated.h"

USTRUCT(BlueprintType)
struct FObjectDefinition
{
    GENERATED_BODY()
    UPROPERTY() FString TypeID;
    UPROPERTY() TSubclassOf<ATableObject> Class;
    UPROPERTY() UStaticMesh* Mesh;
    UPROPERTY() UMaterialInterface* Material;
    UPROPERTY() float Mass           = 1.0f;
    UPROPERTY() float Friction       = 0.6f;
    UPROPERTY() float Restitution    = 0.4f;
    UPROPERTY() FVector DefaultScale = FVector::OneVector;
};

UCLASS()
class INFINITYTABLE_API UTableSpawnManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category="Spawn")
    ATableObject* SpawnObject(const FString& TypeID,
                               FVector Position,
                               FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category="Spawn")
    ATableObject* SpawnObjectWithState(const FObjectDefinition& Def,
                                        FVector Position,
                                        FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category="Spawn")
    void RegisterObjectType(const FObjectDefinition& Definition);

    UFUNCTION(BlueprintCallable, Category="Spawn")
    TArray<FString> GetRegisteredTypeIDs() const;

    UFUNCTION(BlueprintCallable, Category="Spawn")
    bool IsTypeRegistered(const FString& TypeID) const;

private:
    UPROPERTY()
    TMap<FString, FObjectDefinition> ObjectRegistry;

    void RegisterBuiltInTypes();
};
