# Dragonspine and High Pass

The Goblin Mountains connect OakHaven and Ridgeshire across seven stages, with
the Goblin King at Crown Peak on stage 4. His defeat opens a shortcut to the
opposite town; continuing through the remaining stages is also possible.
Ridgeshire's north gate leads along a safe approach to Dragonspine, placing its
snowy peaks at the northern edge of the world alongside Frostfell Wastes.
Leaving the first stage or completing the summit returns to that approach;
walk south to reach Ridgeshire's north gate. Return to Town travels directly
to town, with its portal beside the north gate.
Save version **163** relocates existing approach paths and town portals to this
gate while preserving exploration, loot, combat maps, and quest progress.

Ridgeshire's separate south shortcut gate leads into the enemy-free, one-tile-wide High Pass. Walk south to reach
OakHaven's northeast shortcut, or north from OakHaven to revisit Ridgeshire without
crossing the Goblin Mountains. A matching stone gate marks the Ridgeshire end of
the shortcut, which opens after the Goblin King falls.
OakHaven's main north gate still starts a Goblin Mountains expedition.
Ridgeshire's ordinary south mountain gate permits entering from the other end,
even before the Goblin King is defeated.

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
| 2 | High cliff paths | Goblin Archers, Drakes, Giants |
| 3 | First ash fields | Goblin Archers, Drakes, Giants, Fire Elementals |
| 4 | Burning ascent | Goblin Archers, Drakes, Giants, Fire Elementals |
| 5 | Dragon's lair | Full roster and the Red Dragon |

Drakes close for melee, Giants endure heavy punishment, and Fire Elementals
cast along clear rows or columns. As in other regions, stages 1–4 can be
exited without defeating every enemy. On stage 5, only the Red Dragon blocks
the summit exit. It drops gold and the all-class Dragon Scale Mantle when
defeated.

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

The original **Dragon Scale Mantle** still drops on victory: rare all-class
light armor with **+4 defense, +20 maximum HP and value 1,400**. The existing
gold reward is unchanged. A full loot floor reserves the mantle in a hoard
reliquary without deleting ordinary drops; unclaimed equipment left behind
also becomes recoverable on return. Press **A** beside the cache or **P** on it,
including beneath loot. A full inventory cannot consume it. Claiming restores
the cache's original snow, ash or hoard ground. Once claimed, selling or
dropping a mantle cannot generate a replacement. The summit exit still opens
on the dragon's defeat even if ordinary enemies survive.

See the [native renderer gallery and play-test guide](art-sources/red-dragon-encounter/README.md).

## The Dragon's Hoard

Ilya waits in **Oakhaven's Tavern**, alongside Elowen and Mara. Press
`T` beside her to hear the offer, then `Y` to accept **The Dragon's Hoard**. On
Dragonspine stage 5, find the golden goblet in the dragon's lair, stand on it,
and press `P` (Pick up). Return to Ilya in the Tavern to trade the goblet for a one-time
Potion of Strength and 600 score. Make space in your pack before turning in the
quest. Drinking the potion permanently raises base attack by 1; it is not sold
in shops. The quest can still be completed if the Red Dragon was defeated before
speaking with Ilya.

Her directions lead north through the mountains to Ridgeshire, then north
through its Dragonspine gate. Save version **102** moves Ilya from saved
Ridgeshire maps into saved Tavern interiors while preserving the goblet,
quest completion, boss victories, equipment, and money. Overlapping players
and dropped items move one tile south of her new Tavern position.

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
