# Save Elowen: rescue and homecoming

Approved Chapter 1 feature, 2026-11-01. Bram now offers Save Elowen at
Oakhaven's Inn. Acceptance preserves existing dungeon progress. Elowen starts
as a captive in the Lich King's final chamber, with a walkable ring around
her so she cannot obstruct its entrance, combat props or return stairs.

Defeating the Lich frees her automatically, including without a quest assignment
or with three other active quests. Save Elowen moves straight to Completed;
its reward reminder remains visible. She follows as a noncombat actor, cannot
take damage, and never blocks the player's movement. Ordinary walking leaves
her in the player's previous square; transitions and teleports bring her along.
Stationary NPC conversations take priority over talking to the escort.

Speak to Bram to reunite them and receive 80 gold once. Elowen then stays at
the Inn and reveals that she overheard the Lich claiming to be in league with
the King. Malakar's role is not revealed. Items dropped at her future Inn
position are preserved beside her when she arrives.

The Broken Seals quest, its objectives and guardian groups are retired. The
Lich encounter's two combat seals remain. Save version 229 serializes the
escort's position alongside quest state. As approved for this V2 rewrite,
there is no new migration of the old quest; testing uses fresh isolated saves.

The all-class [Crypt Ring reward](../crypt-ring/README.md) is now implemented.
Aldric recruitment and remaining starting cast changes belong to separate steps. [Alder and Dain's post-Lich arrivals and chained
adventures](../oakhaven-adventure-chains/README.md) are now implemented.

## Native renderer review

These screenshots use `game_draw()` and the quest journal renderer with the
existing Elowen art. Fixtures use elevated player stats and a reduced boss HP
for route and state validation; they do not establish normal combat balance.

- [Captive dialogue](dungeon-captive.png): visible inside the final chamber.
- [Automatic rescue](dungeon-rescued.png): Elowen identifies Bram and the Inn.
- [Completed journal](journal-completed.png): objective checked, active slot
  freed, 80-gold reward pending and a clear reunion instruction.
- [Escort in Oakhaven](oakhaven-escort.png): arrival through the southern route.
- [Bram's reunion](inn-reunion.png): one-time payment and permanent residence.
- [Elowen's testimony](inn-testimony.png): the first clue against the King.

## Validation

- Both game and test runner build; **9,561/9,561 assertions pass**.
- Six seeded rescue journeys cover Warrior, Mage and Rogue, with and without
  accepting Bram's quest. Tests exercise actual moves and stairs across all
  five floors, the door key, captive conversation, melee/spell victory,
  invulnerable following, physical return and the one-way Return to Town spell.
- Unassigned rescues work with a full active journal. Reunion works with a
  full pack and journal, preserves dropped loot, and cannot pay twice.
- Saving/loading preserves captivity, escort position, the Completed entry,
  reunion, testimony and claimed reward. Replayed dungeons do not recapture her.
- A native renderer harness verifies 256 seeded final floors: each contains
  Elowen, retains connectivity, and permits actual key pickup, entry into her
  chamber, conversation, retreat to the entrance and ascent to the prior floor.
- Existing arena checks retain the Lich's two seals, two pillars and valid enemy
  positions. Existing dungeon generation, travel and save tests remain passing.
- `git diff --check` passes. Changes are left uncommitted for manual review.
