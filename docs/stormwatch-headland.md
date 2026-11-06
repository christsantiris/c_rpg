# Stormwatch Headland

Follow Dawnmere's eastern road, where grass turns to boulder-strewn gravel near
the headland, and walk into the watchfort sprite to enter.
The exterior road remains reachable around the town walls during the occupation;
rescuing the mage does not require attacking royal troops.
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

Save version **245** adds Soren's recruitment, position, HP, MP, reserved
level/XP fields (fixed at 1 and 0), and the selected resurrection target.
Older saves gain the two rumor NPCs and
the reopened eastern road. Existing Matriarch victories free Soren without
repeating the fight or awarding its rewards again. Active and cached expeditions
retain their enemies, wounds, loot and exploration.

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

See the [approved design](stormwatch-headland-proposal.md).

## Rescue Mage Soren

Traveler Neris waits in Oakhaven's square at `(17,13)`. Fisher Tovan waits in
Dawnmere's Alehouse at `(12,14)`. Either points to the powerful mage held by
the Tempest Matriarch. These are independent leads, use no quest slots and
are not required to find or rescue him.

Soren is visible inside a violet ward at `(46,12)` in Matriarch's Court. He
cannot be attacked while captive. Defeating the Matriarch breaks the ward;
talk to Soren and answer **Y/N** to his recruitment offer. Captivity has
drained his strength: he joins as a **level 1 Mage**, with **70 HP, 100 MP**
and **Fireball I**. Declining sends him to Dawnmere's Alehouse at `(28,14)`.
Speaking to him there allows recruitment, dismissal and later re-recruitment
without resetting his recruitment status.

He follows alongside Aldric and automatically seeks a clear firing lane.
Fireball deals **27 damage** at his fixed level 1, with range 4,
radius 2 and 20-MP cost. Walls stop the blast and it does not hurt allies.
When out of mana, he fights with his staff. He never initiates combat against
unalerted Dawnmere troops or a neutral tribute convoy.

Soren stays at **level 1**, with **70 max HP and 100 max MP**, and earns no XP.
The player receives their normal XP rewards. Companion progression is deferred
to a later update. Entering a peaceful town restores his HP/MP; earning player
XP does not. Earlier draft saves with a leveled Soren load at level 1, keeping
his wounds and mana within these limits. Death persists until resurrection;
alive companions are never resurrection targets. The existing shrine service
and the player's Resurrection spell can choose either fallen companion.
Soren joins knowing Fireball only; he does not cast Resurrection.

Validation covers all five stages across three seeds, walking through actual
entrances, retreat and onward transitions with either shutter state. Regression
checks cover recruitment and decline, joint travel with Aldric, combat and
neutral soldiers, fixed stats across player level-ups, individual resurrection targets, legacy saves, doors,
skiff crossings and island voyages. Native renderer checks at **1280×720** and
**640×480** cover both rumor NPCs, the fort approach, captive ward, recruitment,
party status, Fireball and shrine selection. An existing local save was loaded
and rendered without writing to the original save files.

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
