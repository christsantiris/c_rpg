# Fallen Sun Guardian encounter

This twelfth approved boss change set implements only the Ruined Temple's
summit Guardian. Pause for review and a manual commit before the Red Dragon.

Sunbound Armor's fixed lances escalate to Cracked Core's sunburst rings, then
Eclipse's alternating solar lanes and lunar bursts. Every signature is warned,
including against adjacent and ranged attackers. Existing Sun/Moon alignment
provides an armor-versus-exposure choice and protection from the announced
element; sanctuary plates provide complete matching protection. Original
health, base attack, defense, XP, boss score, roster and vault progression stay
in effect. Sunwarden Mantle is a direct victory reward, separate from Nahla's
Buried Sun treasure quest. See [the regional guide](../../harbor-and-ruined-isle.md#fallen-sun-guardian)
for exact timing, footprints, guards and rewards.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer and the
existing temple floor, masonry, water, altar, sun-plate, chest and character
art. Cyan M and golden S plates identify solar and lunar refuges respectively.
An A at the base of each summit altar marks its interaction. The mantle uses
the established detailed light-armor icon and player drawing, with amber cloth,
gold trim and cyan icon accents. Fixture health is elevated for inspection;
these are not completed playthroughs. No runtime bitmap art was added.

- [Sunbound lance](lance1.png) and [counterattack recovery](recovery.png)
- [Cracked Core ring: two actions](ring2.png), [one action](ring1.png) and [frozen](frozen-ring.png)
- [Moon sanctuary inside the sunburst](moon-sanctuary.png)
- [Eclipse solar lanes: two actions](solar-lanes2.png) and [one action](solar-lanes1.png)
- [Altar use during a fixed warning](moon-alignment.png)
- [Lunar burst: two actions](lunar-burst2.png), [one action](lunar-burst1.png) and [Sun refuge](sun-sanctuary.png)
- [Mantle on Warrior](mantle-warrior.png), [Mage](mantle-mage.png) and [Rogue](mantle-rogue.png), including equipment-panel icons
- [Direct victory reward](direct-reward.png), [full-pack reliquary](reliquary.png), [cache beneath loot](covered-reward.png) and [claimed cache](claimed-reward.png)
- [Native summit roster](native-summit.png) and [400-pixel viewport](narrow.png)

Amber cells warn solar attacks; cyan cells warn lunar attacks. Numbers show
remaining ordinary actions, and an asterisk on the HUD indicates a frozen
countdown. Matching sanctuary plates are excluded from damage and warning
cells, independent of alignment. The same predicate drives warnings and hits.
On narrower windows the boss panel includes the sealed-foe count, replacing
the overlapping treasure notice and sitting below the minimap.

## Play testing

Use the existing Debug shortcut launcher and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage to exercise the other combat paths. The launcher leaves the Guardian
alive and unlocks town shortcuts. Defeat the Drowned Queen to open the docks,
or use an existing save with that route unlocked. Speak with Captain Rowan,
board the ship, speak with Nahla near the island temple approach, and climb
all five tiers. Awaken and clear each tier's Moonbound Sentinels as well as
its Sun enemies. Buy
supplies before the expedition. Existing saves with a living Guardian migrate
in place.

1. Enter the summit sanctuary. Observe the one-action cardinal lance. Step
   aside, then use its full recovery action to counterattack. Attacking from
   range or standing adjacent must not produce an immediate extra hit.
   The opening phase alternates lances with two-action lunar bursts. Diagonal
   attackers draw bursts, and camping an M/S plate draws its opposite element.
   Verify the counter is declared before moving to a matching refuge.
2. Draw a lance toward a wall, closed door or living actor. The declared
   endpoint stays fixed. Removing the blocker or opening the door cannot
   extend it; new cover may shorten it. An awakened or damaged Guardian can
   threaten the approach corridor, but its closed sanctuary door still blocks
   the lance.
3. At half health, finish any existing warning unchanged. Observe alternating
   lances and two-action sunburst rings. Walk into the adjacent interior or
   beyond the ring's outer edge, or stand on a western M plate inside that edge.
   The matching sanctuary must protect regardless of alignment.
4. At quarter health, finish the current signature before Eclipse begins.
   Solar crosses and lunar bursts alternate with separate two-action warnings.
   Walk twice to escape their fixed centers. Check the cyan lunar countdown
   and the announced Sun guard, then the amber solar warning and Moon guard.
5. Use either summit altar during a warning. Sun reduces damage dealt to the
   boss by one third; Moon exposes its core. Moon halves incoming solar hits,
   while Sun halves lunar-burst hits, after normal defense. Guards reduce damage;
   matching M/S sanctuary plates prevent it completely. Repeat altar use with
   Warrior, Mage and Rogue using ordinary actions and no consumables.
6. Switching alignment must preserve the attack's footprint and remaining
   countdown. Repeated Frost Bolts still deal damage but cannot freeze the
   Guardian or stop its warnings. Existing saved freezes resume after thawing.
   A priest healing the Guardian cannot reverse a latched phase.
7. Open lunar doors and awaken sentinels. Try switching to Sun while a doorway
   contains the player, an enemy or loot. The entire switch must wait. Clear it
   and switch successfully; awakened enemies must not become dormant again.
   Drop loot over an altar and confirm A still works. Existing solar traps
   retain their original Sun/Moon behavior.
8. Save/load active and cached warnings, freeze, recovery and alignment. Leave
   and return through a town portal. Its exact landing is warded if threatened,
   even when temporary cover hides a declared lance. Health and encounter clocks
   otherwise persist.
9. Defeat the Guardian with melee, arrows or each damage spell. The mantle goes
   directly into a spacious pack and the vault opens. Sun and Moon survivors
   still seal the treasure. Nahla's quest remains unfinished until its original
   treasure pickup and return to Nahla on the Ruined Isle.
10. Repeat with full inventory and floor loot. A reliquary reserves the mantle
    without deleting items. Save, portal out, backtrack and return. Free one
    slot, then use A beside it or P on it, including beneath stacked loot.
    Claiming restores the original floor. Selling or dropping the claimed
    mantle must never create a replacement.
11. Equip Sunwarden Mantle on all three classes. Verify +5 defense, +8 maximum
    HP and +7 maximum MP, the amber player palette and equipment-panel icon.
    Save/load the equipped item and check its stats and metadata.
12. Load an older living summit and a previously completed temple. Migration
    must preserve damage, freeze, deaths, exploration, loot, portal landings,
    opened routes and Nahla's progress. Previous victories stay defeated and
    receive no newly manufactured mantle.

## Code, saves and validation

[`sun_guardian.c`](../../../src/game/sun_guardian.c) owns fixed signatures,
walking escapes, Sun armor, sanctuary protection, guaranteed rewards and
migration. [`sun_guardian_renderer.c`](../../../src/renderer/sun_guardian_renderer.c)
draws native props, countdowns and the encounter panel. Existing altar/door
handling in [`game.c`](../../../src/game/game.c) preserves occupants and stacked
underlays. The mantle is defined in [`item.c`](../../../src/game/item.c).

Save version **135** adds one reward state initialized in `game_init()` and
serialized/deserialized in `save_load.c`: 0 not earned, 1 reserved, 2 claimed.
Existing serialized enemy fields hold phase, alternating sequence, fixed
target, lance endpoint, facing, countdown, freeze and recovery. Appended map IDs
preserve M/S plates and the reliquary. Migration resets obsolete boss timing
without resetting health, deaths, alignment, treasure or quest progress. It
protects portal destinations and never grants retroactive or duplicate rewards.

Release and Debug builds passed. The full regression suite passed
**4,942/4,942** checks. All **51/51** focused checks and native renderer captures
passed AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes every
walkable sanctuary position for all four signatures and both alignments,
fixed/frozen/threshold warnings, one complete recovery action, real all-class
altar use, occupied and covered doors, guard damage, all five victory paths,
full inventory/floor, covered caches, portal/backtracking reward recovery,
all-class equipment and saved stats, temporary-cover arrival wards and
version-134 active/cached migration. Existing temple checks retain tier travel,
seals, sentinels, treasure and Nahla's reward coverage.

The sanitizer build reports the existing Dain dialogue truncation warning in
`game_talk_to_dain`; that unrelated dialogue was not changed here.
