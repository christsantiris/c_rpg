# Lord Veyr encounter

This final approved boss change set updates only **Lord Veyr**, on castle floor
6, **Throne of No Return**, and connects his guaranteed victory trophy to the
journal, ending and Hall of Fame. All sixteen approved boss updates are now
implemented. Pause for review and a manual commit before further balance work.
See [castle encounter details](../../castle-design.md) for geometry and state.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer, native
Veyr phase sprites, existing castle props, ward crystals and rune floor. The fire
impact reuses the established low-flame prop. Ending, journal and Hall of Fame
captures call their real SDL renderers; the latter includes ten ranked runs to
check space for trophy labels. Fixture health is raised for inspection, and
some fixtures disable ordinary guards to isolate warning geometry. These are
not completed playthroughs. No runtime bitmap assets or building art were added.

- [Generated throne room and original roster](generated.png)
- [Throne Advance with announced escorts: two actions](advance-and-escort2.png), [one action](advance1.png), [arrival and recovery](advance-recovery.png), and [Oathblade Swing](swing1.png)
- [Royal Edict](edict2.png), [West Ward Lane: W2](west2.png), [W1](west1.png), [East Ward Lane: E2](east2.png), [E1](east1.png), and [disabled source lane](ward-lane-disabled.png)
- [Unbound Crown: three actions](crown3.png), [two](crown2.png), [one](crown1.png), [normal recovery](crown-recovery1.png), and [two-action recovery with both wards spent](crown-recovery2.png)
- [Separately warned throne fire: F2](fire2.png), [F1](fire1.png), [frozen preparation](fire-frozen.png), and [single fire impact](fire-impact.png)
- [Retaliation against corridor fire](corridor-warning.png) and [400-pixel gameplay viewport](narrow.png)
- [Victory trophy](victory-trophy.png), [boss-journal trophy](journal-trophy.png), and [Hall of Fame trophy](hall-of-fame-trophy.png)

The HUD preserves phase, health, attack name, countdown, freeze and recovery on
the narrow viewport. G marks finite escort preparations; W/E match the labeled
pedestals; C/O distinguish gate changes; F marks fire; X combines damage and a
gate change on one tile. Dark text backings keep countdowns readable over props.
Native phase frames and shield-facing markers follow the latched phase, so
crossing a health threshold cannot change a pending attack's appearance.

## Play testing

Start a Debug game with supplies and choose Warrior for this loadout:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,fireball,heal
```

Use staff/Mage or bow/Rogue to test other combat paths. Reach the castle through
Rosemoor's east Crown Road, defeat the Castellan on floor 2 and Royal Arcanist on
floor 4, and climb to floor 6. An existing throne save with Veyr alive also works.
An older completed campaign receives the new trophy without replaying victory.

1. Approach Armored Ruler. The first signature announces the finite two-guard
   call and gives at least two actions. G-marked stones receive up to one Warden
   and one Marksman on resolution. Occupied stones skip arrivals; new guards
   wait until the following turn and award no gold, items, XP or score.
2. Bait Throne Advance, sidestep its fixed three-stone lane and flank during
   recovery. Nearby, check the single-cell Oathblade Swing. Veyr must never
   overlap an actor or advance outside the arena. Freeze must preserve target,
   direction and warning. Frontal protection dims during recovery and disappears
   only after the next health phase starts.
3. Cross two-thirds health during a warning. Finish the original attack before
   Broken Throne begins. Observe Royal Edict, then separately warned west and
   east lanes. Each allows two ordinary walking actions. Gate C/O previews must
   remain fixed, leave escape routes and avoid actors or loot on closure.
4. Use both pedestal counters on every class. They retain their original 20%
   protection and +4 attack per active source. Disabling a lane's source removes
   its footprint and barrier plan; its timer and any escort preparation finish
   safely. Both spent wards leave a vulnerable boss that still attacks.
5. Cross one-third health during preparation. Finish its old shape before
   Unbound Crown. Leave the radius-two diamond in three ordinary steps. Watch
   the subsequent fire lane receive its own two-action warning after recovery.
   Crown and fire must never hit together. One step off the rune row avoids
   the eruption, and it must not keep damaging after its single impact.
6. Disable both pedestals before a crown completes. The HUD must show two full
   recovery actions. Check that warnings and recovery resume after save/load,
   and frozen fire preparation pauses alongside the boss. Existing table,
   pillar and gate geometry must leave ordinary walking escapes.
7. Fire all bows from a clear west approach at maximum range. Once damaged,
   Veyr must answer corridor fire with a marked Royal Edict or crown while
   staying in his arena. The throne blocks shots along the northern centerline;
   ordinary cover remains meaningful. Distant or hidden retreat cancels warnings.
8. Save/load each signature, pending escorts, phase transitions, ward state,
   fire warning and long recovery. Castle floor revisits preserve the attempt.
   Retreat restores a living Veyr, wards and escort allowance; the separate
   Herald's spent patrol call follows its existing castle rules.
9. Defeat him with melee, arrows and every damage spell, including with maximum
   inventory and floor loot. The trophy must be awarded before the ending, the
   10,000 bonus must happen once, and combat/fire must stop immediately. There
   is no crown pickup, gear slot, consumable or postgame movement requirement.
10. Check Crown of the Unbound on the victory screen. Open the boss journal
    with its bound key (default Q); close it to return to the ending. Enter the
    Hall of Fame and inspect the trophy under the winning row. Permanent trophy
    history must remain even if that run does not enter the top ten. Restart
    and load the record; legacy score rows must retain their original data.
11. Load a version-138 live throne save and a cached throne from another floor.
    Existing health, actors, loot, exploration and progress must remain. A queued
    old attack retains its shape and timing; overlapping old fire waits for a
    fresh separate warning. Load a completed old campaign and verify the trophy
    appears without respawning Veyr or adding another victory bonus.

Automated checks cover signature timing, threshold crossings, freeze, ordinary
walking escapes, all bow ranges, bounded movement, finite escorts, occupied
gates/loot, all kill paths, full inventory, active/cached save migration, trophy
claims, legacy high scores and unranked victory history. Manual testing remains
necessary for balance and readability with the full original guard roster active.
