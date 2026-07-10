# InfinityTable — Save System Documentation

## Overview

Persistence is handled by `USaveGameSubsystem`, a `UGameInstanceSubsystem`
backed by a local SQLite database. It is server-authoritative: only the
server (host or dedicated server) writes saves, and `LoadTable` destroys and
respawns every `ATableObject` on the server, which then replicates down to
clients normally.

## Database Location & Schema

The database file lives at:

```
<ProjectDir>/Saved/InfinityTable.db
```

(`FPaths::ProjectSavedDir() / "InfinityTable.db"`, created automatically on
first run via `USaveGameSubsystem::Initialize`).

Schema (single table, JSON blob payload):

```sql
CREATE TABLE IF NOT EXISTS saves (
    slot        TEXT PRIMARY KEY,
    name        TEXT NOT NULL,
    environment TEXT NOT NULL,
    timestamp   INTEGER NOT NULL,
    data        TEXT NOT NULL   -- JSON-serialized FTableSaveState
);
```

Storing the full object list as a single JSON blob (rather than one SQL row
per object) keeps the schema stable as `FTableObjectState` grows new fields —
no migrations needed for additive changes, since `FJsonObjectConverter`
tolerates missing/extra JSON keys.

## What Gets Saved

`FTableSaveState` (see `Save/SaveGameSubsystem.h`):

```cpp
struct FTableSaveState
{
    FString Slot;
    FString Name;
    FString Environment;
    int64   Timestamp;
    TArray<FTableObjectState> Objects;
};
```

Each `FTableObjectState` (see `Objects/TableObject.h`) captures:
position, rotation, linear/angular velocity, physics-enabled flag, the
object's type ID (used to respawn the correct class via
`UTableSpawnManager`), and its color/custom data.

**Not currently captured**, and worth knowing as a limitation: card hand
ownership (`bInHand` / `HandOwner` on `ATableCard`), deck composition state
(`ATableDeck::ShuffledOrder`), and miniature HP/grid-cell state are not part
of `FTableObjectState` yet. A full save/restore of an in-progress RPG combat
or card game currently round-trips object transforms and types correctly but
will reset cards to face-down/not-in-hand and decks to their unshuffled
definition on load. Extending `FTableObjectState` (or adding
subtype-specific state structs merged in via `CustomDataJSON`) is the
natural next step if full-fidelity RPG/card-game saves are required.

## API

```cpp
USaveGameSubsystem* Save = GetGameInstance()->GetSubsystem<USaveGameSubsystem>();

// Save current table state to a named slot
Save->SaveTable(TEXT("slot_1"), TEXT("Friday Game Night"));

// List all save slots, newest first
TArray<FSaveSlotInfo> Slots = Save->GetSaveSlots();

// Load a slot — destroys all current ATableObjects, respawns from saved state
Save->LoadTable(TEXT("slot_1"));

// Delete a slot
Save->DeleteSlot(TEXT("slot_1"));
```

Also exposed to Lua mods (see `Docs/Modding_API.md`) as `Table.save(slot,
name)` / `Table.load(slot)`, gated behind the `writeSave` / `readSave` mod
permissions respectively.

## Autosave

`USaveGameSubsystem::AutosaveIntervalSeconds` (default 300s / 5 minutes)
drives a repeating timer started in `Initialize()`. Autosaves write to the
reserved slot `__autosave__`, overwriting the previous autosave each cycle —
they are not kept as a history. If you need autosave history (e.g. last 5
autosaves), the cleanest extension point is to suffix the slot with a
rotating index (`__autosave_0__` … `__autosave_4__`) in `AutoSave()`.

## Concurrency Notes

`USaveGameSubsystem` opens a single `sqlite3*` handle for the lifetime of the
`GameInstance` and serializes all reads/writes through `ExecSQL`/`QuerySQL`,
which run synchronously on the calling thread (typically the game thread,
since saves are triggered from gameplay code or the autosave timer). For a
single dedicated-server process this is safe by construction — there is no
multi-process contention on the `.db` file in normal deployment, since only
one server instance should ever point at a given `Saved/InfinityTable.db`.

If you back up saves to S3 via `AWS/scripts/backup_saves.sh` while the server
is running, prefer SQLite's `.backup` mechanism or briefly pausing autosave
rather than copying the raw file, to avoid capturing a torn write.

## Backups (AWS deployments)

`AWS/scripts/backup_saves.sh` is wired into the systemd timer / cron
described in `Docs/AWS_Deployment.md` and uploads the `Saved/` directory
(including `InfinityTable.db`) to the S3 bucket provisioned by
`AWS/cloudformation/infinitytable-stack.yaml`.
