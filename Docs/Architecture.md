# InfinityTable — Architecture Guide

## System Overview

```
┌──────────────────────────────────────────────────────────┐
│                    CLIENT (UE5)                          │
│                                                          │
│  ITPlayerController ──► GrabComponent                   │
│         │                    │                           │
│         ▼                    ▼                           │
│  InGameHUD (UMG)      TableObject (replicated)           │
│  RadialMenu           TableDice / TableCard / etc.       │
│  ChatWidget                  │                           │
│  ObjectBrowser               ▼                           │
│                       PhysicsInteractionSubsystem        │
└──────────────────────────────────────────────────────────┘
              │  Unreal NetDriver (UDP)
┌──────────────────────────────────────────────────────────┐
│                  SERVER (Authoritative)                  │
│                                                          │
│  ITGameMode ──► ITGameState (replicated to all)          │
│       │                                                  │
│       ├── LuaSubsystem (Lua 5.4 + sol2)                  │
│       │       └── ModLoadingSubsystem                    │
│       ├── SaveGameSubsystem (SQLite)                     │
│       ├── TableSpawnManager                              │
│       ├── AudioManagerSubsystem                          │
│       └── UITSessionSubsystem (EOS/Steam)                │
│                                                          │
│   Physics Authority: Chaos Physics on Server             │
│   Object RPCs:      Server_PickUp / Server_Release       │
│   State Sync:       DOREPLIFETIME + bReplicatePhysics    │
└──────────────────────────────────────────────────────────┘
```

## Key Design Decisions

### 1. Physics Authority
All physics runs on the **server**. Clients send intent (grab, release, throw velocity) via RPCs. The server applies forces and the resulting state replicates to all clients via `bReplicatePhysics = true` on each `ATableObject`.

### 2. Object Ownership
Each `ATableObject` has an `EObjectOwnership` enum:
- `Public` — anyone can grab
- `Player` — only the owning player (hand cards)
- `Host` — GM/host only
- `Locked` — physics locked, no interaction

### 3. Lua Sandboxing
Scripts run in a restricted `sol::state`. Blocked: `io`, `os`, `require`, `debug`, `rawget/set`. Memory capped at 64 MB per mod. Instruction hook fires every 100,000 ops to prevent infinite loops.

### 4. Save Format
Saves use SQLite for fast indexed lookup. Each save slot stores a JSON blob (`FTableSaveState` serialized via `FJsonObjectConverter`). Objects are individually serialized with `FTableObjectState`.

### 5. Mod Validation
Mods pass through `UModValidator` before any script executes:
1. `manifest.json` parsed and schema-checked
2. All `.lua` files scanned for blocked API patterns
3. All files checked for executable extensions
4. Only then is the Lua entry script loaded

---

# Networking Guide

## Session Flow

```
Host presses "Create Session"
    → UITSessionSubsystem::HostSession()
    → FOnlineSessionSettings configured
    → SessionInterface->CreateSession()
    → On success: ServerTravel("/Game/Maps/TableMap?listen")
    → Clients find via FindSessions() → JoinSession()
    → OnJoinSessionComplete → ClientTravel(ConnectAddress)
```

## Object Sync Strategy

| Data | Method | Frequency |
|---|---|---|
| Position/Rotation | `bReplicatePhysics` | Every physics tick |
| Grab state | `DOREPLIFETIME` bool | On change |
| Object color | `DOREPLIFETIME` + RPC | On change |
| Dice result | `DOREPLIFETIME` int32 | On settle |
| Card face-up | `DOREPLIFETIME` bool | On change |
| Deck order | `DOREPLIFETIME` TArray | On shuffle |

## Bandwidth Budget

```ini
; DefaultEngine.ini
TotalNetBandwidth=32000     ; 32 KB/s total
MaxDynamicBandwidth=7000    ; per connection cap
net.MaxNetUpdateFrequency=60
```

Object net priority scales with distance:
- < 300cm  → priority 3.0 (high)
- 300–1000cm → priority 1.0
- > 1000cm → priority 0.5

## Dedicated Server Startup

```bash
./InfinityTableServer \
    /Game/Maps/TableMap     \
    -server                  \
    -port=7777               \
    -log                     \
    -NOSTEAM                 \
    -MaxPlayers=6
```

### Environment Variables for Docker

```
IT_MAX_PLAYERS=6
IT_SERVER_NAME="My InfinityTable Server"
IT_MAP=TableMap
IT_PASSWORD=""
IT_PORT=7777
```

---

# Modding Guide

## Creating Your First Mod

### Step 1 — Folder structure
```
Mods/my_game/
├── manifest.json
├── scripts/
│   └── main.lua
├── assets/
│   ├── board.obj
│   └── board.png
└── thumbnail.png
```

### Step 2 — manifest.json
```json
{
  "id": "my_game",
  "name": "My Custom Game",
  "version": "1.0.0",
  "author": "YourName",
  "description": "A custom tabletop game",
  "entry": "scripts/main.lua",
  "permissions": ["spawnObjects", "networkSync", "showUI"]
}
```

