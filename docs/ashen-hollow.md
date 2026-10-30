# Ashen Hollow

A path around the western side of Castle Grounds reaches the north gate and
the five-stage volcanic region behind the castle. Beyond the gate the approach
road leaves green woods for charred dead trees, then basalt split by lava pools
around the entrance; see [road scenery](road-scenery.md). Enter each stage from
its south edge and advance through the north exit. The south entrance backtracks;
on stage 1 it returns to the approach back to Castle Grounds' north gate. Ordinary
enemies do not block stage travel.

The hollow is drawn at the Crown Roads' 1.5× zoom: the camera shows about 30 ×
18 tiles around the player. The Cinder Lord acts within twelve tiles; the camera leans toward him to keep him on screen as his telegraphed eruptions begin.

The entrance is a lone, towering fire mountain. A lava river pours from its
notched crater and splits around a jagged, fire-lit cleft at its foot, while
smoke rolls east from the summit. The mountain fills a 7×9 tile footprint;
save version 217 moves anyone saved on its newly raised summit onto the road.

The Cinder Lord guards the last clearing on stage 5. That stage's north exit
remains open while he lives and returns to the approach back to Castle Grounds
without introducing another town. See [boss gates](boss-gates.md). The boss guarantees **Embercleaver** and the original **Potion of Strength**,
granting 1 permanent base attack when consumed. Its defeat appears in the boss
journal. Surviving ordinary enemies do not block the return exit after victory.

Broad basalt terraces and linked bridge routes cross three winding lava
rivers. Stages run through branching shelves rather than a chain of swamp
clearings. Broken paving remains walkable; native lava and basalt walls block
movement. Embers and lava animate while idle.

## Volcanic crossings

Each stage offers short crossings with cycling heat and longer permanent
basalt routes. Stage 1 introduces one crossing, stage 2 has two, and stages
3–5 have three. Every exit remains reachable while avoiding every cycling tile.

- Blue **C6–C1** marks the cooling phase. Orange **!2–!1** gives two full actions
  of warning before the crossing flares.
- Red **H2–H1** is hot for two actions, then cools again. Standing on a flaring
  tile deals 8 base damage, reduced by half defense with a minimum of 2 before
  the existing evasion and blocking rules. Ordinary movement can leave a warned
  crossing; no spell or consumable is required.
- Stand on or beside the nearby blue diversion valve and use the configured
  **Interact** key (**A** by default) to divert the lava away from its crossing.
  This consumes a normal action and works for every class. The crossing becomes
  stable basalt, marked with green checks; the valve turns green and reads
  **ON**. It stays safe without a countdown, so players can fight or explore
  before crossing. Reusing the valve does not restore the lava.
- Diversions survive backtracking, new expeditions and saving/loading, just as
  explored terrain does. Other crossings keep cycling until their own valve is
  used. These valves do not change the boss arena or quest mechanisms.
- Clocks advance with ordinary enemy phases, including when no enemies remain.
  Menus and idle animation do not advance them. Loot does not stop a clock;
  escaping archives the crossing state without providing a return anchor.

The status panel explains the three states. Valves, marked crossings and
permanent bridge routes are independent of the Emberforge quest and the
Cinder Lord's arena sluices.

| Enemy | First stage | Behavior |
|---|---|---|
| Cinder Imp | 1 | Fires magical firebolts on alternating turns |
| Ash Hound | 1 | Closes two tiles per turn, without attacking on its second move |
| Obsidian Guardian | 3 | Slow, armored melee enemy |
| Fire Elemental | 3 | Recurring Dragonspine creature with alternating ranged fire strikes |
| Cinder Lord | 5 | Warned cinder lances and eruptions; below half health, expanding ember rings alternate with eruptions |

Ashen Hollow maintains independent caches for all five stages. Backtracking
preserves enemy health. A new expedition restores regular enemies on every
visited stage while preserving explored terrain, quests, and boss progress.
Return to Town is a one-way escape to the last town visited, with no return
portal. Save format 81 stores the new region;
older saves gain fresh Ashen Hollow progress and an open regional gate without
resetting existing characters, quests, or regional progress.

Ashen Hollow occupies a snow-rimmed volcanic caldera north of Castle Grounds.
Existing stages, combat progress and return portals survive relocation.

Save version **145** also stores diverted valves and stable basalt crossings
in the map tiles. Version 144 crossings and valves remain usable with the new
mechanism, without regenerating terrain or resetting progress. Version 144
introduced crossing clocks in the saved map tiles. Older
active and cached maps gain additional loops, crossings and valves in place.
Existing walking squares, player and portal positions, loot, exploration,
enemy wounds and deaths, quests and boss victories are preserved. The final
chamber's existing terrain and pending boss attacks remain intact. Newly
generated stages use the terraced river layout.

## Cinder Lord encounter

