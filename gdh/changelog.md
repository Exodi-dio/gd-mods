# v5.1.0-beta.6
- Fixed Noclip settings (tint on death not working)
- Fixed Macro Buff (crash)
- Fixed Editor Extension
- TPS Optimization (Android ARM64)
- Fixed icon visiblity (again)
- Improved Cheat Indicator
- Improved Frame Stepper UI (hold support)
- Improved visibility of search results
- Added Search (Mobile)
- Ability to adjust opacity of cheat indicator (Labels)
- Fixed size change of cheat indicator (Labels/Mobile)
- Fixed Show Trajctory (breakable blocks and 1 frame delay)
- Fixed inability to enter tags (Labels/Mobile)
- Added Show Touches
- Added Auto Disable Shake
- Added Auto LDM
- Improved theme editor (ability to change the glow color and shadow of the knobs)

# v5.1.0-beta.5
- Fixed Cheat Indicator (false red color)
- Fixed "Solid Wave Trail"
- Fixed "Black Screen" recorder (Mobile)
- Fixed Startpos Switcher UI
- RingBuffer optimized
- Added "Show Trajectory"
- Added "Continue" mode for Replay Engine
- Added "Show Death Object" (Show Hitboxes)
- Added "Show Player" (Show Hitboxes)
- Added Hitbox Trail Color change
- Added "Update Notification" Popup
- Added Icon Button Settings (Mobile)
- Added Hide icon on game (Mobile)
- Added Hide icon on editor (Mobile)
- Added "Show Cheat Hack List"
- Added Min/Max Opacity for Startpos Switcher
- Improved Macro Buff (removing spikes that interacts with player)
- Added "Legacy Render" for low-end devices (PC)
- Added "No Dash Fire"
- "Hide Pause Menu" now more friendly with other Mod Menus
- Tabs now scrollable (insetad of arrows, mobile)
- Added "Restart Game" shortcut (PC)
- Added Import/Delete button for Replay Engine (Mobile)
- Added CPS for Second Player (Label)
- Added Player Flip in "Player Information" (Label)
- Added Best Progesss/Time (Label)
- Added "All Modes Platofrmer"
- Added "Force Platformer"
- Added "Ship Copter"
- Added more Replay Engine Keybinds
- Other bugfixes

big thanks to community for finding and repoting bugs!

# v5.1.0-beta.4 - Hotfixes
- Fixed Cheat Indicator (Mobile)
- Fixed Autosave (Mobile)
- Optimized Labels
- Added Macro Buff (Experimental)
- Recorder Fixes

# v5.1.0-beta.3
- Added Cheat Indicator
- Fixed Keybinds
- Sync TPS with Lock Delta on replay load
- Fixed Frame Backward Stepper
- Added Frame Multi-Step (PC Only)
- Frame Stepper UI toggle without level reload
- Fixed Audio Recorder on Startposes
- Framerate values now demical and unlocked (hold alt or double-click to edit)
- Added Pause Buffering Bypass
- Added Playtest Zoom Bypass
- Fixed Hitbox Multiplier
- Fixed Show Hitboxes (changes physics in editor)
- Added symbols for better debugging (Windows)
- Fixed Practice Fix (i hope)
- Fixed RE Playback not registering clicks (CPS)
- Confirmation dialog for the default video save path (Android)

# v5.1.0-beta.2
Major Changes:
- Added Frame Stepper (Backward Support)
- Added Recorder (onepass, PC and Mobile Support)
- TPS Optimization (PC Only)
- Fixed UI Scaling on big monitors (up to 4K)
- Improved Keybind System (More functions for binding)
- Label Improvements and Optimization (geode::Label, runs faster)
- Added Variables
- Fixed Ctrl+C not working
- REv3 to REv4 Converter: https://tobyadd.pages.dev/GDH/re3_to_re4

Hacks Changes:
- Added No Exit Dual Effect
- Optimized Hitbox Trail (Ringbuffer)
- Fixed label reading when the file is corrupted
- Fixed Rainbow Color
- Added Noclip 2P, Noclip Limit
- Added Hitbox Multiplier
- Added CBF/COS/CBS Toggles
- Fixed Practice Fix
- Added Dual Clicks
- Added Straight Fly Bot
- Added more Label Variables
- Fixed Menu Gameplay not working on mobile

Replay Engine Changes:
- Fixed "Allow Buttons" limitation
- Added Macro Editor (PC and Mobile)
- Added Accuracy Fix and Velocity Fix
- Added Ignore Inputs on Playback

