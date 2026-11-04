# Geometry Dash 2.2091 Mods (Android)

This repo builds and releases Geometry Dash mods for **GD 2.2091** on **Android (32 + 64-bit)**, using the **Geode nightly SDK** (`6.0.0-alpha.1`).

## Releases

Every release is organized per mod. Download the `.geode` for the mod you want and install it with Geode Mod Loader.

| Mod | Mod ID | Release tag | Status |
|-----|--------|-------------|--------|
| Jukebox | `fleym.nongd` | `jukebox-v1.0-android` | ✅ Built |
| GDH | `tobyadd.gdh` | `gdh-v1.0-android` | ✅ Built |
| xdBot | `zilko.xdbot` | `xdbot-v1.0-android` | ✅ Built |
| Level Thumbnails | `cdc.level_thumbnails` | — | ❌ Blocked (Argon dep) |

Each release contains a single combined `.geode` that works on both Android32 and Android64.

### Downloads

- **Jukebox:** https://github.com/Exodi-dio/gd-mods/releases/tag/jukebox-v1.0-android
- **GDH:** https://github.com/Exodi-dio/gd-mods/releases/tag/gdh-v1.0-android
- **xdBot:** https://github.com/Exodi-dio/gd-mods/releases/tag/xdbot-v1.0-android

## How mods are built

Each mod lives in its own folder (`jukebox/`, `gdh/`, `xdbot/`, `level-thumbnails/`) with its own GitHub Action workflow under `.github/workflows/`. The workflow:

1. Builds the mod for Android32 and Android64 with the Geode nightly SDK.
2. Merges the two platform `.geode` files into one with `geode package merge --zstd`.
3. Uploads the combined `.geode` as a release asset.

## Notes

- **Eclipse Menu** is NOT included: its source repository (`EclipseMenu/EclipseMenu`) returns 404 (deleted/unavailable), so it cannot be ported from source.
- **Level Thumbnails** is blocked: its Argon 1.5.1 dependency is incompatible with Geode 6.x / GD 2.2091 and is unmaintained (no newer version). Fixing it would require removing Argon and rewriting networking/auth using Geode's built-in `web::WebRequest()`.
- **QOLMod** has been removed from this repo: persistent loading screen freeze on GD 2.2091 / Geode 6.x that could not be resolved.
- xdBot is a botting/macro tool — use at your own risk in online levels.
- GDH is a cheat mod menu — use at your own risk in online levels.
