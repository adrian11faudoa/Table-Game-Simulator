#pragma once
namespace sol { class state; }
class UGameInstance;
class INFINITYTABLE_API ULuaAPIBindings
{
public:
    static void RegisterAll(sol::state& L, UGameInstance* GI);
};
