# <c-5DEBFF>G</c><c-68DEFF>l</c><c-75D0FF>o</c><c-84C2FF>b</c><c-95B3FF>a</c><c-A8A3FF>l</c> <c-B994FF>D</c><c-CA84F7>e</c><c-DB75EE>m</c><c-E967DF>o</c><c-F45FCB>n</c><c-FB61B3>l</c><c-FF6997>i</c><c-FF777C>s</c><c-FF8964>t</c> <c-FFA34F>R</c><c-FFBD49>a</c><c-FFD44C>n</c><c-FFEA5A>k</c> <cg>v1.2.1</c>

### <c-66E6FF>See a level's current Global Demonlist placement directly in Geometry Dash.</c>

---

## <c-5DEBFF>WHAT IT DOES</c>

When you open a classic level, the mod looks up its Geometry Dash ID using the public [Global Demonlist API](https://demonlist.org/api-docs).

- <c-FFD65A>**Listed level:**</c> displays a trophy and a placement number using Geometry Dash's built-in gold font below the difficulty and star information.
- <c-74E8FF>**Unlisted level:**</c> leaves the level page unchanged.
- <c-8DDCFF>**Loading:**</c> displays a spinner while the placement request is in progress.
- <c-C59BFF>**Connection unavailable:**</c> briefly displays a red `X` that smoothly fades away.

## <c-8DDCFF>FAST AND UNOBTRUSIVE</c>

Requests run <cg>asynchronously</c>, so the game does not wait for the network. Placements are cached until Geometry Dash restarts to avoid requesting the same level repeatedly.

The label is positioned <c-FFD65A>below the star row</c>, so it does not cover the star count or icon.

Higher placements receive more prestigious built-in Geometry Dash rank trophies, with `#1` using the best trophy tier.

When <cp>IngameListMod</c> is installed, the Global Demonlist row automatically moves below its AREDL placement to prevent overlap.
This also works when reopening a level after IngameListMod has cached its placement. The compact rows use only their trophy and placement number, without extra source tags.

## <c-8DDCFF>MOD API</c>

Other Geode mods can include `GlobalDemonlistRankAPI.hpp` to read cached placement states and listen for live updates.

The API reports <cy>loading</c>, <cg>listed</c>, <c-FFD65A>unlisted</c>, and <cr>error</c> states. Listed results include the Global Demonlist placement number.

## <c-D79BFF>SETTINGS</c>

Use the <cy>Show Global Rank</c> setting to enable or disable the placement label.

---

<c-AEDBFF>Placement data:</c> [Global Demonlist](https://demonlist.org/)

<c-C9B3FF>API documentation:</c> [demonlist.org/api-docs](https://demonlist.org/api-docs)

<c-FFD65A>Developer:</c> **BANANCHIKIREAL**
