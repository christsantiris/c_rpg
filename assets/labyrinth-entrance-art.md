# Labyrinth entrance artwork

Pixel art drawn procedurally at native game resolution by
`tools/sprites/labyrinth.py`, using the shared helpers in
`tools/sprites/pixelkit.py`. It redraws the earlier code-drawn entrance with the
same layout, so the gate can be edited and regenerated.

To change the entrance, edit the script and run `make sprites` (Python 3 with
Pillow). It rewrites both files below; the output is deterministic, so an
unchanged script reproduces them exactly.

Source: `labyrinth-entrance.png`. Runtime asset: `labyrinth-entrance.bmp`,
144 × 72 pixels with alpha preserved: two 72 × 72 frames, sealed on the left
and open on the right. Each frame covers 3 × 3 tiles in Town 2, drawn from one
tile left of and two tiles above the entrance tile `TOWN_LABYRINTH_X/Y`. If the
file is missing, the game falls back to its older rectangle drawing.

## Design

- A mossy stone archway with stepped coping, a stepped arch over a deep
  doorway, and worn steps at its foot.
- A brass-framed ivory rook crest above the arch.
- Buttresses on both sides with iron sconces and climbing ivy.
- Sealed: an iron portcullis with brass rivets and a brass lock plate, and
  unlit sconces.
- Open: the portcullis raised, steps descending into the dark, and lit torches.
