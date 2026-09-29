# Witch's hut artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/witch.py`, using the shared helpers in
`tools/sprites/pixelkit.py`. It redraws the earlier image-generated witch's hut
with the same layout, so the building can be edited and regenerated.

To change the hut, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `witch-hut.png`. Runtime asset: `witch-hut.bmp`, 120 × 96 pixels with
alpha preserved. It covers the 5 × 4 tile witch's lot at `TOWN_WITCH_X/Y` in
Town 2, with the door on `TOWN_WITCH_DOOR_X/Y`.

## Design

- A wide gable roof and lean-to in purple shingles, patched with moss, and a
  mossy stone chimney.
- Half-timbered gable with a purple-glowing window, a flower box, trailing ivy
  and a hanging gold crescent charm.
- Stone ground floor with an arched plank door, stone steps and two lanterns.
- A plaster side wing with a shelf of potions, and a crescent-moon sign on a
  bracket.
- A lean-to potion stall: hanging herbs, a bubbling cauldron over a fire that
  casts a violet glow, a table of bottles and a lantern.
- Red and violet mushrooms around the base.
