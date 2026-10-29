# Stormwatch Headland

Follow Dawnmere's eastern road and walk into the watchfort sprite to enter.
Stormwatch is a five-stage coastal fort region with warm grass, gray masonry,
cliffs, animated teal water, side lookouts and searchable supply chests.
The approach uses travel music; the adventure uses the dungeon theme.
The headland is drawn at the Crown Roads' 1.5× zoom: the camera shows about
30 × 18 tiles around the player, so a wind lane's warnings show only where it
crosses the view.

| Stage | Layout and encounters |
| --- | --- |
| 1 — Windward Approach | Forked coastal fingers, a watch hut, two optional lookouts and a sheltered loop; Harpy Raiders introduce warned swoops. |
| 2 — Broken Gatehouse | Courtyards and guardrooms around a coastal cut; Cliff Gargoyles and the first working storm shutter. |
| 3 — Three Watchtowers | Triangular tower route, supply pockets and two staggered wind lanes; Harpy Stormcallers join the patrols. |
| 4 — Stormwall Fort | Outer patrol loop, connected fort rooms and an exposed shutter shortcut; Storm Elementals defend the crossing. |
| 5 — Matriarch's Court | Rampart loop, side supply rooms and the Matriarch's northern court. The final departure remains available while she lives. |

Every stage has a walking loop and two optional supply branches. Gate markers
sit on the northern and southern map boundaries. Walking onto them advances or
retreats; stairs keys also work while standing on the threshold. Fresh physical
entry always starts at stage one and repopulates the expedition. Backtracking
and loading retain defeated enemies and dropped loot.

## Wind and shutters

Wind advances automatically while the adventure is visible, including when the
player stands still. Each phase lasts one second: two calm phases, two warning
phases, one gust, then one recovery phase. From stage three, a second lane
follows the same cycle at an offset. The HUD shows seconds until the gust;
stepping into exposed ground and releasing the keys does not stop the countdown.

Wind runs separately from combat turns. Idle gusts give enemies no extra moves,
attacks or status ticks. Menus, modal prompts, unfocused/minimized windows and
projectile/transition animations pause the wind. Resuming does not replay missed
gusts. Stormcallers can choose the next gust direction, but cannot change a
warning already shown.

During the final warning second on an exposed tile, press **A** away from a
crank or supply chest to brace. Hold still: the brace lasts until the scheduled
gust, and another player action releases it. Bracing consumes a normal combat
action and does not absorb enemy damage. On other phases, A keeps its normal
movement behavior; the left arrow always moves.

A gust moves the player or a Raider one cardinal tile. Successful pushes
animate a full tile and report their direction. Walls, actors, raised shutters,
water and gateways stop the push without collision damage. Messages explain a
blocked landing or a successful brace. A push cannot land in a declared enemy
attack. Displaced Raiders lose prepared swoops; stone and storm creatures resist
wind.

Use **A** beside either crank to lower or raise the shutter. Raised shutters
block movement, projectiles and wind. Lowered shutters form a timber shortcut.
The outer loop remains open in both states. Raising a shutter can reduce a
declared wind warning; lowering it adds exposure only when the next warning
begins. Actors, dropped items and a return-portal landing prevent raising a
shutter through an occupied threshold.

## Enemies

| Enemy | First stage | HP | Attack | Defense | Behavior |
| --- | --- | --- | --- | --- | --- |
| Harpy Raider | 1 | 36 + 3 × stage | 12 + stage | 2 | Warns a two-tile swoop, then recovers for one action. Gusts can interrupt preparation. |
| Cliff Gargoyle | 2 | 64 + 3 × stage | 16 + stage | 7 front / 2 rear | Warned three-tile lunge; fixed facing and defense 1 during recovery. |
| Harpy Stormcaller | 3 | 46 + 3 × stage | 15 + stage | 3 | Warns fixed lightning ground and selects the next wind direction. |
| Storm Elemental | 4 | 72 + 3 × stage | 18 + stage | 4 | Warned short crosswind fan, blocked by cover. Immune to gust displacement. |
| Tempest Matriarch | 5 | 320 | 29 | 7 / 1 recovering | Alternates a two-action dive warning and directional wing sweep. Below half health, adds two fixed lightning targets and calls one finite pair of Raiders. |

The Matriarch remains on reachable land. Her dives and lightning give two
complete warning actions; every resolved attack gives two recovery actions.
Crossing half health does not replace a pending attack. Freezing pauses an
enemy's preparation or recovery while environmental wind continues. Summoned
Raiders give no XP or loot. First victory awards **220 gold and 2,600 score**;
the boss also gives 450 combat XP. Victory appears in the boss journal.

## Persistence and testing

Save version 199 stores all five compact maps, exploration, actors, enemy attack
states, loot underlays, searched supplies, shutter positions, wind clocks,
declared warning masks, the elapsed fraction of the current second, next
directions, brace state and boss victory. Version 198 saves preserve their maps
and warnings and receive a full second before their next wind phase.
Return to Town is a one-way escape to the last town actually visited and leaves
no return portal. Existing saves gain Dawnmere's eastern road
without resetting gear, quests, portals or other regions.

Travel, including the final departure, remains open while the boss lives;
leaving awards no boss victory or rewards. Shutters and wind retain their normal
behavior. See [boss gates](boss-gates.md).

See the [approved design](stormwatch-headland-proposal.md),
[native renderer captures](art-sources/stormwatch/README.md), and
[atlas placement](art-sources/stormwatch/atlas-edit.md).

## The Silent Stormbell

Watchkeeper Seren assigns this quest beside Dawnmere's east road. Recover the
clapper in the northeast ruined tower on stage 3. Fit it to the bell east of
stage 4's shutter, then interact again for a warning and Y/N confirmation.
Two Harpy Raiders answer, followed by one Harpy Stormcaller after both Raiders
fall. The second wave arrives at the end of a normal enemy phase and cannot
attack during its arrival. Environmental wind never advances the encounter.
The courtyard remains connected with either shutter position; retreat is safe.

The quest tracks discoveries outside the inventory. Surviving quest harpies
retain wounds, positions, and attack warnings through saves, portals, and fresh
expeditions. Ordinary enemies still respawn on physical re-entry. Repeated
ringing cannot restart a wave. Return to Seren for 180 gold and 1,200 score,
awarded once and independently of the Tempest Matriarch. Save version 206 also
repairs older expeditions missing their Matriarch because another enemy took
her spawn, preserving previous boss victories and expedition progress.
