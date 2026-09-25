# The Castle of No Return

A roguelike adventure game inspired by *Castle of the Winds* by SaadaSoft. Explore dungeons, battle enemies, and survive the challenge!

**GitHub:** [https://github.com/christsantiris/c_rpg](https://github.com/christsantiris/c_rpg)

## Table of Contents

- [Overview](#overview)
- [Town and Progression](#town-and-progression)
- [Controls](#controls)
- [Game Documentation](#game-documentation)
- [Screenshots](#screenshots)
- [Building and Running](#building-and-running)
- [Dependencies](#dependencies)
- [Contributing](#contributing)
- [License](#license)
- [Installers](#installers)
- [Roadmap](#roadmap)

## Overview

*The Castle of No Return* is a C/SDL roguelike with turn-based combat and a
retro pixel-art style. Choose a Warrior, Mage, or Rogue, prepare in town, and
explore an eight-floor undead dungeon, the eight-stage Haunted Forest, the
eight-stage Goblin Mountains, and the eight-stage Sunken Coast. Each region
keeps its own generated maps and progression. Completing all four regional
bosses and Tavern quests opens a sea route to the Ruined Isle and its
four-tier Ruined Temple finale.

Advancing never requires clearing every enemy. Final regional exits require
defeating the region's boss, but surviving regular enemies do not block leaving.
Defeating the Necromancer also reveals the final forest exit if its landmark
has not been found.

## Town and Progression

The roads through town lead north to the dungeon, west to the forest, east to
the mountains, and south to the coast. Cain stands near the central crossroads.
Speaking with him explains the dangers beyond town and grants one Scroll of
Return to Town when inventory space is available. Use the scroll from the
inventory to learn the permanent spell.

The Tavern houses Elowen, Dain, Alder, and Mara. Their quests add guarded
objectives throughout the four regions. Accepting a regional quest begins a
fresh expedition through that region so completed maps never turn the quest
into an empty walk. Regular enemies and maps regenerate, while defeated bosses
remain defeated.

The Blacksmith sells weapons, armor, and shields. The Alchemist sells potions,
spell scrolls, and Mage spell tomes. Each shop buys only its own item types.
Stock expands as bosses are defeated; buying costs twice an item's base value,
while selling pays one quarter of base value.

### Town healer

The **HEALER** building is west of the blacksmith, north of the east-west road.
Follow its short entrance path from that road to meet Lysa.
She restores all missing HP for **1 gold per 3 HP, rounded up**. The price is
shown before payment; press Enter or click the treatment button to accept,
or Escape to leave. If HP falls below 25% and full treatment is unaffordable,
Lysa offers a separate free emergency-care option that restores HP to 50%.
Emergency care may be used three times during a game. Paid full treatment
remains available. Full health costs nothing. Healing does not restore MP.

### Town witch

The **WITCH** hut stands east of the alchemist on the road to the Goblin
Mountains. Morwen restores all missing MP for **1 gold per 3 MP, rounded up**.
The price is shown before payment; press Enter or click the restoration button
to accept, or Escape to leave. If MP falls below 25% and full restoration is
unaffordable, Morwen offers a separate free emergency ritual that restores MP
to 50%. The emergency ritual may be used three times during a game. Paid full
restoration remains available. Full mana costs nothing.

### Harbor and Ruined Isle

Captain Rowan waits near the harbor and describes the island before it is
reachable. Defeat the four regional bosses and turn in all four Tavern quests
to extend the town road to the dock. Rowan then gives the Island Treasure Map.
Walk into the dock entrance and choose to board the ship.

The tropical Ruined Isle is a peaceful exploration area with an abandoned
camp, carved marker, broken statue, lagoon, Captain Rowan, and Nahla. Rowan can
sail back to town. Nahla assigns **The Buried Sun**, which leads through the
island gate into the Ruined Temple.

### Goblin Mountains terrain

Stages 2 and 7 funnel combat onto narrow bridges. An unmarked weak span collapses
behind you after crossing; press `A` beside the gap to rebuild it from either
side. Mountain rooms have open routes without internal gate barriers.

On stages 3–6 and 8, amber rockfalls mark optional buried treasure passages. Press `A`
beside either end to expose the passage: falling rocks cost `4 + stage` HP, but
the cave holds a one-time cache worth `15 + 4 × stage` gold. Stand on the cache
and press `A` to collect it. Newly revealed passages appear on the minimap;
exposed caves, claimed caches, and repaired bridges survive
cached revisits and saving/loading. Arrow keys always move;
`A` interacts when beside a mountain obstacle or standing on its cache.

### Sunken Coast water routes

Each new Coast stage has one tide control. Stand on it and press `A` to transfer
water between the blue and amber channels. Every activation
reverses the flow: one basin drains while the other floods. Colored corner
markers identify the channels; drained channels show exposed stone.

Two guarded treasure chambers are accessible at opposite tide levels. Each
chamber cache grants `20 + 4 × stage`
gold once; stand on it and press `P` to collect it. Drain the blue basin to reach
and light Mara's beacons. The control and exit remain reachable in either
tide state, with the Drowned Queen still guarding the final exit.

Operating a control reveals its connected channels on the minimap. Rising water
carries enemies to an open bank and submerges dropped items until the channel
is drained again. Water states, claimed caches, and discoveries persist through
cached revisits and save/load. Existing saved Coast maps lose the redundant
sluice wall and switch while retaining their tide state and treasure chambers.

### Ruined Temple

Talk to Nahla on the Ruined Isle to begin **The Buried Sun**, then walk through
the temple gate. The temple is a four-tier stepped pyramid. Use the upward
stairs on each tier to climb toward the summit and the downward stairs to return
to the tier below. The Fallen Sun Guardian and buried vault appear only on the
summit.

Stand on or next to a solar altar and press `A` to switch between Sun and Moon.
Sun closes lunar doors and activates orange floor traps. Moon opens those doors
and permanently awakens dormant Moonbound Sentinels; switching back does not
petrify them. Walls and closed doors block all ranged attacks.

The Fallen Sun Guardian retaliates when attacked at range. Its armor breaks
below half health, increasing the power of its sunburst. Defeat it, recover the
buried treasure, and return to Nahla for the quest reward. Return to Town works
inside the temple and leaves a portal near the harbor that restores the exact
temple tier, position, and encounter state. Descend from the first tier through
the southern entrance to return to the island surface.

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
| `F` | Fire an equipped bow |
| `H` | Open help |
| `Esc` | Close the current view or return to the main menu |

Inside the inventory, use the arrow keys to select an item, `U` to use it,
`E` to equip it, `O` to equip a one-handed weapon in the off hand, and `D` to
drop it. In the spellbook, select a spell and press `Enter` to equip it. Shops,
the healer, the witch, and the harbor use the arrow keys and `Enter`; shop buy
and sell modes are switched with `Tab`.

## Game Documentation

- [Regional difficulty and progression](docs/difficulty-scaling.md)
- [Weapons, armor, and shields](docs/weapons.md)
- [Spells](docs/spells.md) and [Mage progression](docs/mage-progression.md)
- [Tavern quests](docs/tavern-quests.md) and [quest journal](docs/quest-journal.md)
- [Haunted Forest](docs/haunted-forest.md)
- [Goblin Mountains](docs/goblin-mountains.md)
- [Sunken Coast](docs/sunken-coast.md)
- [Harbor, Ruined Isle, and Ruined Temple](docs/harbor-and-ruined-isle.md)

## Screenshots

Below are screenshots of the game in action:

<img width="2554" height="1428" alt="image" src="https://github.com/user-attachments/assets/2608e154-f4db-48a3-afa9-59fc8775f54f" />

<img width="2554" height="1428" alt="image" src="https://github.com/user-attachments/assets/5dcf54c0-0f35-4a1d-b0db-e788f0214a62" />

<img width="2548" height="1428" alt="image" src="https://github.com/user-attachments/assets/b789ff2d-a9d1-4138-97a8-c8fd152c06c9" />

<img width="2554" height="1434" alt="image" src="https://github.com/user-attachments/assets/2ee519e4-2a53-4cb3-9596-242298ddbba0" />

<img width="2550" height="1434" alt="image" src="https://github.com/user-attachments/assets/6e198c44-2add-4212-b7c7-7aadf8ba9180" />

<img width="3834" height="2138" alt="image" src="https://github.com/user-attachments/assets/59a939ed-29a8-4ea2-aa6c-dffabce33bae" />

<img width="2554" height="1444" alt="image" src="https://github.com/user-attachments/assets/3a55d05f-0c2a-4d6e-b735-c8af4b2c9050" />

<img width="4616" height="2580" alt="image" src="https://github.com/user-attachments/assets/4b6fdb08-713e-451b-bc3e-13b4ba9f3d12" />

## Building and Running

Build a release executable with `make`, or build and start it with `make run`.
The executable is written to `build/conr`.

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

### Clean the build

Run `make clean` to remove the build directory.

### Run the tests

Run `make test` to build and run the unit test executable.

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

## Roadmap

### Production releases

- Signed Apple DMG
- Windows installer
- Fix Linux installer

### High priority

1. More tactical enemy behavior
   Add recognizable combat roles such as protectors, ambushers, healers, summoners, ranged attackers, and enemies that retreat or flank.
2. Boss encounter improvements
   Give every boss multiple phases, telegraphed signature attacks, an arena mechanic, and a guaranteed thematic reward.
3. Combat feedback
   Add clearer hit effects, critical indicators, blocked-damage feedback, status icons, and distinct weapon sounds.
4. More weapon variation
5. Mini quests to get more gold

### Polish priority

1. Accessibility and controls
   Add remappable controls, text scaling, colorblind-safe indicators, adjustable message duration, and clearer interaction prompts.
