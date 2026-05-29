#include "LuaAPIBindings.h"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "Objects/TableSpawnManager.h"
#include "Objects/TableObject.h"
#include "Objects/TableDice.h"
#include "Objects/TableCard.h"
#include "Objects/TableDeck.h"
#include "Objects/TableToken.h"
#include "Objects/TableMiniature.h"
#include "RPG/FogOfWarManager.h"
#include "RPG/GridManager.h"
#include "RPG/InitiativeTracker.h"
#include "EngineUtils.h"

void ULuaAPIBindings::RegisterAll(sol::state& L, UGameInstance* GI)
{
    // Init handler storage table
    L["_IT_handlers"] = L.create_table();

    RegisterObjectsAPI(L, GI);
    RegisterTableAPI(L, GI);
    RegisterEventsAPI(L, GI);
    RegisterUIAPI(L, GI);
    RegisterRPGAPI(L, GI);
    RegisterGameAPI(L, GI);
    RegisterMathAPI(L);
}

void ULuaAPIBindings::RegisterObjectsAPI(sol::state& L, UGameInstance* GI)
{
    auto Objects = L.create_named_table("Objects");

    Objects.set_function("spawn", [GI](std::string TypeID, sol::table Opts) -> ATableObject*
    {
        UTableSpawnManager* SM = GI->GetSubsystem<UTableSpawnManager>();
        if (!SM) return nullptr;

        FVector Pos(0, 0, 10);
        FRotator Rot = FRotator::ZeroRotator;

        if (Opts["position"].valid())
        {
            sol::table P = Opts["position"];
            Pos = FVector((float)P[1], (float)P[2], (float)P[3]);
        }
        return SM->SpawnObject(UTF8_TO_TCHAR(TypeID.c_str()), Pos, Rot);
    });

    Objects.set_function("destroy", [](ATableObject* Obj)
    {
        if (IsValid(Obj)) Obj->Destroy();
    });

    Objects.set_function("getAll", [GI](sol::this_state S) -> sol::table
    {
        sol::state_view View(S);
        sol::table T = View.create_table();
        int32 i = 1;
        for (TActorIterator<ATableObject> It(GI->GetWorld()); It; ++It)
            T[i++] = *It;
        return T;
    });

    Objects.set_function("getByType", [GI](std::string TypeID, sol::this_state S) -> sol::table
    {
        sol::state_view View(S);
        sol::table T = View.create_table();
        FString ID = UTF8_TO_TCHAR(TypeID.c_str());
        int32 i = 1;
        for (TActorIterator<ATableObject> It(GI->GetWorld()); It; ++It)
            if (It->ObjectTypeID == ID) T[i++] = *It;
        return T;
    });

    // Expose ATableObject
    L.new_usertype<ATableObject>("TableObject",
        "typeID",      sol::readonly(&ATableObject::ObjectTypeID),
        "owner",       sol::readonly_property([](ATableObject* O){ return 0; }),
        "setLocked",   [](ATableObject* O, bool L){ O->Server_Lock(L); },
        "flip",        [](ATableObject* O){ O->Server_Flip(); },
        "setColor",    [](ATableObject* O, float R, float G, float B){
                           O->Server_SetColor(FLinearColor(R,G,B,1)); },
        "getPosition", [](ATableObject* O, sol::this_state S) -> sol::table {
                           sol::state_view V(S);
                           sol::table T = V.create_table();
                           FVector P = O->GetActorLocation();
                           T[1]=P.X; T[2]=P.Y; T[3]=P.Z;
                           return T; },
        "setPosition", [](ATableObject* O, float X, float Y, float Z){
                           O->Server_SetTransform(FVector(X,Y,Z), O->GetActorRotation()); },
        "destroy",     [](ATableObject* O){ O->Destroy(); }
    );

    // Expose ATableDice
    L.new_usertype<ATableDice>("Dice",
        sol::base_classes, sol::bases<ATableObject>(),
        "roll",     [](ATableDice* D, float Force){ D->Server_Roll(FVector(0,0,Force)); },
        "result",   &ATableDice::GetCurrentFaceUp,
        "maxValue", &ATableDice::GetMaxValue
    );

    // Expose ATableDeck
    L.new_usertype<ATableDeck>("Deck",
        sol::base_classes, sol::bases<ATableObject>(),
        "shuffle",  [](ATableDeck* D){ D->Server_Shuffle(); },
        "draw",     [](ATableDeck* D, bool FaceUp) -> ATableCard*
                    { return D->Server_DrawTop(FaceUp); },
        "count",    &ATableDeck::GetCount,
        "isEmpty",  &ATableDeck::IsEmpty
    );
}

