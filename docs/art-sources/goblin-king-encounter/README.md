# Round 1: Mountain Goblin King encounter

This third approved boss change set follows the Lich King and Forest
Necromancer. It implements only the Goblin Mountains stage-4 Goblin King.
Pause for review and a manual commit before the Drowned Queen.

The stationary King has Fortress Chief and Powder King phases, fixed axe
lanes, two-turn bomb pockets, three finite powder kegs and recovery openings.
Original stats, regional scaling, shield stats/class and mountain shortcut
progression remain in effect. The keg counter is available to every class.
See [Goblin Mountains](../../goblin-mountains.md) for exact combat timing,
damage, reward recovery and save migration.

## Renderer review

These scripted fixtures use the actual `game_draw()` renderer and native
terrain, sprites, equipment art and warning overlays. Fixture HP is elevated
for inspection; these images are not completed playthroughs. Keg art is a
native 24×24 pixel pattern with rounded wood staves, iron hoops, powder marks
and lit fuses; no runtime bitmap asset was added.

- [Fixed axe warning](axe.png)
- [Keg fuse: two turns](fuse2.png) and [one turn](fuse1.png)
- [Keg stagger and spent barrel](stagger.png)
- [Bomb pockets: two turns](bombs2.png) and [one turn](bombs1.png)
- [Frozen boss warning](frozen.png)
- [Safe recovery opening](recovery.png)
- [Shield reliquary and existing shortcut](reward.png)
- [A generated summit arena](generated.png)
- [Narrow viewport](narrow.png)

Numbered outlines identify each signature footprint and keg fuse. A tile with
both warnings shows attack turns / keg turns. The panel shows phase, health,
attack countdown, frozen state and remaining kegs. It fits beside the minimap
when space permits and below it in a narrow window. Loot does not obscure keg
warnings or prevent priming; fuse changes preserve every covered item underlay.

## Play testing

Build and run the Debug executable directly, then select **New Game**:

```sh
cmake -S . -B build/debug-shortcuts -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug-shortcuts --target conr --parallel 4
./build/debug-shortcuts/conr --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for this loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage to test other attack paths. Use a fresh character without shortcut
unlocks: `tools/debug_shortcuts.sh` automatically marks the mountain boss defeated
and therefore skips this encounter. Buy supplies in Oakhaven, enter its ordinary
north mountain gate and reach stage 4, Crown Peak.

1. Observe an axe warning, sidestep, then counterattack during recovery.
2. Stand beyond a keg to draw an axe through it. Check the two-turn fuse and
   marked 3×3 blast area. Escape using ordinary movement.
3. Interact on or beside an unlit keg with contextual `A` by default. Catch the
   King in its blast and check that the warning cancels and two complete recovery
   turns remain. Verify that spent kegs do not refill on revisits.
4. Cross half health during an axe warning. Its old lane must finish before
   Powder King is announced. Observe alternating axes and fixed bomb pockets.
5. Move out of a bomb pocket using two cardinal actions. Bombs must strike the
   original landing sites, then leave a recovery turn.
6. Freeze the King during a warning. His countdown pauses while already lit keg
   fuses continue. Save/load and portal travel should preserve both clocks.
7. Defeat him with melee, arrows, a spell or a keg. Check the original shield,
   shortcut discovery prompt and route to the opposite town.
8. Leave the shield unclaimed and revisit after a floor refresh. Recover it from
   the named reliquary. Full inventory must leave it available; first claim,
   later sale or dropping must not create another copy.

## Code and saves

[`goblin_king.c`](../../../src/game/goblin_king.c) owns the boss clock, locked
warning predicates, keg fuses, blast/stagger behavior, interactions, persistent
shield recovery and migration. `actions.c` dispatches signatures before generic
melee logic and processes blast kills through ordinary death/quest/XP/loot hooks.
[`goblin_king_renderer.c`](../../../src/renderer/goblin_king_renderer.c) draws the
native kegs, numbered warnings and panel.

Save version **126** adds `goblin_king_reward_state`. Existing enemy serializers
preserve phase, timing and both bomb targets across active/cached encounters.
Keg fuse and spent state are stored as appended tile IDs and item underlays.
Migration adds kegs in place, protects portal landings and restarts old unmarked
attack timing without healing enemies or resetting victories, quests, loot,
exploration or player coordinates. Known old victories remain defeated; only
an identifiable current floor shield establishes an unclaimed reward.

Release and Debug builds passed. The final regression suite passed
**4,487/4,487** checks. All **51/51** focused Goblin King checks and the final
renderer captures passed AddressSanitizer and UndefinedBehaviorSanitizer.
Coverage includes fixed patterns, freeze, recovery, actual cardinal movement,
all-class counters, keg deaths and rewards, floor/inventory capacity, saved
fuses/targets, portal revisits, old-save migration and 100 generated summits.
