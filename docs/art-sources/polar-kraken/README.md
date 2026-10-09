# Polar Kraken sprite and renderer preview

`source.png` is the transparent sprite produced with the built-in imagegen tool.
`tools/sprites/polar_kraken.py` converts it to the runtime
`assets/images/polar-kraken.bmp`: 96 x 96 pixels, nearest-neighbor sampling,
32 colors, with its alpha preserved. Rebuild with `make sprites` or
`python3 tools/sprites/polar_kraken.py`.

`before.png` and `ingame.png` come from the actual SDL `game_draw()` renderer,
using the same generated Frostfell stage 5 map (seed 808), camera, player,
and existing warning turn. `ingame-closeup.png` is a 2x nearest-neighbor crop
of that screenshot. These are renderer captures, not generated game scenes.

This change gives the Kraken a larger sprite, a separate health display, and
warnings covering the marked tile and walkable tiles within one step of lake
holes. Warnings render beneath characters. Damage, collision, attack timing,
map generation, and saved state retain their existing behavior. The larger
art still uses the boss's original center tile for attacks.

## Recommended next fight change

Replace the repeated warning/strike pattern with a readable sequence: mark a
strike, resolve it, expose the core for a recovery turn. Then add a distinct
sweeping tentacle attack, with every affected tile marked one turn ahead.
At half health, vary the patterns rather than simply increasing damage.
Any new persistent phase/target state needs save/load support and migrations.

## Recommended Frostfell improvements

| Priority | Change | Effect |
|---|---|---|
| 1 | Draw connected glacier faces using neighboring wall tiles, with irregular snow caps and outer corners | Removes the repeated block silhouette while retaining collision |
| 2 | Use snow drifts and ice crack patterns that continue across tile boundaries | Removes the repeated horizontal stripes |
| 3 | Cluster pines and vary their shapes, heights, and snow coverage | Replaces the regular isolated-tree pattern with natural groups |
| 4 | Give the lake an irregular shoreline, curved hole rims, and restrained glints | Makes the arena read as one frozen lake |

Terrain changes should keep walls, safe snow, slick ice, thin ice, and lake
holes visually distinct. Orange warning tiles should remain legible above
terrain texture.

## Exact generation prompt

Built-in imagegen mode; transparent background requested. No CLI/API fallback.

> Use case: illustration-story. Asset type: production-ready pixel-art enemy sprite for a C/SDL top-down fantasy RPG using 24-pixel tiles, NOT a scene or concept illustration. Primary request: replace a crude Polar Kraken boss sprite with a detailed and menacing arctic kraken. Output ONE isolated full-body sprite, transparent background, no text, no UI, no border, no floor tile, no scenery. Style: crisp hand-clustered SNES RPG pixel art, limited 18-color palette, hard stepped edges, dark navy outlines, readable when reduced to 80 by 80 pixels; represent an actual roughly 80x80 pixel grid enlarged with nearest-neighbor pixels, avoid microdetail and antialiasing. View: front-facing three-quarter overhead game enemy, the entire silhouette and all curled tips visible, centered with a slim transparent margin. Design: large rounded tapered cephalopod mantle, blue-white ice-encrusted ridges and frosted crown, deep indigo and cold teal organic flesh beneath frost armor, eight thick varied curling tentacles with pale violet sucker rows; two fierce luminous turquoise eyes beneath heavy brows, small dark hooked beak. Tentacles form a broad readable low silhouette around the head with negative space separating them. Mantle rises above them with clear dimensional shading from upper left. Bottom of creature has a SMALL dark oval water breach with broken ivory ice shards directly under it, kept within creature footprint. Use larger intentional pixel clusters, strong silhouette and material separation, subtle purple shadows, luminous cyan accents. Fierce ancient sea monster rather than a cute octopus; not a robot. Make the head roughly 40% of full sprite width, total tentacles spread roughly 80 native pixels. Do not draw realistic smooth tentacles, photograph, high resolution painting, huge halo, environmental water surface or background.
