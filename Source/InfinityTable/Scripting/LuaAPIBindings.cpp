#include "LuaAPIBindings.h"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "Objects/TableSpawnManager.h"
#include "Objects/TableObject.h"
#include "Objects/TableDice.h"
#include "RPG/FogOfWarManager.h"
#include "RPG/GridManager.h"
#include "Physics/TableHingeJoint.h"
#include "Save/SaveGameSubsystem.h"
#include "Engine/World.h"
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
        "duplicate",   [](ATableObject* O){ O->Server_Duplicate(); },
        "setGroup",    [](ATableObject* O, std::string GroupID){ O->Server_SetGroup(UTF8_TO_TCHAR(GroupID.c_str())); },
        "getGroup",    [](ATableObject* O) -> std::string { return TCHAR_TO_UTF8(*O->GroupID); },
        "moveGroupBy", [](ATableObject* O, float DX, float DY, float DZ){ O->Server_MoveGroupBy(FVector(DX,DY,DZ)); },
        "lockGroup",   [](ATableObject* O, bool bLock){ O->Server_LockGroup(bLock); },
        "destroyGroup",[](ATableObject* O){ O->Server_DestroyGroup(); },
        "destroy",     [](ATableObject* O){ O->Destroy(); }
    );

    // Physics — joint/hinge creation (doors, lids, articulated props).
    // See Source/InfinityTable/Physics/TableHingeJoint.h.
    auto Physics = L.create_named_table("Physics");
    Physics.set_function("createHinge", [GI](ATableObject* ObjA, ATableObject* ObjB, float MinAngle, float MaxAngle) -> ATableHingeJoint*
    {
        UWorld* World = GI ? GI->GetWorld() : nullptr;
        if (!World || !ObjA) return nullptr;
        ATableHingeJoint* Joint = World->SpawnActor<ATableHingeJoint>();
        if (Joint)
        {
            Joint->JointType = EJointType::Hinge;
            Joint->ObjectA = ObjA;
            Joint->ObjectB = ObjB; // may be nullptr — anchors ObjA to a fixed point
            Joint->HingeMinAngle = MinAngle;
            Joint->HingeMaxAngle = MaxAngle;
        }
        return Joint;
    });
    Physics.set_function("createFixedJoint", [GI](ATableObject* ObjA, ATableObject* ObjB) -> ATableHingeJoint*
    {
        UWorld* World = GI ? GI->GetWorld() : nullptr;
        if (!World || !ObjA || !ObjB) return nullptr;
        ATableHingeJoint* Joint = World->SpawnActor<ATableHingeJoint>();
        if (Joint)
        {
            Joint->JointType = EJointType::Fixed;
            Joint->ObjectA = ObjA;
            Joint->ObjectB = ObjB;
        }
        return Joint;
    });
    Physics.set_function("breakJoint", [](ATableHingeJoint* Joint) { if (Joint) Joint->Server_BreakJoint(); });

    L.new_usertype<ATableHingeJoint>("TableHingeJoint",
        "getCurrentAngle", &ATableHingeJoint::GetCurrentAngle,
        "setMotor",        [](ATableHingeJoint* J, bool bEnabled, float VelocityDegPerSec)
                           { if (J) J->Server_SetMotorEnabled(bEnabled, VelocityDegPerSec); },
        "breakJoint",      [](ATableHingeJoint* J) { if (J) J->Server_BreakJoint(); }
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
    Table.set_function("save", [GI](std::string Slot, std::string Name) -> bool
    {
        if (USaveGameSubsystem* Save = GI->GetSubsystem<USaveGameSubsystem>())
            return Save->SaveTable(UTF8_TO_TCHAR(Slot.c_str()), UTF8_TO_TCHAR(Name.c_str()));
        return false;
    });
    Table.set_function("load", [GI](std::string Slot) -> bool
    {
        if (USaveGameSubsystem* Save = GI->GetSubsystem<USaveGameSubsystem>())
            return Save->LoadTable(UTF8_TO_TCHAR(Slot.c_str()));
        return false;
    });

    // UI
    auto UI = L.create_named_table("UI");
    UI.set_function("showMessage", [](std::string Msg) { UE_LOG(LogTemp,Log,TEXT("[UI] %s"), UTF8_TO_TCHAR(Msg.c_str())); });

    // Game
    auto Game = L.create_named_table("Game");
    Game.set_function("endGame",     [](sol::table) { UE_LOG(LogTemp,Log,TEXT("Game ended")); });
    Game.set_function("setVariable", [&L](std::string K, sol::object V) { L["_IT_vars"][K]=V; });
    Game.set_function("getVariable", [&L](std::string K) -> sol::object { return L["_IT_vars"][K]; });

    // RPG — wired to the level's AFogOfWarManager / AGridManager actors.
    // Initiative tracking has no dedicated actor (it's a lightweight UI/state
    // concept), so it is kept as an in-memory Lua-side broadcast hook that
    // notifies a Blueprint-bound multicast event for the initiative HUD.
    auto RPG = L.create_named_table("RPG");

    auto FindFog = [GI]() -> AFogOfWarManager*
    {
        if (!GI || !GI->GetWorld()) return nullptr;
        for (TActorIterator<AFogOfWarManager> It(GI->GetWorld()); It; ++It) return *It;
        return nullptr;
    };
    auto FindGrid = [GI]() -> AGridManager*
    {
        if (!GI || !GI->GetWorld()) return nullptr;
        for (TActorIterator<AGridManager> It(GI->GetWorld()); It; ++It) return *It;
        return nullptr;
    };

    auto FoW = RPG.create("fogOfWar");
    FoW.set_function("enable", [FindFog](bool bOn)
    {
        if (AFogOfWarManager* Fog = FindFog()) { if (bOn) Fog->RevealAll(); else Fog->HideAll(); }
    });
    FoW.set_function("hideAll", [FindFog]()
    {
        if (AFogOfWarManager* Fog = FindFog()) Fog->HideAll();
    });
    FoW.set_function("revealAll", [FindFog]()
    {
        if (AFogOfWarManager* Fog = FindFog()) Fog->RevealAll();
    });
    FoW.set_function("revealCircle", [FindFog](sol::table Pos, float Radius)
    {
        if (AFogOfWarManager* Fog = FindFog())
        {
            FVector Center((float)Pos[1], (float)Pos[2], (float)Pos[3]);
            Fog->RevealCircle(Center, Radius);
        }
    });

    auto Grid = RPG.create("grid");
    Grid.set_function("setType", [FindGrid](std::string Type)
    {
        if (AGridManager* G = FindGrid())
            G->GridType = (Type == "hex" || Type == "hexagonal") ? EGridType::Hexagonal : EGridType::Square;
    });
    Grid.set_function("setCellSize", [FindGrid](float Size)
    {
        if (AGridManager* G = FindGrid()) G->CellSize = Size;
    });
    Grid.set_function("show", [FindGrid](bool bShow)
    {
        if (AGridManager* G = FindGrid()) G->Server_SetGridVisible(bShow);
    });

    // Initiative tracker: state lives in Lua (the example mods own the
    // turn-order list); these calls forward to the Lua-side handler table
    // so a UMG initiative-tracker widget bound via Events.on("InitiativeUpdated", ...)
    // can render it without requiring a dedicated native actor.
    auto Init = RPG.create("initiative");
    Init.set_function("setOrder", [&L](sol::table Order)
    {
        sol::table Hs = L["_IT_handlers"];
        if (!Hs["InitiativeUpdated"].valid()) return;
        sol::table EL = Hs["InitiativeUpdated"];
        for (auto& P : EL) { sol::protected_function H = P.second; if (H.valid()) H(Order); }
    });
    Init.set_function("show", [&L](bool bShow)
    {
        sol::table Hs = L["_IT_handlers"];
        if (!Hs["InitiativeVisibility"].valid()) return;
        sol::table EL = Hs["InitiativeVisibility"];
        for (auto& P : EL) { sol::protected_function H = P.second; if (H.valid()) H(bShow); }
    });
    Init.set_function("next", [&L]()
    {
        sol::table Hs = L["_IT_handlers"];
        if (!Hs["InitiativeNext"].valid()) return;
        sol::table EL = Hs["InitiativeNext"];
        for (auto& P : EL) { sol::protected_function H = P.second; if (H.valid()) H(); }
    });
    Init.set_function("startCombat", [&L]()
    {
        sol::table Hs = L["_IT_handlers"];
        if (!Hs["CombatStarted"].valid()) return;
        sol::table EL = Hs["CombatStarted"];
        for (auto& P : EL) { sol::protected_function H = P.second; if (H.valid()) H(); }
    });
    Init.set_function("endCombat", [&L]()
    {
        sol::table Hs = L["_IT_handlers"];
        if (!Hs["CombatEnded"].valid()) return;
        sol::table EL = Hs["CombatEnded"];
        for (auto& P : EL) { sol::protected_function H = P.second; if (H.valid()) H(); }
    });

    // Math
    L.set_function("Vector3", [](float X, float Y, float Z, sol::this_state S) -> sol::table
    {
        sol::state_view V(S); sol::table T=V.create_table();
        T[1]=X; T[2]=Y; T[3]=Z; T["x"]=X; T["y"]=Y; T["z"]=Z;
        return T;
    });
}
