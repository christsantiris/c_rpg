# Stonewake Quarry atlas replacement

Built-in imagegen edit of `docs/images/world-map-coastal-regions.png`.
Replaces the southwest Glassdeep vignette and label in its existing footprint;
does not add Crown Road North or the future eastern area.

## Exact prompt

Use case: precise-object-edit. Edit target: the supplied complete illustrated Castle of No Return world map, 1536x1024. Change ONLY the Glassdeep Caverns landmark and its label in the southwest, south of Stillbury, approximately x=185..515 y=720..912. Replace the glowing blue crystal cave with Stonewake Quarry: a compact warm cream/gray limestone stepped excavation, medieval brown timber lifting crane with ropes, a reinforced dark mine opening with short minecart rails, tiny turquoise excavation pool, old workshop. Match this map's existing detailed painted isometric fantasy style and landmark scale. Replace the two-line label Glassdeep Caverns with exactly 'Stonewake Quarry', in the existing black serif lettering and parchment-backed label style, fitting in the old label's footprint. Keep the quarry north of the beaches, in the old cave footprint. STRICT INVARIANTS: keep every pixel and landmark outside this small southwest replacement as unchanged as possible. In particular preserve the sandy BEACHES immediately south of the quarry, the gray coastal CLIFFS behind those beaches, all coastline geometry, Sunken Coast's C-shaped sheltered cove, the fully connected branching Oakhaven wooden docks, all roads/towns/labels except the one replacement, lowland Dawnmere, the road THROUGH Goblin Mountains to high Ridgeshire, compact sea-level Saltwind Marsh inlet, forest/labyrinth, both Crown Roads, northern Catacombs route, island, oceans, mountains, border/title/compass. Do not move Stillbury, do not expand the quarry onto the shoreline, do not redraw the rest of the map. Output the complete map at exactly its original aspect ratio.

## Review

The result was inspected at native resolution and in enlarged coastline views.
Stonewake remains south of Stillbury and north of the sandy beaches. The coastal
cliffs, Sunken Coast's sheltered cove and connected branching Oakhaven docks
remain. Dawnmere remains in coastal lowland, below the Goblin mountain pass;
Ridgeshire remains surrounded by peaks. The compact Saltwind inlet, separate
labyrinth, Elderwood/Catacombs route and eastern expansion space remain visible.

Generated source: `exec-7e14b18e-5452-4726-aef4-8caf6125d8eb.png` in the session's
Codex generated images. The selected full-resolution result is copied into the
repository as the canonical atlas; runtime quarry sprites use pixelkit instead.
