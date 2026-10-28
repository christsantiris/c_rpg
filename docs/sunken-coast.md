# Sunken Coast and Flooded Ruins

The Sunken Coast is a five-stage adventure entered through the south town
gate. It has an independent level cache and progression record, so visiting
another region does not reset explored coast stages.

The coast is drawn at the Crown Roads' 1.5× zoom: the camera shows about
30 × 18 tiles around the player. Sirens and water elementals fire along lanes of
at most six tiles, so their shots start on screen; the Drowned Queen wakes
within twelve tiles and may begin her telegraphed attacks from just off screen.

## Coastal entrance path

Oakhaven's south gate leads down a safe cobblestone path to a sandy shore.
The sea fills the entire southern half of the approach, with a narrow stone
causeway reaching the drowned shrine entrance. Entering the doorway starts
Coast 1; retreat returns to the same causeway. The water beside it is impassable.

Save version 177 adds the shoreline to older approach saves, keeping players
and dropped loot on reachable ground without resetting exploration, quests,
combat caches or boss victories. The entrance sprite itself is unchanged.

## Progression

Stages 1–4 follow irregular sandbanks and broad shallow crossings around
lagoons and permanent open sea. Curving shoreline routes replace the old
single-tile corridors and square clearings. Lowering the blue basin exposes
wide tidal flats that cut across the longer shoreline routes; raising it
submerges those shortcuts again. The amber inlets flood in the opposite state,
so each tide exposes different treasure shoals.

Stages alternate among north, south, east, and west entrances and exits. A
stage never uses the same edge for both, preventing the player from immediately
walking back into the stage they just completed. Each stage has one brass tide
control and two colored channel basins. Stand on the control and press `A` to
transfer water between them: one channel drains into exposed wet sand while the
other floods. Colored corner markers identify the blue and amber channels.
The water stays in the chosen state until the player operates a control again.
Stage 5 also has two separate throne controls for the Queen's approach lanes;
these leave the ordinary basin control and earlier quest beacons intact.

The control and the main exit remain reachable at either tide level. Stages
1–4 can be exited without clearing enemies. The stage 5 exit requires only the
Drowned Queen to be defeated before returning to town.

The region moves from storm-lashed beaches into increasingly submerged ruins:

1. Shattered Strand
2. Tidepool Causeway
3. Siren Cliffs
4. Flooded Galleries
5. The Drowned Throne

## Enemy roster

- **Illusion:** a one-hit spectral decoy that still threatens careless players.
- **Merfolk:** the dependable spear-wielding frontline enemy.
- **Siren:** a fragile ranged attacker whose song strikes along clear lanes.
- **Giant Crab:** a heavily armored bruiser that advances every other turn.
- **Animated Statue:** the toughest regular defender; slow, durable, and hard
  to damage.
- **Water Elemental:** a mid-range attacker that sends water surges down clear
  lanes.
- **Sea Serpent:** a late-region predator with high attack and mobility.
- **Drowned Queen:** the stage 5 boss. She remains dormant in her throne
  clearing until the player enters or damages her from range, then alternates
  fixed tidal lanes and, in her second phase, flooded pockets.

## Visual identity

Dry sandbanks use muted sand colors, ripple ridges, shells, kelp and driftwood.
Exposed flats are darker wet sand with blue or amber channel markers. Pale
teal shallows are walkable, while flooded channels are deeper blue and the
permanent open sea is darkest. Foam traces the shore, separating land from
water. Remaining ruins retain algae-covered stone and glowing entrance arches.

A Coast status panel on stages 1–4 shows which flats and inlets are exposed and
reminds the player that sand and pale shallows are walkable. Dark open sea
stays impassable in both tide states; only marked tidal water drains. Activating
the tide control changes the actual map tiles, so the current tide remains intact
when the player backtracks, returns through a portal, or reloads a save.

Rising water moves enemies out of a flooding channel to an open bank and
temporarily submerges dropped items until that channel is drained. Using a
control reveals its connected channels on the minimap.

## Treasure and Coast Quest

Every stage has two guarded rocky treasure shoals at opposite tide levels. Each
cache grants `20 + 4 × stage` gold once. Stand on the cache and press `P` to
collect it.

