#include "Save/SaveGameSubsystem.h"
#include "Objects/TableSpawnManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "JsonObjectConverter.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

// SQLite3 amalgamation (see ThirdParty/sqlite3/, fetched via ThirdParty/install.bat or .sh)
#include "sqlite3.h"

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    DBPath = FPaths::ProjectSavedDir() / TEXT("InfinityTable.db");
    OpenDB();

    // Autosave timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AutosaveTimer, this, &USaveGameSubsystem::AutoSave,
            AutosaveIntervalSeconds, true);
    }
}

void USaveGameSubsystem::Deinitialize()
{
    CloseDB();
    Super::Deinitialize();
}

bool USaveGameSubsystem::OpenDB()
{
    if (DB) return true;
    int rc = sqlite3_open(TCHAR_TO_UTF8(*DBPath), (sqlite3**)&DB);
    if (rc != SQLITE_OK) { DB = nullptr; return false; }

    ExecSQL(TEXT(
        "CREATE TABLE IF NOT EXISTS saves ("
        "  slot      TEXT PRIMARY KEY,"
        "  name      TEXT NOT NULL,"
        "  environment TEXT NOT NULL,"
        "  timestamp INTEGER NOT NULL,"
        "  data      TEXT NOT NULL"
        ");"
    ));
    return true;
}

void USaveGameSubsystem::CloseDB()
{
    if (DB) { sqlite3_close((sqlite3*)DB); DB = nullptr; }
}

bool USaveGameSubsystem::ExecSQL(const FString& SQL)
{
    if (!DB) return false;
    char* ErrMsg = nullptr;
    int rc = sqlite3_exec((sqlite3*)DB, TCHAR_TO_UTF8(*SQL), nullptr, nullptr, &ErrMsg);
    if (rc != SQLITE_OK) { sqlite3_free(ErrMsg); return false; }
    return true;
}

TArray<TMap<FString,FString>> USaveGameSubsystem::QuerySQL(const FString& SQL) const
{
    TArray<TMap<FString,FString>> Rows;
    if (!DB) return Rows;

    sqlite3_stmt* Stmt = nullptr;
    if (sqlite3_prepare_v2((sqlite3*)DB, TCHAR_TO_UTF8(*SQL), -1, &Stmt, nullptr) != SQLITE_OK)
        return Rows;

    int ColCount = sqlite3_column_count(Stmt);
    while (sqlite3_step(Stmt) == SQLITE_ROW)
    {
        TMap<FString,FString> Row;
        for (int i = 0; i < ColCount; ++i)
        {
            FString Col  = UTF8_TO_TCHAR(sqlite3_column_name(Stmt, i));
            FString Val  = UTF8_TO_TCHAR((const char*)sqlite3_column_text(Stmt, i));
            Row.Add(Col, Val);
        }
        Rows.Add(Row);
    }
    sqlite3_finalize(Stmt);
    return Rows;
}

bool USaveGameSubsystem::SaveTable(const FString& Slot, const FString& DisplayName)
{
    if (!DB) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    FTableSaveState State;
    State.Slot        = Slot;
    State.Name        = DisplayName;
    State.Timestamp   = FDateTime::UtcNow().ToUnixTimestamp();

    // Collect all TableObjects
    for (TActorIterator<ATableObject> It(World); It; ++It)
    {
        State.Objects.Add(It->GetObjectState());
    }

    FString JSON;
    FJsonObjectConverter::UStructToJsonObjectString(State, JSON);

    FString SQL = FString::Printf(
        TEXT("INSERT OR REPLACE INTO saves (slot,name,environment,timestamp,data) VALUES ('%s','%s','%s',%lld,'%s');"),
        *Slot, *DisplayName, *State.Environment, State.Timestamp, *JSON.ReplaceCharWithEscapedChar());
    return ExecSQL(SQL);
}

bool USaveGameSubsystem::LoadTable(const FString& Slot)
{
    FString SQL = FString::Printf(TEXT("SELECT data FROM saves WHERE slot='%s';"), *Slot);
    TArray<TMap<FString,FString>> Rows = QuerySQL(SQL);
    if (Rows.Num() == 0) return false;

    FTableSaveState State;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(Rows[0][TEXT("data")], &State, 0, 0))
        return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    // Clear existing objects
    for (TActorIterator<ATableObject> It(World); It; ++It) { It->Destroy(); }

    // Spawn saved objects
    UTableSpawnManager* Spawner = GetGameInstance()->GetSubsystem<UTableSpawnManager>();
    if (!Spawner) return false;

    for (const FTableObjectState& ObjState : State.Objects)
    {
        if (ATableObject* Obj = Spawner->SpawnObject(ObjState.ObjectTypeID, ObjState.Position, ObjState.Rotation))
        {
            Obj->ApplyObjectState(ObjState);
        }
    }
    return true;
}

bool USaveGameSubsystem::DeleteSlot(const FString& Slot)
{
    return ExecSQL(FString::Printf(TEXT("DELETE FROM saves WHERE slot='%s';"), *Slot));
}

TArray<FSaveSlotInfo> USaveGameSubsystem::GetSaveSlots() const
{
    TArray<FSaveSlotInfo> Out;
    for (auto& Row : QuerySQL(TEXT("SELECT slot,name,environment,timestamp FROM saves ORDER BY timestamp DESC;")))
    {
        FSaveSlotInfo Info;
        Info.Slot        = Row[TEXT("slot")];
        Info.Name        = Row[TEXT("name")];
        Info.Environment = Row[TEXT("environment")];
        Info.Timestamp   = FCString::Atoi64(*Row[TEXT("timestamp")]);
        Out.Add(Info);
    }
    return Out;
}

void USaveGameSubsystem::AutoSave()
{
    SaveTable(TEXT("__autosave__"), TEXT("Autosave"));
}
