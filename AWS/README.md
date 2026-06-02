# InfinityTable

Physics-based virtual tabletop sandbox engine built on Unreal Engine 5.

## Features

- Real-time Chaos Physics (dice, cards, tokens, miniatures)
- Authoritative multiplayer via Unreal NetDriver
- Lua 5.4 scripting sandbox (sol2 binding)
- SQLite save system with autosave
- Fog of war, grid system, initiative tracker for RPG sessions
- Mod SDK with Steam Workshop integration
- AWS-ready dedicated server stack (CloudFormation + Docker)

## Project Structure

```
InfinityTable/
├── Source/InfinityTable/   C++ source (Core, Objects, Physics, Networking, Scripting, RPG, Save, Audio, Mod)
├── Config/                 DefaultEngine.ini, DefaultGame.ini, DefaultInput.ini
├── SDK/Examples/           Chess, Checkers, DiceGame, RPGCampaign (Lua)
├── AWS/                    CloudFormation stack, deploy scripts, Docker Compose
├── Docs/                   Architecture, networking, modding, save system guides
├── Mods/                   Drop .itmod folders here
└── ThirdParty/             Lua 5.4, sol2, SQLite3, glTFRuntime (download separately)
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
