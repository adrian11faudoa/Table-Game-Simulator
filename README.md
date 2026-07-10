# InfinityTable

Physics-based virtual tabletop sandbox engine built on Unreal Engine 5.

## Features

- Real-time Chaos Physics (dice, cards, tokens, miniatures, joints/hinges for doors and lids)
- Authoritative multiplayer via Unreal NetDriver (EOS/Steam sessions + LAN), with real kick/ban moderation
- Lua 5.4 scripting sandbox (sol2 binding) with a Mod SDK and four reference mods
- SQLite save system with autosave
- Fog of war, grid system for RPG sessions (initiative tracking via Lua event hooks)
- Runtime asset import (GLTF/GLB, PNG/JPG/WEBP, MP3/WAV) plus a Workshop panel for browsing imported assets and mods
- Object duplication and group tagging (move/lock/delete multiple objects as a unit)
- Full UI panel set: Lobby, Chat, Object Browser, Saved Games, Settings, Scripting Console, Workshop, Player List, Radial Menu
- AWS-ready dedicated server stack (CloudFormation + Docker)

Steam Workshop *content distribution* (as opposed to Steam session
hosting, which is implemented) is not currently built — see
`Docs/Modding_API.md` and `Docs/Asset_Import_Guide.md` for the mod/asset
distribution mechanisms that do exist today.

## Project Structure

```
InfinityTable/
├── Source/InfinityTable/   C++ source (Core, Objects, Physics, Networking, Scripting, RPG, Save, Audio, Mod, Assets, UI)
├── Config/                 DefaultEngine.ini, DefaultGame.ini, DefaultInput.ini
├── SDK/Examples/           Chess, Checkers, DiceGame, RPGCampaign (Lua + placeholder assets)
├── AWS/                    CloudFormation stack, deploy scripts, Docker Compose
├── Docs/                   Architecture, build, networking, modding, asset import, UI mockups, save system guides
├── ThirdParty/             Lua 5.4, sol2, SQLite3 (download/build via install.bat or install.sh)
├── Plugins/glTFRuntime/    Fetched separately — see ThirdParty table below
└── Saved/Mods/             Drop mod folders here at runtime (created automatically; see SDK/README.md)
```

## Build (Windows)

```bash
# 1. Download ThirdParty deps (see ThirdParty/install.bat)
ThirdParty\install.bat

# 2. Generate project files + build
setup_and_build.bat
```

## Deploy to AWS

```bash
export KEY_NAME=my-keypair
export AWS_REGION=us-east-1
./AWS/scripts/deploy.sh create
```

See `AWS/README.md` for full deployment guide.

## ThirdParty Dependencies (download manually)

| Lib | URL | Where to place |
|-----|-----|----------------|
| Lua 5.4 | https://luabinaries.sourceforge.net | ThirdParty/lua54/ |
| sol2 | https://github.com/ThePhD/sol2 | ThirdParty/sol2/include/ |
| SQLite3 | https://sqlite.org/download.html | ThirdParty/sqlite3/ |
| glTFRuntime | https://github.com/rdeioris/glTFRuntime | Plugins/glTFRuntime/ |

## License

MIT — see LICENSE
