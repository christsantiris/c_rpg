# Western geography and woodland art

Runtime geography: Ashen Hollow is west of Rosemoor. Ridgeshire has north,
south and west exits. The Haunted Forest and Elderwood share a woodland belt.

Native game art uses `tools/sprites/forest.py`, `tools/sprites/elderwood.py`
and the woodland tile functions in `src/renderer/sprites.c`. Entrance assets
retain their 168×144 footprints. Ordinary actor sprites retain their scale.

Actual SDL renderer captures:

- [Native entrance, blacksmith and player comparison](native.png).
- [Haunted Forest](haunted.png) and [Elderwood](elderwood.png).
- [Rosemoor's west gate](rosemoor.png), [Ashen approach](ashen-approach.png)
  and [Ridgeshire's closed eastern boundary](ridgeshire.png).

The atlas is `docs/images/world-map-coastal-regions.png` (1536×1024).
It was edited with the built-in imagegen tool. Reviewed the complete image,
then enlarged the southern coast and eastern mountain route: the quarry's
beaches and cliff backdrop, Sunken Coast's sheltered cove, continuous branching
dock deck, sea-level Saltwind inlet, lowland Dawnmere and its northern mountain
pass remain present. Rook's Labyrinth occupies a separate roadside clearing.

Save version 197 retires the garden location and NPC, preserves Ashen's numeric
IDs and expedition caches, mirrors older Ashen approaches and retains earned
rewards through the serialized item recovery queue. The retired garden's own
quest, caches and boss mechanics are absent from runtime state and new saves.

Validation: 7,640/7,640 gameplay and save assertions passed. The Debug SDL
launcher suite passed its road, entrance, dock, sailing, jail and renderer
checks. All 528 surviving serialized location, tile and enemy enum values
match their values before this change.

## Atlas edit prompt

```text
Use case: precise-object-edit. Edit target: the supplied finished 1536x1024 fantasy atlas of The Castle of No Return. Make ONLY these approved geography and woodland changes, preserving composition, coastline, artwork quality, readable existing typography, title, compass, parchment border and all other features. (1) Completely REMOVE Moonveil Gardens: its exact label, pink blossom gardens and garden ruin vignette in the FAR NORTHWEST, immediately WEST of Rosemoor. Replace that footprint with ASHEN HOLLOW: an ash-dark volcanic hollow with charcoal basalt crags, small restrained orange lava channels, and a single readable label 'Ashen Hollow'. Join Rosemoor's WEST road continuously to this volcanic hollow. Keep Frostfell's snowy mountains north of it intact, and leave Sunscar desert south/southwest; integrate natural bare foothills between them. (2) Remove the old 'Ashen Hollow' label and orange lava from the far NORTHEAST, EAST of Ridgeshire. That former footprint becomes unlabelled rugged gray mountain terrain continuing Dragonspine's eastern flank toward the coast, with some snowy high peaks. Remove the obsolete eastbound destination spur from Ridgeshire; its north, south and west connections stay. Ashen Hollow appears exactly ONCE, west of Rosemoor; Moonveil appears ZERO times. (3) Keep Haunted Forest and Elderwood Forest next to each other in their current central positions and with exactly their existing labels. Make Haunted Forest, between Stillbury and Oakhaven, visibly darker and more tangled: cooler muted deep-green canopy, scattered bare crooked dead trees, tiny abandoned clearings. Make Elderwood, north of Oakhaven, a healthier ancient living woodland: larger broad lush emerald crowns and warm green-gold canopy highlights, immense old trunks visible in small gaps. Show a natural gradual transition between these connected woods, with no biome boxes. Preserve all forest roads and the north road to the SEPARATE Royal Catacombs and Castle. Rook's Labyrinth remains OUTSIDE the forest south of its road. Critical invariants: preserve all five towns each once at the same positions, castle, northern cold regions, Crown Road East/West, Royal Catacombs, Escape Tunnel, road from Dawnmere THROUGH tightly flanking Goblin Mountains to elevated Ridgeshire, Dawnmere's open coastal lowland with room east for a future area, Oakhaven Dungeon and road, compact sea-level Saltwind Marsh SOUTH of Dawnmere, Stonewake Quarry SOUTH of Stillbury with its sandy beaches BELOW backed by rocky cliffs, C-shaped sheltered Sunken Coast cove with drowned columns and western shoreline intact, Oakhaven's SHORE-CONNECTED BRANCHING WOODEN DOCKS with every mooring arm ATTACHED by solid deck and no gap or severing, the dotted voyage route, open sea, Ruined Isle and Ruined Temple. No Stormwatch label yet. No explanatory descriptions, change annotations, legends, gameplay text, extra labels or new destinations. The atlas is a map of places and routes only. Return the complete edited atlas in the same landscape 3:2 composition.
```
