# Castle of No Return — Codex Coding Rules

## 1. Always use braces on if statements

Prefer:
```c
if (!map_is_walkable(&g->map, ex, ey)) {
    continue;
}
```

Never:
```c
if (!map_is_walkable(&g->map, ex, ey)) continue;
```

This applies to all control flow: `if`, `else`, `for`, `while`.

---

## 2. Single space around operators — no alignment padding

Prefer:
```c
int a = 5;
int bc = 10;
int def = 15;
```

Never:
```c
int a   = 5;
int bc  = 10;
int def = 15;
```

This applies to variable declarations, struct field assignments, and all `=` usage.

---

## 3. Function signatures on one line

Prefer:
```c
static void handle_slot_result(SlotResult result, int slot, int slot_is_save) {
```

Never:
```c
static void handle_slot_result(SlotResult result, int slot,
    int slot_is_save) {
```

If a signature is too long, shorten the parameter names rather than wrapping.

---

## 4. Minimal code per change set

- Provide the smallest possible change per step
- Wait for an explicit "next step" prompt before providing the next change
- Never combine multiple features into one code block
- Never add unrequested changes alongside a requested change
- Finish each feature with validation results and a suggested commit message,
  then stop for the user's manual commit. Additional feedback is a backlog;
  it does not waive this pause unless the user explicitly requests a combined
  change set.

---

## 5. Explain each change

For every code change, briefly explain:
- What the change does
- Why it is needed
- Any side effects or dependencies

---

## 6. Be maximally specific about code location

When instructing where to add or modify code, always specify:
- The exact filename and path
- The exact function name
- The exact surrounding lines to find the insertion point

Prefer:

> In `c_rpg/src/game/actions.c` in `action_resolve_player` replace:
> ```c
> g->level_cleared = 0;
> ```
> with:
> ```c
> g->level_cleared = 1;
> ```

Never:

> Add this after the level cleared block.

---

## 7. All game state changes must be reflected in save/load

Any new field added to `Player`, `GameState`, or any struct that is part of persistent game state must also be:
- Serialized in `save_game()` in `src/systems/save_load.c`
- Deserialized in `load_game()` in `src/systems/save_load.c`

Omitting this causes data to silently reset to zero on load.

Existing saves must remain usable for testing. Add save migrations when game
changes introduce new persistent fields or alter saved maps, preserving the
player's existing progress.

## 8. Code Review

Claude will review your output once you are done.

## 9. Completion Commit Message

The user reviews and commits changes manually. Do not stage changes or create
Git commits unless the user explicitly requests that action. Leave completed
changes uncommitted for the user's review. A request for "one commit per change"
defines change-set scope; it does not authorize committing.

After each change set is complete, provide a suggested Git commit message in
the final response. This signals that the work is finished and ready to commit.

## 10. Building art must match the existing buildings

Use the established `tools/sprites/pixelkit.py` pipeline for town buildings,
matching the existing textures, shading, outlines and doorway scale. Do not
substitute simple rectangle drawings. Inspect new or changed art beside an
existing building at the same scale and in the game renderer before reporting
it complete. Gameplay tests alone do not establish visual quality.

## 11. Every stage must have usable, clearly visible transitions

This is a permanent dungeon and area creation requirement.

- Every playable stage must have its intended entrance, retreat route and onward
  exit or final objective. Outdoor exits belong at the actual map boundary;
  indoor transitions must follow that area's established stair/door rules.
- Validate the actual transition tile and the continuous route to it, not just
  the saved stair/approach coordinates. A reachable coordinate without a working
  transition does not pass validation.
- Main exits must be unmistakable in the game renderer, with readable destination
  labels and discovered minimap markers. Keep labels visible at all four screen
  edges. A single inconspicuous tile is insufficient.
- Terrain shaping, tide changes, encounter preparation, quest props, loot and
  save restoration must preserve these transitions. Check every supported terrain
  state. Intentional boss or puzzle gates must remain visibly identified, and
  the player's retreat must remain available.
- Test every stage of a changed generator across multiple seeds after all map
  postprocessing and encounter placement. Exercise real travel actions through
  the exits; coordinate-only reachability tests are insufficient.
- When repairing existing maps, migrate both active and cached stages in every
  save version. Repair in place, preserving exploration, enemies and wounds,
  loot, equipment, quests and boss victories. Back up local saves before bulk
  migration. Verify an existing affected save in the actual renderer.
