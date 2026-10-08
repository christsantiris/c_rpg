# The Castle of No Return

A roguelike adventure game inspired by *Castle of the Winds* by SaadaSoft. Explore dungeons, battle enemies, and survive the challenge!

**GitHub:** [https://github.com/christsantiris/c_rpg](https://github.com/christsantiris/c_rpg)

## Table of Contents

- [Overview](#overview)
- [Combat Areas and Level Counts](#combat-areas-and-level-counts)
- [Town and Progression](#town-and-progression)
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

*The Castle of No Return* is a C/SDL roguelike with turn-based combat and a
retro pixel-art style. Choose a Warrior, Mage, or Rogue, prepare in town, and
explore an five-floor undead dungeon, the seven-stage Haunted Forest, the
seven-stage Goblin Mountains, the five-stage Sunken Coast, and Stillbury's
seven-level Blackwater Swamp. Beyond the Goblin Mountains, the five-stage
Dragonspine ascent begins at Ridgeshire's east gate. Each region
keeps its own generated maps and progression. Defeating the Drowned Queen on
the Sunken Coast opens a sea route to the Ruined Isle. Its five-tier Ruined
Temple remains a late-game challenge.

Advancing never requires clearing every enemy. Regional bosses guard their
onward passages, but surviving regular enemies do not block leaving.
The Haunted Forest can be entered from OakHaven or Stillbury, with three stages
of increasing difficulty leading to the Necromancer on stage 4. Defeating him
reveals a marked shortcut to the opposite town. Players can instead continue
through the remaining three stages as difficulty decreases.
Forest, swamp, and mountain boss victories show a shortcut discovery prompt
until Enter is pressed. The marked shortcut appears beside the defeated boss,
including when a boss has been lured away from its original room.

## Combat Areas and Level Counts

| Combat area | Number of levels |
| --- | ---: |
| OakHaven Dungeon | 5 |
| Haunted Forest | 7 |
| Goblin Mountains | 7 |
| Blackwater Swamp | 7 |
| Sunken Coast | 5 |
| Dragonspine | 5 |
| Frostfell Wastes | 5 |
| Sunscar Wastes | 5 |
| Moonveil Gardens | 5 |
| Ashen Hollow | 5 |
| Glassdeep Caverns | 5 |
| Royal Catacombs | 5 |
| Ruined Temple | 5 |
| Rook's Labyrinth | 5 |
| Crown Road East | 1 |
| Crown Road West | 1 |
| Castle of No Return interior | 6 |

Combat areas that connect towns have seven levels; standalone adventures have
five. Crown Road East and West each use one combat map. The final Castle of
No Return is a six-floor ascent with minibosses on floors 2 and 4.

The forest, mountains, and swamp can be played from either end, with their
boss on level 4. Defeating that boss opens a shortcut to the opposite town;
the remaining three levels can also be explored.

## OakHaven and Progression

The roads through OakHaven lead north to the mountains, west to the forest, east
to the dungeon, and south to the coast. Cain stands near the central crossroads.
The mountains connect OakHaven's north gate and Ridgeshire's south mountain gate.
Both approaches grow harder toward the Goblin King at Crown Peak on level 4.
Defeating him reveals a shortcut to the opposite town; the remaining three levels
can instead be fought in decreasing difficulty. Dain's map bearers occupy levels
1, 2, and 3 on the OakHaven side, reached after the peak when starting in Ridgeshire.
Ridgeshire's east gate leads directly to Dragonspine. The safe High Pass links a
separate south shortcut gate in Ridgeshire with OakHaven's northeast road, unlocked
by the Goblin King's defeat. Both ordinary mountain gates remain available. Ilya
waits beside Ridgeshire's Dragonspine gate and offers a quest to recover a golden
goblet from the dragon's hoard. Her one-time reward is a Potion of Strength
that permanently adds 1 base attack when consumed.

Ridgeshire's workshop houses Garrick, who sharpens a selected sword, axe, or
dagger for **50 gold**, permanently adding **+1 weapon attack**. Each individual
weapon can be sharpened only once. Press **T** beside him inside the workshop,
select a weapon, and press Enter to pay; Esc returns to the room. Sharpening status
persists through dropping, re-equipping, and saving. The Blacksmith continues
to buy and sell equipment. See [weapon sharpening](docs/weapons.md#workshop-sharpening).

Ridgeshire's **Town Hall** stands northeast of the crossroads. Inside, approach
**Steward Hadrin** and press **T** to accept **Reclaim the Emberforge**. Recover
the guarded forge mechanism on Ashen Hollow stage **2**, then defeat the
Obsidian Guardians and repair the furnace on stage **4**. Press **A** while
standing on or beside each objective. Return to Hadrin for **100 gold and
800 score**, awarded once. Quest objects require no inventory slots, and
accepting the quest preserves previously explored Ashen stages. See
[Ashen Hollow](docs/ashen-hollow.md#reclaim-the-emberforge).

The Frostfell Wastes begin at Rosemoor's north gate. **Quartermaster Brenna** in
**Oakhaven's Tavern** assigns **The Silent Expedition**. She explains the route
through the woods to Stillbury, across the swamp to Rosemoor, and north into
Frostfell. Recover the guarded expedition journal on stage **2** with **A**,
then defeat the captors of **Surveyor Fen** on stage **4** and speak to him with
**T**. He returns home without an escort. Return to Brenna for **100 gold and
800 score**, awarded once. The Polar Kraken is optional for this quest; quest
objects need no inventory space. Acceptance preserves existing maps and boss
victories, and quest progress survives saving. See
[The Silent Expedition](docs/tavern-quests.md#brenna-the-silent-expedition).

Stillbury's west gate opens
onto the five-level Sunscar Wastes: enter from the east, advance west, and
backtrack east. Both the first level's east entrance and the final west exit
return to Stillbury. Return to Town leaves a portal by Stillbury's west gate.
Scarabs and venomous Vipers roam its first level, Mummies join from level two,
Djinn fire magic bolts from level three, and armored Golems appear from level
four. Mummies and Golems move slowly. The Desert Pharaoh guards the final
clearing with alternating magic bolts and drops the Sandstorm Staff, a rare
two-handed Mage weapon. Unclaimed staff rewards survive leaving and saving.
A visual sandstorm sweeps across Sunscar with blowing sand and a light amber haze.
Enter the Adventurer's Guild in Rosemoor and speak with Zara to accept
The Lost Magic Lamp. Recover the lamp on Sunscar level 4, then return to Zara
for a one-time reward of 80 gold and 600 score.
Speaking with Cain explains the dangers beyond town and grants one Scroll of
Return to Town when inventory space is available. Use the scroll from the
inventory to learn the permanent spell.

Oakhaven's Tavern houses Brenna and Liora. **Elowen** now waits in
**Stillbury's Inn**, alongside Alder, Rook, and Bram. Press **T** beside her to accept
**The Broken Seals**, restore the burial seals on **Oakhaven's dungeon floors
2, 3, and 4**, and return to her in Stillbury for **40 gold and 300 score**.

**Alder** assigns **The Lost Wardens** in **Stillbury's Inn**. Press **T**
beside him to accept the forest quest and return to the same Inn for **70 gold
and 500 score**, awarded once. Existing saves retain rescue progress and update
his location when loaded.

**Dain** waits in **Rosemoor's Adventurer's Guild Hall**, alongside Zara. Press
**T** beside him to accept **Recover the Treasure Map**. Defeat the Map Bearers
on **mountain stages 1, 2, and 3 on the Oakhaven side**, then return to Dain in
Rosemoor for **60 gold and 400 score**, awarded once.

**Mara** waits in **Ridgeshire's Town Hall**, alongside Steward Hadrin. Press
**T** beside her to accept **Relight the Drowned Beacons**. Relight the guarded
beacons on **Sunken Coast stages 2, 3, and 4**, south of Oakhaven, then return to
Mara in the Town Hall for **80 gold and 600 score**, awarded once. Existing
saves keep coast quest progress and update her location when loaded.

The original dungeon, mountain, forest, and coast quests add guarded objectives
throughout their regions. Accepting one begins a
fresh expedition through that region so completed maps never turn the quest
into an empty walk. Regular enemies and maps regenerate, while defeated bosses
remain defeated.
The Lost Wardens objectives occupy forest stages 1, 2, and 3 on the OakHaven
side. Entering from Stillbury encounters them after the Necromancer, in order
3, 2, and 1; taking the grove shortcut skips those rescues.

The Blacksmith sells weapons, armor, and shields. The Alchemist sells both
potion types, spell scrolls, and Mage spell tomes in OakHaven. OakHaven's west
forest gate enters stage 1; Stillbury's main east gate enters stage 7. Stillbury
can also be reached from Rosemoor before the Necromancer is defeated.
After his defeat, the separate west gate of OakHaven and upper east gate of
Stillbury permanently open onto a short, enemy-free forest road between the
towns. Both ordinary forest gates remain available and retain explored stages.
Stillbury has an Inn, a healer selling
Health Potions, a witch selling Mana Potions, and Rook's labyrinth. Its north
gate opens onto the seven-level Blackwater Swamp. Each shop buys its own item types.
Stock expands as bosses are defeated; buying costs twice an item's base value,
while selling pays one quarter of base value.

### Stillbury healer

The **HEALER** building is north of Stillbury's east-west road.
Follow its short entrance path from that road to meet Lysa. She sells Health
Potions for **20 gold** each. Each potion restores HP to full when used
from inventory. The Alchemist also sells Health Potions.

### Stillbury witch

The **WITCH** hut stands east of Stillbury's crossroads. Morwen sells Mana Potions
for **20 gold** each. Each potion restores MP to full when used from inventory.
The Alchemist also sells Mana Potions.

### Rook's labyrinth

Speak with Rook in Stillbury's Inn to receive his one-time retrieval quest and open
the labyrinth gate on the eastern outskirts, across the road from the witch's
hut. Explore five maze floors, each with a rune and enemies. Each of the first
four floors has two identical-looking downward stairs: one leads onward, while
the other reaches a short dead-end corridor with a return stair. Light all five
runes, defeat the Minotaur, recover the ivory rook, and return to Rook for
**40 gold** and **500 score**. Stairs stay open without clearing enemies. After
the one-time quest, the labyrinth remains open for further expeditions; ordinary
enemies return, but the Minotaur and quest reward do not.

The Minotaur is a horned, axe-wielding melee boss guarding floor 5's relic
vault. Defeating it drops a Magic Shield and marks the Labyrinth boss journal
entry complete. Existing Maze Warden saves become Minotaurs on load, retaining
their health and previous victories.

### Blackwater Swamp

Blackwater Swamp has seven levels and can be entered from Stillbury's north
swamp gate at level 1 or Rosemoor's main south gate at level 7. Difficulty rises
toward the Swamp Demon on level 4, then falls beyond it. Defeating him reveals
a marked shortcut to the opposite town; players can instead cross the three
remaining levels. He drops a Demonic Sword: a +6 one-handed weapon for every
class, with a two-tile `F` attack. Talk to Bram at the Stillbury Inn to rescue
Mira from a vampire on level 3. From Stillbury she appears before the Demon;
from Rosemoor she appears after it. See [Blackwater Swamp](docs/swamp.md).

### Rosemoor, Crown Roads, and the Castle

Defeating the Swamp Demon also opens a second north gate in Stillbury. It leads
along a safe, enemy-free swamp shortcut to a separate south gate in Rosemoor.
Both towns retain their ordinary swamp gates and explored swamp levels.
Rosemoor has the Apothecary. Two Royal Guards at its east gate warn that Crown Road East swarms
with bandits, archers, and horsemen. That road leads to the grounds of the
Castle of No Return. Its north entrance leads into the six-floor final area. Crown Road West continues
from the castle's east gate to Ridgeshire's west gate. Each road keeps its own
enemies and progress between visits.

The castle introduces shield formations, portcullis levers, warned attacks,
shifting magical barriers, and trapdoors that drop you one floor and remain
visible. Defeat the Castellan on floor 2 and the Royal Arcanist on floor 4 to
unlock permanent passages from the grounds. The final floor requires no
regional collectibles. Return to Town becomes a confirmed one-way escape to Rosemoor:
surviving defenders reset, while miniboss victories, passages, revealed
trapdoors, and dropped items persist. Defeating Lord Veyr ends the campaign with
victory. See [Castle interior and play testing](docs/castle-design.md).

The safe forest road, swamp road, and High Pass shortcuts are one tile wide.
Older saves narrow these routes while preserving exploration and moving players
and dropped items onto the path.

Town destinations use wooden signposts with cream lettering, including both
ends of each unlocked shortcut. Area gate labels keep the same color at every
entrance: forest green, mountains red, swamp lime, and Crown Roads gold.

### Moonveil Gardens

Rosemoor's west gate opens onto the five-stage Moonveil Gardens. Enter from the
east and follow the paths west through glowing flowers, giant mushrooms,
moonlit pools, and ancient stone circles. Fey Tricksters and Giant Moths appear
first, followed by Carnivorous Flowers and Thorn Guardians. The Thorn Regent guards
stage 5's return to Rosemoor and drops a Potion of Strength. Backtracking, later
visits, saves, and Return to Town portals retain the gardens' explored maps and
enemy progress.

**Botanist Liora** in **Oakhaven's Tavern** assigns **The Stolen Moonseed** with
**T**. She directs you through the forest to Stillbury, across the swamp to
Rosemoor, and through its west gate. Defeat the guards and use **A** on or
beside the stage **2** seed pod and stage **3** moonwater spring, in either
order. With both recovered, plant and water the ancient circle on stage **4**.
A Moonflower blooms and the clearing remains restored on later visits.
Return to Liora for **90 gold and 700 score**, awarded once. The Thorn Regent
is optional, quest objects use no inventory space, and acceptance preserves
existing garden progress. See [Moonveil Gardens](docs/moonveil-gardens.md#the-stolen-moonseed).

### Ashen Hollow

Ridgeshire's north gate leads into five stages of Ashen Hollow, a volcanic basin
of basalt columns, charred trees, ruined stonework, and animated lava pools.
Enter from the south and follow winding ash paths north. Cinder Imps throw
firebolts, Ash Hounds rush forward, and Obsidian Guardians defend later stages.
The Cinder Lord guards stage 5's return route to Ridgeshire and drops a Potion
of Strength. Exploration and enemy progress survive backtracking, later visits,
saves, and Return to Town portals. See [Ashen Hollow](docs/ashen-hollow.md).

### Glassdeep Caverns

Stillbury's south gate opens into five stages of Glassdeep Caverns. Descend
south through irregular crystal chambers, underground pools, mineral seams,
and the paving of a buried sanctuary. Crystal Spiders rush forward, Blind
Stalkers wait to ambush nearby explorers, and armored Shard Golems guard later
stages. The Prism Sovereign marks a beam before firing along that fixed path:
step aside during its warning turn. Defeating it opens stage 5's return to
Stillbury and drops a Potion of Strength. Saves and Return to Town portals
preserve the caverns' independent progress.

**Surveyor Orin**, inside **Rosemoor's Adventurer's Guild**, assigns **The Broken
Resonance** with **T**. Restore the guarded Root, Tide, and Crown resonators on
Glassdeep stages **2, 3, and 4**, in any order. Press **A** on or beside each
resonator to read its inscription, then choose **1 Low**, **2 Middle**, or
**3 High**; **Esc** cancels. Incorrect tones allow another attempt. Return to
Orin for **120 gold and 1,000 score**, awarded once. The Prism Sovereign is
optional for this quest, and acceptance preserves existing cavern progress.
See [Glassdeep Caverns](docs/glassdeep-caverns.md#the-broken-resonance).

### Royal Catacombs

The castle grounds' south gate opens into five floors of Royal Catacombs,
a harder undead dungeon with looping burial chambers. Ancient Skeletons can
reassemble once while their ossuary brazier burns; stand beside a brazier and
press **A** to extinguish it. Bone Sentinels, Grave Archers, tougher Crypt Bats
and Wraiths, and reviving Bone Cantors populate the chambers. Burial plates
mark spike lines a turn before striking. The Grave Marshal guards floor 5,
telegraphing polearm sweeps and drawing protection from two braziers. Victory
opens a direct return to the castle and rewards Gravekeeper's Mantle, armor
for every class with +6 defense and +20 maximum health. Return to Town opens
a portal in Rosemoor. Save migration preserves existing progress and adds
the new area. See [Royal Catacombs](docs/catacombs.md).

**Brother Oswin** offers **Rest for the Forgotten** in **Ridgeshire's Town
Hall**. Press **T** beside him, then follow Crown Road West from Ridgeshire's
west gate to the castle grounds and take the south gate. Extinguish the marked
Soldiers', Watchers', and Choir memorial braziers on **Catacombs floors 2, 3,
and 4** with **A**, then defeat the Grave Marshal and recover the burial ledger
on **floor 5**. Braziers can be silenced while guards live, preventing
resurrection. Return to Oswin for **150 gold and 1,500 score**, awarded once.
Already extinguished memorials and previous boss victories count; acceptance
preserves exploration and portals. Quest objects use no inventory space.
The quest is optional and does not gate castle entry or victory.

### Harbor and Ruined Isle

Captain Rowan waits near the harbor and describes the island before it is
reachable. Defeat the Drowned Queen on the Sunken Coast to extend the OakHaven
road to the dock. Speak with Rowan to receive the Island Treasure Map, then
walk into the dock entrance to board the ship. The first voyage consumes the
map and permanently unlocks return trips. If neither the dungeon nor the
mountains has been cleared, Rowan warns that the Ruined Temple is dangerous
and recommends training there first. The advice does not block sailing.

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

The Sunken Coast has five stages, with Mara's beacons on stages 2, 3, and 4
and the Drowned Queen on stage 5. Existing saves migrate coast stages and
portals while preserving beacon progress. Each new Coast stage has one tide
control. Stand on it and press `A` to transfer water between the blue and amber
channels. Every activation
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
the temple gate. The temple is a five-tier stepped pyramid. Use the upward
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

These are the default keys. To change them, press `Esc` during a game and
choose **Controls**, select a command, press `Enter` (or click it), then press
the new key. Picking a key another command uses swaps the two keys. `Esc` and
the arrow keys cannot be changed, so the menu and movement always work, and the
move-left key also interacts. Keys inside views such as the inventory stay the
same. Remapped keys belong to the current character: saving the game keeps
them, new games start with the defaults, and loading a save restores that
save's keys. The help screen (`H` by default) always shows the current keys.

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
`E` to equip it, `O` to equip a one-handed weapon in the off hand, and `D` to
drop it. In the spellbook, select a spell and press `Enter` to equip it. Shops,
the healer, the witch, and the harbor use the arrow keys and `Enter`; shop buy
and sell modes are switched with `Tab`.

## Game Documentation

- [Regional difficulty and progression](docs/difficulty-scaling.md)
- [Weapons, armor, and shields](docs/weapons.md)
- [Spells](docs/spells.md) and [Mage progression](docs/mage-progression.md)
- [Tavern quests](docs/tavern-quests.md) and [quest journal](docs/quest-journal.md)
- [Castle Dungeon](docs/dungeon.md)
- [Haunted Forest](docs/haunted-forest.md)
- [Goblin Mountains](docs/goblin-mountains.md)
- [Dragonspine and High Pass](docs/dragonspine.md)
- [Sunken Coast](docs/sunken-coast.md)
- [Blackwater Swamp](docs/swamp.md)
- [Sunscar Wastes and the magic lamp quest](docs/sunscar-wastes.md)
- [Moonveil Gardens](docs/moonveil-gardens.md)
- [Ashen Hollow](docs/ashen-hollow.md)
- [Glassdeep Caverns](docs/glassdeep-caverns.md)
- [Royal Catacombs](docs/catacombs.md)
- [Castle interior and play testing](docs/castle-design.md)
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

<img width="3836" height="2156" alt="image" src="https://github.com/user-attachments/assets/4d8ce9ce-f7c4-4be0-926a-8407eaf2f95c" />

<img width="3842" height="2134" alt="image" src="https://github.com/user-attachments/assets/d5cfe0da-4660-410c-8221-d8b621f4bed3" />

<img width="3842" height="2134" alt="image" src="https://github.com/user-attachments/assets/5fb19eb3-b565-49c8-b089-b58688d4d13c" />

<img width="2540" height="1432" alt="image" src="https://github.com/user-attachments/assets/529379b1-1317-4510-b5e3-d0f9440952b5" />

## Building and Running

Build a release executable with `make`, or build and start it with `make run`.
The executable is written to `build/conr`.

### Debug interface

To test quest NPCs with every town shortcut already open, run:

```bash
./tools/debug_shortcuts.sh
```

Select **New Game**, enter a name, and choose a class. The forest shortcut
between Oakhaven and Stillbury, swamp shortcut between Stillbury and Rosemoor,
and High Pass between Oakhaven and Ridgeshire are open immediately. This uses
the existing completion flags for the forest, mountain, and swamp bosses;
quests stay unassigned and their NPCs still award their normal rewards.
The unlocks persist if you save this debug character.

The launcher builds a separate debug executable in `build/debug-shortcuts`
and runs it from the project directory. It accepts the existing debug options,
for example `./tools/debug_shortcuts.sh --gold 500 --weapon bow`.

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
| Not applicable | `--unlock-shortcuts` | Flag with no value; automatically supplied by `tools/debug_shortcuts.sh` | Town shortcuts unlock through normal boss victories |

Providing `WEAPON` or `SCROLLS` replaces the normal class starting items in
that category. Use `WEAPON=none` or `SCROLLS=none` to begin without that item
category. Scroll names must be comma-separated without spaces. These settings
are applied after class selection when a new game is created; they do not
modify a loaded save.

Examples:

```bash
# Start NPC testing with all town shortcuts open.
./tools/debug_shortcuts.sh

# Combine unlocked shortcuts with a custom starting loadout.
./tools/debug_shortcuts.sh --gold 500 --weapon bow --scrolls magic-arrow,fireball,heal

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

Debug executables also accept `--unlock-shortcuts` for new characters.
As with the loadout options, it does not modify a loaded save.

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

## Credits
[Pixabay](https://pixabay.com/) for the sound effects

## Roadmap

### Production releases

- Signed Apple DMG
- Windows installer
- Fix Linux installer

### Feature enhancements

- Boss encounter improvements
   Give every boss multiple phases, telegraphed signature attacks, an arena mechanic, and a guaranteed thematic reward.
- Recruit a party to join the adventure
- Improve difficulty scaling and economy
   Warrior and Rogue should struggle at certain levels without better weapons. Mage should struggle without better spells. Purchasing new weapons, armor and tomes should become necessary to progress.
