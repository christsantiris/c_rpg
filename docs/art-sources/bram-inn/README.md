# Bram: Oakhaven Innkeeper

Approved Chapter 1 step, 2026-11-01. Bram now occupies Oakhaven's Inn counter
at (28, 7), using the existing apron-and-mug innkeeper art. His town-square
placement and the duplicate innkeeper in Stillbury's Tavern are removed. The
shared guild interior no longer creates another copy of that innkeeper.

Approach Bram and press `T` for his Lich King warning and supply advice. The
warning acknowledges the Lich King's defeat. Bring Mira Home, its journal entry,
quest offer, rescue interaction, swamp placement and special Vampire Captor
combat behavior are retired. Ordinary Vampires remain. Historical save fields
and tile IDs are reserved; no new V2 save migration was added.

These captures record the earlier Bram relocation step and predate
[Save Elowen](../save-elowen/README.md), which now implements her imprisonment,
escort and permanent Inn residence. [Alder and Dain's post-Lich arrivals and
chained adventures](../oakhaven-adventure-chains/README.md) are also implemented.

## Renderer inspection

These captures use the actual `game_draw()` renderer and existing NPC art:

- [Bram at Oakhaven's counter](oakhaven-counter.png): apron appearance, readable
  name label, and an open approach from the room's entrance.
- [Lich King warning](oakhaven-dialogue.png): dialogue anchored beside Bram,
  with the southward dungeon direction and supply advice fully visible.
- [Stillbury's empty counter](stillbury-counter.png): no duplicate Bram sprite
  or name label; the other residents retain their positions.
- [Oakhaven's exit](oakhaven-retreat.png): the south doorway remains available.

## Validation

- `conr` and `test_runner` build successfully; **9,299/9,299 assertions pass**.
- Three seeded native-renderer fixtures exercise actual building-door actions,
  continuous walks from each entrance to its counter and back to its exit,
  the unique Bram placement, proximity checks, and current save/load round trips.
- Quest tests confirm reserved Mira state consumes no active slot or journal
  row. Three seeded expeditions check all five generated swamp stages for the
  retired encounter's absence and reachable transitions. Existing travel tests
  exercise both swamp entrances, retreat, victory, onward roads and saving.
- `git diff --check` passes. Changes remain uncommitted for manual review.
