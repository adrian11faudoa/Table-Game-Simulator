# InfinityTable — Mod SDK

## Quick Start

```
SDK/
├── README.md
├── Examples/
│   ├── Chess/          -- Full chess with move validation
│   ├── Checkers/       -- Forced captures + king promotion
│   ├── DiceGame/       -- Dice Battle for 2-6 players
│   └── RPGCampaign/    -- D&D RPG with fog of war & GM tools
```

## Create a Mod in 3 Steps

**1. Create folder:** `InfinityTable/Saved/Mods/my_mod/`

(`Saved/Mods/` is created automatically on first run by `UModLoadingSubsystem`;
you can also override the folder name via its `ModsDirectoryName` property if
you'd rather use something else.)

**2. Add manifest.json:**
```json
{
  "id": "my_mod",
  "name": "My Game",
  "version": "1.0.0",
  "author": "You",
  "description": "A fun game.",
  "entry": "scripts/main.lua",
  "permissions": ["spawnObjects", "networkSync", "showUI"]
}
```

**3. Write scripts/main.lua:**
```lua
Events.on("GameStart", function()
    Objects.spawn("d6", { position = {0, 0, 10} })
    UI.showMessage("Game ready!")
end)
```

## Available Lua APIs

| Namespace | Key Functions |
|-----------|--------------|
| `Objects` | spawn, destroy, getAll, getByType |
| `Events`  | on, emit |
| `Table`   | getPlayerCount, broadcast, clearTable |
| `UI`      | showMessage |
| `Game`    | endGame, setVariable, getVariable |
| `RPG`     | fogOfWar, grid, initiative |

## Permissions

| Permission     | Allows |
|----------------|--------|
| spawnObjects   | Objects.spawn / destroy |
| networkSync    | Replicate state to all clients |
| showUI         | UI.showMessage / createPanel |
| readSave       | Table.load |
| writeSave      | Table.save |
