# The Missing Spice Skiff — native art and renderer review

The runtime atlas is
[`assets/images/spice-skiff-props.bmp`](../../../assets/images/spice-skiff-props.bmp),
generated with the established pixelkit pipeline:

```sh
.venv/bin/python tools/sprites/spice_skiff.py
```

[`spice_skiff.py`](../../../tools/sprites/spice_skiff.py) derives the damaged
36×36 hull from Saltwind's existing skiff and reuses the marsh's bark and leaf
palettes for a 72×72 root arch. Sealed and emptied caskets are 24×24. The atlas
is 156×72 with transparent backgrounds. These props are drawn at their native
size, preserving the ground, water, roots, landings, supplies, and enemy art.

![Native prop atlas](native-props.png)

The captures below come from `test_spice_skiff_renderer` in
[`tools/tests/debug_shortcuts_test.c`](../../../tools/tests/debug_shortcuts_test.c).
They use the actual SDL game renderer at 1280×720 and its normal play-area
viewport. PNGs are direct format conversions of the captured BMP surfaces.

| View | Capture |
| --- | --- |
| Existing merchant doorway and its road connection | [Merchant door](merchant-door.png) |
| Corvin's complete quest offer and acceptance prompt | [Offer](corvin-offer.png) |
| Stranded skiff and searchable manifest on Saltwind 2 | [Stranded skiff](stranded-skiff.png) |
| Root arch, cargo, Giant Crab, and Lizardfolk on Saltwind 3 | [Guarded cargo](guarded-cargo.png) |
| Empty cargo casket after recovery | [Recovered cargo](recovered-cargo.png) |
| Both completed objectives awaiting the merchant bounty | [Journal](journal.png) |

The player and shore enemies provide the scale reference in the adventure
captures. The existing supply chest stays visible and distinct from the quest
casket. NPC dialogue fits the game's 192-byte dialogue buffer.

Rebuild and run `build/debug-shortcuts/debug_shortcuts_test` from the project
root to regenerate `/tmp/spice-skiff-*.bmp`. The harness keeps saves in an
isolated temporary working directory. Gameplay and save migration coverage is
in [`tests/test_spice_skiff.c`](../../../tests/test_spice_skiff.c).
