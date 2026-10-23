# Saltwind landing art and renderer checks

The first playable exploration slice uses the established pixelkit pipeline:

```sh
.venv/bin/python tools/sprites/mangrove.py
```

Runtime assets are `assets/images/mangrove-entrance.bmp` (168×144),
`mangrove-root.bmp` (48×72) and `skiff.bmp` (four 36×36 directions).
The trees use the existing forest bark, foliage, shading and outline helpers
with a mangrove palette and exposed roots. The entrance contains a timber
landing and a moored boat. Boat collision occupies its water tile; docking
places the rider on the neighboring landing tile.

[Native comparison](native-comparison.png) places the new landmark, root tree
and skiff beside the existing blacksmith at unscaled pixel resolution.

Actual SDL software-renderer captures:

- [Dawnmere and its southern exit](dawnmere.png)
- [Single-tile southern approach and coastal landmark](approach.png)
- [Four islands, root beds and landing geography](islands.png)
- [Player aboard the skiff](aboard.png)

The island captures reveal the terrain for visual review. Normal play records
explored cells in the minimap. World tiles and actors retain native scale;
the pilot scrolls on smaller displays. Ground uses warm silt, green reeds and
rounded shoreline corners. Dark teal water highlights animate with SDL time.
The approach landmark blends into the grass and water rather than painting a
rectangular ground patch behind its transparent image.

The new [illustrated atlas](../../images/world-map-saltwind.png) was edited with
the built-in imagegen tool. Its exact prompt and geography checks are recorded
in the [world-map source notes](../world-map/prompt.md#saltwind-landing-atlas-save-version-190).
It moves Dawnmere north, adds Saltwind south of town above the offshore islands,
and widens Goblin Mountains around the Ridgeshire road. That illustration
represents the planned full region; the current implementation contains the
first exploration landing, with combat and later stages still to come.
