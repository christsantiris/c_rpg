# Batch 15: Ruined Isle and Temple

The ten enemy identities and the Sun Guardian's broken form were refined with
the built-in imagegen tool. The runtime asset is
[`assets/images/temple-enemies.bmp`](../../../assets/images/temple-enemies.bmp).
Existing enemy types, display sizes, half-health phase selection, and saved
state are retained.

## Art inputs and prompts

- Original identity reference: [`temple-enemies.png`](../temple-enemies.png).
- First refinement, used as the second edit's target: [`refinement-input.png`](refinement-input.png).
- Coarse pixel style reference, captured from the completed batch 14 SDL sprites:
  [`style-reference.png`](style-reference.png).
- Final selected source: [`source.png`](source.png), 1448×1086 with transparency.
- Exact first edit prompt: [`initial-prompt.txt`](initial-prompt.txt).
- Exact second edit prompt: [`refinement-prompt.txt`](refinement-prompt.txt).

The first edit preserved the Temple characters and reorganized the sheet.
Native-size review showed soft details, so the second edit strengthened
logical pixel shapes and material shading using batch 14 as its style reference.

## Rebuilding the runtime atlas

From the repository root, run:

```sh
python3 tools/sprites/temple_enemies.py
```

The importer isolates each character using explicit source rectangles, finds
its silhouette at alpha 128, preserves its aspect ratio, and reduces it with
nearest-neighbor sampling. Each frame has at most 32 opaque colors, binary
alpha, and a one-pixel transparent edge. `pixelkit.save_sprite()` writes the
192×144 runtime BMP with its alpha channel.

The atlas has four columns and three rows of 48×48 cells. Frames are centered
horizontally and bottom-aligned in each cell. The first nine cells hold 28×28
frames in checklist order, followed by intact and broken 40×40 Guardian frames;
the twelfth cell is empty. The renderer samples these native frames directly,
avoiding the stretched silhouettes and neighboring-sprite bleed in the old sheet.

## Renderer review

- [`before-after.png`](before-after.png): ten enemy types and the broken Guardian,
  with native-size crops and nearest-neighbor enlarged views.
- [`temple.png`](temple.png): all ten enemies on Temple terrain.
- [`guardian-and-sentinel.png`](guardian-and-sentinel.png): 51% versus 50% Guardian
  health, low health, inactive defeated enemy, and awakened Sentinel/altar context.
- [`effects.png`](effects.png): frozen overlays, melee damage, and projectile
  travel/impact for the five ranged Temple actors.

These are controlled fixtures through the actual SDL game renderer, not
quest or boss-fight playthroughs. Release and Debug builds and a temporary
AddressSanitizer/UndefinedBehaviorSanitizer renderer harness passed. The harness
checked display bounds, transparent terrain, SDL state preservation, and
missing-texture behavior. All 77 earlier enemy sprites rendered pixel-for-pixel
identically; the ten Temple sprites retained their static timing. Import checks
also verified all eleven frames' transparent borders, palettes, dimensions,
and byte-for-byte reproducible atlas generation.
