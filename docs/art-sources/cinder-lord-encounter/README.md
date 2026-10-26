# Cinder Lord encounter

This tenth approved boss change set implements only the Ashen Hollow stage-5
Cinder Lord. Pause for review and a manual commit before the Prism Sovereign.

Furnace Heart alternates fixed cinder lances and lava-eruption crosses. Broken
Crucible alternates eruptions with a two-front expanding ember ring. Opening a
forge sluice feeding a warned eruption redirects it into cooling channels and
exposes the Lord for two full counterattacks. Existing health, base attack,
defense, XP, score, roster, journal and return exit remain in effect. The arena
controls are independent of the Emberforge quest's mechanism and repaired forge.
See [Ashen Hollow](../../ashen-hollow.md) for timing, damage, reward and save details.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer and existing
Ashen floor, basalt, native lava pools, ruined paving and boss/player sprites.
Sluices and channels use established lever/conduit props; hot tiles combine native
flame art with ember fissures. Embercleaver uses the established axe icon and
held-weapon drawing with a charcoal head, dark handle and amber finish. Fixture
health is elevated for inspection; these are not completed playthroughs. No
runtime bitmap art was added.

- [Cinder lance](lance.png) and [counterattack recovery](recovery.png)
- [Eruption: two actions](eruption2.png) and [one action](eruption1.png)
- [Lingering heat: H2](heat2.png) and [H1](heat1.png)
- [Both ring fronts announced](ring12.png), [outer front remaining](ring-outer1.png), [frozen outer front](frozen-outer.png) and [ring resolved](ring-resolved.png)
- [Eruption feeding a channel](eruption-channel2.png), [correct redirection](redirected.png) and [cooldown](cooldown.png)
- [Embercleaver equipped](embercleaver.png), including the equipment-panel icon and blocked off-hand slot
- [Rewards beneath loot](reward-loot.png), [partial reward claim](reward-partial.png) and [forge reliquary](reward.png)
- [Generated Ashen arena](generated.png) and [400-pixel viewport](narrow.png)

Orange numbers show when each affected tile will hit. The initial ring has
inner 1 and outer 2 labels; after the inner burst, only the originally marked
outer front remains. H labels show floor-heat duration, while blue C and S labels
show channel cooling and sluice cooldown. The same predicate drives warning
countdowns and damage. The narrow HUD uses I/O for inner/outer and keeps freeze
and recovery visible. Sluice labels sit beside their levers to avoid overlapping
channels or ring countdowns.

## Play testing

Use the existing Debug shortcut launcher and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage to exercise the other combat paths. The launcher leaves the Cinder
Lord alive. Travel to Rosemoor, leave through its west gate, and follow
Ashen Hollow to stage 5. Buy supplies before
the expedition. Existing saves with a living Lord migrate in place.

1. Enter the final clearing. Sidestep the one-action cinder lance, then
   counterattack during recovery. Staying adjacent must not trigger an extra
   melee hit. Basalt, native lava and living actors block lances; removing cover
   after declaration cannot create an unmarked target.
2. Observe the two-action eruption cross. Walk two steps outside it and watch
   H2/H1 heat expire. Drop loot on heat and confirm all underlying timers expire
   without losing objects. Native ruined paving and quest props must remain.
3. Cross half health during a warning. It must finish unchanged. Observe
   Broken Crucible's ring: both inner 1 and outer 2 footprints appear together.
   Walk outward once to escape the inner burst, then again to escape the outer
   burst. After the second burst, one whole counterattack action is available.
4. Freeze before or between ring fronts. The exact remaining front and target
   must persist. Existing heat still expires. Thawing must not repeat the inner
   burst or invent a new outer footprint.
5. Stand on a forge sluice before an eruption warning. Its cross should touch
   the adjacent channel. Open the sluice with A: that eruption is redirected,
   C4/S4 cooling appears and two full counterattack actions become available.
   Repeat on the other sluice as Warrior, Mage and Rogue without consumables.
6. Open the wrong sluice or use it during a lance/ring. It cools its channel but
   cannot cancel the unrelated warning or grant extra recovery. A cooling
   channel prevents eruption hits; it does not protect against lances or rings.
   Wait through C/S cooldowns and check that the controls become reusable.
7. Save/load during a warning, half-resolved ring, freeze, hot floor, cooling or
   recovery. Backtrack and portal out: clocks, phase, targets and health persist.
   Portal arrival wards threatening signatures and clears landing heat without
   advancing other arena clocks.
8. Defeat the Lord with melee, arrows or spells. The final north exit opens,
   including with surviving ordinary enemies. Embercleaver and the original
   Strength Potion stay available through travel and saves, even with a full
   loot floor or inventory. Use A beside the forge reliquary or P on it.
9. Free one inventory slot: collect the axe while the potion stays reserved.
   Save/load, free another slot and collect the potion. Both commands must work
   when loot covers the cache. Consuming, selling or dropping claimed rewards
   must not create replacements. Verify the original potion grants +1 base attack.
10. Check Embercleaver's Warrior restriction, two-handed off-hand lock, +11 attack,
    25% penetration and 25% cleave. Its attack/penetration remain below the Magic
    Battle Axe and its cleave below greatswords. Test it directly with:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon embercleaver
```

11. Verify Emberforge stage-2 mechanism and stage-4 repaired forge progress remain
    intact. Old defeated Lords must stay defeated and retain their original
    potion drops without newly manufactured rewards.

## Code, saves and validation

[`cinder_lord.c`](../../../src/game/cinder_lord.c) owns fixed signatures, ring
fronts, walking escapes, finite heat, forge controls, rewards and migration.
[`cinder_lord_renderer.c`](../../../src/renderer/cinder_lord_renderer.c) draws
native props, per-cell countdowns and the encounter panel. The new axe is defined
in [`item.c`](../../../src/game/item.c) and available in the existing Debug
weapon parser in [`main.c`](../../../src/main.c).

Save version **133** adds one pending-reward mask, initialized in `game_init()`
and serialized/deserialized in `save_load.c`. Independent bits reserve the axe
and potion; each clears on claim. Existing enemy serialization and appended map
IDs preserve active/cached signatures, half-resolved rings, health, phase,
freeze, recovery, heat, cooling channels and sluices. Migration resets obsolete
boss timing while protecting exploration, damage, deaths, loot, Emberforge
progress and portal destinations. Previous victories never restart or grant
repeat rewards.

Release and Debug builds passed. The full regression suite passed
**4,843/4,843** checks. All **56/56** focused checks and final native renderer
captures passed AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes
100 generated arenas and per-front walking escapes, two separate sluices,
fixed/frozen/threshold warnings, two complete counterattack actions, all-class
redirection, wrong/off-timing controls, finite heat beneath stacked loot, cooling
underlays, all five victory paths, full floor/inventory, partial claims, actual
weapon penetration and cleave, equipped metadata, portals and version-132
active/cached migration. Existing Ashen tests also cover rosters, travel, quest
defenders, mechanism and repaired-forge persistence.

The sanitizer build reports the existing Dain dialogue truncation warning in
`game_talk_to_dain`; that unrelated dialogue was not changed here.
