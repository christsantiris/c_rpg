# Stonewake implementation verification

The approved concepts are translated into deterministic pixelkit runtime art,
not imported as full-size artwork. The original implementation uses save version
194; the crane rework below adds version 195.

## Initial implementation (194)

- Release: `cmake --build build -j4` and `./build/test_runner` with a 64 MB
  stack limit: **7,937/7,937 passed**.
- Debug: built `conr` and `debug_shortcuts_test`; ran with SDL dummy video/audio.
  All five quarry creatures produce distinct visible pixels at native scale.
  Crane, workshop, entrance and supply textures load; the hammer has a distinct
  inventory icon. Existing Saltwind, docks, town and passage checks pass.
- A separate flood-fill sweep checked 1,000 generated floors. The committed
  quarry tests check 500 floors with both cranes raised, either lowered, both
  lowered and both raised again. Every walkable square remains connected.
- Regression coverage includes enemy reachability and respawning; occupied
  bridges (player/enemy/loot/portal); overlapping loot under controls; one-time
  stores; fixed boss warnings, damage, freeze, recovery and interruption;
  full-pack rewards; all-class weapon swapping, confiscation and save/load;
  version 193 migration, active portals and deferred legacy-layout replacement.
- Loaded the existing local player save read-only through the SDL preview
  harness. It was not overwritten.
- Rebuilt all five BMPs from `stonewake.py`; SHA-256 hashes are unchanged.
- Inspected the new entrance, crane and workshop beside the established
  blacksmith, at the same native scale. The comparison includes a player drawn
  by the real SDL renderer. Reviewed complete renderer captures of the approach,
  work yards and Colossus warning. Images are linked in the region guide.
- Inspected the full atlas and before/after shoreline, dock and mountain-pass
  details. The beaches, cliff-backed shore, C-shaped Sunken Coast cove, connected
  dock branches, lowland Dawnmere and mountain road remain visible.
- Corrected an older mountain portal test fixture that could put its player on
  a random regular enemy. The fixture now clears that landing before opening
  its portal; no mountain production behavior changed.

`git diff --check` passes. Changes remain unstaged and uncommitted for review.
Crown Road North and the Catacombs revamp are not part of this change set.

## Crane routes and salvage platforms (195)

- Fixed the debug refresh that reopened a player-raised bridge before every
  action. Only unused cranes are opened initially; subsequent actions and loads
  respect the selected raised, crossing or salvage setting.
- Release build and full test runner: **7,958/7,958 passed** with a 64 MB stack.
  The regression uses actual interaction and movement actions with debug unlock
  enabled, then verifies that a raised crossing blocks movement after save/load.
- Across 500 layouts, all nine pairs of crane settings retain a connected main
  route. Isolated salvage platforms are reachable exactly when their spur is
  lowered. Ordinary spawns and return-visit respawns stay on connected ground.
- Covered occupied spans and platforms, floor loot, visible and hidden portal
  anchors, one-time supply rewards, version 194 active/cached map migration and
  version 195 round trips. Old store claim states and enemy wounds persist.
- Debug executable and SDL software-renderer checks pass. All three 72×72 crane
  poses produce distinct rendered pixels. Reviewed the actual raised, crossing
  and salvage routes in [this capture comparison](../../images/stonewake-crane-routes.png).
  The [native comparison](../../images/stonewake-native-comparison.png) shows all
  three poses beside the existing blacksmith and renderer-drawn player without
  scaling. Refreshed the workshop and boss renderer captures for the new layout.
- All five pixelkit assets rebuild with unchanged SHA-256 hashes. The existing
  local player save loads read-only through the preview harness and was not
  overwritten. `git diff --check` passes.

Changes remain unstaged and uncommitted for review.
