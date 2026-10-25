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

[![Illustrated world map showing Stonewake Quarry south of Stillbury and a compact Saltwind Marsh inlet south of Dawnmere, coastal docks beside Sunken Coast, the road through Goblin Mountains, Elderwood Forest and the northern road from Oakhaven through the Royal Catacombs to the castle, the five towns, both Crown Roads, regional adventures and the Ruined Isle.](docs/images/world-map-coastal-regions.png)](docs/images/world-map-coastal-regions.png)

An illustrated overview based on the world's town connections. Dawnmere now lies
east of Oakhaven in open coastal lowland, reached by the safe road opened after
defeating the Lich King. Ridgeshire is the elevated mountain city to its north.
The [expansion plan](docs/five-town-expansion.md) documents the connections
and the new [Elderwood Forest](docs/elderwood-forest.md) north of Oakhaven. Its
five levels lead to Elderthorn; defeating it opens the northern road to the
relocated Royal Catacombs. Defeating the Grave Marshal opens the remaining
passage to Castle Grounds. Goblin Mountains
now lies north of Dawnmere, beside the road to Ridgeshire; defeating its King
opens that road. Both mountain entrances start at level 1. Forest and swamp
also have five forward levels, with their boss victories opening the town roads.
Other enemy-area gates lead to safe cobblestone paths and themed entrance
sprites; physical retreat returns to the same path. Rook's Labyrinth stands
beside the Haunted Forest entrance on that road and stays sealed until Rook
assigns the quest in Oakhaven's Tavern. Castle, secret tunnel and Crown Road
travel keep their existing layouts.

Pirate Coast and its merchant treasure quest have been removed.
[Saltwind Marsh](docs/saltwind-marsh.md) is a five-stage adventure south of
Dawnmere, with skiff exploration, lizardfolk, armored turtles and Sszareth's
sanctuary. Ordinary enemies respawn on fresh expeditions; supplies and the
boss bounty pay once. [Stonewake Quarry](docs/stonewake-quarry.md) replaces Glassdeep
south of Stillbury, with five stages, crane walkways, side workshops, new enemies
and Orin's Magic Stone Warhammer quest (+30 maximum HP while equipped). Rowan and
the docks stand outside Oakhaven beside the Sunken Coast entrance. Defeating
the Drowned Queen still opens dock access; Rowan's map unlocks the first voyage.
Dawnmere's merchant has a working door and a single-tile path from the west road.
Merchant Corvin is inside, with directions to Saltwind; a new marsh quest is not
assigned yet. Dawnmere's east route remains closed. On the atlas, the marsh is a small coastal
inlet, preserving Stonewake's beaches and cliffs, Sunken Coast's sheltered cove,
and broad open sea.

Return to Town uses the last town actually entered; bypassing a town leaves
that destination unchanged. Return portals retain the original expedition.

All five towns now stand beside signed exterior roads. Walk into a town sprite
to arrive beside the matching interior exit gate, or follow the road around town to
continue travelling. See [town roads and directional entrances](docs/town-roads.md).

The Ruined Temple is a compact pyramid landmark in the island's central clearing.
Walk into any side to begin its five-floor adventure; retreat returns to the
path beside Nahla. See the [temple sprite and renderer preview](docs/art-sources/temple-entrance/README.md).

## Controls

At **30% HP or below**, the play area gains a slowly pulsing red border and the
HP display turns red with a **LOW HEALTH** label. Heal above 30% to clear the
warning. Open inventory with **I** to use a Health Potion.

These are the default keys. Choose **Controls** from the title menu or press
`Esc` during a game and choose **Controls**. Select a command, press `Enter`
(or click it), then press
the new key. Picking a key another command uses swaps the two keys. `Esc` and
the arrow keys cannot be changed, so the menu and movement always work, and the
move-left key also interacts. Keys inside views such as the inventory stay the
same. Changes save automatically in `saves/controls.json` and apply across
restarts, new characters, and loaded games. If no hotkey settings exist, loading
an existing character adopts that save's keys. The help screen (`H` by default)
always shows the current keys.

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

