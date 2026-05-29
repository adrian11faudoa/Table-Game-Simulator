#pragma once
#include "CoreMinimal.h"

namespace sol { class state; }
class UGameInstance;

class INFINITYTABLE_API ULuaAPIBindings
{
public:
    static void RegisterAll(sol::state& L, UGameInstance* GI);

private:
    static void RegisterObjectsAPI(sol::state& L, UGameInstance* GI);
    static void RegisterTableAPI(sol::state& L, UGameInstance* GI);
    static void RegisterEventsAPI(sol::state& L, UGameInstance* GI);
    static void RegisterUIAPI(sol::state& L, UGameInstance* GI);
    static void RegisterRPGAPI(sol::state& L, UGameInstance* GI);
    static void RegisterGameAPI(sol::state& L, UGameInstance* GI);
    static void RegisterMathAPI(sol::state& L);
};
