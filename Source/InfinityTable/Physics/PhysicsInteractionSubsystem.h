#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Objects/TableObject.h"
#include "PhysicsInteractionSubsystem.generated.h"

UCLASS()
class INFINITYTABLE_API UPhysicsInteractionSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Sleep inactive objects to save physics budget
    UFUNCTION(BlueprintCallable)
    void SleepDistantObjects(FVector ViewCenter, float KeepAwakeRadius = 600.0f);

    // Group management
    UFUNCTION(BlueprintCallable)
    void GroupObjects(const TArray<ATableObject*>& Objects, int32& OutGroupID);

    UFUNCTION(BlueprintCallable)
    void UngroupObjects(int32 GroupID);

    UFUNCTION(BlueprintCallable)
    TArray<ATableObject*> GetGroup(int32 GroupID) const;

    UFUNCTION(BlueprintCallable)
    void MoveGroup(int32 GroupID, FVector Delta);

    // Undo stack (last 20 moves)
    UFUNCTION(BlueprintCallable)
    void RecordState();

    UFUNCTION(BlueprintCallable)
    void Undo();

private:
    int32 NextGroupID = 1;
    TMap<int32, TArray<ATableObject*>> ObjectGroups;

    // Undo stack
    struct FUndoFrame
    {
        TMap<ATableObject*, FTableObjectState> ObjectStates;
        float Timestamp;
    };
    TArray<FUndoFrame> UndoStack;
    static const int32 MaxUndoDepth = 20;

    float SleepCheckInterval = 3.0f;
    float SleepCheckTimer    = 0.0f;
};
