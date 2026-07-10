# InfinityTable — Modding API Documentation

This is the reference companion to `SDK/README.md`'s quick-start. It
documents every Lua binding registered by `ULuaAPIBindings::RegisterAll`
(see `Source/InfinityTable/Scripting/LuaAPIBindings.cpp`), the sandbox model,
and the mod validation/loading pipeline (`UModLoadingSubsystem`).

## Sandbox Model

Mod scripts run inside a `sol2`-wrapped Lua 5.4 VM (`ULuaSubsystem`), one VM
per `GameInstance` shared by all loaded mods. The sandbox:

- Opens only `base`, `math`, `table`, `string`, `coroutine` standard
  libraries.
- Strips `io`, `os`, `package`, `require`, `dofile`, `loadfile`, `load`,
  `debug`, `collectgarbage` entirely — scripts cannot touch the filesystem,
  spawn processes, or load arbitrary bytecode.
- Installs an instruction-count hook (`LUA_MASKCOUNT`, 100,000 instructions)
  that raises a Lua error if a single script execution runs away — this
  catches infinite loops in mod code before they hang the server tick.
- Redirects Lua's `print` into `UE_LOG` under the `[Lua]` tag for debugging.

Before a mod's Lua is ever executed, `UModLoadingSubsystem::ValidateMod`
performs static checks:

- Rejects mods missing `manifest.json` or with malformed/missing required
  fields (`id`, `entry`).
- Recursively scans every file in the mod folder and rejects the mod outright
  if it contains a blocked extension (`.exe`, `.dll`, `.so`, `.bat`, `.sh`).
- Scans every `.lua` file's source text for blocked substrings
  (`os.`, `io.`, `require(`, `loadstring(`, `dofile(`) as defense-in-depth on
  top of the runtime sandbox — this catches obviously-malicious scripts
  before they're even loaded into the VM.

This is pattern-based static scanning, not a full taint/AST analysis — it
raises the bar for casual abuse but should not be treated as a hard security
boundary against a determined attacker with server access. Don't load mods
from untrusted sources on a server you care about.

## Mod Structure

```
Saved/Mods/my_mod/
├── manifest.json
└── scripts/
    └── main.lua
```

`manifest.json` (parsed into `FITModManifest`, see `Mod/ModManifest.h`):

```json
{
  "id": "my_mod",
  "name": "My Game",
  "version": "1.0.0",
  "author": "You",
  "description": "A fun game.",
  "entry": "scripts/main.lua",
  "tags": ["card-game", "2-player"],
  "permissions": ["spawnObjects", "networkSync", "showUI"],
  "objects": [
    { "id": "custom_card", "meshPath": "/Game/Mods/MyMod/Card.Card", "texturePath": "/Game/Mods/MyMod/Face.Face", "mass": 0.01 }
  ]
}
```

## Loading Mods

```cpp
UModLoadingSubsystem* Mods = GetGameInstance()->GetSubsystem<UModLoadingSubsystem>();

for (const FString& ModFolder : Mods->DiscoverMods())
{
    Mods->LoadMod(ModFolder);
}
```

`DiscoverMods()` scans `<ProjectDir>/Saved/Mods/` (configurable via
`ModsDirectoryName`) for subfolders containing a `manifest.json`.
`LoadMod()` validates, then loads the entry script into the shared
`ULuaSubsystem` VM and records the manifest for permission lookups.

## Permission Table

| Permission | Gates |
|---|---|
| `spawnObjects` | `Objects.spawn` / `Objects.destroy` |
| `networkSync` | Replicating mod-driven state to all clients |
| `showUI` | `UI.showMessage` / future `UI.createPanel` |
| `readSave` | `Table.load` |
| `writeSave` | `Table.save` |

**Current implementation note:** `UModLoadingSubsystem::HasPermission(ModID,
Permission)` is implemented and available for gameplay code to call before
honoring a mod's request, but the Lua bindings themselves
(`Objects.spawn`, `Table.save`, etc.) do not yet enforce these permissions
internally — they execute unconditionally once a mod's script is loaded. For
a production deployment hosting untrusted third-party mods, wrap each
permission-gated binding with a `HasPermission` check keyed to the calling
mod's ID before exposing the table to that mod's Lua environment. This is the
natural extension point if/when per-mod sandboxed sub-environments
(`sol::environment`) replace the current shared-VM model.

## Lua API Reference

