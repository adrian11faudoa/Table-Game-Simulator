#include "LuaSubsystem.h"
#include "LuaAPIBindings.h"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

void ULuaSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void ULuaSubsystem::Initialize()
{
    LuaState = MakeUnique<sol::state>();
    OpenSafeLibraries();
    ULuaAPIBindings::RegisterAll(*LuaState, GetGameInstance());
    ApplySandbox();
    bInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("LuaSubsystem: Initialized (Lua 5.4 + sol2)"));
}

void ULuaSubsystem::OpenSafeLibraries()
{
    LuaState->open_libraries(
        sol::lib::base,
        sol::lib::math,
        sol::lib::table,
        sol::lib::string,
        sol::lib::coroutine
        // io, os, package, debug are intentionally excluded
    );
}

void ULuaSubsystem::ApplySandbox()
{
    // Block dangerous globals
    for (const char* Blocked : { "io","os","package","require","dofile",
                                  "loadfile","load","debug","collectgarbage",
                                  "rawget","rawset","rawequal","rawlen" })
    {
        (*LuaState)[Blocked] = sol::nil;
    }

    // Safe print → UE log
    LuaState->set_function("print", [](sol::variadic_args va)
    {
        FString Out;
        for (auto v : va)
        {
            std::string s = v.get<std::string>();
            Out += UTF8_TO_TCHAR(s.c_str());
            Out += TEXT(" ");
        }
        UE_LOG(LogTemp, Log, TEXT("[Lua] %s"), *Out);
    });

    // Instruction count anti-infinite-loop (100k per yield)
    lua_sethook(LuaState->lua_state(),
        [](lua_State* L, lua_Debug*)
        {
            luaL_error(L, "Lua: instruction limit exceeded (possible infinite loop)");
        },
        LUA_MASKCOUNT, 100000);
}

bool ULuaSubsystem::LoadScript(const FString& ScriptPath)
{
    if (!bInitialized) { UE_LOG(LogTemp, Warning, TEXT("Lua not initialized")); return false; }

    FString Source;
    if (!FFileHelper::LoadFileToString(Source, *ScriptPath))
    {
        UE_LOG(LogTemp, Error, TEXT("LuaSubsystem: Cannot read %s"), *ScriptPath);
        return false;
    }

    auto Result = LuaState->safe_script(TCHAR_TO_UTF8(*Source), sol::script_pass_on_error);
    if (!Result.valid())
    {
        sol::error Err = Result;
        UE_LOG(LogTemp, Error, TEXT("Lua error in [%s]: %s"), *ScriptPath, UTF8_TO_TCHAR(Err.what()));
        return false;
    }
    return true;
}

bool ULuaSubsystem::RunString(const FString& Code)
{
    if (!bInitialized) return false;
    auto R = LuaState->safe_script(TCHAR_TO_UTF8(*Code), sol::script_pass_on_error);
    if (!R.valid())
    {
        sol::error E = R;
        UE_LOG(LogTemp, Error, TEXT("Lua RunString error: %s"), UTF8_TO_TCHAR(E.what()));
        return false;
    }
    return true;
}

void ULuaSubsystem::FireEvent(const FString& EventName)
{
    if (!bInitialized) return;

    // Call global Lua function matching pattern: on<EventName>
    FString FnName = TEXT("on") + EventName;
    sol::protected_function Fn = (*LuaState)[TCHAR_TO_UTF8(*FnName)];
    if (Fn.valid())
    {
        auto R = Fn();
        if (!R.valid())
        {
            sol::error E = R;
            UE_LOG(LogTemp, Warning, TEXT("Lua event [%s] error: %s"),
                *EventName, UTF8_TO_TCHAR(E.what()));
        }
    }

    // Also fire via Events.on registered handlers (stored in _IT_handlers table)
    sol::table Handlers = (*LuaState)["_IT_handlers"];
    if (Handlers.valid())
    {
        sol::table EventHandlerList = Handlers[TCHAR_TO_UTF8(*EventName)];
        if (EventHandlerList.valid())
        {
            for (auto& Pair : EventHandlerList)
            {
                sol::protected_function H = Pair.second;
                if (H.valid()) H();
            }
        }
    }
}

void ULuaSubsystem::Deinitialize()
{
    EventHandlers.Empty();
    LuaState.Reset();
    bInitialized = false;
    Super::Deinitialize();
}
