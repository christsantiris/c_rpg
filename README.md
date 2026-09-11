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

<img width="2544" height="1488" alt="image" src="https://github.com/user-attachments/assets/b1f1f35b-c86f-4dfe-beab-08ea6c290305" />

<img width="2542" height="1480" alt="image" src="https://github.com/user-attachments/assets/b1036cd3-446b-4016-b1e1-25a832b7c0bf" />

<img width="2546" height="1484" alt="image" src="https://github.com/user-attachments/assets/a3dbb1ff-28cc-4bcc-924c-6821ce30df64" />

<img width="3664" height="2132" alt="image" src="https://github.com/user-attachments/assets/c011e7e0-4218-4d49-9253-2752eb42decd" />

<img width="2532" height="1474" alt="image" src="https://github.com/user-attachments/assets/43803900-2a68-424c-ab6c-b51b21612d2a" />

<img width="2544" height="1484" alt="image" src="https://github.com/user-attachments/assets/c6063044-6df7-4d6c-8541-14965a03ef24" />

<img width="2552" height="1476" alt="image" src="https://github.com/user-attachments/assets/d1982af0-2ec9-433d-85f1-9a0b26bddfb0" />

<img width="2548" height="1484" alt="image" src="https://github.com/user-attachments/assets/ac612c80-defb-4289-afd8-fb248fc07edc" />

<img width="2542" height="1484" alt="image" src="https://github.com/user-attachments/assets/b9e35f85-42e7-4ee6-8cf5-c1fed16368d3" />

<img width="2556" height="1480" alt="image" src="https://github.com/user-attachments/assets/fd151a8c-7543-4e26-9649-eb27db71ebb2" />

<img width="2542" height="1486" alt="image" src="https://github.com/user-attachments/assets/088a7000-a073-4f31-90e4-e86d6ac98247" />

<img width="2546" height="1484" alt="image" src="https://github.com/user-attachments/assets/dbe7ac91-532b-408d-9de1-77c83ec59beb" />

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
