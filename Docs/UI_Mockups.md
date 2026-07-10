# InfinityTable — UI Mockups

## A note on what this document is

The original spec asked for "UI mockups/screenshots." Actual screenshots
require a compiled, running build with authored UMG Blueprint widgets bound
to the C++ classes in `Source/InfinityTable/UI/` — none of which exists yet
in this repository (the `.h`/`.cpp` files define behavior and data binding
points via `BindWidgetOptional`, but the visual `WBP_*` Blueprint assets
that would actually render pixels haven't been authored, since that's
editor-only work this text-based deliverable can't perform). Fabricating a
fake screenshot image would misrepresent the project's actual state, so
this document instead gives ASCII wireframes of each panel's intended
layout, derived directly from the widget classes' actual bound properties —
every element shown below corresponds to a real `BindWidgetOptional` field
or button in the corresponding `.h` file, not invented UI.

Treat these as layout specs for whoever builds the `WBP_*` Blueprint
widgets, not as a substitute for seeing the real thing running.

## Panel Inventory

| # | Panel | Class | Default Key |
|---|---|---|---|
| 1 | In-Game HUD (status bar) | `UInGameHUDWidget` | always visible |
| 2 | Chat | `UChatWidget` | Enter |
| 3 | Object Browser | `UObjectBrowserWidget` | (toggled via gameplay code) |
| 4 | Radial Menu | `URadialMenuWidget` | Right Mouse / Q |
| 5 | Lobby | `ULobbyWidget` | (pre-game screen) |
| 6 | Saved Games | `USavedGamesWidget` | (menu access) |
| 7 | Settings | `USettingsWidget` | Escape |
| 8 | Scripting Console | `UScriptingConsoleWidget` | ` (tilde) |
| 9 | Workshop | `UWorkshopWidget` | Y |
| 10 | Player List | `UPlayerListWidget` | Tab |

## 1. In-Game HUD (`UInGameHUDWidget`)

Always-on overlay, minimal footprint per the prompt's "Minimal HUD" requirement.

```
┌──────────────────────────────────────────────────────────────┐
│ FantasyTavern          ●●● 4 Players          Host            │  <- EnvironmentText / PlayerCountText / RoleText
│                                                                 │
│                                                                 │
│                         (table viewport)                       │
│                                                                 │
│                                                  ┌─────────────┐│
│                                                  │ Card drawn  ││  <- NotificationStack (auto-dismiss toasts)
│                                                  │ by Alice    ││
│                                                  └─────────────┘│
└──────────────────────────────────────────────────────────────┘
```

## 2. Chat (`UChatWidget`)

Bottom-left dock, per Tabletop-Simulator-style conventions.

```
┌─────────────────────────────┐
│ [Alice]: anyone want to play │  <- MessageScrollBox (scrolling log)
│ [Bob]: sure, hosting now      │
│ [System]: Bob is now Host     │  <- AddSystemMessage (dimmed color)
│ [Alice]: nice                 │
├─────────────────────────────┤
│ Type a message...      [Send]│  <- ChatInput / SendButton
└─────────────────────────────┘
```

## 3. Object Browser (`UObjectBrowserWidget`)

Side panel, grid of spawnable objects with search/filter.

```
┌───────────────────────────────┐
│ Search: [____________]         │  <- SearchBox
├───────────────────────────────┤
│ [All] [Dice] [Cards] [Tokens]  │  <- category filter (FilterByCategory)
│ [Miniatures] [Custom Meshes]   │     "Custom Meshes" = imported assets
├───────────────────────────────┤
│  ┌────┐ ┌────┐ ┌────┐ ┌────┐  │
│  │ d6 │ │d20 │ │card│ │deck│  │  <- ObjectTileView
│  └────┘ └────┘ └────┘ └────┘  │
│  ┌────┐ ┌────┐ ┌────┐ ┌────┐  │
│  │ mini│ │token│ │my_  │ │... │  <- imported assets appear here too
│  └────┘ └────┘ │minia│ └────┘  │     (Category = "Custom Meshes")
│                 └────┘          │
└───────────────────────────────┘
```

## 4. Radial Menu (`URadialMenuWidget`)

Context-sensitive, appears centered on cursor at the targeted object.
Options vary by object type (see `BuildOptionsForTarget` in
`RadialMenuWidget.cpp`).

```
                    [Flip]
            [Lock]          [Rotate]
                  ╲    ╱
                   ╲  ╱
        [Duplicate]──●──[Roll]      <- "Roll" only shown for ATableDice
                   ╱  ╲
                  ╱    ╲
            [Delete]    [Shuffle]   <- "Shuffle"/"Draw" only for ATableDeck
```

## 5. Lobby (`ULobbyWidget`)

Pre-game session browser / host screen.

```
┌──────────────────────────────────────────────┐
│  HOST A GAME                                   │
│  Name: [Friday Night D&D___________]           │  <- GameNameInput
│  Max Players: [6▲▼]  [ ] LAN  [ ] Private       │  <- MaxPlayersInput/LANCheckbox/PrivateCheckbox
│                                    [Host Game]  │  <- HostButton
├──────────────────────────────────────────────┤
│  JOIN A GAME                       [Refresh]   │  <- RefreshButton
│  ┌────────────────────────────────────────┐   │
│  │ Friday Night D&D    4/6   FantasyTavern │   │  <- session list (FITSessionResult)
│  │ Quick Chess Match   1/2   ModernRoom    │   │
│  └────────────────────────────────────────┘   │
│  Status: 2 sessions found.                      │  <- StatusText
└──────────────────────────────────────────────┘
```

## 6. Saved Games (`USavedGamesWidget`)

```
┌──────────────────────────────────────────────┐
│  Save current table as:                        │
│  [My Campaign Session 4____________] [Save]    │  <- SaveNameInput / SaveButton
├──────────────────────────────────────────────┤
│  SAVED GAMES                       [Refresh]   │  <- RefreshButton
│  ┌────────────────────────────────────────┐   │
│  │ My Campaign Session 3   Jun 18  [Load]  │   │  <- save slot list (FSaveSlotInfo)
│  │                          [Overwrite][X] │   │
│  │ __autosave__             Jun 20  [Load] │   │
│  └────────────────────────────────────────┘   │
│  Status: Game loaded.                           │  <- StatusText
└──────────────────────────────────────────────┘
```

## 7. Settings (`USettingsWidget`)

```
┌──────────────────────────────────────────────┐
│  AUDIO                                          │
│  Music Volume   [────●─────────] 70%           │  <- MusicVolumeSlider
│  SFX Volume     [──────────●───] 100%          │  <- SFXVolumeSlider
│                                                  │
│  CAMERA                                         │
│  Mouse Sensitivity [────●──────] 1.0x          │  <- MouseSensitivitySlider
│  [ ] Invert Camera Y                            │  <- InvertCameraYCheckbox
│                                                  │
│  DISPLAY                                        │
│  [ ] Show FPS Counter                           │  <- ShowFPSCheckbox
│                                                  │
│                       [Reset to Defaults] [Apply]│  <- ResetButton / ApplyButton
└──────────────────────────────────────────────┘
```

## 8. Scripting Console (`UScriptingConsoleWidget`)

Host/Co-Host only — see `HasConsolePermission()`.

```
┌──────────────────────────────────────────────┐
│ InfinityTable Scripting Console                │
│ > Objects.spawn("d20", {position=Vector3(0,0,50)})│  <- OutputScrollBox (echoed input)
│ > RPG.fogOfWar.revealAll()                      │
│ Error executing command — see server log.       │  <- error lines tinted red
├──────────────────────────────────────────────┤
│ ] _______________________________      [Run]   │  <- CommandInput / ExecuteButton
└──────────────────────────────────────────────┘
```

## 9. Workshop (`UWorkshopWidget`)

Combines mod management and runtime asset import — the panel that was
missing from the original UI pass; see `Docs/Modding_API.md` and
`Docs/Asset_Import_Guide.md` for the systems it surfaces.

```
┌──────────────────────────────────────────────┐
│  AVAILABLE MODS                    [Refresh]   │  <- RefreshModsButton
│  ┌────────────────────────────────────────┐   │
│  │ coin_flip          [Load]                │   │  <- DiscoverMods() results
│  │ dice_battle (loaded) [Unload]            │   │  <- GetLoadedMods() results
│  └────────────────────────────────────────┘   │
├──────────────────────────────────────────────┤
│  IMPORT ASSET                                   │
│  Path: [C:/Users/Me/my_mini.glb___] [Import]   │  <- ImportPathInput / ImportAssetButton
├──────────────────────────────────────────────┤
│  IMPORTED ASSETS                                │
│  ┌────────────────────────────────────────┐   │
│  │ my_mini.glb   Custom Meshes    2.1 MB [X]│   │  <- GetAllImportedAssets() results
│  └────────────────────────────────────────┘   │
│  Status: Imported 'my_mini.glb'.                │  <- StatusText
└──────────────────────────────────────────────┘
```

## 10. Player List (`UPlayerListWidget`)

Not one of the original 7 panels from the prompt's UI/UX section — added
because "Multiplayer moderation" (SECURITY) and "player permissions"
(MULTIPLAYER NETWORKING) were both explicit requirements with backend
functions (`AITGameMode::KickPlayer`/`BanPlayer`/`SetPlayerPermission`) but
no UI surface to actually click them from. This panel is that surface.

```
┌──────────────────────────────────────────────┐
│  PLAYERS                            [Refresh]   │  <- RefreshButton
│  ┌────────────────────────────────────────┐   │
│  │ ● Alice (Host, you)                      │   │  <- bIsLocalPlayer marker
│  │ ● Bob (Co-Host)        [Demote] [Kick]   │   │  <- only shown if
│  │ ● Carol (Player)  [Promote][Kick][Ban]   │   │     HasModerationPermission()
│  │ ● Dave (Spectator)[Promote][Kick][Ban]   │   │
│  └────────────────────────────────────────┘   │
│  Status: Kicked Dave.                           │  <- StatusText
└──────────────────────────────────────────────┘
```

Players viewing this panel without moderation permission (i.e. anyone who
isn't Host) see the same roster but without the Kick/Ban/Promote buttons —
`KickByName`/`BanByName`/`PromoteByName` all independently re-check
permission server-side regardless of what the client UI shows, so hiding the
buttons client-side is a UX nicety, not the actual security boundary.

## Visual Style Direction (not yet authored as a Blueprint style guide)

The prompt calls for a look "inspired by Tabletop Simulator, modern PC
games, Steam interfaces" with minimal HUD, radial menus, drag-and-drop, and
smooth animations. None of the widget `.cpp` files set colors, fonts, or
animation curves — those are intentionally left to UMG Blueprint styling
(Slate brushes, animation tracks) rather than hardcoded in C++, since visual
polish is exactly the kind of thing that should live in Blueprint/Designer
assets so artists can iterate without recompiling. A reasonable starting
palette: dark charcoal panels (`#1A1A1FCC` translucent), warm amber accents
for interactive elements (`#D9A441`), and the existing `EObjectOwnership`/
role colors already defined in C++ (`AITPlayerState::PlayerColor`) as the
per-player accent used in chat names and turn indicators.
