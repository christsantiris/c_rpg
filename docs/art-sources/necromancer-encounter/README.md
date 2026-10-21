# Round 1: Forest Necromancer encounter

This is the second approved boss change set, after the Lich King. It implements
only the stage-4 Forest Necromancer. The Mountain Goblin King is next, after
review and a manual commit.

The encounter has Grove Binder and Withered Grove phases, fixed spirit-bolt
lanes, two-turn root crossings, two finite grave waves and three consecratable
graves. Interact on or beside a currently warned grave to cancel that wave and
open two recovery turns. Each unconsecrated patch can raise one servant per
wave. Summons receive no repeat rewards and skip their spawning turn. Original
boss stats, regional scaling, Necromancer's Cloak stats/class and shortcut
progression remain in effect. See [Haunted Forest](../../haunted-forest.md) for
exact timing, reward recovery and old-save behavior.

## Renderer review

These are scripted fixtures captured with the actual `game_draw()` renderer,
using native forest terrain, world props, enemy sprites and equipment art.
Fixture HP is elevated for inspection; these images do not represent completed
playthroughs. No runtime bitmap art was added.

- [Spirit bolt warning](bolt.png)
- [Grave wave warning](wave.png)
- [Consecrated grave and interrupted-wave recovery](consecrated.png)
- [Root crossing: two turns](roots2.png) and [one turn](roots1.png)
- [Frozen warning](frozen.png)
- [Recovery opening](recovery.png)
- [Guaranteed cloak reliquary and existing shortcut](reward.png)
- [A generated Forest stage-4 grove](generated.png)
- [Narrow viewport](narrow.png)

Active graves use tomb props; consecrated graves have a pale blue cross.
Numbered warning tiles and the boss panel show the attack and countdown.
Root branches sit below the numbers to keep the warnings readable. Warnings
work over floor loot, and covered grave interactions preserve the loot. The
boss status now appears in the right sidebar in place of the control hints,
keeping the playfield clear. On very short windows it appears above the
character details. The captures above show the original overlay layout.

## Play testing

Build and run the Debug executable directly, then select **New Game**:

```sh
cmake -S . -B build/debug-shortcuts -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug-shortcuts --target conr --parallel 4
./build/debug-shortcuts/conr --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for this loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage to exercise the other attack paths. Boss testing requires a fresh
character without `--unlock-shortcuts`: `tools/debug_shortcuts.sh` automatically
marks the Forest boss defeated to unlock its road. Buy supplies in Oakhaven
and enter the Haunted Forest through the ordinary west gate, reaching stage 4.

1. Observe a bolt warning, sidestep, and attack during recovery.
2. Let the first grave wave resolve. Check that servants rise only on marked
   patches and begin acting on the following turn.
3. In another fresh encounter, consecrate a warned patch. Check the entire wave
   is canceled and the recovery opening lasts two complete turns.
4. Consecrate the other patches; their spent appearances should survive saving,
   backtracking and Return to Town/portal travel.
5. Cross half health during a pending bolt. Its old warning should finish before
   the new phase begins. Freeze a warning and check its countdown pauses.
6. Escape crossing roots using two cardinal moves, with no special ability.
   Observe that roots and grave waves resolve on separate turns.
7. Defeat the boss with melee, bow or spell. Check the original cloak reward and
   shortcut discovery prompt, then use the shortcut to the opposite town.
8. Leave the cloak unclaimed and revisit after a floor refresh. Recover it from
   its labeled reliquary; full inventory must leave it available. Claiming it
   once, then selling or dropping it, must not generate a replacement.

## Code and saves

[`necromancer.c`](../../../src/game/necromancer.c) owns the encounter clock,
warning predicates, grave interactions, reward restoration and migration.
The existing spawn/action/travel code dispatches these hooks; the game renderer
draws the props and warnings, and the info panel draws the boss status. Existing
Crypt Conjurer behavior is preserved.

Save version **125** adds `necromancer_reward_state` and retains both bits of
`boss_wave_used` instead of treating that field as a boolean. The shared enemy
serializer stores pending targets, phase and timing in active and cached maps;
grave consecrations are saved as tiles. Legacy migration prepares living arenas
in place, protects portal landings, and restarts unmarked timings without
healing enemies or resetting progress. Previous victories stay defeated, and
only a known current legacy floor cloak establishes an unclaimed reward.

Release and Debug builds passed. The final normal regression suite passed
**4,436/4,436** checks. The full AddressSanitizer/UndefinedBehaviorSanitizer run
passed **4,434/4,434** before the last reward-pickup and coverage refinements;
the final focused **40/40** Necromancer checks and final renderer captures also
passed both sanitizers. The full sanitizer regression uses a 64 MiB stack limit
because existing tests allocate large game states on the stack.
