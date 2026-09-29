# Inn artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/inn.py`, using the shared helpers in `tools/sprites/pixelkit.py`.
It shares the tavern's timber-and-plaster style but has its own design, so the
Town 2 Inn no longer reuses the Town 1 tavern.

To change the inn, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `inn.png`. Runtime asset: `inn.bmp`, 168 × 120 pixels with alpha
preserved. It covers the 7 × 5 tile inn lot at (5, 16) in Town 2, with the
double door centred on the door tile (8, 20).

## Design

- Twin front gables and a hipped middle roof in blue-grey slate, with a
  sandstone chimney.
- Whitewashed upper floor with dark timber framing, small attic windows, and
  shuttered windows with flower boxes.
- A covered balcony with balusters across the middle, in front of three lit
  windows.
- Sandstone ground floor with timber posts, a double plank door under a lit
  fanlight, stone steps, and two iron lanterns.
- A hanging navy sign with a gold crescent moon and star.
- A horse trough and hitching post, flower pots by the door, and stacked hay
  bales.
