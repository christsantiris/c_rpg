# Castle of No Return artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/castle.py`, not an image generator. Shared drawing helpers live
in `tools/sprites/pixelkit.py`. The tavern, blacksmith and witch hut sprites
were the style references for outlines, shading and window glow.

To change the castle, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `castle-of-no-return.png`. Runtime asset: `castle-of-no-return.bmp`,
408 × 240 pixels with alpha preserved. It covers the 17 × 10 tile moat
footprint in Town 3 (`TOWN_MOAT_*`): the 15 × 8 castle plus a one-tile moat
ring. The drawbridge sits on tile (20, 11), directly below the entrance.
Transparent pixels (the grass banks and the ground behind the castle) show the
town floor drawn underneath.

## Design

- Dark charcoal-violet stone with cracks, grime streaks and waterline slime.
- Needle spires in black-crimson slate with hooked eaves and iron finials; the
  central keep spire is the tallest point.
- Blood-red lit windows and tattered black banners with red skull sigils.
- A black-and-crimson shield with a downward iron sword above the gate.
- Spiked portcullis, iron braziers, and a chained drawbridge flanked by skull
  pikes.
- Murky moat with grass banks and dead reeds, and a stone quay along the plaza.