Mara assigns **Relight the Drowned Beacons** in **Oakhaven's Tavern**.
The quest places beacons on stages 2, 3, and
4. Drain the blue channel to reach each beacon, then stand on it and press `A`.
The beacons are guarded by groups led by a Giant Crab, Animated Statue, and Sea
Serpent. Accepting the quest starts a fresh coast expedition; a previously
defeated Drowned Queen remains dead.
Return to Mara in the Tavern for 80 gold and 600 score once all three are lit.

Save version 84 compresses the former eight-stage coast into five stages.
Existing beacon completion, boss victories, character progress, and active
coast terrain are preserved. Explored stages are merged, prioritizing the
current stage and portal destination. The old stage-1 beacon moves to stage 2;
its completed state carries over.

Save version **143** widens existing Coast routes into banks and adds tidal
flats in place. Existing occupied squares and landmarks stay intact, including
player and portal positions, enemy wounds and deaths, beacon progress,
collected caches, dropped loot and exploration. New flats match the saved tide.
Active and cached maps are upgraded; the Queen's throne and pending attacks
retain their terrain and state. Fresh expeditions keep normal replay behavior.

## The Drowned Throne

The Queen retains her baseline **220 HP, 22 attack, 8 defense and 650 experience**
with existing regional scaling. She stays in the throne clearing. Entering it
or damaging her from a clear ranged lane starts the encounter; retreat beyond
her range pauses the exact pending warning without healing her.

- **Rising Tide, above half health:** two-turn tidal waves alternate one fixed
  horizontal row with one fixed vertical column. Each receives its own warning.
  One ordinary sidestep can leave a lane; the extra turn permits an approach or
  counterattack. Walls clip the lane, and its bounds are saved when declared.
- **Storm Throne, at/below half health:** the sequence becomes row, column,
  then up to two fixed flooded pockets. Every attack has a separate two-turn
  countdown and a recovery turn. Each pocket covers its center and four
  cardinal neighbors; two cardinal moves can leave it. Floods strike once and
  leave no lingering damage or blocked floor.
- Numbered outlines show the exact affected tiles; the panel names the phase,
  current attack, countdown, freeze state and unoperated controls. Crossing
  half health never changes an already announced attack. Freeze pauses its
  countdown while retaining its targets and wave bounds.
- Recovery has no extra adjacent Queen attack. The Queen remains damageable
  throughout the encounter, including when controls are ignored.

The two throne controls are labeled **H** and **V**. Interact on or beside
one with contextual **`A` by default** to drain its horizontal or vertical
approach lane into permanent dry stone. All classes can do this without items,
spells or ammunition. Draining immediately removes those cells from a pending
warning; it never widens or retargets the rest of that attack. The second control
does not reflood the first lane. Paths and controls remain walkable and persist
through backtracking, portals and saves, including when covered by loot.

There are no permanent **SAFE** islands. Avoid warned attacks by moving or
draining an approach lane. The dry paths and control platforms are safe from
these attacks; they do not provide protection against other enemies. Neither control
creates deep water, moves the player or overwrites exits, old basin controls,
treasure caches or beacons. The ordinary blue/amber tide control keeps its old
behavior. Hidden random traps are excluded from the throne; previously revealed
traps in older saves retain their state.

Save version **141** replaces the former shelter islands with ordinary throne
water in active and cached maps, including loot and portal underlays. Enemy
health, queued attacks, quest progress, rewards and player position are preserved.

**Tidecaller Robes** remain the guaranteed reward, with +3 defense, +30 maximum
MP, 8% spell-cost reduction and the Mage restriction. They normally drop on the
floor. A full loot list reserves them in a named throne reliquary without deleting
another item. An earned, unclaimed reward is restored there after a floor refresh.
Interact beside the reliquary or press `P` on it; full inventory leaves it
available. First claim is permanent, so later sale or dropping creates no copy.
Victory still opens the coast exit and road to the docks; existing island travel
requirements remain in effect.

Save version **127** records robes claim state and uses the existing enemy fields
for phase, countdown, fixed wave bounds and both pocket targets. Throne terrain,
drained controls, dry paths and loot underlays are saved in active and cached
maps. Older living encounters gain the arena in place and restart unmarked
attack timing with a visible warning, preserving health, enemy deaths, position,
exploration, inventory, loot, portals and quests. Existing victories remain
completed; only a known current legacy floor reward establishes unclaimed robes.

See the [encounter screenshots and play-testing guide](art-sources/drowned-queen-encounter/README.md).
