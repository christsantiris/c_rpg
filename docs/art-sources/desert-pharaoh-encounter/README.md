# Desert Pharaoh encounter

This eighth approved boss change set implements only the Sunscar Wastes stage-5
Desert Pharaoh. Pause for review and a manual commit before the Thorn Regent.

Sun-Crowned alternates fixed curse beams and burial-sand crosses. Sandbound adds
moving storm bands; four sandstone obelisks block beams and cast directional
storm shelter. Health, base attack, normal enemy roster, lamp quest and the
Sandstorm Staff's stats remain unchanged. See [Sunscar Wastes](../../sunscar-wastes.md)
for timing, damage and save details.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer, native
sandstone/cactus/dune terrain, existing player/enemy/item art and chest prop.
The new pointed obelisks are drawn with shaded sandstone faces and sun carvings.
Fixture HP is elevated for inspection; these are not completed playthroughs.
No runtime bitmap art was added.

- [Curse beam](beam.png) and [counterattack recovery](recovery.png)
- [Burial warning: two actions](burial2.png) and [one action](burial1.png)
- [Lingering sand: P2](sand2.png) and [P1](sand1.png)
- [Eastward storm](storm-east2.png), [sheltered player](shelter-east1.png) and [frozen warning](frozen.png)
- [Southward storm](storm-south2.png), [westward storm](storm-west2.png) and [northward storm](storm-north2.png)
- [Sandstone reward cache](reward.png) and [cache beneath stacked loot](reward-loot.png)
- [Generated Sunscar arena](generated.png) and [400-pixel viewport](narrow.png)

Orange numbers count down damaging signatures. Blue S cells are sheltered from
storms; arrows and the panel show the wind direction. P2/P1 counts down active
burial sand independently. Gameplay and rendering use the same affected-cell
and shelter predicates. The normal amber weather remains visual only. The HUD
names the phase, attack, freeze and recovery, and fits beneath the minimap on
narrow screens.

## Play testing

Use the existing Debug shortcut launcher and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage for other attack paths. The launcher leaves the Pharaoh alive. Travel
to Stillbury using the forest shortcut, leave by its west gate, and advance west
through Sunscar to stage 5. Buy supplies before the expedition. A save with a
living Pharaoh also works; migration preserves damage and progress.

1. Enter the final clearing and observe the one-action fixed curse line. Step
   sideways, then counterattack during recovery. Staying adjacent must not
   trigger an extra melee hit. Test a sandstone obelisk or living enemy blocking
   the beam; removing cover must not create a previously unmarked target.
2. Observe the two-action burial cross. Walk two steps outside it and watch
   the sand expire through P2/P1. Place loot on a patch: its underlying timer
   must still expire, and inventory/journal menus must not consume turns.
3. Cross half health during a warning. That warning must finish unchanged;
   the next declaration announces Sandbound. Freeze a warning and verify its
   exact target and countdown remain fixed while frozen.
4. Observe successive three-tile storm strips moving edge/center/edge, then
   rotating wind. Reach S shelter behind an obelisk or walk out of the strip.
   Check that shelter moves with the wind and only protects against storms.
   The footprint must stay fixed during both countdown actions.
5. Save/load during a warning, recovery or active sand patch. Backtrack and
   portal out: health, phase, storm sequence, direction, timers and obelisks
   persist. A threatening portal landing warning is warded away, and sand on
   the landing cell is cleared.
6. Defeat the Pharaoh with melee, arrows or spells. Confirm the west exit opens
   and one unchanged Sandstorm Staff remains. Leave it unclaimed, revisit and
   save/load: the reward must remain available without reviving the Pharaoh.
7. Test full floor loot and inventory. Existing objects must survive, and the
   sandstone cache must stay available. Interact beside it or press P on it,
   including when other objects cover it. Collecting repairs their underlays.
8. Sell or deliberately drop a claimed staff and revisit: no replacement is
   awarded. Confirm its Mage restriction and existing two-handed staff bonuses.
9. If The Lost Magic Lamp is active, verify stage-4 lamp and journal progress
   remain intact throughout this encounter and its migration.

## Code, saves and validation

[`desert_pharaoh.c`](../../../src/game/desert_pharaoh.c) owns signatures,
walking-escape checks, finite sand, obelisk shelter, portal wards, reward and
migration. [`desert_pharaoh_renderer.c`](../../../src/renderer/desert_pharaoh_renderer.c)
draws the native props, countdowns, shadow cells and encounter panel.

Save version **131** adds the earned/unclaimed/claimed reward tracker while
retaining the known old staff flag. Existing enemy serialization and appended
map IDs preserve active/cached combat and terrain states. Migration protects
players, loot, quest props and portal destinations without healing enemies or
restarting previous victories. Old Pharaoh attack timing resets to the new
warning sequence.

Release and Debug builds passed. The full regression suite passed
**4,740/4,740** checks. All **45/45** focused checks and final native renderer
captures passed AddressSanitizer and UndefinedBehaviorSanitizer. Coverage
includes every arena floor across 100 generated layouts, four obelisks and open
exits, ordinary walking escapes, fixed/frozen/threshold warnings, all four wind
directions, actual in-band shelter, evasion, finite sand beneath stacked loot,
all five victory paths, full floor/inventory, claim/cache persistence, portals,
and active/cached version-130 migration. Existing Sunscar tests also cover
rosters, travel, staff metadata and the lamp quest.

The sanitizer build reports the existing Dain dialogue truncation warning in
`game_talk_to_dain`; that unrelated dialogue was not changed here.
