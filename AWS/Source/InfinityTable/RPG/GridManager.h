#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UENUM(BlueprintType) enum class EGridType : uint8 { Square, Hexagonal };

UCLASS()
class INFINITYTABLE_API AGridManager : public AActor
{
    GENERATED_BODY()
public:
    AGridManager();
    UPROPERTY(EditAnywhere, Replicated) EGridType GridType  = EGridType::Square;
    UPROPERTY(EditAnywhere, Replicated) float     CellSize  = 100.f;
    UPROPERTY(EditAnywhere, Replicated) int32     GridWidth = 24;
    UPROPERTY(EditAnywhere, Replicated) int32     GridHeight= 24;
    UPROPERTY(EditAnywhere, Replicated) bool      bShowGrid = true;
    UFUNCTION(BlueprintCallable) FIntPoint WorldToCell(FVector WorldPos) const;
    UFUNCTION(BlueprintCallable) FVector   CellToWorld(FIntPoint Cell) const;
    UFUNCTION(BlueprintCallable) FVector   SnapToGrid(FVector WorldPos) const;
    UFUNCTION(Server, Reliable, BlueprintCallable) void Server_SetGridVisible(bool bVis);
};
