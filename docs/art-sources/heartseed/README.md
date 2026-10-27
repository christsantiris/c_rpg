# The Stolen Heartseed — native renderer review

These captures come from `test_heartseed_renderer` in
[`tools/tests/debug_shortcuts_test.c`](../../../tools/tests/debug_shortcuts_test.c).
It renders the actual game with SDL's software renderer at 1280×720, using the
play area's normal viewport width. PNG files are direct format conversions of
the captured BMP surfaces.

Tamsin uses the established transparent character renderer in
[`npc_renderer.c`](../../../src/renderer/npc_renderer.c), with a green hood,
leather coat, and bow. Her town capture shows the player and existing residents
at the same native scale, on the existing grass texture. The quest adds no
building art or changes to the town's roads and doors.

The orders, root arch, stakes, heartseed resting place, violet bindings, labels,
and interaction prompts are drawn in
[`elderwood_renderer.c`](../../../src/renderer/elderwood_renderer.c). The before
and after captures show that cut stakes remain, their binding lines disappear,
and the freed guardian leaves while the other Treefolk remains hostile.

| View | Capture |
| --- | --- |
| Tamsin beside Oakhaven's north road | [Town](tamsin-town.png) |
| Stage 2 Dark Elf orders | [Camp](orders-camp.png) |
| Stage 4 guardian and three bindings | [Bound grove](bound-grove.png) |
| Three bindings cut; seed ready to recover | [Freed grove](freed-grove.png) |
| Completed objectives awaiting Tamsin's bounty | [Journal](journal.png) |

Rebuild and run `build/debug-shortcuts/debug_shortcuts_test` from the project
root to regenerate the BMP captures under `/tmp/heartseed-*.bmp`. The harness
uses an isolated temporary working directory for saves.
