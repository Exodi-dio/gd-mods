# Geometry Dash 2.2091 Mods

This repo builds and releases Geometry Dash mods for **GD 2.2091**, using the **Geode latest release SDK** (compatible with Geode pre-release).

## Releases

Every release is organized per mod. Download the `.geode` for the mod you want and install it with Geode Mod Loader.

| Mod | Mod ID | Release tag | Platforms | Status |
|-----|--------|-------------|-----------|--------|
| Jukebox | `fleym.nongd` | `jukebox-v2.2-crossplatform` | Windows + Android32 + Android64 | ✅ Built |
| GDH | `tobyadd.gdh` | `gdh-v4.2-android` | Android32 + Android64 | ✅ Built |
| xdBot | `zilko.xdbot` | `xdbot-v2.2-crossplatform` | Windows + Android32 + Android64 | ✅ Built |
| Node IDs | `geode.node-ids` | `node-ids-v1.23.3-android` | Android32 + Android64 | ✅ Built |
| Icon Gradients | `zilko.icon_gradients` | `icon-gradients-v1.3.11-android` | Android32 + Android64 | ✅ Built |
| Death Animations | `zilko.death_animations` | `death-animations-v1.1.4-android` | Android32 + Android64 | ✅ Built |
| Level Thumbnails | `cdc.level_thumbnails` | — | — | ❌ Blocked (Argon dep) |

Each release contains a **single combined `.geode`** that merges all built platforms into one file.

### Downloads

- **Jukebox:** https://github.com/Exodi-dio/gd-mods/releases/tag/jukebox-v2.2-crossplatform
- **GDH:** https://github.com/Exodi-dio/gd-mods/releases/tag/gdh-v4.2-android
- **xdBot:** https://github.com/Exodi-dio/gd-mods/releases/tag/xdbot-v2.2-crossplatform
- **Node IDs:** https://github.com/Exodi-dio/gd-mods/releases/tag/node-ids-v1.23.3-android
- **Icon Gradients:** https://github.com/Exodi-dio/gd-mods/releases/tag/icon-gradients-v1.3.11-android
- **Death Animations:** https://github.com/Exodi-dio/gd-mods/releases/tag/death-animations-v1.1.4-android

## How mods are built

Each mod lives in its own folder (`jukebox/`, `gdh/`, `xdbot/`, `node-ids/`, `icon-gradients/`, `death-animations/`, `level-thumbnails/`) with its own GitHub Action workflow under `.github/workflows/`. The workflow:

1. Builds the mod for its supported platforms with the Geode latest SDK.
2. Merges the platform `.geode` files into one combined `.geode` with the Geode CLI (`geode package merge`).
3. On a version tag (`gdh-v*`, `jukebox-v*`, `xdbot-v*`, `node-ids-v*`, `icon-gradients-v*`, `death-animations-v*`), the merged `.geode` is published as a GitHub release asset.

## Platform notes

- **macOS and iOS builds are not available** for any mod: the Geode bindings for GD 2.2091 (`Exodi-dio/bindings`) lack addresses for several hooked functions on those platforms, so CMake configuration fails there.
- **GDH** builds Android only: its replay recorder depends on ffmpeg, and only Android ffmpeg libraries are packaged in this repo (Windows/macOS would need their own ffmpeg builds).
- **xdBot** builds Android only: its Windows build fails to configure CMake with the Geode SDK on CI runners.
- **Node IDs**, **Icon Gradients**, and **Death Animations** build Android only.

## Notes

- **Eclipse Menu** is NOT included: its source repository (`EclipseMenu/EclipseMenu`) returns 404 (deleted/unavailable), so it cannot be ported from source.
- **Level Thumbnails** is blocked: its Argon 1.5.1 dependency is incompatible with Geode 6.x / GD 2.2091 and is unmaintained (no newer version). Fixing it would require removing Argon and rewriting networking/auth using Geode's built-in `web::WebRequest()`.
- **QOLMod** has been removed from this repo: its `rift` dependency (`https://git.eclipse.menu/EclipseMenu/RIFT`) is down and not on GitHub, and it had an unresolved loading screen freeze on GD 2.2091 / Geode 6.x.
- xdBot is a botting/macro tool — use at your own risk in online levels.
- GDH is a cheat mod menu — use at your own risk in online levels.
- **Death Animations** does not include More Icons support: the latest More Icons release (2.1.3) is incompatible with Geode 6, so the optional dependency was removed.
