#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LuaSubsystem.generated.h"

namespace sol { class state; }

UCLASS()
class INFINITYTABLE_API ULuaSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    UFUNCTION(BlueprintCallable) void Initialize();
    UFUNCTION(BlueprintCallable) bool LoadScript(const FString& ScriptPath);
    UFUNCTION(BlueprintCallable) bool RunString(const FString& Code);
    UFUNCTION(BlueprintCallable) bool IsInitialized() const { return bInitialized; }
    void FireEvent(const FString& EventName);
    sol::state* GetLuaState() const { return LuaState.Get(); }
private:
    TUniquePtr<sol::state> LuaState;
    bool bInitialized = false;
    void OpenSafeLibraries();
    void ApplySandbox();
};