I might've added some other stuff too, but there are definitely tons of changes!  

Special thanks to Prevter for helping stuff

I created a Patreon so you can support my work: https://www.patreon.com/cw/TobyAdd/membership  
It'll really help motivate me to keep developing new versions of GDH! I will be grateful to every single supporter :D

# v5.1.0-beta.1
This probably isn't what you were expecting, since a lot of features might be missing, but Introducing the new version of GDH v5.1 Beta!

What's new:
- Complete rewrite from scratch - which means the code runs significantly faster and more stable
- Some downgrades compared to the last v5 released for 2.2074, unfortunately - since this is an beta, many features may be absent (they'll be added over time!)
- Redesigned UI - both the desktop and Android interfaces have been rewritten in Material Design style
- Dynamic hooking - this should improve performance since disabled hacks won't perform unnecessary checks, making the entire structure way more optimized than v5
- Frame Extrapolation - probably the first, well actually the second, free implementation that visually smooths out the gameplay experience if you have a monitor above 240Hz
- Theme Editor - you can now customize almost every UI component to match your color preferences
- Keybind system - it now supports modifiers, adding way more key combination variations (unfortunately not everything is bindable yet, but this is still an beta)
- Replay Engine v4 - a new, way more stable replay bot system that's much more accurate and finally supports a stable Practice Fix (unfortunately no backward compatibility with v3 macros yet, but that will be added later)
- Uncomplete Level - it existed before, but it's been improved and now properly cleans level stats (including orbs, stars, and full statistics)
- Reverb Effect - creates a spatial room effect for music to give it a pleasant acoustic feel
- Pitch Shifter - the limit has been increased from -24 semitones to 24 semitones, which is twice as much as before (actually, the software limit has been raised to -72 to 72, but shhh)

Honestly, there's a lot more waiting for you in future beta releases - again, not everything is done yet, but the core features are there and working stable (despite this being an beta)

Thanks to aciddev, prevter, dejid, "malenkya nadeshda" and everyone else who helped with implementations and ideas. And again, I want to apologize if your expectations weren't met with what's done right now - everything will be polished over time

# v5.0.0-beta.8
UI Changes:
- Replay Engine window title is now coloured based on the bot's stauts
- Allow scrolling outside window frame in the Android UI (for convenience in "Labels" tab)

Hacks changes:
- Added "Auto Safe Mode"
- Practice Fix is removed due to crashes (needs investigating)
- Fixed Text Length
- Improved Main Levels Bypass

Labels changes:
- Added CBF Status
- Added Rainbow label
- Fixed CPS Counter coloring
- More color tags
- Improved Startpos Switcher label (will be hidden if there are no startposes)

Replay Engine changes:
- Fixed 1 physic frame pre-recording 
- Fixed erasing for second player
- Fixed inputs auto-releasing

Replay Engine Note:
- Engine v3 may be discontinued, as it uses outdated and non-optimized algorithms from the 2.1 era. It may be replaced by Engine Lite, which will be more modernized and potentially standalone (though that's still under consideration)

# v5.0.0-beta.7
- Layout Mode fix (breaks decor levels in the editor)
- Improved vertical flip algorithm for android recorder

# v5.0.0-beta.6
- Macro Editor
- Disabled Accuracy Fix for 2P Player due to problems and bugs
- Layout Mode Default Colors of Level
- Replay Engine Recorder: Native Mode (any resolution for overlay mode, 100% quality shader trigger recording)
- Overlay Recording fixes

# v5.0.0-beta.5
- Fixed android64, thanks to whoever broke the bindings

# v5.0.0-beta.4
UI Changes:
- Seach box will be cleared when the menu is closed
- Mobile button now using layouts (Node ID dependency)

Hacks changes:
- Added Process priority
- Added Resume Timer (fixes lag when respawning while TPS Bypass is enabled, adjustable in the Framerate window)
- Fast Complete (Faster animation)
- Fixed Fast Chest open
- Added Auto Deafen
- Added Cheat Indicator
- Fixed "Main Levels Bypass" hack for "The Tower" levels
- Fixed Startpos Swither when you in Practice Mode

Labels changes:
- Added text coloring (CPS Counter, Death Counter will now be colored)
- Added Replay Engine State
- Added FPS Counter
- Added Cheat Indicator
- Added Testmode label
- Fixed Noclip Accuracy deaths
- Fixed Session Time
- "Progress" label have now a more customizable floating point

**Note:** labels may have an old formatting mapping, so they should be re-added

Replay Engine/Recorder changes:
- Fixed Accuracy Fix for 2P
- Audio Sync Recording now works perfectly with Music/SFX triggers
- Overlay Mode for Recorder (capturing overlays like reshade)
- Added option to change the volume percentage when recording audio for the showcase
- Practice Fix is back and reworked
- Audio now has better quality when merged with "Merge" tab

# v5.0.0-beta.3
- armeabi-v7a support (no recorder yet)
- startpos switcher keybinds fix (thanks ery)
- "startpos switcher sort objects x" feature
- removed "Hitbox Fill Color" feature due to problem with hitbox trail
- improved keybinds

# v5.0.0-beta.2
A new major version of GDH!

Global changes:
- Complete rewrite from scratch, redesigned almost all hacks to the hook system, allowing them to be used on both Android and Windows
- Android Support
- Replay Engine v3, P1/P2 macro load/merge support (Beta, merging only for PC)
- A lot of new hacks and features
- Variables (Beta, only for PC)
- Improved Labels (acid thanks)
- Shortcuts (Beta, only for PC)
- Recorder for Android (may be unstable, please report crashes/problems!)

GUI changes:
- More customizable UI size selection
- New keybind system
- Mini theme selection

Hacks changes:
- Fixed Speedhack Audio (works perfectly on SFX/Music triggers)
- Smart Startpos
- Layout Mode
- Pitch Shifter
- Pulse Size

# v4.9.0-alpha.1
- Ported to 2.2074

# 4.8.0
- Redesigned labels (thanks aciddev)
- Improved Recorder that ensure less crashes
- Added “Hide Complete Menu” for the recorder 

# 4.7.8
- fixed conflict between two engines
- replay engine v2 macro playback in editor

# 4.7.7
- fixed recorder aspect ratio mismatch
- lock aspect ratio for recording resoluion input (16:9)

# 4.7.6
- fixed replay system (again)

# 4.7.5
- Fixed input frame conflict that could have broken the macro
- Unlocked showcase video recording from 60 to 240 fps (Engine v2)
- Synchronize Audio on Video Recording (Experimental feature, currently breaking on song triggers)
- Improved Practice Fix
- Fixed recorder presets

# 4.7.4
- New Replay System (Engine v2)
- Improved the accuracy of both engines
- Practice Fix
- Recorder presets save/load
- Fixed pixelization shader on video recording
- Ability to disable popup messages (in Replay Engine settings)
- Hide Attempts

# 4.7.3
- Removed the annoying window if there are Russian characters in the path (now prevents recording a video if the path has Russian characters)
- Save the showcase path in byte format to avoid crashes with Russian characters
- Fixed Recorder when recording level with startpos
- More presets

# 4.7.2
- Fixed Russian characters conflict

# 4.7.1
- Fixed FFmpeg stuff

# v4.7.0
- Recorder
- Сhanged to a more safe method of obtaining the list of macros
- Auto-enter macros name based on level name
- Show Hitboxes on Death

# v4.6.8
- More friendly mod description
- Spambot
- Straight Fly Bot
- Popup windows now appear at the bottom right to avoid conflicts with adjacent windows
- TPS bypass has been moved to the "Framerate" tab (+ fixed problems with bypassing physics when it is off)

# v4.6.7
- More friendly mod description
- Spambot
- Straight Fly Bot
- Popup windows now appear at the bottom right to avoid conflicts with adjacent windows
- TPS bypass has been moved to the "Framerate" tab (+ fixed problems with bypassing physics when it is off)

# v4.6.6
- missed

# v4.6.5
- Fixed keybinds
- Popup message system (for showing if frame advance enabled cuz people say that the level is freezing, and it turns out that they accidentally turned it on through the key)
- Some new hacks

# v4.6.4
- Added more keybinds for hacks (Speedhack, Startpos Switcher, Frame Advance, Replay Engine Playback)
- Random Seed
- Wave Trail Size
- Tint on Death
- Anticrash if save data is broken
- Fixed hitbox trail in editor

# v4.6.3
- Show Hitboxes

# v4.6.2
- Fixed some hints
- Added keybinds

# v4.6.1
- Added Respawn Time
- Added "Open Folder" button when selecting replay

# v4.6
- Geode Support
- Added Auto Pickup Coins
- Fixed Allow Low Volume
