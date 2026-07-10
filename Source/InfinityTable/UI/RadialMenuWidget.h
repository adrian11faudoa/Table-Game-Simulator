#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RadialMenuWidget.generated.h"

class ATableObject;

USTRUCT(BlueprintType)
struct FRadialMenuOption
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString ActionID;
    UPROPERTY(BlueprintReadOnly) FString Label;
    UPROPERTY(BlueprintReadOnly) UTexture2D* Icon = nullptr;
};

UCLASS()
class INFINITYTABLE_API URadialMenuWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void OpenForObject(ATableObject* Target);
    UFUNCTION(BlueprintCallable) void Close();
    UFUNCTION(BlueprintCallable) void ExecuteAction(const FString& ActionID);

    UFUNCTION(BlueprintImplementableEvent) void OnOpened(const TArray<FRadialMenuOption>& Options);
    UFUNCTION(BlueprintImplementableEvent) void OnClosed();

private:
    UPROPERTY() ATableObject* CurrentTarget = nullptr;
    TArray<FRadialMenuOption> BuildOptionsForTarget(ATableObject* Target) const;
};
