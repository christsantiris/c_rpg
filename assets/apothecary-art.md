# Apothecary artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/apothecary.py`, using the shared helpers in
`tools/sprites/pixelkit.py`.

To change the apothecary, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `apothecary.png`. Runtime asset: `apothecary.bmp`, 120 × 96 pixels
with alpha preserved. It covers the 5 × 4 tile apothecary lot at
`TOWN_APOTHECARY_X/Y` in Town 3, east of the square, with the door on
`TOWN_APOTHECARY_DOOR_X/Y`. If the file is missing, the game draws the
alchemist sprite in its place.

## Design

- A steep front gable in dark plum shingles with a stone chimney.
- Cream plaster and dark timber upper floor with three lit windows and flower
  strips under the lower two.
- A stone shop front with two lamplit display windows. Their shelves hold red
  healing, blue mana, orange strength and violet intelligence potions, and they
  warm the stone around them.
- An arched plank door with stone steps, a lantern, and a green sign with a
  brass mortar and pestle.
- A crate of bottles and two herb planters.