**Furnace Heart**, above half health, alternates a one-action cinder lance with
a two-action lava-eruption cross. Both lock their original targets. Basalt,
native lava pools and living actors block the lance; removing cover after its
warning cannot expose an unmarked target. Two ordinary steps leave the eruption
cross before it hits. Eruptions leave **H2/H1** heat on ordinary ash floor for two
further actions; ruined paving, channels and quest props retain their terrain.
Lingering heat uses 8 base damage, half defense and existing evasion/block rules.

**Broken Crucible**, at or below half health, alternates eruptions with an
expanding ember ring centered on the boss's locked position. Both fronts are
visible at declaration: the inner ring hits in **1 action**, and the outer ring
in **2 actions**. After the inner hit, the outer countdown becomes 1; no new
unwarned cells appear. Walk outward on each action. Hot floor expires, and a
warning with no ordinary walking escape becomes an empty attack. Each completed
signature leaves a whole counterattack action without an extra melee hit.
Crossing half health cannot change a queued attack. Freeze preserves the exact
target and remaining ring front while existing floor heat continues to expire.

Two walkable **forge sluices** each feed a three-cell channel beside the final
clearing. Stand on or beside one and press `A`. If its channel overlaps a warned
eruption, opening it redirects that eruption and exposes the Lord for **two
complete counterattack actions**. Cooling channels protect against eruptions;
they do not protect against lances or ember rings. Opening the wrong sluice or
using it during another signature cannot cancel that attack. The controls work
for every class without a spell, consumable or quest requirement.

Channels and sluices rest for four further actions, shown by blue **C4/C3/C2/C1**
and **S4/S3/S2/S1** labels, then become reusable. Existing hot tiles and control
clocks continue beneath loot. The channel layout remains walkable and preserves
the original routes to the boss and exits. These arena props are separate from
the Emberforge mechanism and repaired forge on stages 2 and 4. Native impassable
lava pools retain their original visual-only behavior.

The rare **Embercleaver** is a Warrior-only two-handed axe: **+11 attack, 25%
armor penetration and 25% cleave**, using the existing physical combat system.
It clears the off-hand slot when equipped. The Magic Battle Axe retains higher
attack and penetration, and the greatswords retain stronger cleave. The Debug
launcher accepts `--weapon embercleaver` for testing.

Embercleaver and the special Strength Potion wait in a persistent **forge
reliquary**. Use `A` beside it or `P` on it, including beneath other floor loot.
A full floor never discards existing objects. A full pack leaves both rewards
reserved; one free slot collects the axe and leaves the potion available. Each
claim persists independently through backtracking, portals and saving. Consuming,
selling or dropping claimed rewards never creates replacements.

Save version **133** adds the pending-reward mask and migrates existing living
arenas in place. Appended tile IDs and existing enemy fields preserve signatures,
phase, freeze, the half-resolved ring, recovery, heat and cooling clocks in
active and cached maps. Migration resets obsolete attack timing while preserving
health, deaths, exploration, quest progress, loot and portal destinations.
Previously defeated Lords stay defeated and retain their original potion drops;
migration never grants repeat rewards. Return portals ward threatening landing
signatures and clear lingering heat on the landing cell without advancing other
arena clocks. See the [native renderer gallery and play-test guide](art-sources/cinder-lord-encounter/README.md).

## Reclaim the Emberforge

Steward Hadrin assigns this one-time quest inside Ridgeshire's Town Hall,
northeast of the crossroads. Ashen Hollow is reached through Castle Grounds' north gate. Approach him and press **T**. The town wants to
reclaim the abandoned furnace that once supplied its miners and craftspeople.

| Stage | Objective | Added defenders |
|---|---|---|
| 2 | Recover the stolen forge mechanism in the last clearing | Two Ash Hounds and a Cinder Imp |
| 4 | Install the mechanism and repair the furnace in the last clearing | Two Obsidian Guardians and two Cinder Imps |

Stand on or beside an objective and press **A** after defeating its quest
defenders and any ordinary enemies within four tiles. Defenders must be
defeated even if lured away. Ordinary enemies elsewhere on the stage do not
block completion. The mechanism is recorded as quest progress and does not
occupy inventory space. Visiting the furnace first explains that its mechanism
must be recovered before it can be repaired.

Repairing the furnace replaces the cold hearth with an animated fire. Return
to Hadrin for **100 gold and 800 score**. Repeated conversations do not pay
again. The Cinder Lord remains a separate objective with its existing reward;
defeating it is unnecessary for this quest.

The journal tracks both objectives. Save format **88** preserves quest state,
recovered mechanism, repaired furnace, and encounter placement. Guardians do
not respawn on revisits or reloads. Accepting the quest adds its objectives to
previously explored stages without resetting terrain, enemies, or boss
progress. Older saves receive an available quest and the Town Hall; players
and dropped loot on its new building footprint move safely outside.
