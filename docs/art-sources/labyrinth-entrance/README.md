# Labyrinth on the forest approach

The existing `assets/images/labyrinth-entrance.bmp` sheet is unchanged: two
72×72 pixel frames drawn by `tools/sprites/labyrinth.py` with pixelkit, sealed
then open. At native 24px tiles, its doorway now anchors to
`APPROACH_LABYRINTH_X/Y` beside the forest entrance. Its 3×3 collision footprint
leaves the road clear; the only entrance is the south-facing door.

`sealed-preview.png` and `open-preview.png` use the actual SDL game renderer.
The sealed state rejects entry before Rook assigns the quest; assignment opens
the existing gate frame. The sprite was inspected beside the forest landmark,
with aligned doorway, labels, transparency and an uninterrupted town road.
Physical exits return just south of this doorway on the approach used.

See [the labyrinth guide](../../labyrinth.md) for quest, maze and save behavior.
