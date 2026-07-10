#include "RPG/GridManager.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void AGridManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps) const
{
    Super::GetLifetimeReplicatedProps(OutProps);
    DOREPLIFETIME(AGridManager, GridType);
    DOREPLIFETIME(AGridManager, CellSize);
    DOREPLIFETIME(AGridManager, GridWidth);
    DOREPLIFETIME(AGridManager, GridHeight);
    DOREPLIFETIME(AGridManager, bShowGrid);
}

FIntPoint AGridManager::WorldToCell(FVector WorldPos) const
{
    FVector Local = WorldPos - GetActorLocation();
    if (GridType == EGridType::Square)
        return FIntPoint(FMath::FloorToInt(Local.X / CellSize), FMath::FloorToInt(Local.Y / CellSize));

    // Hexagonal offset
    float q = (2.f/3.f * Local.X) / CellSize;
    float r = (-1.f/3.f * Local.X + FMath::Sqrt(3.f)/3.f * Local.Y) / CellSize;
    return FIntPoint(FMath::RoundToInt(q), FMath::RoundToInt(r));
}

FVector AGridManager::CellToWorld(FIntPoint Cell) const
{
    FVector Origin = GetActorLocation();
    if (GridType == EGridType::Square)
        return Origin + FVector(Cell.X * CellSize + CellSize*0.5f, Cell.Y * CellSize + CellSize*0.5f, 0.f);

    // Hexagonal pointy-top
    float X = CellSize * (FMath::Sqrt(3.f) * Cell.X + FMath::Sqrt(3.f)/2.f * Cell.Y);
    float Y = CellSize * (3.f/2.f * Cell.Y);
    return Origin + FVector(X, Y, 0.f);
}

FVector AGridManager::SnapToGrid(FVector WorldPos) const
{
    return CellToWorld(WorldToCell(WorldPos));
}

void AGridManager::Server_SetGridVisible_Implementation(bool bVis)
{
    bShowGrid = bVis;
}
