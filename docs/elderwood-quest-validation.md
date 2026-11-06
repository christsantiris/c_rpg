# Elderwood boss quest validation

Validated on November 6, 2026.

## Implemented behavior

- Ranger Tamsin stands beside Stillbury's eastern road at `(34,13)`.
- Clear Elderwood has one objective: defeat Elderthorn on stage 5.
- Acceptance respects the three-active-quest limit. Defeating the boss without
  accepting also completes the objective, immediately under Completed.
- Tamsin pays 120 gold and 800 score in Stillbury, once, including after loading.
  Elderthorn's existing 180 gold and 2,000 score are separate.
- The Heartseed orders, binding puzzle, altar and special quest encounters are
  retired. Regular forest enemies, bramble cutting and Elderthorn's combat remain.
- Elderwood still opens the Sunken Coast road. The Catacombs cliff and Ridgeshire
  detour are a separate geography change; this feature does not alter world roads.

## Checks

`conr` and `test_runner` build successfully. The full suite passes
**9,010 / 9,010 checks**. New tests in `tests/test_elderwood_quest.c` cover
acceptance, decline, held keys, quest limits, one-objective journal entries,
real boss kills with and without acceptance, pending rewards and repeat-payment
prevention. Every stage is traversed through actual retreat and onward transitions
across seeds **19, 107 and 1301**, including cached revisits, the coastal road
and the return through Stillbury's eastern gate.

Save version **246** persists the new quest state. Legacy fixtures verify
Tamsin's placement around a player and dropped equipment, existing boss victories,
retirement of active and cached Heartseed props, and preservation of wounds,
loot underlays, exploration, money and recorded old quest progress. Old quest
counters remain compatibility data and never take an active quest slot.

Native SDL software rendering was inspected at **1280×720** and **640×480**:
Tamsin beside existing buildings, the offer and reward dialogue, Active and
Completed journals, Elderwood stages 2 and 4 without the old props, and a
migrated Stillbury fixture. An existing local save was loaded and rendered.
Local save files were backed up before work and remain byte-for-byte unchanged;
all test saves were written to isolated temporary directories.