### `Objects`
| Function | Description |
|---|---|
| `Objects.spawn(typeID, opts)` | Spawns a registered object type (`card`, `deck`, `discard_pile`, `d6`, `d20`, `miniature`, `token`, or any custom type registered via `UTableSpawnManager::RegisterObjectType`). `opts.position = {x,y,z}` optional. Returns the spawned `TableObject`. |
| `Objects.destroy(obj)` | Destroys a `TableObject`. |
| `Objects.getAll()` | Returns a Lua array of every `TableObject` currently on the table. |

### `TableObject` methods (on any object returned by `Objects.spawn`/`getAll`)
| Method | Description |
|---|---|
| `obj.typeID` | Read-only type ID string. |
| `obj:setLocked(bool)` | Lock/unlock physics. |
| `obj:flip()` | Flip 180° on the roll axis. |
| `obj:setColor(r, g, b)` | Set tint (0–1 floats). |
| `obj:getPosition()` | Returns `{x, y, z}`. |
| `obj:setPosition(x, y, z)` | Server-validated move (rejects jumps >600 units). |
| `obj:duplicate()` | Spawns a copy of this object's type, scale, and color near its current position. Does not deep-clone subclass-specific instance state (e.g. a card's face, a deck's shuffle order) — see the implementation note in `TableObject.cpp`. |
| `obj:setGroup(groupID)` | Assigns this object to a named group; pass `""` to ungroup. Objects sharing a group ID move/lock/delete together when the group operations below are called on any member. |
| `obj:getGroup()` | Returns this object's current group ID (empty string if ungrouped). |
| `obj:moveGroupBy(dx, dy, dz)` | Moves every object sharing this object's group by the given offset (or just this object, if ungrouped). Locked group members are skipped. |
| `obj:lockGroup(bool)` | Locks/unlocks every object in this object's group (or just this object, if ungrouped). |
| `obj:destroyGroup()` | Destroys every object in this object's group (or just this object, if ungrouped). |
| `obj:destroy()` | Removes the object. |

### `Physics`
| Function | Description |
|---|---|
| `Physics.createHinge(objA, objB, minAngleDeg, maxAngleDeg)` | Creates a hinge joint (e.g. a door or chest lid) constraining `objA`'s rotation relative to `objB` within the given angle range. Pass `nil` for `objB` to instead hinge `objA` to a fixed point in the world rather than to another movable object. Returns a `TableHingeJoint` handle. |
| `Physics.createFixedJoint(objA, objB)` | Rigidly welds two objects together (no relative motion) — useful for compound props assembled from multiple pieces. |
| `Physics.breakJoint(joint)` | Destroys a joint's constraint, freeing both objects to move independently again. |

### `TableHingeJoint` methods (on a handle returned by `Physics.createHinge`/`createFixedJoint`)
| Method | Description |
|---|---|
| `joint:getCurrentAngle()` | Returns the current relative angle (degrees) between the joint's two objects. Returns `0` if either object/mesh is missing. |
| `joint:setMotor(enabled, velocityDegPerSec)` | Enables/disables a powered hinge motor (e.g. an auto-closing door). See the implementation note in `Source/InfinityTable/Physics/TableHingeJoint.cpp` — the underlying UE5 constraint API drives toward a target orientation rather than a raw angular velocity, so a continuously-spinning motor needs the caller to animate the target incrementally (e.g. via a Blueprint timeline) rather than relying on this call alone for continuous rotation. |
| `joint:breakJoint()` | Same as `Physics.breakJoint(joint)`. |

### `Events`
| Function | Description |
|---|---|
| `Events.on(eventName, handler)` | Registers a handler. Built-in events fired by native code: `GameStart`, `PlayerJoined`, `PlayerLeft`, `DiceRolled`. RPG bindings also fire `InitiativeUpdated`, `InitiativeVisibility`, `InitiativeNext`, `CombatStarted`, `CombatEnded` (see RPG section below). |
| `Events.emit(eventName, ...)` | Fires all handlers registered for `eventName` with the given args. |

### `Table`
| Function | Description |
|---|---|
| `Table.getPlayerCount()` | Number of connected players. |
| `Table.broadcast(msg)` | Logs a broadcast message (hook this into chat/UI in your mod via `Events`). |
| `Table.clearTable()` | Destroys every `TableObject` on the table. |
| `Table.save(slot, name)` | Saves current table state via `USaveGameSubsystem`. Gated by `writeSave` permission (see note above). |
| `Table.load(slot)` | Loads a saved table state. Gated by `readSave` permission. |

