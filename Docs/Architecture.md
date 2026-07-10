# InfinityTable Architecture

## System Diagram

```
CLIENT
  ITPlayerController -> GrabComponent -> TableObject (RPC)
  InGameHUD (UMG)    -> RadialMenu, ChatWidget, ObjectBrowser,
                         Lobby, SavedGames, Settings,
                         ScriptingConsole, Workshop

SERVER (Authoritative)
  ITGameMode -> ITGameState (replicated)
     |
     +-- LuaSubsystem (Lua 5.4 + sol2, sandboxed)
     +-- ModLoadingSubsystem (validates + loads mods from Saved/Mods/)
     +-- AssetImportSubsystem (runtime GLTF/PNG/JPG/WEBP/MP3/WAV import;
     |                          FBX/OBJ catalogued, editor-import only)
     +-- SaveGameSubsystem (SQLite, autosave 5min)
     +-- TableSpawnManager (object registry: card/deck/discard_pile/
     |                       d6/d20/miniature/token + mod-registered types)
     +-- AudioManagerSubsystem (positional SFX)
     +-- UITSessionSubsystem (EOS/Steam/LAN sessions)

RPG-SPECIFIC ACTORS (placed per-level, not subsystems)
     +-- AFogOfWarManager (per-level fog grid, multicast reveal/hide)
     +-- AGridManager (square/hex grid, snap-to-grid, replicated)

PHYSICS ACTORS (spawned ad hoc, not subsystems)
     +-- ATableHingeJoint (hinge/fixed/ball-socket constraints, server-only)

PHYSICS: Chaos Physics on server, bReplicatePhysics=true to clients
NETWORK: UE5 NetDriver UDP, DOREPLIFETIME for state, Server RPCs for actions
```

## Key Design Decisions

1. **Physics Authority** - Server owns all physics. Clients send grab/release RPCs.
2. **Lua Sandbox** - Blocks io/os/require/debug. 100k instruction hook (see `LuaSubsystem.cpp`).
3. **Save Format** - SQLite with JSON blobs (FJsonObjectConverter). Autosave every 5min. Card hand/deck-shuffle state is not yet captured — see `Docs/Save_System.md`.
4. **Mod Validation** - Scans Lua for blocked patterns, blocks executable file types. Mods load from `Saved/Mods/`, not a project-root `Mods/` folder.
5. **Object Ownership** - Public / Player / Host / Locked enum per object.
6. **Runtime Asset Import** - GLTF/GLB are runtime-spawnable via the glTFRuntime plugin; FBX/OBJ have no stock UE5 runtime importer and are catalogued only (editor-time import, or an offline conversion step, are required) — see `Docs/Asset_Import_Guide.md`.
7. **UI Panel Model** - `AITInGameHUD` owns one persistent instance per panel (Lobby, Chat, Object Browser, Saved Games, Settings, Scripting Console, Workshop, Player List, Radial Menu), toggling visibility rather than constructing/destroying widgets repeatedly.
8. **Console Access Control** - The in-game Lua scripting console (`UScriptingConsoleWidget`) only executes commands for players with Host or Co-Host role, since it runs arbitrary Lua server-side.
9. **Joints/Hinges** - `ATableHingeJoint` wraps UE5's `UPhysicsConstraintComponent` (hinge/fixed/ball-socket), server-only construction, exposed to Lua via `Physics.createHinge`/`createFixedJoint` — see `Source/InfinityTable/Physics/TableHingeJoint.h`.
10. **Grouping** - Tag-based (`ATableObject::GroupID`), not physics-welded — group members stay independently simulated and are coordinated by `Server_MoveGroupBy`/`LockGroup`/`DestroyGroup` rather than constrained together, keeping it cheap at scale. Continuous click-drag of a whole group isn't implemented, only discrete actions (UI and Lua).
11. **Moderation** - `AITGameMode::KickPlayer`/`BanPlayer` are real (not just permission checks); ban list is in-memory only, enforced via a `PreLogin` override. `UPlayerListWidget` (Tab key) is the UI surface.
12. **Net Throttling** - `ATableObject::Tick` drops `NetUpdateFrequency` from 60Hz to 2Hz once an object's physics velocity has been near-zero for 1s, restoring full rate immediately on grab/release/scripted move — see PERFORMANCE OPTIMIZATION in the original spec and `Docs/Multiplayer_Guide.md`.

## AWS Architecture

```
Internet -> Elastic IP -> EC2 (c5.xlarge)
                            |
                    Docker: it_server (port 7777/UDP)
                    Nginx:  health check (port 80/TCP)
                    S3:     save files + backups
                    CloudWatch: logs + metrics
```
