# Frostfell terrain renderer pass

These screenshots come from the actual SDL `game_draw()` renderer at 1208 x
720, using generated Frostfell maps with seed 808. Each before/after pair
uses the same stage, camera, player, and enemies. Weather uses its existing
animation clock, so snowflake positions can differ slightly.

| Scene | Before | After |
| --- | --- | --- |
| Stage 1 snow and glacier shelves | [Before](snow-before.png) | [After](snow-after.png) |
| Stage 5 Kraken lake, warning turn | [Before](lake-before.png) | [After](lake-after.png) |

[Stage 4 expedition details](expedition.png) also shows fragile shortcut ice
and a slick-ice patch. For this visual fixture, the existing survivor tile is
placed two tiles east of the player; the quest's actual placement is unchanged.

The pass adds connected glacier faces with snow caps, corner cutouts,
icicles, and shaded ice. Snow drifts and lake cracks use map coordinates so
their patterns cross tile boundaries and stay fixed while the camera moves.
Pines form stands with varied height and snow coverage. Small exposed stones
and branches appear in the snow; footprints and partly buried supplies appear
near the journal and survivor. Supplies are decorative and cannot be picked up.

Safe snow, blocking glaciers, teal slick ice, fractured thin ice, and dark
lake holes retain distinct appearances. Kraken warning markers still draw
above the terrain. Lake shore snow and hole rims soften the silhouettes
within existing tiles; collision and shoreline tile positions are unchanged.

This is a renderer change with no new assets loaded by the game, persistent
state, or save migration. Existing live and cached Frostfell maps receive the
new appearance when drawn. All five stages were inspected through the game
renderer, including the journal and survivor fixtures.
