# Ashen entrance

Generate using `python3 tools/sprites/ashen.py` and the established pixelkit
pipeline. Runtime asset: `assets/images/ashen-entrance.bmp`,
168×216 pixels (7×9 tiles) at 24px per tile, with alpha transparency.

A single towering fire mountain, modelled on the classic fantasy image of a
lone volcano above a blasted plain. A steep, ridged cone of charcoal rock rises
to a notched crater. A lava river pours from the notch down the face and splits
around the gate into two glowing pools. Heavy smoke billows from the crater
and rolls east on the wind, and a thinner trail rises from a vent on the
eastern shoulder. Jagged foothill spires darken the foreground. Warm rim light
follows the lava instead of spreading in round halos.

The south-facing doorway is a jagged cleft between low basalt buttresses, lit
from deep inside, and anchors to the area's entrance tile. The full 7×9 tile
footprint blocks movement; a one-tile cobblestone path approaches the doorway
through town grass. Retreat returns onto the same path. Save version 217 moves
players and loot standing on the three newly blocked summit rows onto the road.

`scale-comparison.png` shows this entrance beside the existing forest and Inn
sprites at equal native scale, enlarged 3× with nearest-neighbor sampling.
`renderer-preview.png` was captured using the actual SDL game renderer.
Both were inspected for style, doorway alignment, signage and transparency.
