#include "LuaAPIBindings.h"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "Objects/TableSpawnManager.h"
#include "Objects/TableObject.h"
#include "Objects/TableDice.h"
#include "EngineUtils.h"

void ULuaAPIBindings::RegisterAll(sol::state& L, UGameInstance* GI)
{
    L["_IT_handlers"] = L.create_table();
    L["_IT_vars"]     = L.create_table();

    // Objects
    auto Objects = L.create_named_table("Objects");
    Objects.set_function("spawn", [GI](std::string TypeID, sol::table Opts) -> ATableObject*
    {
        UTableSpawnManager* SM = GI->GetSubsystem<UTableSpawnManager>();
        if (!SM) return nullptr;
        FVector Pos(0,0,10);
        if (Opts["position"].valid()) { sol::table P=Opts["position"]; Pos=FVector((float)P[1],(float)P[2],(float)P[3]); }
        return SM->SpawnObject(UTF8_TO_TCHAR(TypeID.c_str()), Pos, FRotator::ZeroRotator);
    });
    Objects.set_function("destroy", [](ATableObject* O) { if(IsValid(O)) O->Destroy(); });
    Objects.set_function("getAll", [GI](sol::this_state S) -> sol::table
    {
        sol::state_view V(S); sol::table T=V.create_table(); int32 i=1;
        for (TActorIterator<ATableObject> It(GI->GetWorld()); It; ++It) T[i++]=*It;
        return T;
    });

    L.new_usertype<ATableObject>("TableObject",
        "typeID",      sol::readonly(&ATableObject::ObjectTypeID),
        "setLocked",   [](ATableObject* O, bool L){ O->Server_Lock(L); },
        "flip",        [](ATableObject* O){ O->Server_Flip(); },
        "setColor",    [](ATableObject* O, float R, float G, float B){ O->Server_SetColor(FLinearColor(R,G,B,1)); },
        "getPosition", [](ATableObject* O, sol::this_state S) -> sol::table
                       { sol::state_view V(S); sol::table T=V.create_table(); FVector P=O->GetActorLocation(); T[1]=P.X;T[2]=P.Y;T[3]=P.Z; return T; },
        "setPosition", [](ATableObject* O, float X, float Y, float Z){ O->Server_SetTransform(FVector(X,Y,Z), O->GetActorRotation()); },
        "destroy",     [](ATableObject* O){ O->Destroy(); }
    );

    // Events
    auto Events = L.create_named_table("Events");
    Events.set_function("on", [&L](std::string Evt, sol::function H)
    {
        sol::table Hs = L["_IT_handlers"];
        if (!Hs[Evt.c_str()].valid()) Hs[Evt.c_str()] = L.create_table();
        sol::table EL = Hs[Evt.c_str()]; int32 N=1;
        while(EL[N].valid()) ++N; EL[N] = H;
    });
    Events.set_function("emit", [&L](std::string Evt, sol::variadic_args va)
    {
        sol::table Hs = L["_IT_handlers"];
        sol::table EL = Hs[Evt.c_str()];
        if (!EL.valid()) return;
        for (auto& P : EL) { sol::protected_function H=P.second; if(H.valid()) H(va); }
    });

    // Table
    auto Table = L.create_named_table("Table");
    Table.set_function("getPlayerCount", [GI]() -> int32
    {
        int32 C=0;
        for (FConstPlayerControllerIterator It=GI->GetWorld()->GetPlayerControllerIterator(); It; ++It) ++C;
        return C;
    });
    Table.set_function("broadcast", [](std::string Msg) { UE_LOG(LogTemp,Log,TEXT("[Broadcast] %s"), UTF8_TO_TCHAR(Msg.c_str())); });
    Table.set_function("clearTable", [GI]() { for (TActorIterator<ATableObject> It(GI->GetWorld()); It; ++It) It->Destroy(); });

    // UI
    auto UI = L.create_named_table("UI");
    UI.set_function("showMessage", [](std::string Msg) { UE_LOG(LogTemp,Log,TEXT("[UI] %s"), UTF8_TO_TCHAR(Msg.c_str())); });

    // Game
    auto Game = L.create_named_table("Game");
    Game.set_function("endGame",     [](sol::table) { UE_LOG(LogTemp,Log,TEXT("Game ended")); });
    Game.set_function("setVariable", [&L](std::string K, sol::object V) { L["_IT_vars"][K]=V; });
    Game.set_function("getVariable", [&L](std::string K) -> sol::object { return L["_IT_vars"][K]; });

    // RPG stubs
    auto RPG = L.create_named_table("RPG");
    auto FoW = RPG.create("fogOfWar");
    FoW.set_function("enable",       [](bool){});
    FoW.set_function("hideAll",      [](){});
    FoW.set_function("revealAll",    [](){});
    FoW.set_function("revealCircle", [](sol::table, float){});
    auto Grid = RPG.create("grid");
    Grid.set_function("setType",    [](std::string){});
    Grid.set_function("setCellSize",[](float){});
    Grid.set_function("show",       [](bool){});
    auto Init = RPG.create("initiative");
    Init.set_function("setOrder",   [](sol::table){});
    Init.set_function("show",       [](bool){});
    Init.set_function("next",       [](){});
    Init.set_function("startCombat",[](){});
    Init.set_function("endCombat",  [](){});

    // Math
    L.set_function("Vector3", [](float X, float Y, float Z, sol::this_state S) -> sol::table
    {
        sol::state_view V(S); sol::table T=V.create_table();
        T[1]=X; T[2]=Y; T[3]=Z; T["x"]=X; T["y"]=Y; T["z"]=Z;
        return T;
    });
}
