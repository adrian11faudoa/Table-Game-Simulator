# InfinityTable — Multiplayer & Networking Guide

## Architecture Overview

InfinityTable uses Unreal Engine's standard client-server model with a
**fully authoritative server**. All physics simulation, object state, and
game logic execution (including Lua mod scripts) runs on the server; clients
send intent via RPCs and render replicated state.

```
Client A ─┐
Client B ─┼─ UDP (UE5 NetDriver) ─→ Server (Listen or Dedicated)
Client C ─┘                              │
                                          ├─ Chaos Physics (authoritative)
                                          ├─ LuaSubsystem (mod logic)
                                          ├─ SaveGameSubsystem (SQLite)
                                          └─ ITSessionSubsystem (EOS/Steam/LAN)
```

## Roles & Permissions

Defined in `EPlayerRole` (`ITGameMode.h`):

| Role | Granted to | Capabilities |
|---|---|---|
| Host | First player to join | Full permissions: spawn objects, kick players, GM tools, set co-hosts |
| Co-Host | Promoted by Host | Spawn objects, GM tools; cannot kick |
| Player | Default for joiners | Spawn objects, manipulate unlocked objects |
| Spectator | Explicitly assigned | View-only, cannot spawn or manipulate |

`AITGameMode::HasPermission()` is the single source of truth for permission
checks — always gate server-side actions through it rather than checking
`PlayerRole` directly in gameplay code, since the permission matrix may grow
without every caller needing updates.

### Kicking and Banning

`AITGameMode::KickPlayer(Requester, Target, Reason)` and `BanPlayer(...)`
implement the SECURITY requirement for multiplayer moderation. Both require
the requester to hold the `"KickPlayers"` permission (Host by default) and
fail closed otherwise. `BanPlayer` additionally records the target's unique
net ID in an in-memory set checked by a `PreLogin` override, rejecting
reconnect attempts for the rest of the session — this ban list does **not**
persist across server restarts; a persistent ban list (e.g. written to the
same SQLite database `USaveGameSubsystem` uses) is a reasonable follow-up if
long-term bans across sessions are needed.

`UPlayerListWidget` (see `Docs/UI_Mockups.md` panel 10) is the in-game UI
surface for both functions, plus `SetPlayerPermission` for promoting/
demoting players between roles.

## Hosting a Session

```cpp
UITSessionSubsystem* Sessions = GetGameInstance()->GetSubsystem<UITSessionSubsystem>();

FITSessionConfig Config;
Config.GameName      = TEXT("Friday Night D&D");
Config.EnvironmentID = TEXT("FantasyTavern");
Config.MaxPlayers    = 6;
Config.bLAN          = false;   // true for LAN-only sessions, no EOS/Steam lookup
Config.bPrivate      = false;   // true to hide from public session search
Config.Password      = TEXT(""); // optional join password, checked in ITGameMode

Sessions->HostSession(Config);
```

`OnSessionCreated` fires once the session is live; travel the host's
`PlayerController` to the gameplay map via `ServerTravel` (handled
automatically once a level matching `Config.EnvironmentID` is set as the
listen server's current map).

## Joining a Session

```cpp
Sessions->FindSessions(50);
// ... wait for OnSessionsFound(Results) ...
Sessions->JoinSession(ResultIndex);
// ... wait for OnSessionJoined(bSuccess) ...
```

For **LAN multiplayer** specifically, set `bLAN = true` on the config used by
the host, and have joining clients call `FindSessions` with the same flag —
`SessionSearch->bIsLanQuery` is set automatically from this on the search
path. No internet connection or EOS/Steam credentials are required for LAN
play.

## Replication Model

- **Object transforms**: `ATableObject` uses `bReplicatePhysics = true`, so
  the engine's built-in physics replication keeps clients in sync without
  per-tick RPCs. Position snapshots correct client-side prediction error.
- **Grab/release/flip/lock/color/scale**: dispatched as `Server, Reliable`
  RPCs from the owning client's `UGrabComponent` / `ITPlayerController`,
  validated server-side (see `Server_PickUp_Validate`, etc. in
  `TableObject.cpp`), then applied authoritatively and replicated back down
  via the object's `UPROPERTY(Replicated, ...)` fields.
- **Chat**: `Server_SendChatMessage` (server RPC) → server rebroadcasts to
  every connected `AITPlayerController` via `Client_ReceiveChatMessage`
  (client RPC). This avoids relying on Unreal's actor replication for a
  transient, non-persistent event.
- **Fog of war**: `AFogOfWarManager` keeps the revealed-cell bitmap
  server-side and pushes deltas to all clients via
  `Multicast_UpdateFog`, rather than replicating the full `TArray<uint8>`
  every tick.

## Bandwidth & Tick Rate

Defaults (in `Config/DefaultEngine.ini` and `DefaultGame.ini`):

```ini
net.MaxNetUpdateFrequency=60
net.NetServerMaxTickRate=60
TotalNetBandwidth=32000
MaxDynamicBandwidth=7000
MinDynamicBandwidth=4000
```

For sessions heavy with simultaneously-moving physics objects (e.g. a
dice-tower mod rolling 20 dice at once), `ATableObject` automatically lowers
its own `NetUpdateFrequency` from 60Hz down to 2Hz once its physics velocity
has stayed below `SettledVelocityThreshold` for `SettledTimeBeforeThrottle`
seconds (both tunable per-object, default 1 second), and restores full rate
immediately on grab, release-with-velocity, or any server-side transform
change. This is handled in `ATableObject::Tick` — see
`Source/InfinityTable/Objects/TableObject.h/.cpp`.

## Dedicated Server Deployment

For production hosting (vs. one player's machine acting as listen server),
build the `Linux` server target (see `Docs/Build_Instructions.md` §3) and
deploy via:

- `docker-compose.yml` — local/self-hosted Docker deployment
- `AWS/scripts/deploy.sh` — automated AWS EC2 + CloudFormation deployment
  (see `Docs/AWS_Deployment.md` for the full walkthrough)
- `AWS/systemd/infinitytable.service` — systemd unit if running directly on
  a Linux host without Docker

The server listens on UDP port `7777` by default (`Config/DefaultGame.ini`
`[URL] Port=7777`); ensure this port is open in any firewall/security group.

## Troubleshooting

| Symptom | Likely Cause |
|---|---|
| Clients can't find LAN sessions | `bLAN` mismatch between host and searcher, or UDP broadcast blocked by firewall/VPN |
| Objects "rubber-band" for clients | High latency + aggressive `GrabStiffness` in `UGrabComponent`; lower the value for laggier connections |
| Chat messages not appearing | Confirm `AITInGameHUD` widget classes are assigned in the Blueprint subclass — native code looks up `ChatWidget_` by class reference, not by name |
| Session created but no one can join | `bShouldAdvertise` was set false (i.e. `bPrivate = true`) and no direct IP/invite flow is wired up yet — direct-IP join is a planned follow-up, not implemented in `ITSessionSubsystem` |
