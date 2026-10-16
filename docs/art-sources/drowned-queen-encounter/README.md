# Round 1: Drowned Queen encounter

This fourth approved boss change set follows the Lich King, Forest Necromancer
and Mountain Goblin King. It implements only the Sunken Coast stage-5 Drowned
Queen. Pause for review and a manual commit before the Swamp Demon.

Rising Tide alternates two-turn row and column waves. Storm Throne adds two
fixed flooded pockets after each perpendicular wave sequence. Each signature
leaves a recovery opening. Two separate throne controls drain permanent approach
lanes, while raised stone islands offer shelter. Original stats, regional
scaling, Tidecaller Robes, coast exit, harbor and island progression remain in
effect. See [Sunken Coast](../../sunken-coast.md) for exact timing and save behavior.

## Renderer review

These controlled fixtures were captured through the actual `game_draw()`
renderer with native coast terrain, world props, player/enemy sprites and
warning overlays. Fixture HP is elevated for inspection; they are not completed
playthroughs. No runtime bitmap art was added.

- [Row wave: two turns](row2.png) and [one turn](row1.png)
- [Perpendicular column wave](column.png)
- [Flooded pockets: two turns](pools2.png) and [one turn](pools1.png)
- [Frozen warning](frozen.png)
- [Recovery opening](recovery.png)
- [Horizontal lane drained](drained-h.png)
- [Both permanent approach paths](drained-both.png)
- [Tidecaller Robes reliquary](reward.png)
- [A generated throne clearing](generated.png)
- [Narrow viewport](narrow.png)

Numbered outlines and wave symbols show exact footprints and countdowns.
H/V labels distinguish the two arena controls; SAFE labels identify the raised
stone islands. The panel fits beside the minimap when space permits and below
it in a narrow window. Loot preserves each control and its changing underlay.

## Play testing

Build and run the Debug executable directly, then select **New Game**:

```sh
cmake -S . -B build/debug-shortcuts -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug-shortcuts --target conr --parallel 4
./build/debug-shortcuts/conr --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for this loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage for other attack paths. Buy supplies in Oakhaven, enter its south coast
gate and reach stage 5. `tools/debug_shortcuts.sh` also leaves this boss alive,
but its three pre-completed regions increase coast difficulty through the
existing regional scaling; use the direct command for the baseline encounter.

1. Watch a two-turn row wave, sidestep, then attack during recovery. Confirm the
   next column wave receives a separate warning and keeps its original target.
2. Cross half health during a warning. The old attack must finish before Storm
   Throne is announced. Observe the row, column and flooded-pocket sequence.
3. Escape a pocket using two cardinal actions. Check it bursts at its fixed
   landing site and leaves no lingering damage or blocked floor.
4. Interact on or beside H to drain its row, then V to drain its column. Check
   the crossing paths remain dry and remove danger from those cells immediately.
5. Use a SAFE island for shelter. Test the controls with each class and save/load
   or portal travel while an attack is queued; clocks, bounds and dry paths must
   be retained. Freeze should pause the queued warning.
6. Operate the ordinary blue/amber basin control. The throne's dry paths and
   Mara's completed beacon bits must remain unchanged.
7. Defeat the Queen with melee, arrows or a spell. Collect the original robes,
   leave through the coast exit and check harbor/island progression.
8. Leave the robes unclaimed and revisit after a floor refresh. Recover them
   from the named reliquary; full inventory must leave them available. After
   claiming, dropping or selling must not create a replacement.

## Code and saves

[`drowned_queen.c`](../../../src/game/drowned_queen.c) owns the cycle, fixed wave
bounds, pocket predicates, independent controls, reward restoration and migration.
Actions dispatch it before generic melee logic. The Queen now uses ground
signatures; existing projectile tests retain coverage of other ranged enemies.
[`drowned_queen_renderer.c`](../../../src/renderer/drowned_queen_renderer.c) draws
native terrain, control/island labels, numbered warnings and the encounter panel.

Save version **127** adds `drowned_queen_reward_state`. Existing enemy serializers
save phase, countdown, locked targets and auxiliary wave bounds/pocket coordinates
in active/cached encounters. Appended tile IDs and item underlays preserve the
throne controls, drained lanes and islands. Migration updates living arenas in
place and protects portal landings without healing enemies or resetting progress.
Old victories remain completed, and only identifiable current floor robes become
an unclaimed reward. The ordinary coast basin and quest beacon tile IDs are retained.

Release and Debug builds passed. The final regression suite passed
**4,535/4,535** checks. All **50/50** focused Drowned Queen checks and final
renderer captures passed AddressSanitizer and UndefinedBehaviorSanitizer.
Coverage includes fixed waves/pockets, ordinary movement, freeze and recovery,
all-class controls, basin/beacon independence, loot and inventory capacity,
claimed/unclaimed rewards, active/cached targets and wave bounds, portals,
legacy saves, 100 reachable throne layouts, coast exit and harbor access.
