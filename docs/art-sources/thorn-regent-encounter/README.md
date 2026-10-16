# Thorn Regent encounter

This ninth approved boss change set implements only the Moonveil Gardens stage-5
Thorn Regent. Pause for review and a manual commit before the Cinder Lord.

Garden Regent alternates fixed thorn lances and creeping-root crosses. Briar
Crown adds warned temporary thorn strips beside two flowering nodes. Tending a
node opens its flank or cancels that node's regrowth for two full counterattack
turns. Original stats, normal roster, boss journal, exits and Liora's Moonseed
quest remain in effect. See [Moonveil Gardens](../../moonveil-gardens.md) for
combat, reward and save details.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer, established
Moonveil floor, stone circles, pools, glowing flowers and mushroom/hedge terrain.
Arena nodes and reward caches use existing detailed flower/chest props; temporary
briars combine native planting art with shaded stems and pointed thorns. New
cloak art uses the established cloak icon and equipped-player palettes, with a
teal weave and lavender icon trim. Fixture health is elevated for inspection;
these are not completed playthroughs. No runtime bitmap art was added.

- [Thorn lance](lance.png) and [counterattack recovery](recovery.png)
- [Root warning: two actions](roots2.png) and [one action](roots1.png)
- [Lingering roots: R2](root-patches2.png) and [R1](root-patches1.png)
- [Regrowth warning: G2](regrowth2.png), [G1](regrowth1.png) and [frozen warning](frozen.png)
- [Grown W4 briars](walls4.png), [cleared flank](cleared.png), [resting node](node-cooldown.png) and [regrowth interrupt](interrupt.png)
- [Reward beneath loot](reward-loot.png), [partially claimed reward](reward-partial.png) and [flowered reliquary](reward.png)
- Cloak equipped by [Warrior](cloak-warrior.png), [Rogue](cloak-rogue.png) and [Mage](cloak-mage.png), including equipment-panel icons
- [Generated Moonveil arena](generated.png) and [400-pixel viewport](narrow.png)

Purple numbers warn damaging signatures; green G numbers warn upcoming growth.
R, W and N numbers show active roots, remaining wall duration and node cooldown.
The warning predicate also determines attack damage and the locked growth mask.
The panel names the phase, signature, freeze and recovery opening, fitting below
the minimap on narrow screens. Temporary walls are impassable in both movement
and minimap rendering; roots and flowers remain walkable.

## Play testing

Use the existing Debug shortcut launcher and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage for other attack paths. The launcher leaves the Regent alive. Travel
to Rosemoor via the forest and swamp shortcuts, leave by its west gate, then
advance west through Moonveil to stage 5. Buy supplies before the expedition.
A save with a living Regent also works; damaged health and exploration migrate.

1. Enter the final clearing and observe the one-action fixed thorn line. Step
   sideways, then counterattack during recovery. Being adjacent must not
   trigger an extra melee hit. Hedges, pools and living actors block the lance;
   removing cover after declaration must not expose an unmarked target.
2. Observe the two-action root cross. Walk two steps outside it, then watch
   R2/R1 patches expire. Drop loot on roots and confirm their underlays expire
   without losing items. Native stone-circle tiles must remain intact.
3. Cross half health during a warning. That signature must finish unchanged;
   Briar Crown is announced before its next preparation. Freeze a warning and
   verify its exact countdown, target and growth mask stay fixed.
4. Observe G2/G1 strips alternating beside the flowering nodes. W4 briars
   change the flanking route, count down and disappear. Leave a player, enemy
   or item on a warned cell: regrowth must skip it without damaging, moving or
   discarding it. Other routes to the boss and exits must remain open.
5. Tend the node during its own regrowth warning with A. Growth vanishes and
   two full counterattacks are available. Tend after its strip grows: the
   adjacent walls recede. Check N4/N3/N2/N1 prevents continuous clearing. A
   different node must not cancel an unrelated lance or growth warning.
6. Repeat the node counter as each class, without spells or ammunition. No
   quest acceptance or consumable is needed. Ignore both nodes in another
   attempt: the boss must still be damageable and walls must expire.
7. Save/load during warnings, freeze, recovery, active roots/walls or flower
   cooldowns. Backtrack and portal out: exact terrain/combat state persists.
   Return portals ward threatening landing warnings and clear roots on the
   landing tile without changing boss health or other arena clocks.
8. Defeat the Regent with melee, bow or spells. Confirm the west exit opens,
   including with surviving ordinary enemies. Both guaranteed rewards remain
   available through travel and saving, even with a full floor or inventory.
9. Free one inventory slot and collect from the reliquary: the cloak arrives,
   while the Strength Potion remains reserved. Save/load, then free another
   slot and collect it. Both A and P must reach a cache covered by other loot.
   Confirm +4 defense and 10% evasion on every class; the original potion still
   grants one permanent base attack. Consuming, selling or dropping claimed
   rewards must not produce replacements.
10. Verify Liora's stage-2 seed pod, stage-3 spring, stage-4 planting circle and
    restored Moonflower remain independent of the arena nodes. Old defeated
    Regents must stay defeated and retain their original potion drops.

## Code, saves and validation

[`thorn_regent.c`](../../../src/game/thorn_regent.c) owns signatures, walking
escapes, route checks, finite terrain, node interactions, reward and migration.
[`thorn_regent_renderer.c`](../../../src/renderer/thorn_regent_renderer.c)
draws the native arena props, countdowns and encounter panel.
[`item.c`](../../../src/game/item.c) defines the approved all-class cloak.

Save version **132** adds one root-level pending-reward mask, initialized in
`game_init()` and serialized/deserialized in `save_load.c`. Each reward clears
its own bit; zero prevents repeat grants. Existing enemy serialization and
appended tile IDs preserve active/cached targets, growth masks, node selection,
phase, freeze, recovery, root/wall clocks and cooldowns. Migration protects loot,
players, quest terrain and portal destinations without healing enemies or
restarting previous victories; obsolete boss timing resets to the new sequence.

Release and Debug builds passed. The full regression suite passed
**4,787/4,787** checks. All **47/47** focused checks and native renderer captures
passed AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes walking
escapes on every arena floor across 100 native layouts, two independent nodes,
connected routes after regrowth, fixed/frozen/threshold warnings, occupancy and
loot protection, connectivity rechecks, all-class interrupts, both counterattack
actions, cooldowns, finite roots under stacked loot, all five victory paths,
full floor/inventory, partial reward claims, portals, active/cached saves and
version-131 migration preserving Moonseed progress. Existing Moonveil tests
also cover ordinary rosters, quest defenders, restored gardens and travel.

The sanitizer build reports the existing Dain dialogue truncation warning in
`game_talk_to_dain`; that unrelated dialogue was not changed here.
