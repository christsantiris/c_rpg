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

## Screenshots
Below are screenshots of the game in action:

<img width="2542" height="1474" alt="image" src="https://github.com/user-attachments/assets/e0f3e8e4-159b-48c1-a1f7-e4f0c0903baa" />

<img width="2542" height="1474" alt="image" src="https://github.com/user-attachments/assets/f71af623-4d6b-4efc-bc8c-3189dc3e8dde" />

<img width="2546" height="1420" alt="image" src="https://github.com/user-attachments/assets/fcd38796-6c77-42bc-949f-d0ab2c969ba4" />

<img width="2546" height="1420" alt="image" src="https://github.com/user-attachments/assets/6e8c3429-21e6-4831-b10a-aab72305ca6e" />

<img width="2546" height="1420" alt="image" src="https://github.com/user-attachments/assets/4892f5c5-f7c1-4863-bd5f-3056ee5b08be" />

<img width="2546" height="1420" alt="image" src="https://github.com/user-attachments/assets/f3e57182-cfa5-4780-92d8-6bfb725e9e19" />

<img width="2546" height="1420" alt="image" src="https://github.com/user-attachments/assets/6de113c3-ad31-474b-9531-4cf32c1e9841" />

<img width="2542" height="1484" alt="image" src="https://github.com/user-attachments/assets/ea6c2dd5-4412-42fe-b2fd-3642430b704e" />

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
High priority
2. Stronger regional mechanics
   Expand each region’s identity beyond its terrain:
   - Dungeon: keys, traps, locked crypts, switches
   - Forest: hidden trails, landmarks, misleading paths
   - Mountains: bridges, collapses, caves, fortifications
   - Coast: tides, flooded chambers, movable water routes
3. More tactical enemy behavior
   Add recognizable combat roles such as protectors, ambushers, healers, summoners, ranged attackers, and enemies that retreat or flank.
4. Boss encounter improvements
   Give every boss multiple phases, telegraphed signature attacks, an arena mechanic, and a guaranteed thematic reward.
5. Economy and loot balancing
   Review gold income, shop prices, sell values, drop rates, and upgrade pacing now that high-tier weapons exist. Rare equipment should feel attainable without making shops irrelevant.
Medium priority
6. Region replay tiers
   After defeating a regional boss, unlock a harder expedition version with stronger enemies, altered layouts, improved rewards, and possibly a remixed boss. This is better as an endgame layer after normal-order difficulty scaling works.
7. More quest varieties
   Move beyond three-stage interaction quests:
- Rescue and escort
- Hunt a named enemy
- Recover a carried item
- Defend a location
- Solve a switch or environmental puzzle
- Choose between competing outcomes
8. Town progression
   Let the town change as bosses and quests are completed. Shops can visually improve, NPC dialogue can react, and new services can open.
9. Watchtower functionality
   The watchtower could provide regional scouting, reveal unexplored entrances, display boss status, preview danger level, or launch higher-tier expeditions.
10. Loot affixes
   Introduce carefully limited modifiers such as Vampiric, Swift, Reinforced, Mana-Bound, or Venomous. Keep named magical weapons stronger and more distinctive than generic affixed drops.
Polish priority
11. Combat feedback
   Add clearer hit effects, critical indicators, blocked-damage feedback, status icons, and distinct weapon sounds.
12. Accessibility and controls
   Add remappable controls, text scaling, colorblind-safe indicators, adjustable message duration, and clearer interaction prompts.
13. Documentation refresh
   docs/weapons.md and parts of the README still describe the older weapon catalogue and should be updated.
