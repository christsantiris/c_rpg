# Crown Road gate artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/crownroad_gate.py`, using the shared helpers in
`tools/sprites/pixelkit.py`. It replaces the earlier rectangle-drawn gate.

To change the gate, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `crownroad-gate.png`. Runtime asset: `crownroad-gate.bmp`, 72 × 72
pixels with alpha preserved. It covers 3 × 3 tiles from the tile left of
`CROWNROAD_X`, at Town 2's north exit and Town 3's south exit. Its archway is
transparent, so the road drawn underneath shows through it. If the file is
missing, the game falls back to the older rectangle drawing.

## Design

- A weathered royal gatehouse in pale stone: two crenellated towers with arrow
  slits, string courses and plinths, joined by a lower crenellated wall.
- A pointed arch over the road, with the portcullis raised into the passage.
- A cracked gold crown relief above the arch, for the fallen Crown Road.
- Torn crimson banners edged in gold with small crowns, a torch on each side of
  the arch, and ivy up the outer tower walls.
