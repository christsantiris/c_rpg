# Red Dragon encounter

This thirteenth approved boss change set implements only Dragonspine's stage-5
Red Dragon. Pause for review and a manual commit before the Castellan.

Fire breathing is the primary attack: a fixed, two-action cone with an animated
stream of flame from the dragon's mouth. Hoard Defender adds close claw/tail
sweeps, Scorched Wings adds warned landings and finite burning ground, and Last
Inferno links two separately warned breaths before a long recovery. Stable and
breakable basalt columns provide cover without blocking existing routes.
The original Dragon Scale Mantle, gold, roster, goblet quest and summit exit
remain in effect. See [Dragonspine](../../dragonspine.md#red-dragon-encounter)
for exact geometry, timing, recovery, rewards and migration.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer and existing
Dragonspine snow, ash, hoard, cliff and character art. Columns, rubble, embers
and the reliquary reuse established pillar, rubble, flame and chest props.
Flame streams use native pixel drawing and follow the warned fire paths. The
mouth origin stays aligned with the regional camera, including at the summit's
east edge. The existing mantle icon and equipped-player art are unchanged.
Fixture health is elevated for inspection; these are not completed playthroughs.
No runtime bitmap art was added.

- [Fire breath: two actions](breath2.png) and [one action](breath1.png)
- [Animated fire-breath stream](fire-breath-stream.png) and [impact](fire-breath-impact.png)
- [Diagonal breath warning](diagonal-breath2.png) and [diagonal flame stream](diagonal-fire-breath.png)
- [Claw/tail sweep](claw-tail1.png) and [normal recovery](recovery.png)
- [Column threatened](column-warning.png), [cracked cover](column-cracked.png), [break warning](column-break2.png), [rubble](column-rubble.png) and [later exposed cone](new-exposure2.png)
- [Wingbeat: two actions](wingbeat2.png) and [one action](wingbeat1.png)
- [Fire H3](heat3.png), [H2 while frozen](heat2-frozen.png), [H1](heat1.png) and [restored terrain](heat-expired.png)
- [Inferno first cone](inferno-first2.png), [separate second cone](inferno-second2.png), [frozen second cone](inferno-second-frozen.png) and [long recovery](long-recovery.png)
- [Original mantle equipped](mantle.png), [ordinary reward drop](reward-drop.png), [full-floor reliquary](full-floor-reliquary.png), [covered cache](covered-reward.png) and [claimed reward](claimed-reward.png)
- [Generated summit](generated.png), [fire breathing with the native summit camera](generated-fire-breath.png) and [400-pixel viewport](narrow.png)

Numbers show remaining ordinary actions. Wingbeat uses a lighter warning;
H3/H2/H1 marks active floor fire, and C2/C1 marks a column receiving a breath.
S labels stable cover, W labels a weak column, and ! identifies cracked cover
that will break on a later breath. Color, geometry, symbols and countdowns work
together. The narrow panel retains phase, attack, remaining actions, freeze
and recovery. The same saved footprint drives warning cells, hit detection,
column impacts and breath-animation endpoints.

## Play testing

Use the existing Debug shortcut launcher and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage to exercise the other combat paths. Town shortcuts open; the Red
Dragon remains alive. Reach Ridgeshire by the mountain shortcut, leave through
its east gate, and follow Dragonspine to stage 5. Buy supplies first. Accept
Ilya's goblet quest in **Oakhaven's Tavern** to check its separate progression,
or use an existing summit save with the dragon alive.

1. Enter the lair. Observe the two-action cone and its fixed direction. Step
   sideways before fire resolves. Watch an actual stream of flame leave the
   dragon's mouth. Ranged attacks and standing adjacent must not cause an
   immediate extra hit. Check diagonal bow lines and longbow distance too.
   Test the animation near both map edges as well as in
   the center, so its mouth origin and impact remain aligned with the terrain.
2. Use the full recovery action to counterattack. Nearby, the next attack can
   be a one-action claw/tail sweep. Walk one step beyond its neighboring lanes.
   No extra melee attack should occur during either warning or recovery.
3. Shelter behind basalt while drawing a breath toward it. The warning ends
   at the cover. Stable S columns remain. A W column becomes visibly cracked
   and labeled ! after its first warned impact; a later breath can break it.
   The breaking cone must never spread into previously sheltered cells.
4. Draw a later breath through the rubble. Only that new declaration should
   warn the exposed route. All columns remain outside the original goblet
   location and preserve walking access around them and to the exit.
5. Cross two-thirds health during a warning. Finish the existing attack before
   Scorched Wings begins. Observe a two-action landing cross. Move twice out
   of it, watch the dragon land, and confirm it stays vulnerable throughout.
   Standing still takes the warned hit but cannot overlap player and dragon.
6. Check H3 → H2 → H1 fire on eligible landing ground. Damage begins on the
   following action and stops after three actions. Freeze the dragon: existing
   ground fire continues to expire, while an unresolved landing waits. Drop
   stacked loot on the fire and verify every underlay restores its exact native
   terrain. Goblet, cover, cache and travel tiles must never become fire.
7. Cross one-third health during a warning. Last Inferno begins with a fixed
   breath. After it resolves, a quarter-turn cone receives a new two-action
   warning. Freeze between breaths and save/load; the second cone must resume
   without replaying the first. After both resolve, use two full counterattack
   actions. Healing the dragon cannot reverse the phase.
8. Save/load warnings, facing, phase, wingbeat position, column cracks, fire
   timers and recovery. Retreat and return through a town portal. The exact
   landing is warded against a queued signature and cleared of fire, with the
   boss's health and surrounding fire preserved.
9. Defeat the dragon with melee, arrows or each damage spell. The original
   mantle and gold drop remain, and the summit exit opens even with ordinary
   enemies alive. Equip the mantle on any class: +4 defense and +20 maximum HP.
   Boss health, base damage, XP, ordinary roster and mantle value remain.
10. Repeat with maximum floor loot and full inventory. Existing drops must not
    be deleted; gold is credited if no drop slot is available, and a reliquary
    reserves the mantle. Leave ordinary unclaimed equipment behind, portal out,
    save/load, and return: it remains recoverable. Free a slot and use A nearby
    or P on the cache, including beneath loot. Sold or dropped claimed mantles
    must not regenerate.
11. Recover Ilya's goblet and return to **Ilya in Oakhaven's Tavern** for the
    original Strength Potion and 600 score. Receiving or claiming the mantle
    cannot collect the goblet or complete her quest. Existing quest completion
    and permanent attack gains must persist.
12. Load older living and completed summits. Damage, freeze, enemy deaths,
    exploration, gold, loot, goblet state and portal destinations must survive.
    Legacy dragons outside the lair return without healing. Old unclaimed
    mantle drops stay recoverable; prior victories without an unclaimed drop
    receive no newly manufactured equipment.

## Code, saves and validation

[`red_dragon.c`](../../../src/game/red_dragon.c) owns fixed signatures, walking
escapes, landing fire, gradual column damage, guaranteed reward recovery and
migration. [`red_dragon_renderer.c`](../../../src/renderer/red_dragon_renderer.c)
draws native props, per-cell countdowns and the encounter panel.
[`game_renderer.c`](../../../src/renderer/game_renderer.c) animates fire streams
and keeps them aligned with Dragonspine's regional camera.

Save version **136** adds one initialized, serialized reward state: 0 not
earned, 1 unclaimed, 2 claimed. The new `Enemy.dragon_warning` stores an exact
42-cell footprint in two 21-bit words, using the saved origin and
facing, including diagonal firing lines. Its cone spans up to twelve tiles,
widening to three across. Compact
storage avoids inflating all enemy arrays and level caches. Enemy serialization
preserves the active and cached masks; existing fields hold signature, phase,
sequence, target, facing, freeze and recovery. Appended map IDs preserve fire
duration, original floor, cracked cover and reliquaries. Migration preserves
progress and never grants replacement rewards after an earlier victory.

Release and Debug builds passed. The full regression suite passed
**5,002/5,002** checks. All **60/60** focused checks and native renderer captures
passed AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes 100
generated lairs, stable and breakable cover, preserved route connectivity and
rosters, sampled walking escapes for every signature, withheld boxed strikes,
fixed/frozen/threshold warnings, diagonal and long-range fire, each Inferno breath and two full recovery
actions, fire duration and native underlays beneath loot, landing occupancy,
all five victory paths, full floor/inventory, reward travel and claims,
active/cached masks, portal wards and version-135 migration. Existing Dragonspine
checks retain travel, goblet placement, Ilya's reward and boss-exit coverage.
