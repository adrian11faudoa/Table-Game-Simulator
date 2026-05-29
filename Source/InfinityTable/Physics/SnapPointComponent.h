#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SnapPointComponent.generated.h"

UENUM(BlueprintType)
enum class ESnapType : uint8
{
    Any, Card, Dice, Token, Miniature
};

UCLASS(ClassGroup=(InfinityTable), meta=(BlueprintSpawnableComponent))
class INFINITYTABLE_API USnapPointComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    USnapPointComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) ESnapType SnapType    = ESnapType::Any;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float     SnapRadius  = 8.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool      bOccupied   = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool      bShowDebug  = false;

    UFUNCTION(BlueprintCallable) bool CanSnap(class ATableObject* Obj) const;
    UFUNCTION(BlueprintCallable) void SetOccupied(bool bOcc) { bOccupied = bOcc; }

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;
};
