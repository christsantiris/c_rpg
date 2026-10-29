# The Castle of No Return

A roguelike adventure game inspired by *Castle of the Winds* by SaadaSoft. Explore dungeons, battle enemies, and survive the challenge!

**GitHub:** [https://github.com/christsantiris/c_rpg](https://github.com/christsantiris/c_rpg)

## Table of Contents

- [Overview](#overview)
- [World Map](#world-map)
- [Controls](#controls)
- [Game Documentation](#game-documentation)
- [Screenshots](#screenshots)
- [Building and Running](#building-and-running)
- [Dependencies](#dependencies)
- [Contributing](#contributing)
- [License](#license)
- [Installers](#installers)
- [Credits](#credits)
- [Roadmap](#roadmap)

## Overview

*The Castle of No Return* is a C/SDL roguelike adventure with turn-based combat and a
retro pixel-art style. Choose a Warrior, Mage, or Rogue, prepare in town, and
explore a five-floor undead dungeon, the five-level haunted forest, a five-level goblin mountain climb
and many other adventures.

## World Map

[![World map of the Castle of No Return](docs/images/world-map-giants-peaks.png)](docs/images/world-map-giants-peaks.png)

An illustrated overview based on the world's town connections.

## Controls

| Key | Action |
| --- | --- |
| `WASD` or arrow keys | Move; `A` interacts when at a contextual object |
| `.` / `,` | Take the onward route / return by stairs or region entrance |
| `P` | Pick up an item or cache on the current tile |
| `T` | Talk to a nearby NPC or rescue a forest warden |
| `I` | Open or close inventory |
| `B` | Open spellbook |
| `Q` | Open the quest and boss journal |
| `C` | Cast the equipped spell |
| `F` | Use an equipped bow or the Demonic Sword's ranged attack |
| `H` | Open help |
| `Esc` | Close the current view or return to the main menu |

Inside the inventory, use the arrow keys to select an item, `U` to use it,
`E` or `Enter` to equip it, `O` to equip a one-handed weapon in the off hand,
`X` to unequip the selected item, and `D` to drop it. Unequipping the main-hand
weapon promotes an off-hand weapon to the main hand; the removed weapon stays
in inventory and can then be equipped with `O` to swap hands. Inventory action
labels also support mouse clicks. In the spellbook, select a spell and press
`E` or `Enter` to equip it. Shops and the docks use the arrow keys and
`Enter`; shop buy and sell modes are switched with `Tab`.

Using an item keeps the inventory open, even after consuming the last item.
Close it manually with **I** (or the remapped inventory key) or **Esc**.
Drinking any potion still consumes one combat turn: enemies respond, and
poison and freezing advance with that turn.
Attempting to drink at full HP or MP keeps the potion and costs no turn;
reading scrolls and equipping, unequipping or dropping items remain free
inventory actions.

## Game Documentation

- [Regional difficulty and progression](docs/difficulty-scaling.md)
- [Weapons, armor, and shields](docs/weapons.md)
- [Spells](docs/spells.md) and [Mage progression](docs/mage-progression.md)
- [Tavern quests](docs/tavern-quests.md) and [quest journal](docs/quest-journal.md)
- [Castle Dungeon](docs/dungeon.md)
- [Haunted Forest](docs/haunted-forest.md)
- [Goblin Mountains](docs/goblin-mountains.md)
- [Ironfang Mines](docs/ironfang-mines.md)
- [Dragonspine and High Pass](docs/dragonspine.md)
- [Sunken Coast](docs/sunken-coast.md)
- [Saltwind Marsh — five stages and skiff exploration](docs/saltwind-marsh.md)
- [Stormwatch Headland](docs/stormwatch-headland.md)
- [Blackwater Swamp](docs/swamp.md)
- [Sunscar Wastes and the magic lamp quest](docs/sunscar-wastes.md)
- [Ashen Hollow](docs/ashen-hollow.md)
- [Giant's Peaks](docs/giants-peaks.md)
- [Stonewake Quarry](docs/stonewake-quarry.md)
- [Complete enemy names and area mapping](docs/enemy-roster.md)
- [Elderwood Forest and the northern road](docs/elderwood-forest.md)
- [Royal Catacombs](docs/catacombs.md)
- [Castle interior and play testing](docs/castle-design.md)
- [Royal Dungeon, escape, and equipment recovery](docs/royal-dungeon.md)
- [Docks, Ruined Isle, and Ruined Temple](docs/harbor-and-ruined-isle.md)

## Screenshots

Below are screenshots of the game in action:

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/a3408d27-396f-4a2e-83ab-79e3fbe19e35" />

<img width="2588" height="1466" alt="image" src="https://github.com/user-attachments/assets/63928e08-9684-4fe2-acd5-ffbcaf092608" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/121f3fe6-1684-4589-b0aa-a6f3e8b18f7d" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/807a7b67-b66c-4432-ad6a-cfdd0d2f0ece" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/ac746feb-21ba-4d4f-b7e7-87d04bb317fb" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/079c3db6-72fe-477f-bdb8-e903016a9aa1" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/bb5d4b04-c555-4b4c-82bc-1ae2e81196bc" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/a3cefd1d-b62c-48d1-b2ee-99099ff52d55" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/9c480504-6f05-490d-bfd8-255b8d964996" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/36cb3d56-c0cb-4345-a1bd-4b5e652d2ab4" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/d4388a92-993c-4a2b-afa8-f30a797c369f" />

<img width="2586" height="1466" alt="Screenshot 2026-10-21 at 10 37 28 AM" src="https://github.com/user-attachments/assets/e25ea54f-80a9-417c-8d79-a9d4db962ba5" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/2fd11475-3ef2-44c3-96c6-6ffa94500c16" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/354b1f25-4ba0-4318-8be7-9ac5f7a57a29" />


## Building and Running

Build a release executable with `make`, or build and start it with `make run`.
The executable is written to `build/conr`.

### Debug interface

To test with all passage locks open, run:

```bash
./tools/debug_shortcuts.sh
```

For a Long Sword and 1,100 gold:

```bash
./tools/debug_shortcuts.sh --gold 1100 --weapon long-sword
```

See the [parameter table](#debug-script-parameters) and
[weapon-to-parameter mapping](#debug-weapon-names) below for exact accepted values.

Select **New Game**, enter a name, and choose a class, or load an existing
character. All four town-to-town roads, the Sunken Coast dock road, island sailing
without a treasure map, and the labyrinth entrance are open immediately.
Locked dungeon doors, mountain barriers, temple doors, the labyrinth vault,
quarry crane walkways and castle portcullises open as you explore. Each unused
quarry crossing lowers once; its crane then respects manual raised, crossing and
salvage settings. Boss-blocked exits,
temple stairs and castle floor progression are accessible. The Royal Dungeon's secret
hatch stays hidden until you accept Tomas's escape quest, including in debug mode.
This adds a saved debug passage-unlock flag without marking bosses defeated.
Quests stay unassigned, existing quest progress remains intact,
and bosses remain available to fight.
The unlocks apply to new and loaded characters and persist if you save them.
Loading keeps the character's existing gold, equipment, inventory and quests.

The launcher builds a separate debug executable in `build/debug-shortcuts`
and runs it from the project directory. It accepts the existing debug options,
for example `./tools/debug_shortcuts.sh --gold 500 --weapon bow`.

#### Debug script parameters

| Script option | Accepted values | Example | Make equivalent |
| --- | --- | --- | --- |
| `--weapon NAME` | An exact name from the [weapon table](#debug-weapon-names), or `none` | `--weapon long-sword` | `WEAPON=long-sword` |
| `--gold N` | A whole number from `0` through `999999` | `--gold 1100` | `GOLD=1100` |
| `--scrolls LIST` | Up to three comma-separated entries: `magic-arrow`, `fireball`, `heal`; or `none` | `--scrolls magic-arrow,heal` | `SCROLLS=magic-arrow,heal` |
| `--unlock-shortcuts` | Flag; automatically enabled by this script | No need to pass it to the script | Executable option only |

Omitting weapon, gold or scroll options keeps the selected class's normal
starting values. The script always enables passage unlocks.

The `debug` Make target accepts the equivalent variables:

```bash
make debug WEAPON=bow GOLD=500 SCROLLS=magic-arrow,fireball,heal
```

Providing `WEAPON` or `SCROLLS` replaces the normal class starting items in
that category. Use `WEAPON=none` or `SCROLLS=none` to begin without that item
category. Scroll names must be comma-separated without spaces. These settings
are applied after class selection when a new game is created; they do not
modify a loaded save.

#### Debug weapon names

Every weapon, including boss rewards, is available for testing. Names are
case-sensitive; use the exact spelling and hyphens shown. For example,
**Long Sword** maps to `--weapon long-sword`. The same names work with `WEAPON`.
Normal class restrictions still apply; open inventory
with **I** and equip the weapon with **E** or **Enter**.

| Weapon | `--weapon` value |
| --- | --- |
| Rusty Sword | `rusty-sword` |
| Short Sword | `short-sword` |
| Long Sword | `long-sword` |
| Magic Long Sword | `magic-long-sword` |
| Demonic Sword | `demonic-sword` |
| Greatsword | `greatsword` |
| Magic Greatsword | `magic-greatsword` |
| Cryptblade | `cryptblade` |
| Battle Axe | `battle-axe` |
| Magic Battle Axe | `magic-battle-axe` |
| Magic Stone Warhammer | `stone-warhammer` |
| Embercleaver | `embercleaver` |
| Staff | `staff` |
| Runed Staff | `runed-staff` |
| Magic Staff | `magic-staff` |
| Sandstorm Staff | `sandstorm-staff` |
| Hunting Bow | `hunting-bow` or `bow` |
| Shortbow | `shortbow` |
| Longbow | `longbow` |
| Crossbow | `crossbow` |
| Magic Longbow | `magic-longbow` |
| Krakenbone Bow | `krakenbone-bow` |
| Rusty Dagger | `rusty-dagger` |
| Dagger | `dagger` |
| Magic Dagger | `magic-dagger` |
| No starting weapon | `none` |

Examples:

```bash
# Start NPC testing with all town shortcuts open.
./tools/debug_shortcuts.sh

# Combine unlocked shortcuts with a custom starting loadout.
./tools/debug_shortcuts.sh --gold 500 --weapon bow --scrolls magic-arrow,fireball,heal

# Start a Warrior test with unlocked shortcuts, 1,000 gold, and a Magic Greatsword.
./tools/debug_shortcuts.sh --gold 1000 --weapon magic-greatsword

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

Debug executables also accept `--unlock-shortcuts` for new and loaded characters.
Weapon, gold and scroll overrides still apply only to new characters. Loaded
characters receive only the shortcut unlocks, and their save files change only
when you save.

Unknown options, unsupported item names, lists longer than three scrolls, and
gold values outside the accepted range cause the program to print usage
information and exit before starting the game.

### Clean the build

Run `make clean` to remove the build directory.

### Run the tests

Run `make test` to build and run the unit test executable.

After building the shortcut debug executable, check new and loaded characters
through the application load handler with:

```sh
cmake --build build/debug-shortcuts --target test-debug-shortcuts
```

This check uses temporary saves and verifies that loaded characters can walk to
Dawnmere without changing their equipment, gold or quests.

## Dependencies

- CMake 3.20 or newer
- A C99 compiler
- SDL2
- SDL2_ttf
- SDL2_mixer
- pkg-config on Linux

## Contributing

Pull requests that improve the game are welcome.

## License

MIT

## Installers
For MacOS run `chmod +x package/macos/build_dmg.sh` then `make dmg`
and drag the file into applications and double click!

For Linux run `chmod +x package/linux/build_linux.sh` then `make linux`.
Extract `dist/CastleOfNoReturn-linux-x86_64.tar.gz`, then either:
- Run directly: `cd linux && ./run.sh`
- Install as a desktop app: `./install.sh` — adds the game to your app launcher with a double-clickable icon

## Credits
[Pixabay](https://pixabay.com/) for the sound effects

## Roadmap

### Production releases

- Signed Apple DMG
- Windows installer
- Fix Linux installer

### Feature enhancements
- Expand world map
   Farms between areas for rest to improve hp. New story lines. More coherent quest structure. Use larger scale sprites. 
- Recruit a party to join the adventure
   NPCs that can team up with the player character and accompany on the quest. Implement resurrection. 
- Improve difficulty scaling and economy
   Warrior and Rogue should struggle at certain levels without better weapons. Mage should struggle without better spells. Purchasing new weapons, armor and tomes should become necessary to progress.
- Add paypal/other integration to buy gold
   Add real money feature to buy in-game gold.
- DnD style stats instead of static class based stats
   Roll for stats. Choose a class based on strengths. Ascending armor class chance to hit. 
- Add new equipment types
   Add helmets, rings, gauntlets
- Revamp quests. Chain quests together. Unlock new quests with completion of another. NPCs can offer quest part 2. Max number of active quests - NPC can say come back when you have completed xyz task.

