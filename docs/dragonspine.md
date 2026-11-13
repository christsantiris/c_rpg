# Dragonspine and High Pass

Ridgeshire is reached from Castle Grounds by Crown Road East. Its former
southern mountain connection is closed; neither Dawnmere nor Stillbury's
coastal adventures lead there.
Ridgeshire's north gate leads along a safe approach through rock walls to
Dragonspine, placing its snowy peaks at the northern edge of the world alongside
Frostfell Wastes. See [road scenery](road-scenery.md).
Leaving the first stage or completing the summit returns to that approach;
walk south to reach Ridgeshire's north gate. Return to Town travels directly
to town, with its portal beside the north gate.
Save version **163** relocates existing approach paths and town portals to this
gate while preserving exploration, loot, combat maps, and quest progress.

Dragonspine is drawn at the Crown Roads' 1.5× zoom: the camera shows about 30 ×
18 tiles around the player. The Red Dragon acts within sixteen tiles or whenever the player is in its chamber; the camera leans toward it, though at 16 rows above or below it can sit just off screen. Its breath reaches twelve tiles, so every cone that can reach the player starts on screen.

Dragonspine has five generated stages with independent progress and saved
floor caches. Broad cliff ledges fork around deep ravines, joining a lower
ridge, a high rim and a connecting shelf. Trails at the west and east map edges
lead back and forward. Its pale cliff walls and snow paths give way to black ash and glowing embers
higher up. The summit contains the Red Dragon's gilded lair.
Cliff faces connect across tiles, with chipped rims and broad rock shading.
Snow drifts cross tile boundaries; ash and gold form irregular patches instead
of dark square floor tiles. Ravines share the native blue-gray rock shading
and snowy edges, keeping their impassable ground consistent with the cliffs.
These visuals apply to existing saved maps.

## Movable boulder cover

Four loose boulders sit on the ledges of each new stage. Stand cardinally beside
one and use the configured **Interact** key (**A** by default) to push it one
tile directly away from the player. Every class can push them. A boulder blocks
movement and projectiles, so changing its position changes the available cover
against Goblin Archers and Fire Elementals.

Boulders move only onto empty snow or ash. They cannot cover living enemies,
loot, return portals, quest objects or the final clearing. A push that would
isolate any walking tile is refused. Moving a boulder restores its original
snow or ash; positions and both terrain types persist through backtracking,
new expeditions and saving. The Red Dragon's stable and weak arena columns are
separate from these movable rocks. All routes and objectives remain reachable
without pushing a boulder.

| Stage | Route | Main threats |
| --- | --- | --- |
| 1 | Snowbound approach | Goblin Scouts, Goblin Archers, Drakes |
| 2 | High cliff paths | Goblin Archers, Drakes, Stone Giants |
| 3 | First ash fields | Goblin Archers, Drakes, Stone Giants, Fire Elementals |
| 4 | Burning ascent | Goblin Archers, Drakes, Stone Giants, Fire Elementals |
| 5 | Dragon's lair | Full roster and the Red Dragon |

Drakes close for melee, Stone Giants endure heavy punishment, and Fire Elementals
cast along clear rows or columns. As in other regions, stages 1–4 can be
exited without defeating every enemy. The stage 5 summit departure also remains
available while the Red Dragon lives; see [boss gates](boss-gates.md). Defeating
the dragon awards gold, a choice of dragonforged armor, and the binding
evidence for Edric’s local investigation.

## Red Dragon encounter

The dragon stays within its generated lair and keeps its existing health,
attack, defense, XP and boss score. It wakes on lair entry or after being
damaged from range. Leaving its engagement range pauses its queued signature;
it cannot pursue into other chambers. Ranged attackers can engage through
clear terrain and receive the same warnings as melee attackers.

| Phase | Health | Attacks |
| --- | --- | --- |
| Hoard Defender | Above two-thirds | Two-action fire breath; one-action claw/tail sweeps when nearby |
| Scorched Wings | Two-thirds or below | Alternating breath and two-action wingbeat landings |
| Last Inferno | One-third or below | Two linked breaths in different directions, each with its own two-action warning |

Fire breath starts as a narrow cardinal or diagonal cone and widens to three tiles across,
up to twelve tiles from the mouth. Numbered outlines mark its exact cells
before it fires. The flame animation follows the declared route. Walls and
basalt clip the cone when it is declared; removing cover can never extend that
saved footprint. Close claw/tail sweeps cover the eight neighboring cells and
allow one ordinary move to retreat. Each normal signature leaves one full
counterattack action before another declaration.

