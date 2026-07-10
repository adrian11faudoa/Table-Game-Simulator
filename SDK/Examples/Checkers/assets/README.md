# Placeholder Assets

The `.obj` and `.png` files in this folder are **procedurally generated
placeholder geometry and flat-color textures**, not final game art. They
exist so this example mod's `manifest.json` references resolve to real,
loadable files out of the box rather than 404ing — you should be able to
load this mod immediately and see basic geometric stand-ins (boxes,
cylinders) on the table, then swap in real art.

To replace them: keep the same filenames (or update `manifest.json` to
point at new ones) and import real assets through the editor's standard
content pipeline, or via the runtime `UAssetImportSubsystem` for
player-uploaded content — see `Docs/Asset_Import_Guide.md` for format
support and limitations (GLTF/GLB are runtime-spawnable; this example uses
`.obj` for editor-time/offline-conversion workflows, per that guide).
