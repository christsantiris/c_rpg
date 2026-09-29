# Healer building artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/healer.py`, using the shared helpers in
`tools/sprites/pixelkit.py`. It redraws the earlier image-generated healer's
cottage with the same layout, so the building can be edited and regenerated.

To change the healer, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `healer.png`. Runtime asset: `healer.bmp`, 120 × 96 pixels with alpha
preserved. It covers the 5 × 4 tile healer lot at `TOWN_HEALER_X/Y` in Town 2,
with the door on `TOWN_HEALER_DOOR_X/Y`.

## Design

- Front-facing gable in sage-green shingles with a stone chimney.
- Half-timbered upper floor with cream plaster and a lit, shuttered window.
- Stone ground floor with an arched plank door, stone steps, and a lantern on
  each side of the door.
- A sage lean-to over a herb stall: a small lit window, hanging lavender and
  white herb bundles, a shelf of bottles, and a table with a mortar.
- A green sign with an ivory healing cross, a lantern, and three planters of
  herbs and flowers.
