# Frostfell enemy artwork review

The six regular enemies use native 24×24 templates in `src/renderer/frostfell_enemy_art.h`. The Polar Kraken fallback uses a native 48×48 template in `src/renderer/polar_kraken_art.h`, displayed at 2× scale. Both Kraken paths fit the existing 96×96 texture destination; attack coordinates and collision stay on the original tile.

The refined Kraken source is saved at `docs/art-sources/polar-kraken/source.png`. The runtime bitmap is `assets/images/polar-kraken.bmp`, rebuilt with `python3 tools/sprites/polar_kraken.py`: nearest-neighbor sampling to 96×96, 32-color quantization, and transparency preserved through the existing sprite pipeline.

`before-after.png` compares all six regular enemies at native size and nearest-neighbor enlargement. `kraken-before-after.png` compares both Kraken paths at native 96×96 and 2× enlargement. `frostfell.png` and `kraken-fallback.png` are full SDL game renderer captures. `kraken-phases.png` places native-size crops of targeted warning, single sweep, enraged double sweep, and recovery captures together. All scenes use controlled room fixtures for artwork review rather than gameplay or quest playthroughs.

Release and Debug builds, AddressSanitizer/UndefinedBehaviorSanitizer renderer checks, sprite bounds, transparency, and SDL scale/viewport/clip/blend preservation passed. All 41 previously updated enemy sprites render pixel-for-pixel identically. Rimefang identification, expedition objectives, frozen overlays, health bars, melee feedback, and Frost Archer projectile travel/impact were also inspected.

## Exact Kraken edit prompt

Built-in imagegen edit mode, using the previous `docs/art-sources/polar-kraken/source.png` as the edit target and requesting a transparent background. No CLI/API fallback was used.

> Use case: precise-object-edit. Asset type: existing Polar Kraken boss sprite for a C/SDL pixel-art RPG. Edit target: the supplied image. Preserve the exact creature identity, front-facing overhead view, domed frosted mantle, icy crown, cyan eyes, dark beak, indigo and teal flesh, lavender suckers, full curled tentacle pose, compact water breach, centered square composition and transparent background. Refine ONLY the pixel-art detailing for display at 96x96 pixels: stronger dark indigo contours where tentacles overlap, slightly larger pale lavender sucker clusters with dark centers, clearer blue-violet undersides, small brighter cyan eye highlights, a compact dark water shadow at the base for contrast against pale ice. Maintain the current overall silhouette and mantle-to-tentacle proportions. Crisp clustered retro pixel art with hard stepped edges and a limited 24-color palette, no antialiasing, no added scenery, no extra characters, no text, no glow halo. Leave a small transparent margin so every tentacle tip remains inside the image.
