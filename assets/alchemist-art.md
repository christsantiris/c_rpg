# Alchemist artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/alchemist.py`, using the shared helpers in
`tools/sprites/pixelkit.py`. It redraws the earlier image-generated alchemist
with the same layout, so the building can be edited and regenerated.

To change the alchemist, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `alchemist.png`. Runtime asset: `alchemist.bmp`, 120 × 96 pixels with
alpha preserved. It covers the 5 × 4 tile alchemist lot at
`TOWN_ALCHEMIST_X/Y` in Town 1, with the door on the door tile (31, 10).

## Design

- A broad gable in teal shingles whose long right slope shelters a herb stall,
  and a stone chimney.
- Mauve plaster and dark timber upper floor, with a violet-lit window holding a
  potion flask.
- A shop window of glowing green potions under a small teal awning, which
  casts a green light on the stone nearby.
- A stone ground floor with an arched plank door, steps, and a keg on each
  side.
- An open stall with hanging herb bundles and a shelf of bottles, a crate, a
  lantern, and a hanging green flask sign.
