# Sunscar Wastes

Stillbury's west gate enters Sunscar Wastes. Its five stages run from east to
west; backtracking uses the east edge. The first stage's east exit and the final
stage's west exit return to Stillbury. The Desert Pharaoh does not block the final exit
and drops the Sandstorm Staff.

The wastes are drawn at the Crown Roads' 1.5× zoom: the camera shows about 30 ×
18 tiles around the player. The Desert Pharaoh acts within twelve tiles, so it
may begin its telegraphed beams from just off screen.

Incoming combat damage is reduced by **15% after defense**, rounded to the
nearest whole HP with a minimum of 1, before shield blocks. This includes the
Pharaoh's attacks and lingering sand. Enemy health, patrol counts, attack
patterns, poison and rewards are unchanged. The reduction applies immediately
to existing saves to ease potion use during longer expeditions.

## Dune basins and oasis wells

Dune basins are broken up by broad sandstone mesas, staggered rock spines and
smaller outcrops. Their irregular edges bend routes, block lines of fire and
provide cover between fights. Multiple sandy approaches remain open; each
obstacle cluster is checked before placement so it cannot trap a walking tile,
well or exit. Outcrop positions vary with the stage seed. The final clearing
still supports the Pharaoh's obelisks, warnings and reward cache.

Stages 1–5 now have **22, 26, 30, 34 and 38 enemies** respectively, including the
Pharaoh when alive on stage 5. The existing stage progression introduces
Scarabs and Vipers first, followed by Mummies, Djinn and Golems. The larger
patrols and tighter approaches make reaching and rationing oasis water useful.
Quest encounters retain their separate guard reservations.

Each stage has one turquoise oasis well. Stand on or beside it and use the
configured **Interact** key (**A** by default) to restore **25% of maximum HP
and MP**, capped at each maximum. Drinking takes an ordinary action and works
for every class. A full-health, full-mana player leaves the water unused. After
drinking, the well reads **DRY** until a new expedition from town; backtracking,
portals and saving do not refill it. Starting a new expedition refills wells on
all cached stages along with restoring ordinary enemies.

Save version **146** stores full/dry wells directly in active and cached map
tiles. Older maps gain broader approaches and a well in place, preserving their
existing paths, exploration, quest objects, loot, enemies and portal landings.
The last chamber's terrain and pending boss attacks stay intact.

Save version **147** adds mesa cover to older active and cached Sunscar maps
without covering players, living or fallen enemies, loot, portal landings,
wells or quest objects. The final chamber keeps its terrain, and exploration
and current expedition deaths and wounds remain unchanged. Existing sparse
stages gain their larger regular patrols on the next expedition from town;
loading alone does not repopulate cleared stages. Expanded cache capacity
preserves the larger rosters through travel and saving.

Impassable terrain appears as connected sandstone shelves with layered cliff
faces and eroded corners. Scattered spires and boulders retain their varied
shapes, while tall and barrel cacti form small stands. Curved dune ridges cross
tile boundaries on the open sand, with occasional pebbles and dry scrub.
These details stay fixed when moving, revisiting, or loading. Existing saved
maps retain their progress during the layout migration. See
the [renderer comparisons](art-sources/sunscar/README.md).

Blowing sand and a light amber haze animate across the play area, including while
standing still. The sandstorm is visual and does not affect combat or movement.

## Desert Pharaoh encounter

The Pharaoh retains its existing health, base attack and staff bonuses. Four impassable sandstone obelisks appear in the final clearing without
replacing its rocky/cactus terrain or closing the entrance and exit routes.

**Sun-Crowned**, above half health, alternates a one-action curse beam and
two-action burial-sand cross. The beam locks a line to its original target;
obelisks, sandstone and living enemies block it. Removing cover after a warning
cannot create an unmarked target. Burial sand erupts only on the marked cross
and leaves **P2/P1** patches for two further actions. Walk two steps out of the
cross; neither spell nor consumable is required. Lingering sand uses 8 base
damage with half defense and the existing evasion/shield rules.

**Sandbound**, at or below half health, adds a two-action storm band after burial
sand. Each three-tile-wide band has a fixed, fully numbered footprint. Successive
storms move from one arena edge through the center to the opposite edge, then
rotate wind direction through east, south, west and north. The arrows and panel
show the locked wind. Obelisks cast two sheltered **S** tiles downstream; these
shadows move with each newly announced direction. Shelter prevents storm damage
and never protects against burial sand. Moving outside the strip also works.
The ambient visual sandstorm remains independent of these damaging signatures.

Attack warnings allow ordinary walking escapes. A phase threshold cannot change
an already queued attack. Freeze pauses its exact countdown. Each resolution
leaves one complete counterattack action without another Pharaoh attack. Retreat
beyond the original activation range pauses his attack sequence, while existing
burial sand still expires. Portal return wards a threatening landing warning and
clears lingering sand on the landing tile.

The guaranteed **Sandstorm Staff** keeps its original two-handed Mage stats,
class restrictions and art. A full loot floor reserves it in a sandstone
reliquary without discarding existing objects. Interact beside it with `A`, or
stand on it and press `P`; full inventory leaves it reserved. Both commands work
when other loot covers the cache. Unclaimed rewards survive portals,
backtracking and saving. Claiming, then selling or dropping the staff never
creates a replacement.

Save version **131** persists the reward's earned/unclaimed/claimed state.
Existing enemy serialization and appended map tiles preserve targets, phase,
wind, storm sequence, freeze, recovery, obelisks, patch clocks and loot underlays
in active and cached maps. Older saves migrate in place: obsolete Pharaoh attack
timers reset, while health, deaths, exploration, quest objects and portal
destinations remain intact. The existing unclaimed-staff flag preserves reward
availability after an old victory. See the
[native renderer gallery and play-test guide](art-sources/desert-pharaoh-encounter/README.md).

## The Lost Magic Lamp

Enter Stillbury's Inn. Approach Zara and press `T` to hear her offer, then `Y`
to accept her quest. Travel to Sunscar through Stillbury's west gate and find
the golden magic lamp in the last clearing of stage 4. Stand on it and press
`P` (Pick Up) to recover it, then return to Zara in the Inn.

The one-time reward is **80 gold and 600 score**. The lamp uses quest state rather
than inventory space, so a full pack does not prevent recovery. The journal
tracks the objective and marks it ready to return after pickup.

Accepting the quest preserves existing desert exploration, enemies, and portals.
The quest works after the Pharaoh has been defeated. Saving preserves the Inn,
quest progress, and reward completion; older testing saves start with the quest
unassigned.
