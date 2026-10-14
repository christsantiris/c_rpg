# Minotaur encounter

This sixth approved boss change set begins batch 2 and implements only the
Labyrinth floor-5 Minotaur. Pause for review and a manual commit before the
Grave Marshal.

Vault Keeper warns fixed charges or short axe fronts. Bloodied Bull independently
warns two charges, then a wide axe arc. Stone impacts grant two actions of stagger;
two designated pillar caps can crack and break after explicit warnings. Future
victories award Labyrinth Aegis, matching Magic Shield stats/class rules while
preserving old shields. The five-rune gate and Rook's ivory-rook quest remain
intact. See [Rook's Labyrinth](../../labyrinth.md) for timing and save details.

## Renderer review

These controlled fixtures use the actual `game_draw()` renderer and the native
labyrinth map, established pillar/rubble/chest props, enemy/player sprites and
warning overlays. Fixture HP is elevated and Aegis is equipped early for art
inspection; these are not completed playthroughs. No runtime bitmap art was added.

- [Fixed charge](charge1.png), [frozen warning](frozen.png) and [resolved charge](charge-resolved.png)
- [Short axe front](axe.png)
- [Pillar crack warning](pillar-crack-warning.png) and [two-action stagger](pillar-cracked.png)
- [Pillar break warning](pillar-break-warning.png) and [opened flank](pillar-broken.png)
- [Separately warned second charge](second-charge.png)
- [Wide axe: two actions](wide2.png), [one action](wide1.png) and [recovery](recovery.png)
- [Named reward cache](reward.png) and [cache beneath loot](reward-under-loot.png)
- [Narrow encounter panel](narrow.png)
- [Magic Shield/Aegis icon and player comparison](shield-art.png)

Numeric outlines use the same predicate as damage. C1/B1/H1 distinguish pillar
crack, phase-two break and other stone impacts. Ordinary walls never change.
The encounter panel is drawn after the labyrinth's map scale is restored, with
remaining recovery actions and a compact frozen indicator in narrow windows.
The labyrinth retains its existing whole-map scaling behavior.

The new shield uses the established 24px icon silhouettes/palette and a native
bull motif in the player's hand. Existing Magic Shield artwork is unchanged.
Equipped and inventory views share the same Aegis visual ID.

## Play testing

For quick access to Stillbury, use the existing Debug shortcut launcher and
select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. The launcher marks forest, swamp and mountain
shortcuts complete; **it leaves the Minotaur alive**. Take the safe forest road
to Stillbury. In its Inn, speak to Rook and accept the ivory-rook quest. Enter
the gate across from the witch's hut. Light all five runes and reach floor 5.
Use `--weapon bow` with Rogue or `--weapon staff` with Mage for other attack paths.
For the ordinary campaign, run the Debug executable directly without
`--unlock-shortcuts`; the Minotaur's base stats remain the same.

1. Observe a charge warning before movement. Sidestep its lane, or step back
   from a shortened corridor charge. Counterattack during recovery.
2. Approach within one tile and observe the short axe front. Step back, then
   counterattack; no extra adjacent retaliation should occur during recovery.
3. Cross half health during a warning. It must finish unchanged. Observe two
   separately warned charges followed by a two-action wide axe arc.
4. Bait a charge at a pillar or wall, then move off the marked lane. Check the
   two-action stagger. A reinforced cap must show C1 before cracking; an already
   cracked cap can show B1 and break only on a warned Bloodied Bull impact.
5. Ignore the pillars for another attempt. All signatures still provide ordinary
   walking escapes and recovery; no consumable or specific class is required.
6. Freeze a charge or wide axe, then save/load during its warning or stagger.
   Targets, orientation, phase and sequence must persist. Backtrack and retreat
   to Stillbury, then re-enter; the living boss must retain its health and caps.
7. Defeat the Minotaur with melee, arrows or spells. Recover the ivory rook,
   return to Rook in Stillbury's Inn, and verify the existing quest reward.
8. Equip Labyrinth Aegis as Warrior and inspect the off-hand icon and player art.
   Its stats match Magic Shield; Mage and Rogue cannot equip it.
9. Leave Aegis unclaimed and revisit after a floor refresh. Use the bull-marked
   cache. Full inventory leaves it available; selling/dropping a claimed shield
   must not regenerate one. Older unclaimed Magic Shields keep their old name.

## Code, saves and validation

[`minotaur.c`](../../../src/game/minotaur.c) owns fixed footprints, charge
movement, collision/stagger, cap state, reward recovery and old-save migration.
Actions dispatch it before generic melee. The labyrinth loader restores unclaimed
rewards; new expeditions retain a living boss's cached fifth floor.
[`minotaur_renderer.c`](../../../src/renderer/minotaur_renderer.c) draws native
props, collision labels, countdowns and the encounter panel.

Save version **129** persists reward identity/claim state; existing enemy
serialization and appended map/item IDs preserve active and cached combat/art
state. Prior victories remain completed. Only identifiable currently dropped
legacy Magic Shields establish an unclaimed old-variant reward.

Release and Debug builds passed. The full regression suite passed
**4,644/4,644** checks. All **55/55** focused checks and renderer captures passed
AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes walking escapes
across over 1,000 vault/approach preparations, charge endpoints, phase/freeze,
actor cover, cap warnings and limited breakage, both recovery actions, all five
victory paths, rune/relic routes, retreat/backtracking, save migration, shield
stats/class/art identity, saturated floor loot, inventory and legacy rewards.
