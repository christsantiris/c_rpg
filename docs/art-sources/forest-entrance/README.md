# Haunted Forest entrance

Generate with `python3 tools/sprites/forest.py`, using the established pixelkit
layer, polygon, bark, foliage and outline pipeline. Runtime asset:
`assets/images/forest-entrance.bmp` (168×144 RGBA; 7×6 native 24px tiles).
The south-facing passage anchors to the approach's entrance tile.

The gnarled trunks and intertwined crowns frame a dark route into the woods.
Hanging moss, violet tree hollows and a weathered warning charm carry the
haunted theme. Doorway scale and lighting match the existing mountain entrance.

`scale-comparison.png` compares both entrances at the same native scale, enlarged
3× using nearest-neighbor sampling. `renderer-preview.png` uses the actual SDL
game renderer on the new road. The landmark occupies the same coordinates
when approached from Oakhaven or Stillbury.
