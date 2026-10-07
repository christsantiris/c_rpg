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
| Thorn Regent | 5 | Guards the final clearing; alternates gathering power and firing thorn bolts |

The Regent drops a Potion of Strength, which permanently grants 1 base attack
when consumed. Its defeat is recorded in the boss journal and survives later
visits. Surviving ordinary enemies do not block the final exit after victory.

Moonveil has its own stage caches and depth tracking. Backtracking and returning
through Rosemoor's west gate retain explored maps and enemy state. Return to
Town opens a portal beside that gate, restoring the original garden stage when
used. Saves preserve active and cached maps, enemy health, explored terrain,
portals, and the Regent's defeat. Older testing saves migrate to fresh Moonveil
progress without resetting existing character, quest, or regional progress.

## The Stolen Moonseed

**Botanist Liora** waits in **Oakhaven's Tavern**. Approach her and press `T` to
accept the quest. She once cared for Moonveil's gardens, but Fey Tricksters
stole their last viable Moonseed. She explains the route through the forest
to Stillbury, across the swamp to Rosemoor, and through Rosemoor's west gate.

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

Return to Liora in **Oakhaven's Tavern** for **90 gold and 700 score**, awarded
once. The journal tracks the seed, moonwater, and planting separately and
retains all three objectives after collecting the reward.

Acceptance preserves explored gardens, enemy health, boss victories, and
existing portals. Quest encounters appear when their stages are visited;
cached encounters preserve damaged or defeated quest defenders. Save version
**95** records quest state, objective progress, guard encounters, and the
restored terrain. Older saves gain an unassigned quest without losing existing
progress. Saves inside the Tavern receive Liora, with any overlapping player
or loot moved safely beside her.
