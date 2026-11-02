# Haunted Forest

The Haunted Forest is a **five-level adventure** beside a straight, one-tile
cobblestone road between **Oakhaven (west)** and **Dawnmere (east)**. The road
cuts through dense green woodland, with a narrow grass verge and a clearing at
its town end. A large haunted-tree entrance stands in a clearing north of that road. Leaving either town first
enters the safe approach; entering the forest always starts at **Forest 1**.
Its interior starts at stage one from either town. Ordinary backtracking within
an expedition remains available.

The forest is drawn at the Crown Roads' 1.5× zoom: the camera shows about 30 ×
18 tiles around the player. The Necromancer acts within twelve tiles; the camera leans toward him to keep him on screen as his warned spirit bolts and root lanes begin.

The Necromancer occupies **Forest 5**. Defeat him, collect his reward, and use
the normal grove exit to emerge onto the road. Walk **west to Oakhaven** or
**east to Dawnmere**. Victory permanently opens the through road. There is no
forest shortcut marker, boss branch, or shortcut confirmation menu.

Forest maps use rounded clearings connected by wandering trails with loops
and false branches. Stage passages vary between the east, west, north, and
south edges. The incoming passage remains open for retreat. Reaching the
glowing ancient landmark reveals the onward passage and hidden trails in
either direction. Defeating every regular enemy is never required.

| Level | Region | Layout | Difficulty tier |
| --- | --- | --- | --- |
| 1 | Gloomwood Border | Two-route fork with a middle crossover | 1 |
| 2 | Spiderweb Thicket | Three pockets around a central trail | 2 |
| 3 | Dark Elf Territory | Braided lanes and hidden trails | 4 |
| 4 | Sunken Grove | Corrupted clearings and looping trails | 5 |
| 5 | Necromancer's Grove | Trails narrowing into the final boss grove | 8 |

Difficulty controls the regular enemy roster, enemy count, and trap count.
Clearing dimensions vary when a stage is first generated. Completing other
regions can strengthen enemy rosters through the existing region-order scaling.
Travel score is awarded on first visits to stages from either direction.

## Enemy Roles

- **Pixie:** fragile skirmisher that moves twice per turn.
- **Blighted Wolf:** fast melee hunter that moves twice per turn.
- **Giant Spider:** melee controller whose bite applies poison.
- **Dark Elf Archer:** archer that fires along an unobstructed row or column.
- **Dark Elf Ranger:** bow skirmisher that fires every third turn and repositions after shooting.
- **Forest Troll:** high-health bruiser that moves every other turn.
- **Dark Elf Guard:** armored melee protector for nearby archers, with the same
  base 36 HP, 11 attack, 7 defense and 44 experience as a Hobgoblin Guard.
  Guards join the Dark Elf Territory encounters on stage 3 and remain in the
  deeper forest roster. Existing region-order scaling can introduce them earlier.

## Central Encounter and Shortcuts

The Necromancer waits in the stage-5 grove and remains dormant until the
player enters it or damages him from range. He stays in the grove and blocks
crossing while alive, but permits retreat toward the expedition's entry town.
His health, attack, defense and experience reward retain their existing scaling.

- **Grove Binder, above half health:** spirit bolts mark a fixed lane for one
  player turn. Step aside before the countdown ends. After the first bolt and
  recovery, one grave wave warns all unconsecrated patches for two turns.
- **Withered Grove, at/below half health:** one second finite grave wave is
  followed by alternating spirit bolts and two-turn crossing root lanes.
  Roots lock the player's row and column inside the grove when declared.
  Two ordinary cardinal steps can leave the crossing; no special ability is
  required. Roots deal one hit and do not leave impassable tiles or lingering
  damage. Roots and grave waves resolve separately.
- Every signature has a recovery turn without extra adjacent boss attacks.
  The panel shows phase, health, attack countdown, freeze and remaining graves.
  Numbered outlines show the declared footprint. Freezing pauses the countdown;
  crossing half health never changes an existing warning. Retreat beyond attack
  range pauses the warning and does not refund its wave.
- Three grave patches can be consecrated on or beside the patch with contextual
  **Interact (`A` by default)**. Consecration permanently prevents that patch
  from raising a servant. Consecrating any currently warned grave cancels the
  entire pending wave and grants two complete recovery turns. Loot on a grave
  stays available and does not prevent interaction. Every class can do this.

