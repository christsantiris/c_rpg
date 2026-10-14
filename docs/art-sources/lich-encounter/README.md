# Round 1: Lich King encounter

The first approved boss update implements the Dungeon floor-5 Lich King.
The remaining bosses are unchanged. Base health, attack, experience and the
Cryptblade's equipment stats remain intact; the existing regional scaling
still applies.

## Encounter

- **Crypt Sovereign:** a one-turn fixed necrotic lance, followed by recovery.
  Stone pillars block both the warning footprint and its projectile/damage.
- **Unbound Soul:** starts on the next attack preparation at or below half
  health. One warned wave raises up to two Lich Shades; later attacks alternate
  lances and two-turn soul rings. A ring occupies cardinal distance three from
  the stationary Lich, leaving its inner and outer floor safe.
- **Burial seals:** Interact on or beside each. Every active seal supplies 15
  percentage points of damage reduction and +2 soul-ring damage. Silencing both
  cancels the current preparation and grants two complete recovery turns.
  Already announced attacks never grow when a health threshold is crossed.
- **Recovery and freeze:** recovery has no extra adjacent Lich attack. Freeze
  pauses countdowns and preserves locked targets. UI screens do not advance
  the encounter's combat-turn clock.
- **Finite summons:** marked positions are saved. Spawned Shades start acting
  next turn, respect enemy capacity, and award no loot, score or experience.
  Occupied grave positions are skipped rather than retargeted onto the player.
- **Reward:** the original Cryptblade normally drops as floor loot. A full
  floor list reserves it in a labeled reliquary without deleting other items.
  A full inventory leaves it available. Unclaimed rewards survive floor
  refreshes and new expeditions; first pickup records the claim permanently.
  Reliquary pickup gives the same item-value score as normal ground pickup.

Existing Lich Guards, key/door access, return stairs, Elowen's quest and other
bosses retain their progression. The new seals and pillars use the existing
native world-prop art; no runtime bitmap assets were added. New hidden random
traps are removed from the signature-attack chamber, while already revealed
legacy traps retain their saved state.

## Renderer review

The images are controlled fixtures captured through `game_draw()` using the
actual tile, enemy, equipment, warning, health and message renderers. Fixture
HP is elevated to inspect all states; these are not completed gameplay runs.

- [All encounter states](states.png)
- [Necrotic lance](lance.png) and [recovery](recovery.png)
- [Grave wave warning](wave.png)
- [Soul ring: two turns](ring2.png) and [one turn](ring1.png)
- [Frozen warning](frozen.png)
- [Silenced seals](silenced.png)
- [Reward reliquary](reward.png)
- [Narrow play-area layout](narrow.png)

Outlines and numbered tiles identify the exact footprint; the encounter panel
names the phase and attack, displays its remaining turns, and highlights
recovery. Warnings remain visible over loot, and seals work beneath item markers.

## Play testing

For a Warrior play test, start a new character with:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Use `--weapon bow` and choose Rogue for the bow test, or `--weapon staff` and
choose Mage for spell testing. Buy supplies in Oakhaven, enter its dungeon and
descend to floor 5. The debug shortcut unlocks preserve their existing enemy
scaling effects; there is no new direct boss-warp option in this change.

1. Pick up the golden key with `P`, open the boss door and enter the chamber.
2. Stand in a warned lance once to confirm damage. On the next cycle, step
   sideways or use a pillar; confirm the struck lane stays fixed.
3. Attack in the green recovery opening. Cross half health during a pending
   lance; it should resolve as a lance before Unbound Soul is announced.
4. Watch both grave positions before the Shades appear. The wave must not repeat.
5. Escape a soul ring by stepping inside or outside its numbered diamond.
6. Use `A` (or the rebound contextual move-left key) beside each seal. Silence
   the second during a preparation and confirm the two-turn opening.
7. Save/load during a warning and during recovery. Return to Town and use the
   return portal; targets, phase, silenced seals and finite-wave use should persist.
8. Kill with melee, a bow and a spell in separate fresh test characters. Confirm
   Cryptblade, the boss journal entry and the open upward return staircase.
9. Leave an unclaimed reward, revisit or begin a new dungeon expedition, and
   recover it from the labeled reliquary. A full inventory must not consume it;
   claiming, dropping or selling it must not award another copy.

## Code and validation

The cycle, arena interaction, reward restoration and migration are isolated in
[`lich.c`](../../../src/game/lich.c). The existing spawn/travel code initializes
arena tiles and summons; actions dispatch the boss before generic melee logic
and apply seal protection consistently to melee, cleave, bows and damage spells.
[`game_renderer.c`](../../../src/renderer/game_renderer.c) draws native props,
fixed numbered warnings and the encounter panel.

Save version **124** adds four enemy fields (signature, wave-use flag and second
summon coordinates) and the global reward claim state. Existing enemy serializers
cover active and cached encounters; seal states are stored in their maps.
Migration updates arenas in place and restarts old unmarked bolt timing safely,
without healing the boss or changing existing victories, loot, position, quests
or exploration. Saved portal landing tiles remain free of new cover. Previously held/sharpened legacy Cryptblades remain claimed;
a known current unsharpened floor reward can establish an unclaimed gift.

Release and Debug builds passed. The normal regression suite passed **4,396/4,396**
checks. The full AddressSanitizer/UndefinedBehaviorSanitizer regression run passed
**4,392/4,392** before the final reward-score/legacy-claim/portal refinements; the final
focused **45/45** Lich checks and final renderer fixtures also passed under both sanitizers.
The sanitizer regression run uses a 64 MiB stack limit because existing tests
allocate large game states on the stack; the game itself uses a static state.

Coverage includes fixed targeting, cover/projectile clipping, ring damage and
walking escapes, freeze and threshold crossings, recovery, contextual interaction
under loot, enemy capacity, all five melee/ranged/spell victory paths, no summon
rewards, full inventories/loot lists, active/cached saves, portal return, fresh
expeditions, legacy migration and 100 generated arenas. `git diff --check` and
focused `-Wall -Wextra` syntax checks passed. One earlier run failed the existing forest exit-edge fixture; it passed on rerun.
Forest generation is unchanged. Balance still needs player feedback.

Paused after this boss for manual review and commit. Next: Forest Necromancer.
