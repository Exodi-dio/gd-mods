# v1.2.1

- Fixed Global and IngameListMod AREDL ranks overlapping when reopening a level whose AREDL placement was already cached.
- Added compatibility with IngameListMod v1.2.6 cached labels, which do not receive its usual node ID.
- Removed the `GLOBAL` and `AREDL` text tags, leaving the trophy and placement number centered.

# v1.2.0

- Fixed the Global row shifting left by centering only the trophy and placement number.
- Added all visible level coin icons to the vertical layout calculation, preventing overlap when IngameListMod is not installed.
- Added a public Geode event API with cached lookup and live `Loading`, `Listed`, `Unlisted`, and `Error` states.
- Packaged `include/GlobalDemonlistRankAPI.hpp` for use by other mods.

# v1.1.6

- Fixed the v1.1.5 regression that placed the Global Demonlist and AREDL rows on top of each other.
- Reduced the compatibility lift so Global stays on a separate row without sitting as low as in v1.1.4.
- Reduced the `GLOBAL` and `AREDL` source-tag sizes.

# v1.1.5

- Moved the Global Demonlist row closer to the IngameListMod AREDL row so it no longer sits too low.
- Added compact `GLOBAL` and `AREDL` source tags beside their respective placement numbers.
- Kept the placement number on Geometry Dash's built-in gold font for texture-pack compatibility.

# v1.1.4

- Fixed compatibility with IngameListMod by detecting its actual AREDL placement label on the level info screen.
- Places the Global Demonlist trophy and number below the IngameListMod row, including after its asynchronous response arrives.
- Changed the placement number to Geometry Dash's built-in gold font and removed the forced purple tint for texture-pack compatibility.

# v1.1.3

- Added dynamic layout compatibility with Integrated Demonlist v1.7.13.
- Detects the Integrated Demonlist rank label even when its API response arrives later.
- Places the Global Demonlist trophy and number on a separate row below the AREDL placement.

# v1.1.2

- Made all four outer icon corners genuinely transparent.
- Increased the blue frame thickness while preserving the supplied demon artwork.
- Added a loading spinner while the Global Demonlist request is in progress.
- Added a fading red cross when the request fails because the service or network is unavailable.
- Replaced the plain `Global #...` text with a built-in Geometry Dash trophy and a large purple placement number.
- Added automatic trophy tiers for #1, top 10, top 50, top 100, and progressively lower placement ranges.

# v1.1.1

- Replaced the in-Geode Russian description with English text supported by the default Geode font.
- Increased the icon frame radius and stroke width so the rounded blue border remains visible at Geode thumbnail size.

# v1.1.0

- Fixed the Global Demonlist label overlapping the level's star count and star icon.
- Positioned the label below the complete difficulty and star information block.
- Reduced the label size for better readability and compatibility with texture packs.
- Added a rounded blue edge frame to the user-supplied mod icon without altering its central artwork.
- Added a detailed in-Geode description with a cyan, purple, and gold gradient.

# v1.0.0

- Added Global Demonlist placement below the level difficulty icon.
- Added asynchronous lookups by Geometry Dash level ID through the public Global Demonlist API.
- Added session caching for successful and confirmed unlisted lookups.
- Added graceful handling for unavailable, malformed, and not-found API responses.
- Added an option to enable or disable the placement label.
