#include "LuaSubsystem.h"
#include "LuaAPIBindings.h"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

void ULuaSubsystem::Initialize(FSubsystemCollectionBase& Collection) { Super::Initialize(Collection); }

void ULuaSubsystem::Initialize()
{
    LuaState = MakeUnique<sol::state>();
    OpenSafeLibraries();
    ULuaAPIBindings::RegisterAll(*LuaState, GetGameInstance());
    ApplySandbox();
    bInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("LuaSubsystem: Initialized"));
}

void ULuaSubsystem::OpenSafeLibraries()
{
    LuaState->open_libraries(sol::lib::base, sol::lib::math,
                              sol::lib::table, sol::lib::string, sol::lib::coroutine);
}

void ULuaSubsystem::ApplySandbox()
{
    for (const char* B : {"io","os","package","require","dofile","loadfile","load","debug","collectgarbage"})
        (*LuaState)[B] = sol::nil;
    LuaState->set_function("print", [](sol::variadic_args va)
    {
        FString Out;
        for (auto v : va) { std::string s = v.get<std::string>(); Out += UTF8_TO_TCHAR(s.c_str()); Out += TEXT(" "); }
        UE_LOG(LogTemp, Log, TEXT("[Lua] %s"), *Out);
    });
    lua_sethook(LuaState->lua_state(), [](lua_State* L, lua_Debug*) { luaL_error(L,"Lua: limit exceeded"); }, LUA_MASKCOUNT, 100000);
}

bool ULuaSubsystem::LoadScript(const FString& ScriptPath)
{
    if (!bInitialized) return false;
    FString Source;
    if (!FFileHelper::LoadFileToString(Source, *ScriptPath)) return false;
    auto R = LuaState->safe_script(TCHAR_TO_UTF8(*Source), sol::script_pass_on_error);
    if (!R.valid()) { sol::error E=R; UE_LOG(LogTemp,Error,TEXT("Lua: %s"), UTF8_TO_TCHAR(E.what())); return false; }
    return true;
}

bool ULuaSubsystem::RunString(const FString& Code)
{
    if (!bInitialized) return false;
    auto R = LuaState->safe_script(TCHAR_TO_UTF8(*Code), sol::script_pass_on_error);
    return R.valid();
}

void ULuaSubsystem::FireEvent(const FString& EventName)
{
    if (!bInitialized) return;
    FString FnName = TEXT("on") + EventName;
    sol::protected_function Fn = (*LuaState)[TCHAR_TO_UTF8(*FnName)];
    if (Fn.valid()) { auto R = Fn(); if (!R.valid()) { sol::error E=R; UE_LOG(LogTemp,Warning,TEXT("Lua event %s: %s"),*EventName,UTF8_TO_TCHAR(E.what())); } }
}

void ULuaSubsystem::Deinitialize() { LuaState.Reset(); bInitialized = false; Super::Deinitialize(); }
