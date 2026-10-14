# Grave Marshal encounter

This seventh approved boss change set implements only the Royal Catacombs
floor-5 Grave Marshal. Pause for review and a manual commit before the Desert
Pharaoh.

Honored Commander retains approach movement and fixed polearm fronts. Funeral
March adds cross sweeps, a funeral channel and one warned honor-guard wave.
Extinguishing a native tomb brazier during the channel interrupts it and grants
two counterattack actions. Existing protection, Gravekeeper's Mantle, resurrection
rules, return passage and Brother Oswin's memorial/ledger quest remain in effect.
See [Royal Catacombs](../../catacombs.md) for timing and save details.

## Renderer review

These controlled fixtures use the actual `game_draw()` renderer, native catacomb
floor shading, established tomb/brazier/chest props and existing boss/guard art.
Fixture HP is elevated for inspection; they are not completed playthroughs.
No runtime bitmap art was added.

- [Polearm front](polearm.png) and [recovery](recovery.png)
- [Honor call: two actions](guards2.png), [one action](guards1.png) and [guards arriving](guards-born.png)
- [Frozen warning](frozen.png)
- [Cross sweep: two actions](cross2.png) and [one action](cross1.png)
- [Funeral channel](channel2.png) and [two-action interrupt opening](channel-interrupted.png)
- [Both native braziers cold](both-cold.png)
- [Royal burial ledger preserved](ledger.png)
- [Mantle reliquary](reward.png) and [reliquary beneath loot](reward-loot.png)
- [Generated tomb](generated.png)
- [Narrow viewport](narrow.png)

Cyan numbers count down damaging signatures; H2/H1 count down guard arrivals.
The same predicates determine marked and damaged cells. The panel names the
phase, attack, freeze and remaining recovery actions. Native brazier art retains
its blue flames and cold state. New niche/reward props use the existing catacomb
floor underlay; the ordinary orange spike warnings remain separate.

## Play testing

Use the existing Debug shortcut launcher and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage for other attack paths. The launcher leaves the Grave Marshal alive.
Travel to Rosemoor using the safe forest/swamp roads, take Crown Road East to the
castle grounds and enter the south catacomb gate. Descend to floor 5. Buy supplies
before the expedition. An old save with a living Marshal can also be loaded;
damaged health and cold braziers migrate in place.

1. Approach and observe the one-action polearm warning. Step back or sideways,
   then counterattack during recovery without an extra Marshal retaliation.
2. Cross half health during a warning. It must finish unchanged. Observe the
   honor call's two fixed H2/H1 niches and two guards appearing after the warning.
   They must not act immediately or provide rewards/resurrection when killed.
3. Observe a two-action cross, escape with ordinary moves, then observe the
   separate funeral channel. Its fixed target must not follow movement.
4. Extinguish a tomb brazier during a channel. Check the channel vanishes,
   protection drops from 50% to 25%, and two complete counterattacks are available.
   Repeat with the second flame: protection reaches 0% and both stay cold.
5. Use a brazier during a polearm/cross sweep or honor call. It still reduces
   protection but must not erase that other warning. Try each class; no item or
   spell is required. Active guards remain a threat during the boss's recovery.
6. Freeze and save/load during a sweep, honor call or recovery. Backtrack and
   portal out: health, wave use, guard no-reward flags, niche and flame states
   persist. A threatening landing signature is warded away; an honor call stays
   queued and can never create a second wave.
7. Defeat the Marshal with melee, arrows or spells. The castle return passage
   opens even with surviving guards. Leave the mantle unclaimed, revisit and
   confirm one reward remains. Full inventory leaves a reserved cache intact.
8. Collect the mantle, then sell or deliberately drop it: no replacement should
   appear. Check the all-class +6 defense/+20 maximum HP bonuses are unchanged.
9. If Rest for the Forgotten is active, verify memorial flags and the floor-5
   ledger remain intact. Recover the ledger after victory and return to Brother
   Oswin in Ridgeshire's Town Hall for the original reward.

## Code, saves and validation

[`grave_marshal.c`](../../../src/game/grave_marshal.c) owns fixed signatures,
finite niche targeting, brazier interrupts, portal wards, reward and migration.
Actions retain the existing approach pathfinding, constrained to the royal tomb.
The guard helper appends slots without consuming old corpse records. Existing
catacomb interaction/damage routines retain protection and quest behavior.
[`grave_marshal_renderer.c`](../../../src/renderer/grave_marshal_renderer.c)
draws native props, signature/niche countdowns and the encounter panel.

Save version **130** adds the earned/unclaimed/claimed tracker while retaining
the known old mantle flag. Existing enemy serialization and appended map IDs
preserve active/cached combat state, cold braziers, niches, guards and reward
underlays. Migration protects portal destinations and never heals enemies or
restarts previous victories.

Release and Debug builds passed. The full regression suite passed
**4,695/4,695** checks. All **51/51** focused checks and final renderer captures
passed AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes walking
escapes in 100 tombs, fixed/threshold/frozen warnings, one finite wave, actor/corpse
capacity, no-reward guards, all-class interrupts, both recovery actions, armor
protection, full loot/inventory, all five victory paths, cache/claim persistence,
portals, old saves and surviving-guard castle returns. Existing catacomb tests
cover memorial, ledger, ordinary revival and trap behavior.

The sanitizer build also reports an existing Dain dialogue truncation warning
in `game_talk_to_dain`; that unrelated dialogue was not changed here.
