# The Castle of No Return

A roguelike adventure game inspired by *Castle of the Winds* by SaadaSoft. Explore dungeons, battle enemies, and survive the challenge!

**GitHub:** [https://github.com/christsantiris/c_rpg](https://github.com/christsantiris/c_rpg)

## Table of Contents
- [Overview](#overview)
- [Screenshot](#screenshot)
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
dungeon, an eight-stage Haunted Forest, and the eight-stage Goblin Mountains.
Each adventure keeps its own generated maps and progression.

## Screenshot
Below are screenshots of the game in action:

<img width="2566" height="1420" alt="image" src="https://github.com/user-attachments/assets/4e30e7fe-69da-4e80-8fdc-885a43638691" />

<img width="2566" height="1420" alt="image" src="https://github.com/user-attachments/assets/a557eecb-4453-466d-ab67-c78083a3fbd1" />

<img width="2546" height="1420" alt="image" src="https://github.com/user-attachments/assets/fcd38796-6c77-42bc-949f-d0ab2c969ba4" />

<img width="2546" height="1420" alt="image" src="https://github.com/user-attachments/assets/6e8c3429-21e6-4831-b10a-aab72305ca6e" />


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
- Difficulty scaling
- Quests
- NPCs
- Signing the app with codesign and submit to Apple with xcrun notarytool
- windows installer
- fix linux installer icon not showing
