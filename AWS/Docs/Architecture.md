# InfinityTable Architecture

## System Diagram

```
CLIENT
  ITPlayerController -> GrabComponent -> TableObject (RPC)
  InGameHUD (UMG)    -> RadialMenu, ChatWidget, ObjectBrowser

SERVER (Authoritative)
  ITGameMode -> ITGameState (replicated)
     |
     +-- LuaSubsystem (Lua 5.4 + sol2, sandboxed)
     +-- ModLoadingSubsystem (validates + loads .itmod)
     +-- SaveGameSubsystem (SQLite, autosave 5min)
     +-- TableSpawnManager (object registry)
     +-- AudioManagerSubsystem (positional SFX)
     +-- UITSessionSubsystem (EOS/Steam sessions)

PHYSICS: Chaos Physics on server, bReplicatePhysics=true to clients
NETWORK: UE5 NetDriver UDP, DOREPLIFETIME for state, Server RPCs for actions
```

## Key Design Decisions

1. **Physics Authority** - Server owns all physics. Clients send grab/release RPCs.
2. **Lua Sandbox** - Blocks io/os/require/debug. 64MB memory cap. 100k instruction hook.
3. **Save Format** - SQLite with JSON blobs (FJsonObjectConverter). Autosave every 5min.
4. **Mod Validation** - Scans Lua for blocked patterns, blocks executable file types.
5. **Object Ownership** - Public / Player / Host / Locked enum per object.

## AWS Architecture

```
Internet -> Elastic IP -> EC2 (c5.xlarge)
                            |
                    Docker: it_server (port 7777/UDP)
                    Nginx:  health check (port 80/TCP)
                    S3:     save files + backups
                    CloudWatch: logs + metrics
```
