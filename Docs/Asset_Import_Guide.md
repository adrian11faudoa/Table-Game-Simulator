# InfinityTable — Asset Import Guide

## Two Different Import Paths

InfinityTable has two distinct ways assets get into the game, and it's
important to know which one applies to what you're doing:

1. **Editor-time import** — standard Unreal Editor asset import (drag a file
   into the Content Browser). This is what you use while *building* the
   game/mods themselves (e.g. authoring a custom dice skin as a project
   asset). UE5's built-in FBX/OBJ/glTF importers, texture importers, and
   audio importers all work normally here — nothing InfinityTable-specific
   is needed, just use the editor as you would for any UE5 project.

2. **Runtime import** — a player, mid-session, drags in or picks a file from
   their own computer to add custom content to a live game (a custom
   miniature, a card-face texture, a custom dice-roll sound). This is what
   `UAssetImportSubsystem` (`Source/InfinityTable/Assets/`) handles, and it's
   what the rest of this guide is about.

Runtime import is fundamentally more constrained than editor import, because
there is no asset cooking/compilation step available while a game session is
live — content has to be usable as-is or converted on the fly.

## Supported Formats & Runtime Limitations

| Format | Category | Runtime spawnable? | Notes |
|---|---|---|---|
| `.gltf` / `.glb` | 3D Mesh | **Yes** | Loaded via the glTFRuntime plugin (already a project dependency — see `InfinityTable.uproject`). This is the recommended format for any mesh you want players to import live. |
| `.fbx` | 3D Mesh | No (catalogued only) | Stock UE5 has no first-party *runtime* FBX importer outside the Editor's Interchange pipeline. Files are validated, staged under `Saved/ImportedAssets/Meshes/`, and catalogued — but won't appear as a spawnable object in a live session until converted to glTF (see below). |
| `.obj` | 3D Mesh | No (catalogued only) | Same limitation as FBX — no stock runtime importer. |
| `.png` / `.jpg` / `.jpeg` / `.webp` | Texture | **Yes** | Loaded directly via `UTexture2D` runtime decode (`FImageUtils`/`IImageWrapper`); usable immediately as a card face, token skin, or material override. |
| `.mp3` / `.wav` | Audio | **Yes** | Loaded via `USoundWave` runtime import; usable immediately as a custom sound trigger or dice-skin sound. |

**Why FBX/OBJ aren't runtime-spawnable:** there's no way around this without
either licensing a commercial runtime FBX SDK (e.g. paid plugins from the
Marketplace) or running an offline conversion step. If you need FBX/OBJ
support for player-imported meshes specifically, the practical path is:

1. Accept the FBX/OBJ upload (already handled — `ImportAsset()` validates
   and stages it).
2. Run an offline conversion (e.g. a small headless `assimp`/Blender CLI
   pipeline on your dedicated server, converting to GLTF).
3. Re-import the converted GLTF through the same `ImportAsset()` path.

This conversion step is **not implemented** in the current codebase — it's
the natural next addition if FBX/OBJ player-import becomes a priority over
just asking players to export glTF from their tool of choice (Blender,
which most hobbyist 3D modelers already use, exports glTF natively).

## File Size Limits

Configurable on `UAssetImportSubsystem` (defaults shown):

| Type | Default Limit |
|---|---|
| Mesh (FBX/OBJ/GLTF/GLB) | 64 MB |
| Texture (PNG/JPG/WEBP) | 16 MB |
| Audio (MP3/WAV) | 32 MB |

Files exceeding these are rejected with `EAssetImportResult::FileTooLarge`
before any disk I/O beyond the size check itself.

## API

```cpp
UAssetImportSubsystem* Assets = GetGameInstance()->GetSubsystem<UAssetImportSubsystem>();

FImportedAssetInfo Info;
EAssetImportResult Result = Assets->ImportAsset(TEXT("C:/Users/Me/Downloads/my_miniature.glb"), Info);

if (Result == EAssetImportResult::Success)
{
    // Info.AssetID, Info.DisplayName, Info.Category, Info.ThumbnailPath are populated
}
```

`OnAssetImported` (a `BlueprintAssignable` multicast delegate) fires for
every import attempt — success or failure — so a drag-and-drop UI can show
progress/error toasts without polling.

Imported assets are catalogued in memory for the lifetime of the
`GameInstance` and staged on disk under:

```
Saved/ImportedAssets/
├── Meshes/<asset-guid>/<original-filename>
├── Textures/<asset-guid>/<original-filename>
└── Audio/<asset-guid>/<original-filename>
```

**Note:** this catalogue is currently in-memory only (`TMap` on the
subsystem) and is not yet persisted across server restarts or included in
`USaveGameSubsystem`'s save state — see the corresponding note in
`Docs/Save_System.md`. Re-importing the same file after a restart is
required today; wiring imported-asset metadata into the save/load round-trip
is a reasonable follow-up if persistent custom content is a priority.

## Object Browser Integration

Successfully imported meshes (GLTF/GLB only, per the table above)
automatically appear in `UObjectBrowserWidget` under a "Custom Meshes"
category alongside the built-in object types (dice, cards, tokens,
miniatures), so players can drag them onto the table the same way they would
any built-in object — see `Docs/Modding_API.md` for how mods can register
their own object types alongside imported ones via
`UTableSpawnManager::RegisterObjectType`.

## Thumbnails

Texture imports use the source image directly as their own thumbnail.
Mesh and audio thumbnails (a rendered preview / waveform icon respectively)
are **not generated by `UAssetImportSubsystem` itself** — it requires a live
renderable `UWorld` (for a `SceneCapture2D`-style mesh preview) that isn't
guaranteed to be available at arbitrary import time, so thumbnail rendering
is left to the UI layer (`UObjectBrowserWidget`) to perform once an asset is
actually placed in a scene, or via a generic placeholder icon in the
meantime.

## Drag-and-Drop UI

The actual OS-level "drag a file from your desktop into the game window"
interaction is a platform/windowing concern handled by the client
application shell (outside `UAssetImportSubsystem`'s scope, which only
validates and stages an already-resolved file path). On Windows this
typically means handling `WM_DROPFILES` via a custom `FGenericWindow`
override or a small native plugin; this glue code is not included in this
codebase and would need to be added per-platform if file-drop (as opposed to
an in-game file-picker dialog, which is simpler and works cross-platform via
`IDesktopPlatform::OpenFileDialog`) is required.
