#include "SaveGameSubsystem.h"
#include "sqlite3.h"
#include "JsonObjectConverter.h"
#include "Objects/TableSpawnManager.h"
#include "EngineUtils.h"

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    DBPath = FPaths::ProjectSavedDir() / TEXT("InfinityTable.db");
    if (!OpenDB()) return;

    ExecSQL(R"(
        CREATE TABLE IF NOT EXISTS save_slots (
            slot        TEXT PRIMARY KEY,
            name        TEXT NOT NULL,
            environment TEXT DEFAULT 'FantasyTavern',
            timestamp   INTEGER NOT NULL,
            obj_count   INTEGER DEFAULT 0,
            data        TEXT NOT NULL
        );
    )");

    // Autosave timer
    GetGameInstance()->GetWorld()->GetTimerManager().SetTimer(
        AutosaveTimer,
        this, &USaveGameSubsystem::AutoSave,
        AutosaveIntervalSeconds, true);

    UE_LOG(LogTemp, Log, TEXT("SaveGameSubsystem: DB ready at %s"), *DBPath);
}

void USaveGameSubsystem::Deinitialize()
{
    CloseDB();
    Super::Deinitialize();
}

bool USaveGameSubsystem::OpenDB()
{
    sqlite3* RawDB = nullptr;
    int Rc = sqlite3_open(TCHAR_TO_UTF8(*DBPath), &RawDB);
    if (Rc != SQLITE_OK)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveSystem: Cannot open DB: %s"),
            UTF8_TO_TCHAR(sqlite3_errmsg(RawDB)));
        sqlite3_close(RawDB);
        return false;
    }
    DB = RawDB;
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
    int Rc = sqlite3_exec((sqlite3*)DB, TCHAR_TO_UTF8(*SQL), nullptr, nullptr, &ErrMsg);
    if (Rc != SQLITE_OK)
    {
        UE_LOG(LogTemp, Error, TEXT("SQL Error: %s"), UTF8_TO_TCHAR(ErrMsg));
        sqlite3_free(ErrMsg);
        return false;
    }
    return true;
}

TArray<TMap<FString,FString>> USaveGameSubsystem::QuerySQL(const FString& SQL) const
{
    TArray<TMap<FString,FString>> Rows;
    if (!DB) return Rows;

    sqlite3_stmt* Stmt = nullptr;
    if (sqlite3_prepare_v2((sqlite3*)DB, TCHAR_TO_UTF8(*SQL), -1, &Stmt, nullptr) != SQLITE_OK)
        return Rows;

    while (sqlite3_step(Stmt) == SQLITE_ROW)
    {
        TMap<FString,FString> Row;
        int ColCount = sqlite3_column_count(Stmt);
        for (int i = 0; i < ColCount; ++i)
        {
            FString ColName = UTF8_TO_TCHAR(sqlite3_column_name(Stmt, i));
            FString ColVal  = UTF8_TO_TCHAR((const char*)sqlite3_column_text(Stmt, i));
            Row.Add(ColName, ColVal);
        }
        Rows.Add(Row);
    }
    sqlite3_finalize(Stmt);
    return Rows;
}

FString USaveGameSubsystem::EscapeSQL(const FString& Input) const
{
    // Basic SQL injection prevention — replace single quotes
    return Input.Replace(TEXT("'"), TEXT("''"));
}

