# Frostfell entrance

Generate using `python3 tools/sprites/frostfell.py` and the established pixelkit
pipeline. Runtime asset: `assets/images/frostfell-entrance.bmp`,
168×144 pixels at 24px per tile, with alpha transparency.

A dark, frost-rimed fortress is built into icy cliffs beneath a towering snow
peak, after a painted frozen-citadel reference. Two slender towers with needle
spires flank a crenellated gatehouse; one flies a pennant. Snow clings to every
parapet and spire, icicles hang from the walls and the pointed gate, and amber
light shows in the arrow slits and deep inside the gate. A blue Frostfell banner
bearing the Frost Kraken's mark hangs above the arch. Three faceted peaks fill
the sky behind, dark cliffs frame the keep on both sides, deep drifts bank
against its base, and wind-driven snow streaks across the scene.

The south-facing gate anchors to the area's entrance tile (sprite columns
72–95). The surrounding 7×6 tile footprint blocks movement; a one-tile
cobblestone path approaches the gate through town grass. Retreat returns onto
the same path. The footprint, door tile and saves are unchanged from the
previous glacier-cave sprite.

`scale-comparison.png` shows this entrance beside the existing forest and Inn
sprites at equal native scale, enlarged 3× with nearest-neighbor sampling.
`renderer-preview.png` was captured using the actual SDL game renderer on the
Rosemoor approach road. Both were inspected for style, gate alignment, signage
and transparency.
