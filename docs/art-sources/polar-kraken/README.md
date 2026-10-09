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
holes. Warnings render beneath characters. The larger art still uses the
boss's original center tile for attacks.

## Recovery turn

The fight now repeats warning → strike → recovery. After a strike, the boss
display turns green and reads "RECOVERING - ATTACK NOW". The next player
action has no Kraken retaliation, including its adjacent bite and attacks
beside lake holes. Then a fresh warning begins. The adjacent bite still
applies during warning turns; attack damage and collision are unchanged.

`recovery.png` captures this opening through the actual SDL `game_draw()`
renderer on the same seed 808 arena, immediately after a dodged strike.

The existing saved enemy timer stores the three-turn cycle. Save version 115
migrates older live and cached Kraken timers: an odd timer keeps its pending
strike and marked tile; an even timer prepares a new warning. Health, maps,
quest progress, and rewards are preserved. Recovery and warning turns survive
save/load and town portal visits.

## Sweeping tentacles

The Kraken alternates its targeted strike with a sweep of one row or column
across the lake, aimed at the player's marked position. The direction uses
the dominant axis from the Kraken to that position, with horizontal sweeps
on ties. A perpendicular step escapes the lane; moving along it still takes
damage. The marked position remains threatened even outside the lake.

Every walkable tile in the lane is marked orange for one turn, with a "SWEEP
NEXT TURN" cue. The original warning still raises tentacles beside lake
holes; the sweep does not attack unrelated holes. Both patterns keep the
same damage and a full recovery opening. `sweep.png` is an actual SDL renderer
capture of the vertical warning on seed 808.

`kraken_tile_threatened()` supplies both the warning tiles and the damage
check. The existing saved timer now covers six turns, retaining the original
phases 0–2 for version 115 saves. Version 116 saves also preserve sweep
warnings, recovery, and the next pattern across save/load and portal visits;
the version 115 migration still handles older two-turn saves.

## Half-health phase

At 50% health or below, newly warned sweeps mark two parallel lanes, separated
by one safe lane. The added lane lies two tiles toward the Kraken's center;
when the original lane passes through the center, it lies two tiles in the
positive map direction. One perpendicular step still escapes the attack.
Targeted strikes, damage, adjacent bites, and recovery timing stay the same.

The boss display adds "ENRAGED", a warm health bar, and a "DOUBLE SWEEP" cue.
`enraged.png` captures the actual SDL renderer on seed 808 with the Kraken
set to half health before the sweep's warning.

The existing enemy `attack_phase` field locks the pattern at warning time,
so reducing the Kraken's health during wind-up cannot add unmarked danger
tiles. That field is already serialized for live and cached enemies. Save
version 117 preserves double sweeps through save/load and town portal visits.
Older saves keep their pending single-lane patterns; subsequent warnings
select the half-health pattern without resetting maps, health, or progress.

## Recommended Frostfell improvements

The terrain rendering improvements below are now implemented. See the
[Frostfell terrain captures](../frostfell/README.md) for matching comparisons
and expedition details. The original Kraken captures above retain the older terrain.

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
