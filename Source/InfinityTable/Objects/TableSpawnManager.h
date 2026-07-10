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
    UPROPERTY() float Mass        = 1.0f;
    UPROPERTY() float Friction    = 0.6f;
    UPROPERTY() float Restitution = 0.4f;
    UPROPERTY() FVector DefaultScale = FVector::OneVector;
};

UCLASS()
class INFINITYTABLE_API UTableSpawnManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    UFUNCTION(BlueprintCallable) ATableObject* SpawnObject(const FString& TypeID, FVector Position, FRotator Rotation);
    UFUNCTION(BlueprintCallable) void RegisterObjectType(const FObjectDefinition& Def);
    UFUNCTION(BlueprintCallable) TArray<FString> GetRegisteredTypeIDs() const;
private:
    UPROPERTY() TMap<FString, FObjectDefinition> ObjectRegistry;
    void RegisterBuiltInTypes();
};
