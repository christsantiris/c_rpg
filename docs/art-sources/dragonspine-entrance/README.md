# Dragonspine entrance

Generate using `python3 tools/sprites/dragonspine.py` and the established pixelkit
pipeline. Runtime asset: `assets/images/dragonspine-entrance.bmp`,
168×216 pixels at 24px per tile, with alpha transparency.

Pale granite needles after high alpine granite spires: a broad jagged mass to
the west, two sharp needles in the centre and a towering summit behind them
to the east. Creases split each needle into chiselled slabs, from pale
sunlit west faces to deep blue-grey eastern shadow. The needles' shadowed
faces stand against the great summit's sunlit wall. Summits break into small
teeth, short fractures and long sheared seams cross the faces, and a field of
fallen granite blocks lies at the foot. Two red dragons circle the peaks and a
third perches on a ledge. The cave is a narrow, ragged cleft with a faint
ember glow on its threshold.

The south-facing doorway anchors to the area's entrance tile. The surrounding
footprint blocks movement; a one-tile cobblestone path approaches the doorway
through town grass. Retreat returns onto the same path.

`scale-comparison.png` shows this entrance beside the existing forest and Inn
sprites at equal native scale, enlarged 3× with nearest-neighbor sampling.
`renderer-preview.png` was captured using the actual SDL game renderer.
Both were inspected for style, doorway alignment, signage and transparency.
