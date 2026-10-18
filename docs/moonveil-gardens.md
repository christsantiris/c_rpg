# Moonveil Gardens

Rosemoor's west gate enters the five-stage Moonveil Gardens. The gate opens as
soon as Rosemoor is accessible. Stages run east to west: the east entrance
backtracks, and the west exit advances without requiring every enemy to die.
Stage 1's east entrance returns to Rosemoor's west gate. The Thorn Regent blocks
stage 5's west exit until defeated; afterward it also returns to Rosemoor.

Giant flowers, glowing mushrooms, and tangled hedges mark impassable terrain.
Moonlit pools are also impassable. Mossy paths and ancient stone circles are
walkable. Gentle flower movement and water ripples animate while standing still.

| Enemy | First stage | Behavior |
|---|---|---|
| Giant Moth | 1 | Closes distance quickly, moving twice per turn |
| Fey Trickster | 1 | Fires magical sparkles on alternating turns |
| Carnivorous Flower | 2 | Advances slowly and fires ranged pollen |
| Thorn Guardian | 3 | Slow, armored melee enemy |
| Thorn Regent | 5 | Fixed thorn lances and root crosses; below half health, warned briar regrowth reshapes the flanks |

The Regent guarantees a **Briarweave Cloak** and the original **Potion of Strength**.
The rare cloak is all-class light armor with **+4 defense and 10% evasion**. The
potion still permanently grants 1 base attack when consumed. Both wait in a
flowered reliquary: use `A` beside it, or stand on it and press `P`. A full pack
leaves both reserved; one free slot collects the cloak and leaves the potion
available. Full floor loot never discards existing objects, and both commands
reach the cache beneath other loot. Pending rewards persist through travel and
saving; collecting, then consuming, selling or dropping them never awards a
replacement. Its defeat is recorded in the boss journal and survives later
visits. Surviving ordinary enemies do not block the final exit after victory.

## Thorn Regent encounter

**Garden Regent**, above half health, alternates a one-action thorn lance with a
two-action creeping-root cross. Both lock their original targets and use the
same affected-cell predicate for warnings and damage. Hedges, pools, living
actors and temporary thorn walls block the lance. Removing cover cannot expose
an unmarked target. Two ordinary steps leave the root cross before it erupts;
**R2/R1** marks lingering roots on mossy floor for two further actions. Existing
stone circles, quest props and flowering nodes retain their terrain.

**Briar Crown**, at or below half health, adds a two-action **G2/G1** regrowth
warning after roots. A fixed three-cell strip beside a flowering node grows
temporary **W4/W3/W2/W1** thorn walls. Regrowth itself causes no direct damage.
It skips occupants and loot, respects the original warning mask, and checks
connectivity both before the warning and before growth. It never seals an
existing route through the connected garden. Strips alternate between two
flowering nodes and expire after four actions.

All classes can stand on or beside a flowering node and tend it with `A`. This
clears its adjacent grown strip, opening a flank toward the Regent. Tending the
node during its own regrowth warning interrupts that growth and leaves two
complete counterattack actions. Tending a different node cannot cancel an
unrelated warning. The node rests for four further actions; **N4/N3/N2/N1**
shows its remaining cooldown. It can be used again after resting. No spell,
ammunition, consumable or Moonseed quest progress is required.

Each completed signature leaves at least one whole counterattack action without
another Regent attack. Damage crossing the health threshold cannot widen a
queued signature. Freeze pauses its exact countdown; already active roots,
walls and node cooldowns continue to expire, including beneath loot. Retreat
beyond the old activation range pauses the boss sequence. Return portals ward
threatening landing warnings and clear lingering roots on the landing cell.
The boss's existing health, base attack, defense, XP, score and regular roster
remain unchanged. Liora's seed pod, spring, planting circle and restored
Moonflower are independent of the stage-5 arena nodes.

Save version **132** persists a reward mask for the unclaimed cloak and potion.
Existing enemy serialization and appended tile IDs preserve phase, countdown,
targets, growth mask, node selection, freeze, recovery, root/wall durations and
flower cooldowns in active and cached maps. Version-131 and older saves migrate
the living arena in place, resetting obsolete attack timing while preserving
damage, deaths, exploration, loot, quest progress and portal destinations.
Previously defeated Regents remain defeated and retain their original potion
drops; migration does not grant repeat rewards. See the
[native renderer gallery and play-test guide](art-sources/thorn-regent-encounter/README.md).

Moonveil has its own stage caches and depth tracking. Backtracking retains enemy
state. Returning through Rosemoor's west gate starts a new expedition with
regular enemies restored on every visited stage; explored maps, quests, and
boss progress remain intact. Return to
Town opens a portal beside that gate, restoring the original garden stage when
used. Saves preserve active and cached maps, enemy health, explored terrain,
portals, and the Regent's defeat. Older testing saves migrate to fresh Moonveil
progress without resetting existing character, quest, or regional progress.

## The Stolen Moonseed

**Botanist Liora** waits in **Rosemoor's town center**. Approach her and press `T` to
hear the offer, then `Y` to accept the quest. She once cared for Moonveil's
gardens, but Fey Tricksters stole their last viable Moonseed. She explains the
route through Rosemoor's west gate.

| Stage | Objective | Defenders |
|---|---|---|
| 2 | Recover the Moonseed from a stolen seed pod | One Fey Trickster and two Giant Moths |
| 3 | Gather moonwater from a marked spring on a pool's bank | One Carnivorous Flower |
| 4 | Plant and water an ancient stone circle | Two Thorn Guardians |

Defeat the objective's quest defenders and nearby threats, then press `A` while
standing on or beside it. Gather the seed and moonwater in either order; both
are needed to restore the stage 4 circle. The moonwater spring is reachable
from walkable ground, so you never need to enter its impassable pool. The
objects use no inventory slots and can be collected with a full inventory.

Planting replaces the tangled circle with an animated Moonflower. Nearby
growth recedes and blossoms spread over the restored clearing. The flower and
clearing remain visible when backtracking, returning through portals, or
visiting again. The Thorn Regent on stage 5 is optional for this quest.

Return to Liora in **Rosemoor's town center** for **90 gold and 700 score**, awarded
once. The journal tracks the seed, moonwater, and planting separately and
retains all three objectives after collecting the reward.

Acceptance preserves explored gardens, enemy health, boss victories, and
existing portals. Quest encounters appear when their stages are visited;
cached encounters preserve damaged or defeated quest defenders. Save version
**95** records quest state, objective progress, guard encounters, and the
restored terrain. Older saves gain an unassigned quest without losing existing
progress. Save version **105** moved Liora to Stillbury's Inn without resetting
her quest or garden exploration. That migration moved overlapping players and
loot beside her and removed her former Tavern tile.

Save version **114** moves Liora from the Inn to Rosemoor's town center,
preserving her quest and garden progress. Saves inside Rosemoor gain Liora,
with overlapping players and loot moved safely beside her.
