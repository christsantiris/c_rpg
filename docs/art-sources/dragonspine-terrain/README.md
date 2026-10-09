# Dragonspine terrain renderer captures

These PNGs are captures of the actual SDL `game_draw()` renderer, using a
1208 × 720 software-rendered surface and native 24-pixel tiles. They are not
generated concept art.

- `before.png` and `after.png`: the same seed 2026 stage 3 map, player at
  (60, 6), camera, and enemies. The stage's existing ash tiles are unchanged.
- `summit.png`: seed 2026 stage 5, player at (57, 51), with Ilya's quest active
  and the golden goblet placed by `game_refresh_quest_encounters()`.

`draw_dragonspine_floor()` in `src/renderer/sprites.c` now draws snow drifts
in world coordinates, with small irregular ash and coin patches over a common
snow base. `draw_dragonspine_wall()` uses continuous rock shading and exposed
edges derived from neighboring map tiles, instead of repeating brick stamps.
The texture is deterministic and uses no gameplay random numbers. Maps,
collision, paths, quest progress, and save data are unchanged.

The golden goblet is collected through `ACTION_PICK_UP` (default key P).
It no longer intercepts the move-left/interact key A. Existing collection,
quest progress, full-inventory handling, and save/load behavior are reused.
