# Elderwood art review

`tools/sprites/elderwood.py` uses pixelkit's layered masks, clustered foliage,
textured bark and one-pixel outlines at the same native 24-pixel tile scale as
the existing entrance sprites and buildings. It creates:

- `assets/images/elderwood-entrance.bmp`: 168 × 144 living woodland arch.
- `assets/images/elderwood-enemies.bmp`: five 64-pixel atlas cells containing
  native-size wolf, boar, dryad, Treefolk and Elderthorn silhouettes.

[Native comparison](native-comparison.png) places Elderwood, Haunted Forest and
the Inn beside each other without rescaling. The warmer emerald/amber palette
separates living Elderwood from the darker Haunted Forest.

The actual SDL software renderer captures show the [locked northern road](elder-road.png),
[relocated Catacombs](elder-catacombs-road.png) and [Elderthorn charge warning](elder-boss.png).
Dark Elves retain their existing renderer and appearance.
