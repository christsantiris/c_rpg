# Castellan encounter

This fourteenth approved boss change set updates only the Castellan in the
castle's second floor, **Iron Keep**. Pause for review and a manual commit before
the Royal Arcanist. See [castle encounter details](../../castle-design.md) for
geometry, recovery, gate coordinates and save migration.

**Shield Commander** retains frontal protection, a fixed charge and close
shield swing. **Broken Formation**, at half health, adds a finite marked guard
rally and alternates wider two-action sweeps with warned charges. Side levers
can interrupt charges, and victory reserves the new **Castellan's Bulwark** in
the southeast armory chest. Castle travel, regrouping and progression remain.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer, existing
castle sprites, gate/lever/muster/chest props and native equipment art. The new
shield uses the established tower shield shape with steel, burgundy and gold
in both its equipment icon and the player's off hand. Phase two warms the
existing Castellan sprite; shield-facing markers still dim during recovery.
Fixture health is raised for inspection. These are not completed playthroughs.
No runtime bitmap assets were added.

- [Generated Iron Keep and guard detail](generated.png)
- [Charge warning](charge1.png), [close shield swing](swing1.png) and [normal recovery](recovery1.png)
- [Formation Sweep: two actions](sweep2.png), [one action](sweep1.png) and [frozen warning](sweep-frozen.png)
- [Guard rally: G2](rally2.png), [G1](rally1.png) and [arrivals](rally-arrival.png)
- [Charge crossing a side gate](gate-charge.png), [interrupted charge: two recovery actions](gate-recovery2.png) and [one remaining](gate-recovery1.png)
- [Reserved reward with full inventory](reserved-reward.png) and [Bulwark equipped](bulwark-equipped.png)
- [400-pixel gameplay viewport](narrow.png)

The HUD shows phase, health, signature, remaining actions and freeze. Warning
numbers use the same saved footprint as damage; G identifies harmless muster
preparations. Side controls display WEST/EAST and the default Interact key A.
The reward chest labels its Bulwark while unclaimed.

## Play testing

Use a Debug build and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout; use bow/Rogue or staff/Mage to exercise other
combat paths. Buy supplies, reach the castle through Rosemoor's east Crown Road,
enter the keep and climb to floor 2. An existing Iron Keep save with a living
Castellan also works. Previously defeated Castellans remain defeated and reserve
the new shield once after migration.

1. Approach the arena. Confirm the original Warden and Marksman guard detail,
   two side levers and two muster stones. Above half health, bait a one-action
   charge, sidestep and counter during recovery. Close range should produce a
   one-action shield swing. Flanking and recovery bypass frontal protection.
2. Draw a charge across either side portcullis, then use its lever. The charge
   must cancel and display two full recovery actions after the interaction.
   Repeat with all classes. Check guard and masonry collisions too: the boss
   gains a longer opening, and struck guards lose pending attacks and protection.
3. Try closing a gate with an enemy or player on one of its cells. That cell
   must stay open. Move the actor, finish closing and reopen it. Open a gate
   during a queued attack and confirm no previously unmarked tiles are added.
   Both levers and the onward stair must remain reachable around closed gates.
4. Cross half health during an attack. Its original warning must finish before
   Broken Formation begins. Observe G2, G1 and up to two guard arrivals. Stand
   on one muster stone to prevent that arrival. New guards must wait until the
   following turn and give no gold, items or XP when killed.
5. Observe the wider two-action sweep, recovery and a separately warned charge.
   The sweep stays fixed when moving behind him. Near arena corners and cover,
   ordinary movement must have an escape; a blocked attack misses. Freeze must
   pause an existing warning without changing its geometry or facing.
6. Save/load a warning, frozen rally and long recovery. Travel between castle
   floors and return. Warnings, gates and health must resume. Retreat outside
   the castle and return: surviving defenders regain health, gates reopen,
   and a spent rally stays spent. Defeated minibosses stay dead.
7. Defeat him with melee, arrows and each damage spell. Confirm 100 gold, the
   floor-3 unlock and the labeled armory chest in the southeast of Iron Keep.
   Interact beside it to claim Castellan's Bulwark. Equip it on Warrior with
   a one-handed weapon: +8 defense, 30% block, 50% damage reduction on blocks.
   Check both the equipment slot and the shield carried by the player.
8. Repeat with full inventory and maximum floor loot. The reward must remain
   reserved through travel, retreat and save/load. Free an inventory slot and
   claim it once. Dropping or selling the claimed shield must not regenerate it.
9. Load a version-136 save on Iron Keep and one elsewhere with Iron Keep cached.
   Existing position, HP, loot, exploration, traps, gold and victories must remain.
   A prior Castellan victory receives only the new shield reservation, without
   respawning the boss or repeating gold/XP. Resaving must preserve its claim.

Automated checks cover fixed footprints, phase transitions, freeze, sweep escapes
throughout the arena, gate counters, occupied gates, finite summons, all damage
kill paths, reward persistence and live/cached save migration. Manual testing
remains necessary for encounter balance and how clearly the warnings read while
the original guard detail is active.
