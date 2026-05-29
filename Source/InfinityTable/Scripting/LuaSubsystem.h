#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LuaSubsystem.generated.h"

// Forward declare sol2 state without including heavy headers in .h
namespace sol { class state; }

UCLASS()
class INFINITYTABLE_API ULuaSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Called after UE subsystems are ready
    UFUNCTION(BlueprintCallable, Category="Lua")
    void Initialize();

    UFUNCTION(BlueprintCallable, Category="Lua")
    bool LoadScript(const FString& ScriptPath);

    UFUNCTION(BlueprintCallable, Category="Lua")
    bool RunString(const FString& Code);

    UFUNCTION(BlueprintCallable, Category="Lua")
    bool IsInitialized() const { return bInitialized; }

    // Fire a named event to all registered Lua handlers
    template<typename... Args>
    void FireEvent(const FString& EventName, Args&&... args);

    void FireEvent(const FString& EventName);

    // Register/unregister handlers from Lua side
    void RegisterHandler(const FString& EventName, void* LuaFnRef);
    void UnregisterHandler(const FString& EventName, void* LuaFnRef);

    sol::state* GetLuaState() const { return LuaState.Get(); }

private:
    TUniquePtr<sol::state> LuaState;
    bool bInitialized = false;

    // EventName -> list of sol::protected_function ptrs
    TMap<FString, TArray<TSharedPtr<void>>> EventHandlers;

    void OpenSafeLibraries();
    void ApplySandbox();
};
