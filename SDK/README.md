# InfinityTable — Modding SDK

Welcome to the InfinityTable Mod SDK. This directory contains everything you need to create, test, and publish tabletop games and tools for InfinityTable.

## Quick Start

```
SDK/
├── README.md               ← You are here
├── LuaAPI.md               ← Complete Lua API reference
├── ModManifest.schema.json ← JSON schema for manifest validation
└── Examples/
    ├── Chess/              ← Full 8x8 chess implementation
    ├── Checkers/           ← 8x8 checkers with forced captures
    ├── DiceGame/           ← Dice Battle for 2-6 players
    └── RPGCampaign/        ← D&D-style RPG with fog of war
```

## Creating a Mod

1. Create a folder inside `InfinityTable/Mods/your_mod_id/`
2. Add a `manifest.json` (see schema below)
3. Write your `scripts/main.lua` using the Lua API
4. Place assets in `assets/` (`.obj`, `.gltf`, `.png`)
5. Load in-game: **Main Menu → Workshop → Load Local Mod**

## Manifest Minimum Example

```json
{
  "id": "my_game",
  "name": "My Game",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "A fun tabletop game.",
  "entry": "scripts/main.lua",
  "permissions": ["spawnObjects", "networkSync", "showUI"]
}
```

## Lua Entry Point Pattern

```lua
-- Every mod starts by listening for GameStart
Events.on("GameStart", function()
    -- spawn your board, set up state, show welcome message
end)
```

## Useful Resources

- **Lua API Reference**: `SDK/LuaAPI.md`
- **Architecture Overview**: `Docs/Architecture.md`
- **Save System**: `Docs/Architecture.md#save-system-documentation`
- **Asset Import**: `Docs/Architecture.md#asset-import-guide`
- **Community Discord**: https://discord.gg/infinitytable
- **Workshop**: https://steamcommunity.com/app/infinitytable/workshop/

## License

All SDK example code is released under the MIT License.  
You retain full ownership of your mod's content.