### `UI`
| Function | Description |
|---|---|
| `UI.showMessage(msg)` | Logs a UI message (hook into `AITInGameHUD::ShowNotification` in your Blueprint/mod glue for an on-screen toast). |

### `Game`
| Function | Description |
|---|---|
| `Game.endGame(table)` | Signals game-end (logs; hook into your mod's win-condition flow). |
| `Game.setVariable(key, value)` | Stores an arbitrary Lua value in a shared `_IT_vars` table, persisted for the VM's lifetime (not across server restarts — combine with `Table.save` for persistence). |
| `Game.getVariable(key)` | Retrieves a previously-set variable. |

### `RPG`
| Function | Description |
|---|---|
| `RPG.fogOfWar.enable(bool)` | Reveals (`true`) or hides (`false`) the entire fog grid. |
| `RPG.fogOfWar.hideAll()` | Hides the entire fog grid. |
| `RPG.fogOfWar.revealAll()` | Reveals the entire fog grid. |
| `RPG.fogOfWar.revealCircle({x,y,z}, radius)` | Reveals a circular area around a world position. |
| `RPG.grid.setType("square"\|"hex")` | Sets the active `AGridManager`'s grid type. |
| `RPG.grid.setCellSize(size)` | Sets grid cell size in world units. |
| `RPG.grid.show(bool)` | Toggles grid visibility (server RPC, replicated). |
| `RPG.initiative.setOrder(table)` | Forwards a turn-order table to any `Events.on("InitiativeUpdated", ...)` listener — pair with a UMG initiative-tracker widget bound to that event. |
| `RPG.initiative.show(bool)` | Forwards to `Events.on("InitiativeVisibility", ...)` listeners. |
| `RPG.initiative.next()` | Forwards to `Events.on("InitiativeNext", ...)` listeners. |
| `RPG.initiative.startCombat()` | Forwards to `Events.on("CombatStarted", ...)` listeners. |
| `RPG.initiative.endCombat()` | Forwards to `Events.on("CombatEnded", ...)` listeners. |

`RPG.fogOfWar.*` and `RPG.grid.*` require an `AFogOfWarManager` /
`AGridManager` actor to be present in the loaded level (the bindings find
the first instance of each via `TActorIterator`; if none exists, calls are
silently no-ops). `RPG.initiative.*` has no dedicated native actor — it's
intentionally left as an event-forwarding layer since initiative-tracker UI
is highly mod-specific (see `SDK/Examples/RPGCampaign/scripts/main.lua` for
a complete reference implementation that listens to these events and renders
its own combat-order broadcast).

### Math Helpers
| Function | Description |
|---|---|
| `Vector3(x, y, z)` | Constructs a position table usable anywhere a `{x,y,z}` argument is expected. |

## Example: Minimal Custom Game

```lua
-- Saved/Mods/coin_flip/scripts/main.lua
Events.on("GameStart", function()
    Objects.spawn("token", { position = Vector3(0, 0, 50) })
    UI.showMessage("Coin Flip ready! Right-click the token to flip it.")
end)
```

```json
// Saved/Mods/coin_flip/manifest.json
{
  "id": "coin_flip",
  "name": "Coin Flip",
  "version": "1.0.0",
  "author": "Example",
  "description": "The simplest possible mod.",
  "entry": "scripts/main.lua",
  "permissions": ["spawnObjects", "showUI"]
}
```

See `SDK/Examples/` for four complete, more substantial reference mods
(Chess, Checkers, DiceGame, RPGCampaign) covering move validation, forced
captures, multi-die roll tracking, and full RPG session tooling respectively.

## Custom Mesh & Texture Assets

Mods that reference their own `objects[].mesh` / `objects[].texture` paths in
`manifest.json` (see the Chess/Checkers/RPGCampaign examples) are pointing at
files within the mod's own folder, loaded through the editor-time content
pipeline when the mod's assets are imported into the project. If you instead
want **players to import custom meshes/textures live during a session**
(rather than shipping them as part of the mod), that's a different system —
see `Docs/Asset_Import_Guide.md` for `UAssetImportSubsystem`, which handles
runtime GLTF/GLB/PNG/JPG/WEBP/MP3/WAV import and surfaces imported assets in
the Workshop panel's asset browser.
