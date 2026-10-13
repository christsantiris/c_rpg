# Batch 16: Castle of No Return

The eight castle enemy types and Lord Veyr's three forms were refined with the
built-in imagegen tool. The runtime sheets are
[`castle-enemies.bmp`](../../../assets/images/castle-enemies.bmp) and
[`castle-bosses.bmp`](../../../assets/images/castle-bosses.bmp).
Display sizes, guard markers, attack warnings, phase thresholds, enemy IDs,
and saved state are retained.

## Sources and exact prompts

- Original identity references: [`castle-enemies.png`](../castle-review/castle-enemies.png)
  and [`castle-bosses.png`](../castle-review/castle-bosses.png).
- Established pixel style reference from the completed batch 14 sprites:
  [`style-reference.png`](style-reference.png).
- Selected regular source: [`regular-source.png`](regular-source.png).
- Selected boss source: [`boss-source.png`](boss-source.png).
- Boss spacing edit input: [`boss-spacing-input.png`](boss-spacing-input.png).
- Exact regular edit prompt: [`regular-prompt.txt`](regular-prompt.txt).
- Exact boss edit prompt: [`boss-prompt.txt`](boss-prompt.txt).
- Exact boss spacing correction: [`boss-spacing-prompt.txt`](boss-spacing-prompt.txt).

Both selected source sheets are 1536×1024 with transparency. The first two
edits strengthen facial highlights, armor facets, weapon grips, and cloth
folds while preserving the existing royal palette and character identities.
The boss spacing edit separates the damaged and spectral Veyr silhouettes
so their outstretched swords do not appear in neighboring crops.

## Rebuilding the runtime atlases

From the repository root, run:

```sh
python3 tools/sprites/castle_enemies.py
```

The importer isolates each pose with explicit source rectangles, finds its
silhouette at alpha 128, preserves its aspect ratio, reduces with nearest-neighbor
sampling, and quantizes to at most 32 opaque colors. Each native frame has
binary alpha and a one-pixel transparent edge. `pixelkit.save_sprite()` writes
the alpha BMPs. The high-resolution sheets' transparent background RGB is not
used as terrain or a backdrop.

The regular atlas is 96×64 with three columns and two rows of 32×32 cells.
Its 24×26 frames are centered horizontally and bottom-aligned. The boss atlas
is 120×80 with 40×40 cells containing 36×36 frames. Both sixth cells are empty.
Regular frames follow checklist order; boss frames are Castellan, Arcanist,
armored Veyr, damaged Veyr, and spectral Veyr. `castle_draw_enemy()` samples
these native frames directly through the existing live castle rendering path.
The generic castle cases in `draw_enemy()` and the missing-texture Orc fallback
are unchanged.

## Renderer review

- [`before-after.png`](before-after.png): eight enemy types and three Veyr forms,
  with native crops and nearest-neighbor enlarged views.
- [`castle.png`](castle.png): all ten poses on castle stone and crimson carpet.
- [`veyr-phases.png`](veyr-phases.png): three forms with their focused, radius-one,
  and radius-two targeting warnings, plus low-health spectral Veyr.
- [`veyr-boundaries.png`](veyr-boundaries.png): 67, 66, 34, and 33 HP out of 99,
  left to right, exercising both exact phase boundaries.
- [`guards-and-effects.png`](guards-and-effects.png): guard facing/recovery,
  Castellan charge/sweep, Marksman/Hexer targeting, fire-rune countdown,
  frozen overlays, and melee damage feedback.
- [`fallback.png`](fallback.png): both castle textures absent through `game_draw()`.

These are controlled castle-room and throne fixtures through the actual SDL
renderer, not boss-fight playthroughs. Release and Debug builds and a temporary
AddressSanitizer/UndefinedBehaviorSanitizer renderer harness passed. Checks
covered display bounds, transparent terrain, SDL state preservation, static
timing, and both exact Veyr phase boundaries. All 87 earlier enemy sprites,
the broken Sun Guardian, and all ten missing-texture fallback renders remain
pixel-for-pixel identical. Import checks verified frame dimensions, palettes,
transparent borders, and byte-for-byte reproducible atlas output.
