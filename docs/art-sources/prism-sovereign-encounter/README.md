# Prism Sovereign encounter

This eleventh approved boss change set implements only the Glassdeep Caverns
stage-5 Prism Sovereign. Pause for review and a manual commit before the Fallen
Sun Guardian.

Focused Light retains the fixed cardinal beam and adds a recovery opening.
Fractured Crown announces both reflected branches two actions before firing.
Rotate a crystal face between attacks to return a beam to the crown for two
full counterattack actions. Existing health, attack, defense, XP, score,
ordinary enemies, boss journal and south exit remain in effect. The arena
faces are independent of Surveyor Orin's three quest resonators. See
[Glassdeep Caverns](../../glassdeep-caverns.md#prism-sovereign-encounter) for
timing, beam limits, rewards and save details.

## Native renderer review

These controlled fixtures use the actual SDL `game_draw()` renderer and existing
cavern floors, crystal walls, underground pools, sanctuary paving and character
sprites. Reflectors and the reliquary use established crystal and chest props.
Prismweave Robes use the detailed robe icon and equipped-player drawing, with
blue-violet cloth, cyan trim and lilac accents in the equipment icon. Fixture
health is elevated for inspection; these are not completed playthroughs. No
runtime bitmap art was added.

- [Focused Light: one action](beam1.png) and [normal recovery](recovery.png)
- [Split beams: two actions](split2.png) and [one action](split1.png)
- [Frozen warning](frozen.png) and [rotation locked during freeze](locked.png)
- [Face rotated right](rotated-right.png), [single returned beam](return-beam1.png) and [two-action stagger](stagger2.png)
- [Split attack with returned branch](split-return2.png) and [split-attack stagger](split-stagger2.png)
- [Prismweave Robes equipped](prismweave.png), including the equipment-panel icon
- [Rewards beneath loot](reward-loot.png), [partial claim](reward-partial.png) and [reliquary](reward.png)
- [Generated sanctuary](generated.png) and [400-pixel viewport](narrow.png)

Violet numbered cells show the remaining actions before a beam fires. Thin
lines show its reflected legs, with squares at their endpoints. Each branch
has eight total steps and at most one visible bounce. Face labels sit beside
their crystals; Return has a green tint. The narrow panel retains the phase,
countdown, freeze indicator and rotation hint. The same segment calculation
drives warning cells, hit detection and beam projectiles.

## Play testing

Use the existing Debug shortcut launcher and select **New Game**:

```sh
./tools/debug_shortcuts.sh --gold 1000 --weapon short-sword --scrolls magic-arrow,heal
```

Choose Warrior for that loadout. Use `--weapon bow` with Rogue or `--weapon staff`
with Mage to exercise the other combat paths. The launcher leaves the Sovereign
alive. Reach Stillbury through the forest shortcut, leave through its south
gate, and descend Glassdeep to stage 5. Buy supplies before the expedition.
Existing saves with a living Sovereign migrate in place.

1. Enter the final sanctuary. Sidestep a one-action beam, then counterattack
   during recovery. Adjacent attacks must still receive a warning rather than
   triggering an extra melee hit. Walls, pools and living actors block beams;
   removing cover after declaration must never extend the warned endpoint.
2. Stand on or beside a crystal face and press A between attacks. Check the
   Left → Right → Return cycle. Rotation consumes an ordinary action. Use
   recovery and preparation to set Return, draw a beam toward the face, then
   step off its route. Watch it retrace to the crown and allow two full player
   actions before another declaration. Repeat with Warrior, Rogue and Mage.
3. Cross half health during a warning. That route must finish unchanged.
   Observe the next attack: both opposing branches and every reflected leg
   appear immediately with countdown 2. Walk outside the marked cells; no
   extra bounce or unmarked extension should appear when the beams fire.
4. Set a Return face during phase 2. Its branch must end at the crown while
   the other branch follows its independently announced route. Returning the
   beam grants two-action recovery without directly reducing boss health.
5. Try A during a warning, then freeze the Sovereign and try again. The face
   angles and endpoints remain locked; countdowns resume after thawing. New
   blockers may shorten a ray, and blocking a returned ray prevents its
   stagger. Removing an earlier blocker cannot invent a return to the crown.
6. Drop loot on a face and rotate it. Picking up stacked loot must reveal its
   latest angle. Check that crystals and native sanctuary paving stay walkable
   and that pools retain their original appearance and collision.
7. Save/load during a split warning, freeze or two-action stagger. Backtrack
   and use Return to Town: phase, endpoints, health, angles and remaining
   recovery persist. Returning through a portal wards an attack threatening
   its landing without healing the boss or advancing other arena clocks.
8. Defeat the Sovereign with melee, arrows or spells. The south exit opens
   even with ordinary enemies alive. Prismweave Robes and the original Strength
   Potion remain reserved through travel and saves, including with a full
   floor and inventory. Use A beside the reliquary or P on it, even beneath loot.
9. Free one slot: claim the robes while the potion remains reserved. Save/load
   and free another slot to claim the potion. Emptying the reliquary restores
   its original floor or ruined paving. Sold, dropped or consumed rewards
   must not regenerate. The potion must still grant +1 permanent base attack.
10. Equip the robes as Mage. Verify +3 defense, +35 maximum MP and 12% spell-cost
    reduction using actual spells, the new equipment icon and player palette.
    Warrior and Rogue must retain the class restriction. Archmage Robes retain
    their stronger +4 defense, +60 MP and 20% reduction.
11. Visit Orin's Root, Tide and Crown resonators on stages 2–4. Their tuning,
    defenders, lit state and quest progress remain independent. Old defeated
    Sovereigns stay defeated with their original drops and no new repeat reward.

## Code, saves and validation

[`prism_sovereign.c`](../../../src/game/prism_sovereign.c) owns fixed routes,
walking escapes, crystal controls, guaranteed rewards and migration.
[`prism_sovereign_renderer.c`](../../../src/renderer/prism_sovereign_renderer.c)
draws native props, countdowns, segments, endpoints and the encounter panel.
The new armor is defined in [`item.c`](../../../src/game/item.c).

Save version **134** adds one pending-reward mask, initialized in `game_init()`
and serialized/deserialized in `save_load.c`. Independent bits reserve robes
and potion; each clears on claim. Existing serialized enemy fields store the
two branch endpoints, facing, phase, countdown, freeze and recovery. Appended
map IDs preserve face angles and distinguish normal-floor and paved caches.
Migration resets obsolete boss timing while protecting exploration, damage,
deaths, loot, portal destinations and Orin's quest progress. Previous victories
never restart or acquire replacement rewards.

Release and Debug builds passed. The full regression suite passed
**4,890/4,890** checks. All **47/47** focused checks and final native renderer
captures passed AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes
100 generated sanctuaries and walking escapes for both phases, all three face
modes, fixed/frozen/threshold warnings, blocker changes, two complete stagger
actions, all-class rotation, stacked loot, all five victory paths, full
floor/inventory, partial claims, actual robe spell costs and saved metadata,
portals and version-133 active/cached migration. Existing Glassdeep checks also
cover rosters, stage travel, sanctuary layout and Orin's resonators.

The sanitizer build reports the existing Dain dialogue truncation warning in
`game_talk_to_dain`; that unrelated dialogue was not changed here.