### Step 3 — main.lua entry point
```lua
-- Your mod's entry point
-- Called once when the mod is loaded into a session

local function onGameStart()
    -- Spawn your board
    Objects.spawn("my_game.board", { position = {0,0,0} })
    UI.showMessage("My game is ready!")
end

Events.on("GameStart", onGameStart)
```

### Step 4 — Register object types (in manifest.json)
```json
"objects": [
  {
    "id": "board",
    "mesh": "assets/board.obj",
    "texture": "assets/board.png",
    "mass": 0.5
  }
]
```

Object type IDs are prefixed with your mod ID automatically:  
`my_game.board` → spawned as `Objects.spawn("my_game.board", ...)`

## Permission System

| Permission | What it allows |
|---|---|
| `spawnObjects` | `Objects.spawn()` and `Objects.destroy()` |
| `networkSync` | `Network.syncVariable()` and `Network.callOnAll()` |
| `readSave` | `Table.load()` |
| `writeSave` | `Table.save()` |
| `showUI` | `UI.createPanel()`, `UI.showMessage()`, `UI.showDialog()` |

## Lua Security Restrictions

These globals are permanently removed:

```lua
io        -- file I/O
os        -- OS calls
require   -- module loading
dofile    -- file execution
loadfile  -- file loading
load      -- code loading
debug     -- debug library
package   -- package system
rawget/rawset/rawequal -- raw table access
collectgarbage -- memory control
```

Use only the `InfinityTable.*` API for all game functionality.

## Publishing to Workshop

1. Create your mod folder in `Mods/`
2. Test locally in sandbox mode
3. Run `Tools > Workshop > Publish Mod` from the in-game menu
4. Fill in title, description, tags, and thumbnail
5. Click Publish — mod uploads to Steam Workshop

---

# Save System Documentation

## Save Slot Format

Saves are stored in `Saved/InfinityTable.db` (SQLite):

```sql
CREATE TABLE save_slots (
    slot        TEXT PRIMARY KEY,
    name        TEXT NOT NULL,
    environment TEXT DEFAULT 'FantasyTavern',
    timestamp   INTEGER NOT NULL,
    obj_count   INTEGER DEFAULT 0,
    data        TEXT NOT NULL    -- JSON blob
);
```

## JSON Structure (FTableSaveState)

```json
{
  "Slot": "slot_001",
  "Name": "Friday Night Session",
  "Environment": "FantasyTavern",
  "Timestamp": 1720000000,
  "Objects": [
    {
      "Position": {"X":100.0, "Y":0.0, "Z":10.0},
      "Rotation": {"Pitch":0.0, "Yaw":45.0, "Roll":0.0},
      "LinearVelocity":  {"X":0,"Y":0,"Z":0},
      "AngularVelocity": {"X":0,"Y":0,"Z":0},
      "bPhysicsEnabled": true,
      "ObjectTypeID": "d20",
      "ObjectColor": {"R":1,"G":0.8,"B":0.2,"A":1},
      "CustomDataJSON": ""
    }
  ]
}
```

## Autosave

Autosave runs every **300 seconds** (5 minutes) to slot `"autosave"`.  
Configure in `DefaultGame.ini`:

```ini
[/Script/InfinityTable.USaveGameSubsystem]
AutosaveIntervalSeconds=300
```

## Export / Import

```lua
-- Export to JSON file (for sharing)
Table.save("my_slot")
-- Then from C++/Blueprint: SaveGameSubsystem->ExportToJSON("my_slot", FilePath)

-- Import from JSON
-- SaveGameSubsystem->ImportFromJSON(FilePath, "imported_slot")
```

---

# Asset Import Guide

## Supported Formats

| Type | Formats | Notes |
|---|---|---|
| 3D Mesh | `.gltf`, `.glb`, `.fbx`, `.obj` | GLTF preferred |
| Texture | `.png`, `.jpg`, `.webp` | PNG for transparency |
| Audio | `.mp3`, `.wav`, `.ogg` | WAV for low-latency SFX |

## Import via In-Game UI

1. Open **Object Browser** (F5 or sidebar)
2. Click **Import Asset**
3. Select your file
4. Asset appears in **My Assets** category
5. Drag onto table to spawn

## Import via Lua (Mod SDK)

```lua
-- Import a texture at runtime
Assets.importTexture("https://example.com/card_front.png", function(tex)
    if tex then
        local card = Objects.spawn("card", { position = {0,0,10} })
        card.setTexture(tex)
    end
end)
```

## Optimization Tips

- Keep meshes under 10,000 polygons for table objects
- Use 512×512 or 1024×1024 textures (power of 2)
- For dice: use 256×256 face texture atlases
- Normals should point outward (standard UE5 convention)
- Apply transforms before export (apply scale/rotation in Blender)

## Physics Mesh

Physics uses a simplified convex hull auto-generated by Chaos Physics.  
For complex shapes, provide a separate low-poly physics mesh:

```json
"objects": [
  {
    "id": "my_object",
    "mesh": "assets/visual.gltf",
    "physics_mesh": "assets/physics_low.gltf"
  }
]
```
