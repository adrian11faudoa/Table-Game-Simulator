#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class USlider;
class UCheckBox;
class UButton;

UCLASS()
class INFINITYTABLE_API USettingsWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable) void ApplyMusicVolume(float Volume);
    UFUNCTION(BlueprintCallable) void ApplySFXVolume(float Volume);
    UFUNCTION(BlueprintCallable) void ApplyMouseSensitivity(float Sensitivity);
    UFUNCTION(BlueprintCallable) void ApplyInvertCameraY(bool bInvert);
    UFUNCTION(BlueprintCallable) void ApplyShowFPS(bool bShow);
    UFUNCTION(BlueprintCallable) void ResetToDefaults();
    UFUNCTION(BlueprintCallable) void SaveSettings();
    UFUNCTION(BlueprintCallable) void LoadSettings();

protected:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) USlider*   MusicVolumeSlider;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) USlider*   SFXVolumeSlider;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) USlider*   MouseSensitivitySlider;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UCheckBox* InvertCameraYCheckbox;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UCheckBox* ShowFPSCheckbox;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*   ResetButton;
    UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) UButton*   ApplyButton;

private:
    UFUNCTION() void OnMusicVolumeChanged(float Value);
    UFUNCTION() void OnSFXVolumeChanged(float Value);
    UFUNCTION() void OnInvertCameraChanged(bool bChecked);
    UFUNCTION() void OnShowFPSChanged(bool bChecked);
    UFUNCTION() void OnResetClicked();
    UFUNCTION() void OnApplyClicked();

    float CachedMouseSensitivity = 1.0f;
    bool  bCachedInvertCameraY   = false;
    bool  bCachedShowFPS         = false;
};