- [Five-town expansion and Dawnmere](docs/five-town-expansion.md)
- [Stillbury healer and witch](docs/healer.md)
- [Regional difficulty and progression](docs/difficulty-scaling.md)
- [Weapons, armor, and shields](docs/weapons.md)
- [Spells](docs/spells.md) and [Mage progression](docs/mage-progression.md)
- [Tavern quests](docs/tavern-quests.md) and [quest journal](docs/quest-journal.md)
- [Castle Dungeon](docs/dungeon.md)
- [Haunted Forest](docs/haunted-forest.md)
- [Goblin Mountains](docs/goblin-mountains.md)
- [Dragonspine and High Pass](docs/dragonspine.md)
- [Sunken Coast](docs/sunken-coast.md)
- [Saltwind Marsh — five stages and skiff exploration](docs/saltwind-marsh.md)
- [Blackwater Swamp](docs/swamp.md)
- [Sunscar Wastes and the magic lamp quest](docs/sunscar-wastes.md)
- [Moonveil Gardens](docs/moonveil-gardens.md)
- [Ashen Hollow](docs/ashen-hollow.md)
- [Stonewake Quarry](docs/stonewake-quarry.md)
- [Elderwood Forest and the northern road](docs/elderwood-forest.md)
- [Royal Catacombs](docs/catacombs.md)
- [Castle interior and play testing](docs/castle-design.md)
- [Royal Jail, escape, and equipment recovery](docs/royal-jail.md)
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

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/9c480504-6f05-490d-bfd8-255b8d964996" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/36cb3d56-c0cb-4345-a1bd-4b5e652d2ab4" />

<img width="2586" height="1466" alt="image" src="https://github.com/user-attachments/assets/d4388a92-993c-4a2b-afa8-f30a797c369f" />

<img width="2586" height="1466" alt="Screenshot 2026-10-21 at 10 37 28 AM" src="https://github.com/user-attachments/assets/e25ea54f-80a9-417c-8d79-a9d4db962ba5" />


## Building and Running

Build a release executable with `make`, or build and start it with `make run`.
The executable is written to `build/conr`.

### Browser play and GitHub Pages

The browser build compiles the same C/SDL2 game to WebAssembly, following the
[Wolfenstein browser build](https://github.com/christsantiris/wolfenstein2026/commit/fc2bd59fc61dcad1221c365d1081a009f196cf4a)
and its [Pages workflow](https://github.com/christsantiris/wolfenstein2026/commit/a25b666f895b49a70bbd55ec4dc5f9b10b89024a),
including the [CI build-command correction](https://github.com/christsantiris/wolfenstein2026/commit/e6c8b711048eb36db69b59265cebcbac00b8322d).
Players need a desktop browser, keyboard and mouse; they do not install the game.
See [browser build and deployment instructions](docs/browser.md) for local
preview, GitHub Pages setup and troubleshooting.

From the project root, use the same installed SDK as Wolfenstein:

```sh
source /private/tmp/wolf-browser-emsdk/emsdk_env.sh
make browser-run
```

Open **http://localhost:8000/**, then select **Play**. Run `source` again in
each new terminal. If the temporary SDK directory is removed, install
Emscripten 4.0.23 in a permanent location and update that path.

Once these changes reach `master`, the **Browser Pages** workflow builds and
deploys `dist/browser/`. In the repository's **Settings → Pages**, first set
**Build and deployment → Source** to **GitHub Actions**. After a successful
deployment, the game will be available at
[Castle of No Return on Pages](https://christsantiris.github.io/c_rpg/).

Save slots, hotkey settings, and high scores persist in this
browser's site storage. Desktop saves are not uploaded or bundled. Clearing
site data removes browser saves; storage restrictions can make them temporary.

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
temple stairs and castle floor progression are accessible. The jail's secret
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
   Farms between areas for rest to improve hp. New story lines. Improve area sprites. Building diversity in towns. Crown Road North between forest and castle
- Recruit a party to join the adventure
   NPCs that can team up with the player character and accompany on the quest. Implement resurrection. 
- Improve difficulty scaling and economy
   Warrior and Rogue should struggle at certain levels without better weapons. Mage should struggle without better spells. Purchasing new weapons, armor and tomes should become necessary to progress.
- Add paypal integration to buy gold
   Add real money feature to buy in-game gold.
- DnD style stats instead of static class based stats
   Roll for stats. Choose a class based on strengths. Ascending armor class chance to hit. 
- Add new equipment types
   Add helmets, rings, gauntlets

