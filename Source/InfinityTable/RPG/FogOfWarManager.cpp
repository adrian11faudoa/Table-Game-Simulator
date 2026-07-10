#include "RPG/FogOfWarManager.h"
#include "Net/UnrealNetwork.h"

AFogOfWarManager::AFogOfWarManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AFogOfWarManager::BeginPlay()
{
    Super::BeginPlay();
    RevealedCells.SetNumZeroed(GridWidth * GridHeight);
}

FVector2D AFogOfWarManager::WorldToGrid(FVector P) const
{
    return FVector2D(P.X / CellSizeWorld, P.Y / CellSizeWorld);
}

void AFogOfWarManager::RevealCircle(FVector WorldCenter, float RadiusWorld)
{
    FVector2D Center = WorldToGrid(WorldCenter);
    float RadiusCells = RadiusWorld / CellSizeWorld;

    for (int32 X = 0; X < GridWidth; ++X)
    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        float Dx = X - Center.X;
        float Dy = Y - Center.Y;
        if (Dx*Dx + Dy*Dy <= RadiusCells*RadiusCells)
            RevealedCells[Y * GridWidth + X] = 1;
    }

    if (HasAuthority())
        Multicast_UpdateFog(RevealedCells);
}

void AFogOfWarManager::HideAll()
{
    FMemory::Memset(RevealedCells.GetData(), 0, RevealedCells.Num());
    if (HasAuthority()) Multicast_UpdateFog(RevealedCells);
}

void AFogOfWarManager::RevealAll()
{
    FMemory::Memset(RevealedCells.GetData(), 1, RevealedCells.Num());
    if (HasAuthority()) Multicast_UpdateFog(RevealedCells);
}

bool AFogOfWarManager::IsCellRevealed(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight) return false;
    return RevealedCells[Y * GridWidth + X] != 0;
}

void AFogOfWarManager::Multicast_UpdateFog_Implementation(const TArray<uint8>& Data)
{
    RevealedCells = Data;
    // Blueprint event / material update would be triggered here
}
