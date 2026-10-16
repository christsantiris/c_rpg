# Royal Arcanist encounter

This fifteenth approved boss change set updates only the Royal Arcanist in
**Crown Chapel**, the castle's fourth floor. Pause for review and a manual
commit before Lord Veyr. The recently updated Castellan and the other regional
bosses retain their encounters.

Warded Scholar retains the original two-action blast and ward effects.
Shattered Liturgy adds two-action crosses and sequential west/east ward lanes.
Disabling a prepared lane's source cancels it and its barrier change, leaving
two recovery actions. Victory preserves the 100 gold and onward stair unlock
and reserves the new Crown Chapel Vestments. See the
[castle encounter details](../../castle-design.md) for exact geometry and saves.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer and the
existing chapel, Arcanist, crystal, gate and chest art. Phase two cools the
existing boss sprite. The new robes use established equipment and player art
with purple cloth, gold trim and cyan accents. Fixture health is raised for
inspection; these are not completed playthroughs. No runtime bitmap assets or
building artwork were added.

- [Generated chapel and original guard detail](generated.png)
- [Warded blast: two actions](blast2.png), [one action](blast1.png) and [normal recovery](recovery1.png)
- [Chapel Cross: two actions](cross2.png), [one action](cross1.png) and [frozen preparation](cross-frozen.png)
- [West Ward Lane: W2](west2.png) and [W1](west1.png)
- [Separately warned East Ward Lane: E2](east2.png) and [E1](east1.png)
- [Combined spell and gate warning: X2](combined-warning.png)
- [Interrupted lane: two full recovery actions](ward-counter2.png) and [one remaining](ward-counter1.png)
- [Full-inventory reward reservation](reserved-reward.png) and [Vestments equipped on Mage](vestments-equipped.png)
- [Retaliation against corridor fire](corridor-warning.png) and [400-pixel gameplay viewport](narrow.png)

Plain numbers identify blast and cross countdowns. W/E identify the source
pedestal for each lane; C/O identify gate closures/openings. Combined X2/X1
labels include both a gate change and a damaging warning. These
use the same saved masks and barrier plan as resolution. Pedestals are labeled
WEST/EAST with A for the default Interact key, changing to OFF when spent.
The HUD retains phase, health, signature, remaining actions, freeze and recovery
on the narrow viewport. The reserved chest labels its reward until claimed.

## Play testing

Use a Debug build and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon staff --scrolls magic-arrow,fireball,heal
```

Choose Mage for that loadout. Use short-sword/Warrior or bow/Rogue to check
other combat paths. Buy supplies, reach the castle through Rosemoor's east
Crown Road, defeat the Castellan on floor 2 and climb to Crown Chapel on
floor 4. An existing chapel save also works. Previously defeated Arcanists
remain defeated and reserve the new robes once when migrated.

1. Approach the Arcanist. Check Warded Scholar, the five-stone diamond warning
   and its 2 → 1 countdown. Walk twice beyond the diamond and counter during
   its recovery. Cross half health during preparation: the queued blast must
   keep its small original footprint before Shattered Liturgy begins.
2. Check the phase-two cross. Its three-stone cardinal spokes must leave safe
   diagonals reachable by ordinary walking. Warning cells and facing must not
   follow the player. Check chapel corners, tables, pillars and both sides of
   the barrier gates. No attack should require a movement spell to escape.
3. Observe West Ward Lane on a fixed row, then East Ward Lane on a separately
   declared column. Each must give two full actions of warning. The east lane
   can target the player's new position only when its own warning starts.
   Finish the sequence and use its recovery before the next cross.
4. While a west/east lane is preparing, Interact beside its matching pedestal.
   That lane and its pending gate changes must disappear. Check two full
   recovery actions after Interact, on all three classes. Disabling the other
   pedestal must leave the current lane fixed and skip that source's later
   preparation. With both spent, crosses and normal blasts continue.
5. Check the original ward effects: two active pedestals reduce incoming boss
   damage by 40% and add 8 to attack damage; one gives 20% and +4; neither gives
   any bonus. Repeat with loot covering a pedestal. The interaction must update
   its underlying tile without deleting the item.
6. Observe C/O gate previews. Only announced cells should change. Stand on a
   closing cell or put an enemy/loot there; it must stay open. Open a gate during
   an existing preparation and confirm no new unmarked damage cells appear.
   Paths around cover and to both pedestals and the onward stair remain usable.
7. Freeze during each signature and between the two lanes. Both the warning
   and gate plan must wait. Save/load and change castle floors: the same target,
   footprint, source, countdown, phase and recovery should resume. A castle
   retreat restores a living Arcanist, wards and gates according to existing
   regrouping rules; a defeated Arcanist remains dead.
8. Fire each bow from its maximum range along the north approach. Once hit,
   the Arcanist must answer with a warned attack while remaining in the arena.
   Longbows must not be able to damage an inert boss indefinitely. Hiding or
   retreating far outside the arena cancels the queued warning.
9. Defeat him with melee, arrows and each damage spell. Check 100 gold, the
   floor-5 unlock and the southeast Vestments chest. Interact beside it to
   claim Mage robes with +4 defense, +45 maximum MP and 15% cheaper spells.
   Check the inventory icon, equipped armor slot and player appearance.
10. Repeat with full inventory and maximum floor loot. The reward stays
    reserved through retreat, travel and save/load. Claim it once after freeing
    a slot. Dropping or selling a claimed copy cannot regenerate it. Load an
    older save with a historical victory: only the new robe reservation is
    granted; gold, XP and the boss fight must not repeat.

Automated checks cover all four signatures, phase/freeze timing, both ward
counters for every class, chapel-wide ordinary walking escapes accounting for
gate plans, all bow ranges, all damage kill paths, reward claims, active/cached
save round trips, legacy pending attacks and completed victories. Manual play
testing remains necessary for balance and readability with all guards active.
