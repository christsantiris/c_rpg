# Ashen Hollow

Ridgeshire's north gate opens onto a five-stage volcanic region as soon as the
town is accessible. Enter each stage from its south edge and advance through
the north exit. The south entrance backtracks; on stage 1 it returns to
Ridgeshire's north gate. Ordinary enemies do not block stage travel.

The Cinder Lord guards the last clearing on stage 5. Its defeat opens that
stage's north exit, which returns to Ridgeshire without introducing another
town. The boss guarantees **Embercleaver** and the original **Potion of Strength**,
granting 1 permanent base attack when consumed. Its defeat appears in the boss
journal. Surviving ordinary enemies do not block the return exit after victory.

Ashy paths wind through irregular clearings, basalt columns, charred trees,
and ruined stonework. Rock formations and animated lava pools block movement.
Broken paving is walkable. Lava occupies previously solid rock, preserving
connected paths; it does not inflict damage. Embers and lava animate while idle.

| Enemy | First stage | Behavior |
|---|---|---|
| Cinder Imp | 1 | Fires magical firebolts on alternating turns |
| Ash Hound | 1 | Closes two tiles per turn, without attacking on its second move |
| Obsidian Guardian | 3 | Slow, armored melee enemy |
| Cinder Lord | 5 | Warned cinder lances and eruptions; below half health, expanding ember rings alternate with eruptions |

Ashen Hollow maintains independent caches for all five stages. Backtracking
and later visits preserve explored terrain, enemy health, and boss progress.
Return to Town opens a portal beside Ridgeshire's north gate and restores the
original stage and position when used. Save format 81 stores the new region;
older saves gain fresh Ashen Hollow progress and an open north gate without
resetting existing characters, quests, or regional progress.

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
northeast of the crossroads. Approach him and press **T**. The town wants to
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
