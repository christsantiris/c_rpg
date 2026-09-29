# Blacksmith artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/blacksmith.py`, using the shared helpers in
`tools/sprites/pixelkit.py`. It redraws the earlier image-generated blacksmith
with the same layout, so the building can be edited and regenerated.

To change the blacksmith, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `blacksmith.png`. Runtime asset: `blacksmith.bmp`, 120 × 96 pixels with
alpha preserved. It covers the 5 × 4 tile blacksmith lot at
`TOWN_BLACKSMITH_X/Y` in Town 1, with the door on the door tile (13, 10).

## Design

- Front-facing gable in dark slate with a stone chimney.
- Half-timbered upper floor with cream plaster and a lit, shuttered window.
- Stone ground floor with an arched plank door in a stone surround and steps.
- A slate lean-to over the forge: a stone hearth with a glowing fire that
  warms the nearby stones, hanging tongs and hammers, and an anvil on a stump.
- An anvil sign, a lantern, a firewood stack, a crate and a barrel.
