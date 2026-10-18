# Glassdeep Caverns

Stillbury's south gate leads into five stages of crystal caverns as soon as
the town is accessible. Enter each stage from its north edge and advance
through the south exit. The north entrance backtracks; on stage 1 it returns
to Stillbury's south gate. Ordinary enemies do not block stage travel.

Winding chambers contain jagged rock, cyan and violet crystal formations,
pale mineral seams, underground pools, and ancient paving. Rock and water
block movement; ruins remain walkable. Glowing fragments and reflections
animate while idle. The pools do not cause damage. Stage 5 ends in a buried
sanctuary guarded by the Prism Sovereign.

| Enemy | First stage | Behavior |
|---|---|---|
| Crystal Spider | 1 | Closes two tiles per turn without attacking on its second move |
| Blind Stalker | 1 | Waits until an explorer comes within four tiles or damages it, then pursues |
| Shard Golem | 3 | Slow, armored melee enemy |
| Prism Sovereign | 5 | Holds its sanctuary; fixed beams escalate to reflected split beams below half health |

### Prism Sovereign encounter

**Focused Light** marks an eight-step cardinal beam with violet outlines and
one action to escape. After firing, the Sovereign grants one full action for
a counterattack. At half health, **Fractured Crown** begins on the next attack
declaration: two opposing beams pass through the sanctuary's crystal faces,
with two actions to escape. Crossing the threshold never changes an already
announced attack. Adjacent players receive the same warning instead of an
extra unannounced melee attack.

Each branch travels at most eight steps in total, including its incoming and
reflected legs, and can bounce only once. Every threatened cell, reflected
segment and terminal cell appears when the warning begins. Walls, pools and
living enemies stop beams. Removing a blocker cannot extend a declared route;
new blockers can shorten it. Freezing the Sovereign preserves its warning,
phase and remaining countdown until it thaws.

Two walkable crystal faces provide the counter. Press **A** on or beside one
between attacks to cycle **Left → Right → Return → Left**. Left and Right turn
relative to the incoming beam. Return retraces it to the crown, exposing the
Sovereign for **two full counterattack actions** without directly damaging it.
Rotation costs an ordinary action and requires no class ability or item.
Faces lock throughout warnings, including while the boss is frozen, so the
announced geometry cannot change. Use recovery and preparation to set a Return
face, then step off the incoming beam before it fires. Ordinary walking escapes
remain available in both phases.

Defeating the Sovereign opens stage 5's south exit, which returns to Stillbury
without adding another town, and records the victory in the boss journal.
Its prismatic reliquary reserves two guaranteed rewards independently:

- **Prismweave Robes:** rare Mage armor granting **+3 defense, +35 maximum MP
  and 12% spell-cost reduction**. This remains below Archmage Robes' +4 defense,
  +60 MP and 20% reduction.
- **Potion of Strength:** grants one permanent base attack when consumed.

Press **A** beside the reliquary or **P** while standing on it, including when
other loot covers it. A full floor or inventory cannot lose either reward.
With one free slot, collect the robes first and leave the potion reserved.
Once claimed, sold, consumed or dropped rewards never regenerate. Emptying the
cache restores its original cavern floor or sanctuary paving. Native pools and
Orin's quest resonators remain independent of these arena controls.

All five stages maintain independent caches. Backtracking preserves enemy
health. A new expedition restores regular enemies on every visited stage while
preserving explored terrain, quests, and boss progress. Return to Town
opens a portal beside Stillbury's south gate and restores the original stage
and position. Save format **134** adds the pending-reward mask; existing enemy
fields and map serialization preserve phase, fixed endpoints, facing, freeze,
recovery, stagger and crystal angles in active and cached sanctuaries. Older
saves reset obsolete boss timing and add faces around a living Sovereign while
preserving health, deaths, loot, exploration, portal landings and quest
progress. Previous victories retain their original potion drops without
creating new rewards or reviving the boss. Saves predating the region still
gain fresh Glassdeep progression and the open south gate while retaining their
characters and other regional progress.

See the [native renderer gallery and play-test guide](art-sources/prism-sovereign-encounter/README.md).


## The Broken Resonance

Surveyor Orin waits inside **Rosemoor's Adventurer's Guild**. Approach him and
press `T` to hear his offer, then `Y` to accept his survey quest. He directs you to **Stillbury's south gate** and asks you to
restore three ancient resonators.

| Stage | Resonator | Defenders | Inscription and tone |
|---|---|---|---|
| 2 | Root, on ancient survey paving | Two Crystal Spiders | Roots answer with a **low** rumble |
| 3 | Tide, beside an underground pool | Two Blind Stalkers | Between the deep and the sky, the **middle** voice flows |
| 4 | Crown, on ruined paving | One Shard Golem | The crown rings **high** above the stone |

Defeat the quest defenders and nearby enemies, then press `A` while standing
on or beside a resonator. Its inscription opens a tuning prompt: `1` selects
Low, `2` Middle, and `3` High. Number-pad digits also work. `Esc` or `Enter`
cancels. Other commands wait until the tuning prompt closes. A wrong tone
repeats the clue without dealing damage or resetting progress.

The correct tone lights the crystal with an animated glow and marks its
objective complete in the quest journal. Complete the resonators in any order,
then return to Orin in Rosemoor for **120 gold and 1,000 score**, awarded once.
The stage 5 Prism Sovereign is optional for this quest. Resonators occupy no
inventory slots.

Accepting the quest preserves previously explored maps and enemy progress.
Quest encounters are added when their stages are visited; returning to a
cached stage preserves guardian health and defeated guardians. Restored
resonators remain lit when revisiting, including through Return to Town
portals. Save version **94** preserves quest state, individual restorations,
guard encounters, and an open tuning prompt. Older saves initialize the new
quest without losing existing progress; saves inside the Guild receive Orin
with any overlapping player or loot moved beside him.
