# Sunscar terrain renderer pass

These captures come from the actual SDL `game_draw()` renderer at 1208 x 720,
using generated Sunscar maps with seed 808. Each comparison uses the same
stage, camera, player, and enemies. The existing sandstorm uses its animation
clock, so blowing-sand positions can differ slightly.

| Scene | Before | After |
| --- | --- | --- |
| Stage 1 sandstone and cactus stands | [Before](stage-1-before.png) | [After](stage-1-after.png) |
| Stage 5 Pharaoh clearing | [Before](stage-5-before.png) | [After](stage-5-after.png) |

[Stage 4 lamp fixture](lamp.png) checks the existing quest art on the new sand.
For that capture, the existing lamp tile is placed two tiles west of the
player. Actual quest placement is unchanged.

The terrain pass joins neighboring blocked tiles into sandstone outcrops,
with continuous strata, shaded exposed faces, and eroded corner cutouts.
Scattered boulders and spires preserve the rocky theme. Tall and barrel cacti
vary in height, arms, ribs, and occasional flowers, forming small stands.
Open sand has curved dune ridges, pebbles, and dry scrub. Dunes use map
coordinates so their patterns join across tiles and remain fixed as the
camera moves. Sand details and sandstone colors are drawn in batches.

This is a renderer change. Wall positions, collision, enemies, drops, quest
state, and saves are preserved. No new runtime art assets or persistent fields
are required. Existing live and cached maps receive the new art when drawn.
All five stages were checked through the game renderer.
