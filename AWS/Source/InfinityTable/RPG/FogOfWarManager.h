#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogOfWarManager.generated.h"

UCLASS()
class INFINITYTABLE_API AFogOfWarManager : public AActor
{
    GENERATED_BODY()
public:
    AFogOfWarManager();
    virtual void BeginPlay() override;
    UPROPERTY(EditAnywhere) int32  GridWidth  = 64;
    UPROPERTY(EditAnywhere) int32  GridHeight = 64;
    UPROPERTY(EditAnywhere) float  CellSizeWorld = 100.f;
    UFUNCTION(BlueprintCallable) void RevealCircle(FVector WorldCenter, float RadiusWorld);
    UFUNCTION(BlueprintCallable) void HideAll();
    UFUNCTION(BlueprintCallable) void RevealAll();
    UFUNCTION(BlueprintCallable) bool IsCellRevealed(int32 X, int32 Y) const;
    UFUNCTION(NetMulticast, Reliable) void Multicast_UpdateFog(const TArray<uint8>& Data);
private:
    TArray<uint8> RevealedCells;
    FVector2D WorldToGrid(FVector P) const;
};