void ULuaAPIBindings::RegisterTableAPI(sol::state& L, UGameInstance* GI)
{
    auto Table = L.create_named_table("Table");

    Table.set_function("getPlayers", [GI](sol::this_state S) -> sol::table
    {
        sol::state_view V(S);
        sol::table T = V.create_table();
        int32 i = 1;
        for (FConstPlayerControllerIterator It = GI->GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            sol::table P = V.create_table();
            P["index"] = i;
            P["name"]  = TCHAR_TO_UTF8(*It->GetName());
            T[i++] = P;
        }
        return T;
    });

    Table.set_function("getPlayerCount", [GI]() -> int32
    {
        int32 Count = 0;
        for (FConstPlayerControllerIterator It = GI->GetWorld()->GetPlayerControllerIterator(); It; ++It)
            ++Count;
        return Count;
    });

    Table.set_function("broadcast", [GI](std::string Msg)
    {
        // Implement via game state chat broadcast
        UE_LOG(LogTemp, Log, TEXT("[Table Broadcast] %s"), UTF8_TO_TCHAR(Msg.c_str()));
    });

    Table.set_function("clearTable", [GI]()
    {
        for (TActorIterator<ATableObject> It(GI->GetWorld()); It; ++It)
            It->Destroy();
    });

    Table.set_function("getTime", [GI]() -> float
    {
        return GI->GetWorld()->GetTimeSeconds();
    });
}

void ULuaAPIBindings::RegisterEventsAPI(sol::state& L, UGameInstance* GI)
{
    auto Events = L.create_named_table("Events");

    Events.set_function("on", [&L](std::string EventName, sol::function Handler)
    {
        sol::table Handlers = L["_IT_handlers"];
        FString Event = UTF8_TO_TCHAR(EventName.c_str());
        const char* Evt = EventName.c_str();

        if (!Handlers[Evt].valid())
            Handlers[Evt] = L.create_table();

        sol::table EventList = Handlers[Evt];
        int32 N = 1;
        while (EventList[N].valid()) ++N;
        EventList[N] = Handler;
    });

    Events.set_function("emit", [&L](std::string EventName, sol::variadic_args va)
    {
        sol::table Handlers = L["_IT_handlers"];
        sol::table EventList = Handlers[EventName.c_str()];
        if (!EventList.valid()) return;
        for (auto& P : EventList)
        {
            sol::protected_function H = P.second;
            if (H.valid()) H(va);
        }
    });
}

void ULuaAPIBindings::RegisterUIAPI(sol::state& L, UGameInstance* GI)
{
    auto UI = L.create_named_table("UI");

    UI.set_function("showMessage", [](std::string Msg)
    {
        UE_LOG(LogTemp, Log, TEXT("[UI Message] %s"), UTF8_TO_TCHAR(Msg.c_str()));
        // TODO: Show on HUD toast widget
    });

    UI.set_function("updateCounter", [](int32 PlayerIdx, int32 Value)
    {
        // TODO: Update player score display
    });

    UI.set_function("setActivePlayer", [](int32 PlayerIdx)
    {
        // TODO: Highlight active player in UI
    });
}

void ULuaAPIBindings::RegisterRPGAPI(sol::state& L, UGameInstance* GI)
{
    auto RPG    = L.create_named_table("RPG");
    auto FoW    = RPG.create("fogOfWar");
    auto Grid   = RPG.create("grid");
    auto Init   = RPG.create("initiative");

    FoW.set_function("enable", [GI](bool bEnable)
    {
        for (TActorIterator<AFogOfWarManager> It(GI->GetWorld()); It; ++It)
        {
            if (bEnable) It->RevealAll();
            else         It->HideAll();
            break;
        }
    });

    FoW.set_function("revealCircle", [GI](sol::table Pos, float Radius)
    {
        FVector WPos((float)Pos[1], (float)Pos[2], (float)Pos[3]);
        for (TActorIterator<AFogOfWarManager> It(GI->GetWorld()); It; ++It)
        {
            It->RevealCircle(WPos, Radius);
            break;
        }
    });

    FoW.set_function("hideAll", [GI]()
    {
        for (TActorIterator<AFogOfWarManager> It(GI->GetWorld()); It; ++It)
        { It->HideAll(); break; }
    });
}

void ULuaAPIBindings::RegisterGameAPI(sol::state& L, UGameInstance* GI)
{
    auto Game = L.create_named_table("Game");

    Game.set_function("endGame", [GI](sol::table Result)
    {
        UE_LOG(LogTemp, Log, TEXT("Game ended via Lua"));
        // TODO: Trigger end-game sequence
    });

    Game.set_function("setVariable", [&L](std::string Key, sol::object Value)
    {
        sol::table Vars = L["_IT_vars"];
        if (!Vars.valid()) { L["_IT_vars"] = L.create_table(); Vars = L["_IT_vars"]; }
        Vars[Key] = Value;
    });

    Game.set_function("getVariable", [&L](std::string Key) -> sol::object
    {
        sol::table Vars = L["_IT_vars"];
        if (!Vars.valid()) return sol::nil;
        return Vars[Key];
    });
}

void ULuaAPIBindings::RegisterMathAPI(sol::state& L)
{
    // Vector3 helper
    L.set_function("Vector3", [](float X, float Y, float Z, sol::this_state S) -> sol::table
    {
        sol::state_view V(S);
        sol::table T = V.create_table();
        T[1]=X; T[2]=Y; T[3]=Z;
        T["x"]=X; T["y"]=Y; T["z"]=Z;
        return T;
    });
}
