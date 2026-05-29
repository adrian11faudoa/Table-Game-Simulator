#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "FogOfWarManager.generated.h"

UCLASS()
class INFINITYTABLE_API AFogOfWarManager : public AActor
{
    GENERATED_BODY()
public:
    AFogOfWarManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32  GridWidth       = 64;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32  GridHeight      = 64;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float  CellSizeWorld   = 100.0f; // cm per cell
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector GridOrigin     = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool   bFogEnabled     = true;

    UFUNCTION(BlueprintCallable) void RevealCircle(FVector WorldCenter, float RadiusWorld);
    UFUNCTION(BlueprintCallable) void RevealRect(FVector WorldCenter, float W, float H);
    UFUNCTION(BlueprintCallable) void HideAll();
    UFUNCTION(BlueprintCallable) void RevealAll();
    UFUNCTION(BlueprintCallable) void SetCellRevealed(int32 X, int32 Y, bool bRevealed);
    UFUNCTION(BlueprintCallable) bool IsCellRevealed(int32 X, int32 Y) const;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_UpdateFogTexture(const TArray<uint8>& NewData);

    UPROPERTY(BlueprintReadOnly)
    UTextureRenderTarget2D* FogTexture;

private:
    TArray<uint8> RevealedCells; // 0=hidden, 255=revealed
    bool bDirty = false;

    FVector2D WorldToGrid(FVector WorldPos) const;
    void UpdateRenderTarget();
};
