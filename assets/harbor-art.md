# Harbor artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/harbor.py`, using the shared helpers in
`tools/sprites/pixelkit.py`. It redraws the earlier image-generated harbor with
the same layout, so the building can be edited and regenerated.

To change the harbor, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `harbor.png`. Runtime asset: `harbor.bmp`, 120 × 96 pixels with alpha
preserved. It covers the 5 × 4 tile harbor lot at `TOWN_HARBOR_X/Y` in the
south-east corner of Town 1. The dock crosses row `TOWN_HARBOR_ENTRANCE_Y`,
where the harbor road ends. The harbor screen draws the same image enlarged by
a whole-number scale.

## Design

- A harbour house with a red clay gable roof, a stone chimney, a half-timbered
  upper floor with a lit shuttered window, and a stone ground floor with an
  arched door.
- A plank dock on rope-wrapped pilings, with a barrel, a crate, a coil of rope
  and a lantern on a bracket.
- A crane post whose hook hangs over a small sailboat with a cream sail and
  rigging.
- Harbour water with an irregular edge and ripples, in the coast palette.
