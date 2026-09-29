# Tavern artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/tavern.py`, using the shared helpers in
`tools/sprites/pixelkit.py`. It redraws the earlier image-generated tavern with
the same layout, so the building can be edited and regenerated.

To change the tavern, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `tavern.png`. Runtime asset: `tavern.bmp`, 168 × 120 pixels with alpha
preserved. It covers the 7 × 5 tile tavern lot at (5, 16) in Town 1, with the
door centred on the door tile (8, 20). The Town 2 Inn has its own art in
`inn-art.md`.

## Design

- Red clay tile roof with a ridge line, a central cross gable, two dormers and
  a stone chimney.
- Half-timbered upper floor: cream plaster, dark beams and diagonal braces, and
  warm lit windows with plank shutters.
- Stone ground floor with timber posts, two shuttered windows, an arched plank
  door in a stone surround, and stone steps.
- Two iron lanterns by the door and a hanging beer-mug sign on the left.
- Barrels, a crate and a bench along the front.
