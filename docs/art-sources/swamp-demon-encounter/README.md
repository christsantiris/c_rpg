# Round 1: Swamp Demon encounter

This fifth approved boss change set implements only the Blackwater Swamp
stage-4 Swamp Demon. Pause for review and a manual commit before the Minotaur.

Bog Tyrant alternates a one-action frontal cleave and two-action aimed rot pool.
Unshackled Demon adds a second pocket and a two-action miasma fan. Pools persist
for two further actions, with a strict ten-tile limit. Three reusable root vents
drain nearby rot, cancel a signature and grant two counterattack actions, then
cool for four actions. Original stats, regional scaling, Demonic Sword, central
shortcut, town road and Mira/Bram rescue progression remain in effect.
See [Blackwater Swamp](../../swamp.md) for exact timing and save behavior.

## Renderer review

These controlled fixtures use the actual `game_draw()` renderer with native
swamp terrain, existing world props, player/enemy sprites and numbered overlays.
Fixture HP is elevated for inspection; they are not completed playthroughs.
No runtime bitmap art was added.

- [Frontal cleave](cleave.png) and [recovery](recovery.png)
- [Aimed pool: two actions](pool2.png) and [one action](pool1.png)
- [Frozen pool warning](frozen.png)
- [Lingering rot: two actions](rot2.png) and [one action](rot1.png)
- [Twin pocket warning](twins.png) and [twin lingering pools](twin-rot.png)
- [Hazard/vent beneath loot](loot.png)
- [Vent drained, cooldown four](vent4.png)
- [Miasma fan](fan.png)
- [Demonic Sword root cache](reward.png)
- [Generated clearing](generated.png) and [generated fan](generated-fan.png)
- [Narrow viewport](narrow.png), [frozen warning](narrow-frozen.png) and [recovery](narrow-recovery.png)

Slash, bubble and fan symbols distinguish signatures. Plain numbers count down
warnings; R2/R1 count down active rot. VENT labels show independent cooldowns.
Expiry overlays remain visible over loot. The encounter panel sits beside the
minimap where possible, otherwise below it; narrow frozen warnings show `*`.

## Play testing

Build and run the Debug executable directly, then select **New Game**:

```sh
cmake -S . -B build/debug-shortcuts -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug-shortcuts --target conr --parallel 4
./build/debug-shortcuts/conr --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage for other attack paths. Reach Stillbury through the forest, buy supplies
there and enter the ordinary north swamp gate. Reach stage 4. A save with a living
Demon can also be loaded; its damaged health and rescue progress migrate in place.
**Do not use `tools/debug_shortcuts.sh` or `--unlock-shortcuts` for this fight:**
they mark the Swamp Demon already defeated.

1. Step beside the Demon, observe the cleave and step back within its one-action
   warning. Counterattack during recovery; no extra adjacent hit should occur.
2. Wait for a pool warning. Move twice to leave its cross. Observe R2, then R1,
   then ordinary mud. Standing in rot takes one lingering hit per action.
3. Cross half health during a warning. Its footprint must remain fixed. Observe
   the next phase's cleave, twin pocket and fan cycle; walk out of the fan.
4. Freeze a pending pool. Its targets/countdown must pause, while any already
   active rot and cooling vents continue their visible clocks.
5. Interact on or beside a vent. Check nearby rot drains, the queued attack is
   canceled and two full counterattacks are available. A cooling vent cannot be
   spammed; it becomes ready after its four displayed recovery actions.
6. Save/load and portal out during a warning, active pool or vent cooldown.
   Backtrack from both towns. Timers, targets, health and rescue progress persist.
7. Defeat the Demon with melee, bow or spells. Collect the original Demonic Sword
   and check both the central shortcut and safe road unlock normally.
8. Leave the sword unclaimed and revisit after a floor refresh. Recover it from
   the named root cache. Full inventory leaves it available; selling/dropping a
   claimed sword must not create another reward.

## Code, saves and validation

[`swamp_demon.c`](../../../src/game/swamp_demon.c) owns the fixed signature cycle,
walking-escape check, rot clocks, vent interactions, reward and migration.
[`swamp_demon_renderer.c`](../../../src/renderer/swamp_demon_renderer.c) draws
native props, signature symbols, expiry labels and the encounter panel.
Actions dispatch floor clocks before frozen enemies, and signatures before generic
melee. Save version **128** adds `swamp_demon_reward_state`; existing enemy fields,
appended map tiles and loot underlays retain active/cached encounter timing.
Migration protects hidden portal landings and preserves previous progress.

Release and Debug builds passed. The final regression suite passed
**4,589/4,589** checks. All **54/54** focused checks and final renderer captures
passed AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes ordinary
movement, phase crossings, fixed targets, freeze, cover/actors, rot expiry under
stacked loot, all-class vents, full loot/inventory, all five victory paths,
claimed/unclaimed rewards, caches, portals, old saves and 100 generated arenas.
Existing swamp travel tests cover both town approaches, rescue and shortcuts.
