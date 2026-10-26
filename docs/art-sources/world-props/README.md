# World prop sprite review

This pass refines interactive tile props and small furnishings with native
24×24 pixel art. Outlines, shaped silhouettes, material shading, wood grain,
metal fittings, cracks, cloth folds and readable symbols replace flat blocks.

## Coverage

| Group | Updated props and states |
| --- | --- |
| Loot and rewards | Item bags, coin piles, combined loot, crypt/mountain caches, coast caches, temple treasure, Dragon Goblet, desert lamp, labyrinth relic |
| Doors and navigation | Dungeon and crypt doors/keys, mountain gate, Dragonspine entry arch, tavern exit, return exits, portals, up/down stairs, sealed/open dungeon return stairs, labyrinth exits/stairs/gates/switches |
| Temple | Entry steps, sun/moon altars, closed moon door, vault door, active/spent solar plates, dormant sentinel, rubble |
| Forest and coast | Real/false forest markers, sluice control, intake and connected conduits, tide control, lit/unlit beacons |
| Castle | Active/spent wards, muster panels, banquet tables, bookcases, throne, pillars, banners, closed/raised gates, levers, open trap hatches, stairs |
| Regional quests | Frost journal; Glassdeep dark/lit resonators; Emberforge mechanism and cold/lit furnace; cold/lit watchfires; ossuary and memorial braziers; burial ledger, plate and sarcophagus |
| Interiors and goods | Tavern tables; jail bars, hatch, tunnel exit and straw bed; bread, meat, candle and spice displays |

Open moon doors still show walkable floor. Closed/raised gates, bright/spent
runes, and lit/cold mechanisms keep their existing state distinctions. The
castle fire warning triangle, hidden traps, and combat overlays retain their
existing art. Three resonator pedestal marks remain visible when unlit.

Existing town buildings, large island artwork, road signs, regional entrance
structures, and terrain features retain their established art. No buildings
were redrawn in this tile-prop pass.

## Art and rendering

The 64 native silhouettes/frames are in
[`world_prop_art.h`](../../../src/renderer/world_prop_art.h).
[`world_props.h`](../../../src/renderer/world_props.h) declares drawing identities
that are separate from saved tile IDs. `draw_world_prop()` in
[`sprites.c`](../../../src/renderer/sprites.c) supplies the shared material palette
and per-object accent colors. Transparent pixels preserve the terrain underlay;
row runs reduce the number of SDL drawing calls.

Existing callers in `sprites.c`, `game_renderer.c`, `castle_renderer.c`,
`jail_renderer.c`, and `town_life_renderer.c` select the appropriate art. Region
floors, adjacent door supports and conduit endpoints remain in their existing
rendering paths. The flame silhouettes are new; their original seeded phases
and animation intervals are preserved, as are spring glints, flower pulses,
resonator halos and furnace flicker.

## Review images

Each comparison shows the previous sprite on the left and the updated sprite
on the right, both enlarged with nearest-neighbor scaling and at native size.

- [Loot, doors and navigation](loot-and-doors.png)
- [Furniture, castle fixtures, jail and shop goods](fixtures-and-goods.png)
- [Traps, temple, forest and coast props](traps-and-regional.png)
- [Regional quest props through the game renderer](quest-props.png)
- [Animation states at the existing intervals](animation-states.png)
- Actual game-renderer views: [castle](castle-game.png),
  [temple](temple-game.png), [coast](coast-game.png),
  [catacombs](catacombs-game.png)

These are controlled renderer fixtures, including the player for scale, rather
than quest playthroughs. Comparisons cover 73 public/furnishing state views and
20 regional quest states; unchanged warning/open-door states are included for
context.

## Validation

- Release and Debug builds passed.
- A temporary AddressSanitizer/UndefinedBehaviorSanitizer harness rendered the
  73 prop views and ten regional scenes at eight clock samples per scene.
- All 64 shared silhouettes fit within a 24×24 tile. Transparent corners leave
  the underlying floor visible, and drawing preserves SDL viewport, scale,
  clipping and blend mode. Invalid art IDs safely draw nothing.
- All native rows have exactly 24 pixels and use valid palette keys.
- Native-size and enlarged comparisons, live region floors and animation
  samples were visually reviewed. Closed gates no longer spill a bar into the
  next tile.
- `git diff --check` passed.

Collision, map layouts, quest logic, combat, loot rules, persistent state and
save formats are unchanged. Existing saves remain usable; no migration is
needed.
