# Coast entrance

Generate using `python3 tools/sprites/coast.py` and the established pixelkit
pipeline. Runtime asset: `assets/images/coast-entrance.bmp`,
168×144 pixels at 24px per tile, with alpha transparency.

The south-facing doorway anchors to the area's entrance tile. The surrounding
footprint blocks movement. Grass gives way to a sandy shoreline; open sea spans
the full southern half of the approach and continues behind the landmark's
transparent edges. A one-tile cobblestone causeway reaches the doorway over
the water. Retreat returns onto that causeway, leading north to Oakhaven.

Save version 177 upgrades older grass approaches, relocating players and dropped
loot from newly submerged tiles onto the causeway while preserving exploration,
quest progress and cached combat maps.

`scale-comparison.png` shows this entrance beside the existing forest and Inn
sprites at equal native scale, enlarged 3× with nearest-neighbor sampling.
`renderer-preview.png` was captured using the actual SDL game renderer.
Both were inspected for style, doorway alignment, signage and transparency.
