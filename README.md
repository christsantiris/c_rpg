# The Castle of No Return

A roguelike adventure game inspired by *Castle of the Winds* by SaadaSoft. Explore dungeons, battle enemies, and survive the challenge!

**GitHub:** [https://github.com/christsantiris/c_rpg](https://github.com/christsantiris/c_rpg)

## Table of Contents
- [Overview](#overview)
- [Screenshots](#screenshots)
- [Building and Running](#building-and-running)
  - [Compile the Game](#compile-the-game)
  - [Run the Game](#run-the-game)
  - [Clean the Build](#clean-the-build)
- [Dependencies](#dependencies)
- [Contributing](#contributing)
- [License](#license)
- [Installers](#installers)
- [Roadmap](#roadmap)

## Overview
*The Castle of No Return* is a C/SDL roguelike with turn-based combat and a
retro pixel-art style. The central town connects to an eight-floor undead
dungeon, an eight-stage Haunted Forest, the eight-stage Goblin Mountains, and
the eight-stage Sunken Coast. Each adventure keeps its own generated maps and
progression.

### Goblin Mountains terrain

Stages 2 and 7 funnel combat onto narrow bridges. A marked weak span collapses
behind you after crossing; press `A` beside the gap to rebuild it from either
side. On stages 3–6 and 8, goblin gates divide fortified rooms. Press `A` beside
a gate to open it, then beware the hidden trap and the archer and guard beyond it.

Amber rockfalls mark buried cave passages around the defended gates. Press `A`
beside either end to expose the passage: falling rocks cost `4 + stage` HP, but
the cave holds a one-time cache worth `40 + 10 × stage` gold. Stand on the cache
and press `A` to collect it. Newly revealed passages appear on the minimap;
opened gates, exposed caves, claimed caches, and repaired bridges survive
cached revisits and saving/loading. Existing saved maps retain their layouts;
the new fortifications appear on newly generated stages. Arrow keys always move;
`A` interacts when beside a mountain obstacle or standing on its cache.

### Sunken Coast water routes

Each new Coast stage has two linked controls. Stand on either control and press
`A` to transfer water between the blue and amber channels. Every activation
reverses the flow: one basin drains while the other floods. Colored corner
markers identify the channels; drained channels show exposed stone.

The sluice room has alternate crossings, and two guarded treasure chambers are
accessible at opposite tide levels. Each chamber cache grants `50 + 10 × stage`
gold once; stand on it and press `P` to collect it. Drain the blue basin to reach
and light Mara's beacons. Both controls and the exit remain reachable in either
tide state, with the Drowned Queen still guarding the final exit.

Operating a control reveals its connected channels on the minimap. Rising water
carries enemies to an open bank and submerges dropped items until the channel
is drained again. Water states, claimed caches, and discoveries persist through
cached revisits and save/load. Existing saved Coast maps keep their layouts;
the linked channels and treasure chambers appear on newly generated stages.

## Screenshots
Below are screenshots of the game in action:

<img width="3806" height="2270" alt="image" src="https://github.com/user-attachments/assets/3202fc6c-1c54-4f2f-9f06-1ca426340715" />

## Compile the game
To compile the app run `make run` in the root directory.

### Debug interface

The `debug` target builds the game with debug support and accepts optional
Make variables for configuring the next new character:

```bash
make debug WEAPON=bow GOLD=500 SCROLLS=magic-arrow,fireball,heal
```

| Make variable | Executable option | Accepted values | Behavior when omitted |
| --- | --- | --- | --- |
| `WEAPON` | `--weapon NAME` | `rusty-sword`, `short-sword`, `long-sword`, `battle-axe`, `staff`, `bow`, `none` | Keep the selected class's normal starting weapon |
| `GOLD` | `--gold N` | Any whole number from `0` through `999999` | Keep the normal starting gold |
| `SCROLLS` | `--scrolls LIST` | Up to three comma-separated values chosen from `magic-arrow`, `fireball`, and `heal`, or `none` | Keep the selected class's normal starting scrolls |

Providing `WEAPON` or `SCROLLS` replaces the normal class starting items in
that category. Use `WEAPON=none` or `SCROLLS=none` to begin without that item
category. Scroll names must be comma-separated without spaces. These settings
are applied after class selection when a new game is created; they do not
modify a loaded save.

Examples:

```bash
# Override every supported starting value.
make debug WEAPON=bow GOLD=500 SCROLLS=magic-arrow,fireball,heal

# Test an empty weapon and scroll loadout while retaining normal starting gold.
make debug WEAPON=none SCROLLS=none

# Override only gold and retain the selected class's normal equipment.
make debug GOLD=10000
```

After building a debug executable, the corresponding command-line options can
also be passed directly:

```bash
./build/conr --weapon bow --gold 500 --scrolls magic-arrow,heal
```

Unknown options, unsupported item names, lists longer than three scrolls, and
gold values outside the accepted range cause the program to print usage
information and exit before starting the game.

## Clean the build
Run `make clean` to destroy the compiled game and start over if you make your own changes

## Clean the tests
Run `make test` to run unit tests

## Dependencies
cmake sdl2 sdl2_ttf sdl2_mixer pkg-config (if linux)

## Contributing
I would gladly welcome any pull requests to help improve the game

## License
MIT

## Installers
For MacOS run `chmod +x package/macos/build_dmg.sh` then `make dmg`
and drag the file into applications and double click!

For Linux run `chmod +x package/linux/build_linux.sh` then `make linux`.
Extract `dist/CastleOfNoReturn-linux-x86_64.tar.gz`, then either:
- Run directly: `cd linux && ./run.sh`
- Install as a desktop app: `./install.sh` — adds the game to your app launcher with a double-clickable icon

## Roadmap
Production Releases
- Signed apple dmg
- windows installer
- fix linux installer
Highest priority
1. Difficulty scaling across area order
   Players can complete the four regions in any order, so later regions must remain challenging. Scale primarily from bosses defeated and completed regions, with player level as a smaller modifier. Avoid directly matching every enemy to the player because that makes leveling feel pointless.
E4. nemy silhouettes. Start with the mountain goblin family, where the shared body makes archer, bomber, guard, and shaman easiest to confuse. 
4. Sparse environmental props. The floors already include deterministic small details; add only larger accents where rooms still feel empty, while keeping combat lanes clear.
5. Ambient animation. Projectiles, impacts, and area entry already animate. Water movement, beacon or torch flicker, and enemy idle motion remain the next small-animation pass. 
High priority
1. More tactical enemy behavior
   Add recognizable combat roles such as protectors, ambushers, healers, summoners, ranged attackers, and enemies that retreat or flank.
2. Boss encounter improvements
   Give every boss multiple phases, telegraphed signature attacks, an arena mechanic, and a guaranteed thematic reward.
3. Economy and loot balancing
   Review gold income, shop prices, sell values, drop rates, and upgrade pacing now that high-tier weapons exist. Rare equipment should feel attainable without making shops irrelevant.
Medium priority
1. Region replay tiers
   After defeating a regional boss, unlock a harder expedition version with stronger enemies, altered layouts, improved rewards, and possibly a remixed boss. This is better as an endgame layer after normal-order difficulty scaling works.
2. Loot affixes
   Introduce carefully limited modifiers such as Vampiric, Swift, Reinforced, Mana-Bound, or Venomous. Keep named magical weapons stronger and more distinctive than generic affixed drops.
Polish priority
1. Combat feedback
   Add clearer hit effects, critical indicators, blocked-damage feedback, status icons, and distinct weapon sounds.
2. Accessibility and controls
   Add remappable controls, text scaling, colorblind-safe indicators, adjustable message duration, and clearer interaction prompts.
3. Documentation refresh
   docs/weapons.md and parts of the README still describe the older weapon catalogue and should be updated.