Each wave raises at most one **Grove Revenant** per unconsecrated grave. Recent
fallen forest servants return at half health; when no recent corpse is available,
a buried Blighted Wolf rises instead. Occupied patches and a full enemy roster
skip that spawn without moving the warning or replacing a living enemy. Servants
begin acting on the following turn and grant no repeat gold, items, experience
or score. Each health phase spends its wave when declared, even if interrupted.
Entering phase two before the first wave skips the unused first wave. Graves,
phase, exact attack targets and consumed waves survive saves and revisits.
Hidden random traps are excluded from the grove; previously revealed traps in
older saves remain intact.

The guaranteed **Necromancer's Cloak** keeps its +5 defense, 8% evasion and Rogue
restriction. It normally drops on the boss tile. If the floor loot list is full,
a named grove reliquary preserves it without deleting another item. An earned,
unclaimed cloak is restored there after backtracking or a map refresh. Interact
beside the reliquary or press `P` while standing on it; a full inventory leaves
it available. Claiming it once prevents later sale or dropping from producing
a replacement. The reward does not depend on consecrating the graves.

Defeating him reveals the normal grove exit, even with ordinary enemies still
alive. Use it when ready to leave: the return point is on the safe town road.
The entrance sprite and road occupy the same coordinates from either town;
V2 travel runs west toward Oakhaven and east toward Dawnmere.

Return to Town is a one-way escape to the last town actually visited. It leaves
no return portal. Physical re-entry uses the normal forest entrance.

## Forest Quest Chain

Alder appears in **Oakhaven's Inn after the Lich King's defeat**. He offers
**The Lost Rangers**: free one ranger on **Forest 1** and one on **Forest 2**.
Each is held by three Giant Spiders named **Spider Captor**. Defeat those
captors and press `T` beside the ranger; clearing ordinary enemies is optional.
Rescued rangers do not respawn. Return to Alder for **70 gold and 500 score**
once, without needing to venture beyond the second stage.

He next offers **Defeat the Necromancer**, explaining that victory opens the
road east to Dawnmere to investigate why evil creatures descended on the
kingdom. Accept or decline it at the Inn; accepted hunts count toward the
three active quests and preserve the rescues and expedition. The boss remains
on **Forest 5** and is reachable without either quest or any NPC dialogue.
Victory completes the follow-up immediately, with no extra Inn turn-in. Earlier
boss victories count when the rangers are turned in. Existing boss loot remains
available; the follow-up adds no extra NPC payment.

The V2 chain's two ranger bits and later boss-hunt states use the existing
serialized quest fields. Save version 230 records the expanded state values.

## Save Compatibility

The V2 geography uses fresh saves; the notes below describe earlier V1 migrations.

Save version **172** moves the old level-4 boss to level 5, old level 5 to level
4, and merges the former outer trails into levels 2 and 3. Occupied and
portal-linked snapshots take priority when trails merge. Existing older saves
first pass through their historical migrations. The rescue bits retain their
identities: old Forest 7, 6 and 5 objectives now belong to 2, 3 and 4.

Player stats, money, rescued wardens, boss victory, active loot, exploration,
portal anchors and coordinates remain intact. The boss retains health,
attack phases, consumed grave waves and reward state. Retired shortcut tiles
and underlays become normal forest floor; their pending travel prompts close.
Old approach crossroads and forest roads migrate onto the new town road.
Saving again does not repeat these migrations.

Version **125** adds the grave patches to active and cached living encounters
without regenerating the map or resetting boss health, enemy deaths, loot,
exploration, player position, portals or quests. Old unmarked attack timing
restarts with a visible warning. Previous victories stay defeated; only a known,
currently dropped legacy cloak is treated as unclaimed. New saves store cloak
claim state and both finite-wave flags, including when the player is elsewhere.

Renderer captures and focused play-testing notes are in
[the Necromancer encounter gallery](art-sources/necromancer-encounter/README.md).


## Labyrinth on the entrance road

Rook's Labyrinth stands beside the forest sprite on the shared road between
Oakhaven and Dawnmere. Its gate is visible but sealed until Rook assigns
The Ivory Rook in Oakhaven's Inn. Both town approaches reach it before forest
victory. Leaving the labyrinth returns to that approach, preserving the town
used; the Necromancer still controls the through road between towns.

The woodland has dense green crowns, brown trunks, moss and shaded clearings,
restoring the earlier forest appearance. Elderwood keeps its distinct living-forest palette.
