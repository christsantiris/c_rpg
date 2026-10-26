# Saltwind Marsh art and renderer checks

The entrance, mangroves, skiff and five lizardfolk enemy types use the
established pixelkit pipeline at native game scale:

```sh
python3 tools/sprites/mangrove.py
python3 tools/sprites/mangrove_enemies.py
```

Runtime assets: `mangrove-entrance.bmp` (168×144), `mangrove-root.bmp` (48×72),
`skiff.bmp` (four 36×36 directions) and `mangrove-enemies.bmp` (five 48×48 cells).
Names in code and serialized fields retain the original mangrove identifier for
compatibility; the displayed area name is **Saltwind Marsh**.

The original [native comparison](native-comparison.png) places the landmark, roots, skiff
and enemy atlas beside the existing blacksmith and Elderwood art at unscaled
resolution. Humanoid bodies match the player and Sszareth has a taller crest.
Giant Crabs reuse the Sunken Coast sprite. All actors retain a single combat square.

Actual SDL software-renderer captures:

- [Stillbury's restored Glassdeep gate](stillbury.png)
- [Dawnmere's complete Saltwind gate](dawnmere.png)
- [Merchant path and working doorway](merchant-exterior.png)
- [Corvin inside the merchant shop](merchant-interior.png)
- [Southern approach and marsh landmark](approach.png)
- [Saltwind Landing](islands.png)
- [Splitroot Delta](stage-2.png)
- [Mudback Pools](stage-3.png)
- [Stiltwater Village](stage-4.png)
- [Heartroot Sanctuary](stage-5.png)
- [Sszareth's fixed charge warning](boss.png)
- [Player aboard the skiff](aboard.png)
- [Coastal docks and Rowan on the beach](docks.png)
- [Docks before the Drowned Queen victory](docks-locked.png)

The stage captures reveal terrain for review. Ordinary play retains exploration
in the minimap. Water highlights animate with SDL time; rounded mud banks and
root clusters interrupt the shoreline. Village decks use weathered timber.
The entrance blends into the surrounding grass and water. Rowan inherits the
sand under his feet instead of introducing a grass square.

`debug_shortcuts_test` also compares rendered pixels with and without each of
the six enemies in the actual adventure, and compares Rowan's corner pixel to
the underlying sand. These checks cover the formerly missing combat-area draw
branch and the NPC underlay regression. Native texture loading, dock traversal,
new/loaded debug unlocks and sailing remain covered by the same SDL executable.

The [illustrated atlas](../../images/world-map-coastal-regions.png) shows Saltwind
Marsh as a compact inlet south of Dawnmere, Glassdeep south of Stillbury and docks beside Sunken
Coast. Glassdeep retains its beaches with cliffs behind them, and Sunken Coast keeps
its curved sheltered cove. The docks use a small branching wharf attached to
shore. The [source notes](../world-map/repair-193.md) record the final image-edit prompts
and the geography review: the wharf branches are connected, the mountain road
passes between peaks, and the marsh meets the sea at water level. The eastern
headland stays available for a future area.

The merchant captures follow actual movement from Dawnmere's west road through
the restored doorway, walking up to Corvin, explicit Talk and walking out to the
same doorstep. The established exterior texture is reused unchanged. Version
193 migration and interior save/load are covered by `test_town_life`.