Wingbeat marks a five-cell landing cross centered on the player's current
position. Walk twice to escape before impact. The dragon lands on the vacated
center only if no player or living enemy occupies it; it remains vulnerable
throughout the warning. Eligible ground within the cross burns for **three
future actions**, marked **H3 → H2 → H1**. Fire damage begins on the following
action, then expires, restoring the exact snow, ash or hoard terrain, including
under stacked loot. Existing goblets, entrances, exits, cover and reward caches
are never replaced by burning terrain. Landings stay inside the lair.

Last Inferno's second cone turns a quarter-turn from the first. It is declared
only after the first resolves and allows two new escape actions. Neither
warning follows the player. After the second breath, the dragon allows **two
full counterattack actions**. Health thresholds wait for a new sequence;
healing cannot reverse a latched phase. Freeze pauses the queued attack and
any future landing fire, while existing fire continues to expire.

Two stable basalt columns (**S**) remain as permanent cover. Two weak columns
(**W**) crack on the first warned breath impact, becoming **!**. A later warned
breath breaks them into walkable rubble. **C1/C2** identifies a column threatened
by the next breath. Cracking or breaking never spreads the current cone into
unmarked cells. Column placement requires an open ring of surrounding floor,
preserving existing walking routes and the goblet's preferred location.
Ordinary movement is sufficient for every class. An attack with no safe walking
escape through current cover and fire is withheld instead of forcing damage.

The Red Dragon's reliquary offers **one** armor reward. Open it with Interact
and press Pick Up on its released loot to choose:

| Armor | Class | Bonuses |
| --- | --- | --- |
| Dragonplate | Warrior | +12 defense, +60 maximum HP |
| Dragonscale Leathers | Rogue | +8 defense, +25 maximum HP, 15% evasion |
| Emberweave Robes | Mage | +5 defense, +70 maximum MP, 20% cheaper spells |

Esc postpones the choice. A full inventory preserves the unclaimed reward;
claiming any one armor consumes the choice permanently. The boss's gold, XP,
score and combat behavior are unchanged. Legacy saves with a previous victory
or pending mantle retain the original all-class Dragon Scale Mantle instead.

## The Dragon's Binding and The Dragon's Hoard

**Runewright Edric**, in Ridgeshire's Runewright's Lodge, offers **The Dragon's
Binding**. Defeating the Red Dragon records its runic shackle in Evidence and
moves the quest directly to Completed, even without acceptance or a free quest
slot. Return to Edric, choose the investigation topic, and collect **350 gold
once**. No inventory space is needed for the shackle.

**Ilya** offers **The Dragon's Hoard** in **Ridgeshire's Tavern & lodgings**.
Find the golden goblet in the stage-5 lair and press Pick Up on it. The goblet
exists without accepting the quest, needs no bag slot, and marks the quest
Completed on collection. Return to Ilya here for **300 gold once**. Oakhaven is
no longer a turn-in location. The goblet, binding evidence and armor choice
are independent rewards. All three rewards remain available after travel and
save/load.

Archivist Oswin in the Town Hall compares the shackle with Ironfang's
requisitions and the royal crown inventory. Either area can be completed first;
neither boss requires a quest. See [Ridgeshire's story](ridgeshire-arc.md).

### Earlier save formats

Save version **136** adds one initialized, serialized mantle-recovery state
and two compact enemy bitfields for the exact warned cells. Existing enemy
fields store phase, signature, facing, footprint origin, landing target,
sequence and recovery. Enemy serialization covers both the active encounter
and every level cache; map IDs preserve column damage, fire duration and
original terrain. Old saves reset obsolete attack timing and add cover while
preserving health, freeze, enemy deaths, exploration, loot, goblet progress and
portal destinations. Legacy dragons that pursued beyond their lair return to
it without healing. Existing unclaimed mantle drops remain recoverable;
previous victories without such a drop receive no replacement equipment.
Town portals ward a threatening signature and clear only their landing's fire.

Save version **148** stores movable boulders and their underlying snow or ash
in map tiles, alongside the new ravines. Older active and cached stages gain
broader ledges and connecting routes without regenerating maps or moving
actors. New boulders occupy added ground, preserving original walking squares.
The last clearing, goblet progress, boss warnings, enemy deaths, loot,
exploration and portal destinations remain intact.
