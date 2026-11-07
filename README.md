# Geometry Dash 2.2091 Mods

This repo builds and releases Geometry Dash mods for **GD 2.2091**, using the **Geode nightly SDK** (`6.0.0-alpha.1`).

## Releases

Every release is organized per mod. Download the `.geode` for the mod you want and install it with Geode Mod Loader.

| Mod | Mod ID | Release tag | Platforms | Status |
|-----|--------|-------------|-----------|--------|
| Jukebox | `fleym.nongd` | `jukebox-v1.3-crossplatform` | Windows + Android32 + Android64 | ✅ Built |
| GDH | `tobyadd.gdh` | `gdh-v3.0-android` | Android32 + Android64 | ✅ Built |
| xdBot | `zilko.xdbot` | `xdbot-v1.3-crossplatform` | Android32 + Android64 | ✅ Built |
| QOLMod | `thesillydoggo.qolmod` | — | — | ❌ Blocked (rift dep down + unresolved freeze) |
| Level Thumbnails | `cdc.level_thumbnails` | — | — | ❌ Blocked (Argon dep) |

Each release contains a **single combined `.geode`** that merges all built platforms into one file.

### Downloads

- **Jukebox:** https://github.com/Exodi-dio/gd-mods/releases/tag/jukebox-v1.3-crossplatform
- **GDH:** https://github.com/Exodi-dio/gd-mods/releases/tag/gdh-v3.0-android
- **xdBot:** https://github.com/Exodi-dio/gd-mods/releases/tag/xdbot-v1.3-crossplatform

## How mods are built

Each mod lives in its own folder (`jukebox/`, `gdh/`, `xdbot/`, `qolmod/`, `level-thumbnails/`) with its own GitHub Action workflow under `.github/workflows/`. The workflow:

1. Builds the mod for its supported platforms with the Geode nightly SDK.
2. Merges the platform `.geode` files into one combined `.geode` with the Geode CLI (`geode package merge`).
3. On a version tag (`gdh-v*`, `jukebox-v*`, `xdbot-v*`, `qolmod-v*`), the merged `.geode` is published as a GitHub release asset.

## Platform notes

- **macOS and iOS builds are not available** for any mod: the Geode bindings for GD 2.2091 (`Exodi-dio/bindings`) lack addresses for several hooked functions on those platforms, so CMake configuration fails there.
- **GDH** builds Android only: its replay recorder depends on ffmpeg, and only Android ffmpeg libraries are packaged in this repo (Windows/macOS would need their own ffmpeg builds).
- **xdBot** builds Android only: its Windows build fails to configure CMake with the Geode SDK on CI runners.

## Notes

- **Eclipse Menu** is NOT included: its source repository (`EclipseMenu/EclipseMenu`) returns 404 (deleted/unavailable), so it cannot be ported from source.
- **Level Thumbnails** is blocked: its Argon 1.5.1 dependency is incompatible with Geode 6.x / GD 2.2091 and is unmaintained (no newer version). Fixing it would require removing Argon and rewriting networking/auth using Geode's built-in `web::WebRequest()`.
- **QOLMod** source has been restored to the repo but cannot be released yet: its `rift` dependency (`https://git.eclipse.menu/EclipseMenu/RIFT`) is down and not mirrored on GitHub, and it has an unresolved loading screen freeze on GD 2.2091 / Geode 6.x. The `rift` library is used directly in source (`#include <rift.hpp>`, `rift::compile(...)`), so the mod cannot compile without it.
- xdBot is a botting/macro tool — use at your own risk in online levels.
- GDH is a cheat mod menu — use at your own risk in online levels.
