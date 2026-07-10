#include "UI/SettingsWidget.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Audio/AudioManagerSubsystem.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
    const TCHAR* SettingsSection = TEXT("InfinityTable.UserSettings");
}

void USettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (MusicVolumeSlider)      MusicVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMusicVolumeChanged);
    if (SFXVolumeSlider)        SFXVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnSFXVolumeChanged);
    if (InvertCameraYCheckbox)  InvertCameraYCheckbox->OnCheckStateChanged.AddDynamic(this, &USettingsWidget::OnInvertCameraChanged);
    if (ShowFPSCheckbox)        ShowFPSCheckbox->OnCheckStateChanged.AddDynamic(this, &USettingsWidget::OnShowFPSChanged);
    if (ResetButton)            ResetButton->OnClicked.AddDynamic(this, &USettingsWidget::OnResetClicked);
    if (ApplyButton)            ApplyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnApplyClicked);

    LoadSettings();
}

void USettingsWidget::ApplyMusicVolume(float Volume)
{
    if (UGameInstance* GI = GetGameInstance())
        if (UAudioManagerSubsystem* Audio = GI->GetSubsystem<UAudioManagerSubsystem>())
            Audio->SetMusicVolume(Volume);
}

void USettingsWidget::ApplySFXVolume(float Volume)
{
    if (UGameInstance* GI = GetGameInstance())
        if (UAudioManagerSubsystem* Audio = GI->GetSubsystem<UAudioManagerSubsystem>())
            Audio->SetSFXVolume(Volume);
}

void USettingsWidget::ApplyMouseSensitivity(float Sensitivity)
{
    // No dedicated camera-sensitivity subsystem exists yet; cached here and
    // persisted, ready for ITPlayerController/ITSpectatorPawn to read once
    // a sensitivity multiplier is wired into their input handling (currently
    // those classes use a fixed PanSpeed — see Core/ITSpectatorPawn.h).
    CachedMouseSensitivity = FMath::Clamp(Sensitivity, 0.1f, 5.0f);
}

void USettingsWidget::ApplyInvertCameraY(bool bInvert)
{
    bCachedInvertCameraY = bInvert;
}

void USettingsWidget::ApplyShowFPS(bool bShow)
{
    bCachedShowFPS = bShow;
    if (UWorld* World = GetWorld())
    {
        // Mirrors the console command so the checkbox and "stat fps" stay in sync.
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PC->ConsoleCommand(bShow ? TEXT("stat fps") : TEXT("stat none"), true);
        }
    }
}

void USettingsWidget::ResetToDefaults()
{
    if (MusicVolumeSlider)     MusicVolumeSlider->SetValue(0.7f);
    if (SFXVolumeSlider)       SFXVolumeSlider->SetValue(1.0f);
    if (MouseSensitivitySlider) MouseSensitivitySlider->SetValue(1.0f);
    if (InvertCameraYCheckbox) InvertCameraYCheckbox->SetIsChecked(false);
    if (ShowFPSCheckbox)       ShowFPSCheckbox->SetIsChecked(false);

    ApplyMusicVolume(0.7f);
    ApplySFXVolume(1.0f);
    ApplyMouseSensitivity(1.0f);
    ApplyInvertCameraY(false);
    ApplyShowFPS(false);

    SaveSettings();
}

void USettingsWidget::SaveSettings()
{
    GConfig->SetFloat(SettingsSection, TEXT("MusicVolume"), MusicVolumeSlider ? MusicVolumeSlider->GetValue() : 0.7f, GGameUserSettingsIni);
    GConfig->SetFloat(SettingsSection, TEXT("SFXVolume"),   SFXVolumeSlider ? SFXVolumeSlider->GetValue() : 1.0f, GGameUserSettingsIni);
    GConfig->SetFloat(SettingsSection, TEXT("MouseSensitivity"), CachedMouseSensitivity, GGameUserSettingsIni);
    GConfig->SetBool (SettingsSection, TEXT("InvertCameraY"), bCachedInvertCameraY, GGameUserSettingsIni);
    GConfig->SetBool (SettingsSection, TEXT("ShowFPS"), bCachedShowFPS, GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

void USettingsWidget::LoadSettings()
{
    float MusicVol = 0.7f, SFXVol = 1.0f, MouseSens = 1.0f;
    bool bInvert = false, bFPS = false;

    GConfig->GetFloat(SettingsSection, TEXT("MusicVolume"), MusicVol, GGameUserSettingsIni);
    GConfig->GetFloat(SettingsSection, TEXT("SFXVolume"),   SFXVol,   GGameUserSettingsIni);
    GConfig->GetFloat(SettingsSection, TEXT("MouseSensitivity"), MouseSens, GGameUserSettingsIni);
    GConfig->GetBool (SettingsSection, TEXT("InvertCameraY"), bInvert, GGameUserSettingsIni);
    GConfig->GetBool (SettingsSection, TEXT("ShowFPS"), bFPS, GGameUserSettingsIni);

    if (MusicVolumeSlider)      MusicVolumeSlider->SetValue(MusicVol);
    if (SFXVolumeSlider)        SFXVolumeSlider->SetValue(SFXVol);
    if (MouseSensitivitySlider) MouseSensitivitySlider->SetValue(MouseSens);
    if (InvertCameraYCheckbox)  InvertCameraYCheckbox->SetIsChecked(bInvert);
    if (ShowFPSCheckbox)        ShowFPSCheckbox->SetIsChecked(bFPS);

    ApplyMusicVolume(MusicVol);
    ApplySFXVolume(SFXVol);
    ApplyMouseSensitivity(MouseSens);
    ApplyInvertCameraY(bInvert);
    ApplyShowFPS(bFPS);
}

void USettingsWidget::OnMusicVolumeChanged(float Value)   { ApplyMusicVolume(Value); }
void USettingsWidget::OnSFXVolumeChanged(float Value)      { ApplySFXVolume(Value); }
void USettingsWidget::OnInvertCameraChanged(bool bChecked) { ApplyInvertCameraY(bChecked); }
void USettingsWidget::OnShowFPSChanged(bool bChecked)      { ApplyShowFPS(bChecked); }
void USettingsWidget::OnResetClicked()                     { ResetToDefaults(); }
void USettingsWidget::OnApplyClicked()                     { SaveSettings(); }
