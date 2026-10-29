# Regional road checkpoints

Source: [`tools/sprites/road_checkpoints.py`](../../../tools/sprites/road_checkpoints.py),
using the established [`pixelkit.py`](../../../tools/sprites/pixelkit.py) pipeline.
Runtime assets: `assets/images/road-checkpoints.bmp` and `road-banks.bmp`.

The checkpoint atlas has six regional rows and four columns: horizontal closed,
horizontal open, vertical closed, vertical open. Each 72×96 frame is drawn at
native scale for a 24px road tile. Posts use shaded timber, iron bands, rivets,
stone feet and small regional pennants. Crypt checkpoints use masonry posts and
iron beams; woodland checkpoints carry vines. Open beams remain raised beside
the road. Transparent backgrounds preserve the underlying terrain.

The bank atlas uses 16 connected-edge masks, four surface variations and six
regional palettes. Rounded outer corners, joined canopies and rock faces frame
the crossing. Roads that cut through scenery (Haunted Forest, Blackwater Swamp,
Goblin Mountains and the northern road) draw their banks with that scenery's
tiles instead, so only the Dungeon road still shows the atlas. Terrain stays
after victory; only the beam and passage status change.

## Renderer inspection

These captures use `game_draw()` through SDL's software renderer, at native
player and landmark scale. The renderer harness captures both states from both
approaches for Dungeon, Haunted Forest, Mountains and Swamp, plus both northern
checkpoints. Existing town-gate, Rowan and quest rendering checks run afterward.

- [Elderwood closed/open comparison](elderwood-comparison.png)
- [Catacombs from the castle side](catacombs-closed.png)
- [Goblin Mountains](mountains-closed.png)
- [Haunted Forest and the separate Labyrinth entrance](haunted-forest-closed.png)

To regenerate and inspect:

```sh
.venv/bin/python tools/sprites/road_checkpoints.py
cmake --build build --target debug_shortcuts_test
./build/debug_shortcuts_test
```

The harness writes `/tmp/boss-gate-<route>-<state>.bmp`, with state 0 closed and
1 open. Routes 1/15 are Dungeon, 0/4 Forest, 2/11 Mountains, 5/9 Swamp, 16
Elderwood and 14 Catacombs. The captures show the actual closed beam, raised
beam, player proportions, area landmark, passage label, approach panel and
minimap symbols together.

Travel policy and save behavior are documented in [boss gates](../../boss-gates.md).
