# Glassdeep entrance

Generate using `python3 tools/sprites/glassdeep.py` and the established pixelkit
pipeline. Runtime asset: `assets/images/glassdeep-entrance.bmp`,
168×144 pixels at 24px per tile, with alpha transparency.

The south-facing doorway anchors to the area's entrance tile. The surrounding
footprint blocks movement; a one-tile cobblestone path approaches the doorway
through town grass. Retreat returns onto the same path.

`scale-comparison.png` shows this entrance beside the existing forest and Inn
sprites at equal native scale, enlarged 3× with nearest-neighbor sampling.
`renderer-preview.png` was captured using the actual SDL game renderer.
Both were inspected for style, doorway alignment, signage and transparency.
