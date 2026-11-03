# Geometry Dash 2.2091 Mods (Android + iOS)

This repo builds and releases Geometry Dash mods for **GD 2.2091** on **Android (32 + 64-bit)** and **iOS**, using the **Geode nightly SDK**.

## Releases

Every release is organized per mod. Download the `.geode` for the mod you want and install it with Geode Mod Loader.

| Mod | Mod ID | Release tag |
|-----|--------|-------------|
| Jukebox | `fleym.nongd` | `jukebox-v*` |
| QOLMod | `thesillydoggo.qolmod` | `qolmod-v*` |
| Level Thumbnails | `cdc.level_thumbnails` | `level-thumbnails-v*` |

Each release contains a single combined `.geode` that works on both Android32, Android64 and iOS.

## How mods are built

Each mod lives in its own folder (`jukebox/`, `qolmod/`, `level-thumbnails/`) with its own GitHub Action workflow under `.github/workflows/`. The workflow:

1. Builds the mod for Android32, Android64 and iOS with the Geode nightly SDK.
2. Merges the three platform `.geode` files into one with `geode package merge --zstd`.
3. Uploads the combined `.geode` as a release asset.

## Notes

- **Eclipse Menu** is NOT included: its source repository (`EclipseMenu/EclipseMenu`) returns 404 (deleted/unavailable), so it cannot be ported from source.
- QOLMod is a cheat mod menu — use at your own risk in online levels.
