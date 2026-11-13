# Predictive Input Buffer - Android (touch) port

Port of the `ai.predictivebuffer` mod (v4.7) to **Android + Windows**, with a QOLMod / Eclipse
style touch interface. Target: Geometry Dash **2.2091** with Geode **6**.

> The original `.geode` only shipped a Windows `.dll` (x86-64 code). That cannot be
> "converted" to Android: the mod has to be **compiled from source for ARM**. The original
> source was never published, so this port was rebuilt by reading the binary (see below).

## 1) Building

The build runs in GitHub Actions:

1. Push this folder to a GitHub repository (including `.github/workflows`).
2. Open the **Actions** tab and run the *Build Predictive Input Buffer* workflow.
3. When the run is green, open it -> **Artifacts** -> `Build Output` and download the `.geode`
   (Android 32 and 64 bit).

## 2) Installing on Android

Copy the `.geode` to the Geode Launcher mods folder
(usually `Android/media/com.geode.launcher/game/geode/mods/`; on older launchers
`Android/data/com.geode.launcher/files/game/geode/mods/`) and launch GD from the Geode Launcher.

## 3) Usage (touch)

| Where | What it does |
|-------|--------------|
| Floating **IB** button inside the level | **Tap** = cycle the stage `1/1 -> 2/2 -> 4/4 -> 8/8 -> 16/8 -> OFF` (replaces the **F6** key). **Drag** = move it. It flashes green when the buffer delays an input. |
| **IB** button in the pause menu / main menu | Full menu: pick a stage, show/hide the floating button, opacity, logging. |

Settings (stage, button position, opacity) are saved between sessions.
**F6** still works on Windows.

Stages (1 tick = 1/240 s): **1/1**, **2/2**, **4/4**, **8/8**, **16/8** (all modes / wave) and **OFF**.

## What was kept from the original (read from the binary)

- Trajectory simulation with 2 "ghost" players (cube/ball/etc. and wave/dual).
- Simulation horizon: 48 ticks (28 in mini-wave, 38 in wave).
- If the immediate input survives the whole horizon -> it is left untouched. Otherwise delays
  are tried. In wave the candidates are compared and the one with the longest simulated
  survival is chosen.
- Wave: press **and** release are buffered; robot: release too; everything else: press only.
- Ship: conservative one-tick assist only on 1/1; the other stages keep raw input.
  A dead player and fast wave click spam keep their safety bypasses.
- During the simulation side effects are disabled (counted jumps, wave trail, shines, triggers,
  end of level animation) and the ghost only collides with solids, hazards and pending objects.

## v1.2.1 improvements (wave + 1/1 stage)

- Wave evaluates every available delay and selects the one with the longest simulated survival;
  on a tie it picks the shorter delay.
- The time budget is split between the base simulation and the candidates instead of sizing
  the horizon from the first simulation only.
- The time limit is checked inside the simulation loop. If there is no time left to decide,
  the original input is sent without delay.
- The 1/1 stage was added. In it the ship can get a predictive delay of at most one tick; no
  presses are generated automatically.
- The wave logic stays active on every stage with wave enabled, including 16/8.

## v1.2.0 optimizations (mid-range devices: Helio G85, 4 GB)

- **CPU budget per click** (IB menu -> *CPU budget / click*, 6 ms by default): if the prediction
  does not fit, the raw input is let through. The deadline is checked during the simulation;
  a single engine call can make the limit slightly inexact.
- **Adaptive horizon**: measures how long one simulated tick takes on your phone and sizes the
  horizon for the whole candidate set, not only for the initial simulation.
- **Delays 1,2,3,4,6,8,10...** instead of 1,2,3,4,5,6...: ~40% fewer simulations on stage 4.
- The ghost player nodes (particles/animations) stay **paused**: zero work while idle.
- Settings are cached in global state; the floating button no longer has a per-frame `update`
  (only while it flashes) and its text always fits inside the rectangle.
- With *Log saves* enabled, the Geode log shows how long each buffered click took
  (`us` and `us/tick`) so the budget can be tuned with real data.

## What changed

- F6 key -> touch button (F6 still works on Windows).
- Windows console -> Geode log (*Log saves* option).
- The ghosts live in an invisible container (so their trails/particles are not seen).
- The state is reset when the level restarts (`resetLevel`).

## Important / limitations

- The player -> ghost state copy uses `copyAttributes` (when available) plus known fields;
  it can differ slightly from the original.
- On slow phones the **16/8** stage runs more simulations and can cause a small hitch:
  use 2/2 or 4/4.
- If the original source ever shows up, it can be adapted 1:1 instead of this reconstruction.
