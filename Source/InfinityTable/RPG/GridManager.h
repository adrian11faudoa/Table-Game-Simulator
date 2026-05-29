#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UENUM(BlueprintType)
enum class EGridType : uint8 { Square, Hexagonal };

UCLASS()
class INFINITYTABLE_API AGridManager : public AActor
{
    GENERATED_BODY()
public:
    AGridManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) EGridType GridType    = EGridType::Square;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) float     CellSize    = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32     GridWidth   = 24;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) int32     GridHeight  = 24;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated) bool      bShowGrid   = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)             FLinearColor GridColor = FLinearColor(1,1,1,0.15f);

    UFUNCTION(BlueprintCallable) FIntPoint WorldToCell(FVector WorldPos) const;
    UFUNCTION(BlueprintCallable) FVector   CellToWorld(FIntPoint Cell) const;
    UFUNCTION(BlueprintCallable) FVector   SnapToGrid(FVector WorldPos) const;
    UFUNCTION(BlueprintCallable) bool      IsCellValid(FIntPoint Cell) const;

    UFUNCTION(BlueprintCallable)
    TArray<FIntPoint> GetReachableCells(FIntPoint Origin, int32 MoveRange) const;

    UFUNCTION(BlueprintCallable)
    TArray<FIntPoint> GetPathBetween(FIntPoint From, FIntPoint To) const;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_SetGridType(EGridType NewType);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_SetGridVisible(bool bVisible);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    void DrawGrid();
    TArray<FIntPoint> GetNeighbors(FIntPoint Cell) const;
};
