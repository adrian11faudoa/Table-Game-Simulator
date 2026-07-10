# InfinityTable — Build Instructions

## Prerequisites

- Unreal Engine 5.3 or 5.4 (Epic Games Launcher or source build)
- Visual Studio 2022 (Windows) with the "Game development with C++" workload, or
  Clang/GCC + UE5 Linux cross-compile toolchain (Linux)
- Git with Git LFS (large binary assets are tracked via LFS)
- ~60 GB free disk space (UE5 install + project + intermediate build files)

## 1. Fetch Third-Party Dependencies

InfinityTable depends on four third-party components that are not
redistributed in this repository for licensing/size reasons. Place them under
`ThirdParty/` and `Plugins/` exactly as described in the root `README.md`
table, or run the helper script:

**Windows:**
```bat
ThirdParty\install.bat
```

**Linux/macOS:**
```bash
./ThirdParty/install.sh
```

This downloads:
- Lua 5.4 (static lib + headers) → `ThirdParty/lua54/`
- sol2 (header-only) → `ThirdParty/sol2/include/`
- SQLite3 (amalgamation) → `ThirdParty/sqlite3/`
- glTFRuntime plugin → `Plugins/glTFRuntime/`

If `install.bat`/`install.sh` cannot reach the internet in your environment,
download each dependency manually from the URLs in the README and place them
in the same folders.

## 2. Windows Build

```bat
setup_and_build.bat
```

This will:
1. Locate your UE5 install (checks common Epic Games Launcher paths).
2. Run `UnrealBuildTool -projectfiles` to generate `InfinityTable.sln`.
3. Build the `InfinityTableEditor` target in Development configuration.
4. Open the generated solution so you can launch from Visual Studio (F5).

To build a packaged (non-editor) game build afterward, use the Unreal Editor's
**Platforms → Windows → Package Project** menu, or run:

```bat
"%UE5_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
  -project="%CD%\InfinityTable.uproject" -noP4 -platform=Win64 ^
  -clientconfig=Shipping -cook -build -stage -pak -archive ^
  -archivedirectory="%CD%\Build\Windows"
```

## 3. Linux Build (Dedicated Server)

```bash
export UE5_ROOT=$HOME/UnrealEngine   # path to your UE5 engine checkout/install
./build_linux.sh
```

This generates Linux Makefiles and builds the `InfinityTable` target. The
resulting binary is at `Binaries/Linux/InfinityTable`.

To build a **dedicated server** binary specifically (no rendering, headless),
add the `Server` target type:

```bash
"$UE5_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
  -project="$(pwd)/InfinityTable.uproject" -noP4 -platform=Linux \
  -serverconfig=Shipping -server -cook -build -stage -pak -archive \
  -archivedirectory="$(pwd)/Build/Linux"
```

This is the binary used by `docker-compose.yml` and the AWS deployment
scripts (see `Docs/Multiplayer_Guide.md` and `Docs/AWS_Deployment.md`).

## 4. First Run Checklist

- Open the project in the Unreal Editor and let it compile shaders / build
  DDC on first load (can take 10–20 minutes the first time).
- Confirm `Config/DefaultEngine.ini` points `GlobalDefaultGameMode` at
  `AITGameMode` (already set) — verify under **Project Settings → Maps & Modes**.
- Create at least one level under `Content/Maps/` named to match
  `GameDefaultMap` / `ServerDefaultMap` in `DefaultEngine.ini`
  (`MainMenu`, `FantasyTavern`) — these are referenced by config but the
  actual `.umap` assets are not included in source control and must be
  authored in-editor or imported from the Marketplace.
- Place an `AGridManager` and `AFogOfWarManager` actor in any RPG-capable
  level if you intend to use the RPG mode Lua bindings (`RPG.grid.*`,
  `RPG.fogOfWar.*`) — see `Docs/Modding_API.md`.

## 5. Common Build Issues

| Symptom | Cause | Fix |
|---|---|---|
| `lua54.lib` not found | ThirdParty deps not fetched | Run `ThirdParty/install.bat`/`.sh` |
| `sol/sol.hpp: No such file` | sol2 not placed in `ThirdParty/sol2/include` | Re-check folder structure matches README |
| `sqlite3.h` not found | SQLite3 amalgamation missing | Download `sqlite3.c`/`sqlite3.h` to `ThirdParty/sqlite3/` |
| Editor crashes loading `OnlineSubsystemEOS` | EOS SDK not configured | Either configure EOS credentials in `DefaultEngine.ini` `[OnlineSubsystemEOS]`, or set `DefaultPlatformService=NULL` and disable the EOS plugin in `.uproject` for local/LAN-only testing |
| Linking errors for Chaos/PhysicsCore | UE5 install missing physics modules | Re-run Epic Games Launcher "Verify" on the engine install |