bool USaveGameSubsystem::SaveTable(const FString& Slot, const FString& DisplayName)
{
    if (!DB) return false;

    FTableSaveState State;
    State.Slot      = Slot;
    State.Name      = DisplayName;
    State.Timestamp = FDateTime::UtcNow().ToUnixTimestamp();

    // Collect environment
    if (UWorld* W = GetGameInstance()->GetWorld())
    {
        // TODO: Read from ITGameState
        State.Environment = TEXT("FantasyTavern");
    }

    // Collect all objects
    for (TActorIterator<ATableObject> It(GetGameInstance()->GetWorld()); It; ++It)
        State.Objects.Add(It->GetObjectState());

    FString JSON;
    FJsonObjectConverter::UStructToJsonObjectString(State, JSON);
    JSON = EscapeSQL(JSON);

    FString SQL = FString::Printf(
        TEXT("INSERT OR REPLACE INTO save_slots VALUES('%s','%s','%s',%lld,%d,'%s');"),
        *EscapeSQL(Slot),
        *EscapeSQL(DisplayName),
        *EscapeSQL(State.Environment),
        State.Timestamp,
        State.Objects.Num(),
        *JSON
    );

    bool bOK = ExecSQL(SQL);
    OnTableSaved.Broadcast(bOK, Slot);
    return bOK;
}

bool USaveGameSubsystem::LoadTable(const FString& Slot)
{
    FString SQL = FString::Printf(
        TEXT("SELECT data FROM save_slots WHERE slot='%s';"), *EscapeSQL(Slot));

    auto Rows = QuerySQL(SQL);
    if (Rows.IsEmpty()) { OnTableLoaded.Broadcast(false, Slot); return false; }

    FString JSON = Rows[0]["data"];
    FTableSaveState State;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(JSON, &State))
    {
        OnTableLoaded.Broadcast(false, Slot);
        return false;
    }

    // Clear table
    for (TActorIterator<ATableObject> It(GetGameInstance()->GetWorld()); It; ++It)
        It->Destroy();

    // Respawn
    UTableSpawnManager* SM = GetGameInstance()->GetSubsystem<UTableSpawnManager>();
    for (const FTableObjectState& ObjState : State.Objects)
    {
        ATableObject* Obj = SM->SpawnObject(ObjState.ObjectTypeID,
                                             ObjState.Position, ObjState.Rotation);
        if (Obj) Obj->ApplyObjectState(ObjState);
    }

    OnTableLoaded.Broadcast(true, Slot);
    return true;
}

bool USaveGameSubsystem::DeleteSlot(const FString& Slot)
{
    return ExecSQL(FString::Printf(
        TEXT("DELETE FROM save_slots WHERE slot='%s';"), *EscapeSQL(Slot)));
}

TArray<FSaveSlotInfo> USaveGameSubsystem::GetSaveSlots() const
{
    TArray<FSaveSlotInfo> Results;
    auto Rows = QuerySQL(
        TEXT("SELECT slot,name,environment,timestamp,obj_count FROM save_slots ORDER BY timestamp DESC;"));

    for (const auto& Row : Rows)
    {
        FSaveSlotInfo Info;
        Info.Slot        = Row["slot"];
        Info.Name        = Row["name"];
        Info.Environment = Row["environment"];
        Info.Timestamp   = FCString::Atoi64(*Row["timestamp"]);
        Info.ObjectCount = FCString::Atoi(*Row["obj_count"]);
        Results.Add(Info);
    }
    return Results;
}

bool USaveGameSubsystem::ExportToJSON(const FString& Slot, const FString& FilePath) const
{
    auto Rows = QuerySQL(FString::Printf(
        TEXT("SELECT data FROM save_slots WHERE slot='%s';"), *EscapeSQL(Slot)));
    if (Rows.IsEmpty()) return false;
    return FFileHelper::SaveStringToFile(Rows[0]["data"], *FilePath);
}

bool USaveGameSubsystem::ImportFromJSON(const FString& FilePath, const FString& NewSlot)
{
    FString JSON;
    if (!FFileHelper::LoadFileToString(JSON, *FilePath)) return false;
    FTableSaveState State;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(JSON, &State)) return false;
    State.Slot = NewSlot;
    return SaveTable(NewSlot, State.Name);
}

void USaveGameSubsystem::AutoSave()
{
    SaveTable(TEXT("autosave"), TEXT("Autosave"));
    UE_LOG(LogTemp, Log, TEXT("SaveSystem: Autosave completed"));
}
