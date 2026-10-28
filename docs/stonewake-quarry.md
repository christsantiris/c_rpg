# Stonewake Quarry

Stonewake replaces Glassdeep Caverns on Stillbury's southern road. Walk into
its limestone quarry landmark to start stage 1; the rail ramps at the northern
and southern boundaries use the standard area travel controls. The final exit
returns to Stillbury's quarry approach even while the Quarry Colossus lives.
Boss victory and quest rewards remain separate. See [boss gates](boss-gates.md).
The atlas keeps the beaches and cliff-backed shoreline below the quarry,
Sunken Coast's sheltered cove and Oakhaven's connected branching docks.

The quarry is drawn at the Crown Roads' 1.5× zoom: the camera shows about 30 ×
18 tiles around the player. The Quarry Colossus acts within twelve tiles, so it
may begin its telegraphed windups from just off screen.

## Five stages

| Stage | Name | Exploration and quest |
| --- | --- | --- |
| 1 | Outer Workings | Learn the connected yard/ramp layout and optional crane walkways. |
| 2 | Excavation Pits | Reclaim the guarded West Winch. |
| 3 | Buried Workshops | Reclaim the guarded Workshop Hoist. |
| 4 | Abandoned Galleries | Reclaim the guarded Main Gantry; Iron Sentinels join the roster. |
| 5 | Colossus Yard | Defeat the Quarry Colossus, using the nearby crane to interrupt windups. |

Each generator produces nine work yards with two flooded excavation cuts,
permanent ramp circuits, two optional crane crossings, timber workshops and
side stores. Yard proportions and connections vary by stage and seed. The
route loops through the site and its side yards; it is not a single winding
corridor. Quarry walls are cut limestone, the ground is warm gravel with moss,
and turquoise pool ripples animate separately from the timber machinery.

Press **A / Action** beside a crane or its far-bank lever to cycle its routes:

| Setting | Traversal |
| --- | --- |
| Raised | Both gangways are lifted; take the permanent ramps around the pit. |
| Crossing | A nine-tile timber walkway connects the two banks; salvage access stays closed. |
| Salvage | The west half stays down and a north spur reaches an isolated supply platform; the east half lifts. |

The salvage platform cannot be reached by the crossing or the permanent ramps.
Choose between a quick through route and a detour for supplies. The crane's load,
walkways, status label and next-action hint change together. The minimap marks
controls and unopened supplies. Both banks retain a permanent route around the
pit, so an untouched crane never blocks progression.

A walkway cannot retract under the player, an enemy, dropped loot or a return
portal. Occupants, loot or a portal on the salvage platform also hold its access
open. Crane settings persist through stairs, saves and return visits. Supplies
pay 25–45 gold (depending on stage) once each, after nearby guards are defeated;
an opened store remains empty. Ordinary enemies respawn on connected main ground
when a fresh expedition begins, including enemies previously killed on a platform.

The debug shortcut launcher initially lowers each unused crossing once. Further
actions respect the player's selected setting, including raised bridges after
save/load; it no longer lowers a raised crane again before every movement action.

## Enemies

| Enemy | Role | Base HP / attack / defense |
| --- | --- | --- |
| Rockworm | Small, fast melee pursuer. | 30 / 11 / 2 |
| Quarry Construct | Ambush melee guard with an iron-bound stone body. | 38 / 16 / 3 |
| Stone Troll | Slow, durable close combat threat. | 110 / 18 / 9 |
| Iron Sentinel | Heavily armored melee guard introduced on stage 4. | 95 / 20 / 10 |
| Quarry Colossus | Stage 5 boss, roughly twice player height. | 275 / 24 / 8 |

These are base values; the existing region/player progression scaling still
applies. Fresh physical expeditions respawn ordinary enemies. Stair travel,
loading keep the current expedition's damage and deaths. Return to Town is a
one-way escape to the last town visited, with no return portal.
Boss victory and its guaranteed rewards remain one time.

The 48×48 atlas cells contain padding, not oversized ordinary bodies. Construct
and Sentinel are 23 pixels tall beside the 24-pixel player; the Rockworm is
23×14. The deliberately larger Stone Troll is 29×30 and the Colossus is 44×42.
All five share the player's ground line. See the [renderer before/after scale
comparison](images/stonewake-enemy-scale.png) and the [complete enemy/area
mapping](enemy-roster.md).

### Quarry Colossus

The Colossus marks a fixed 3×3 hammerfall with **two response actions** before
impact. Leave the amber squares, then use its two-action recovery window to
attack. At half health it alternates hammerfall with a diamond-shaped
shockwave, allowing **three response actions**. Warnings retain their original
target when the player moves and pause with the existing freeze system.
Attacks are reduced or withheld if no escape route exists.

Operating a nearby crane during a windup drops/jars its load, interrupts the
attack and opens a two-action recovery. A five-action interrupt cooldown
prevents repeatedly toggling a crane from locking the boss down indefinitely.
Warnings, recovery, phase, cooldown and wounds survive saves. The inherited
strongbox reward remains Prismweave Robes and a Strength Potion; it waits if
the pack is full. The quest warhammer is a separate reward from Orin.

## Reclaim the Quarry

Surveyor Orin in **Rosemoor's Adventurer's Guild** assigns this quest. Clear the
guards and press Action beside the West Winch on stage 2, Workshop Hoist on 3
and Main Gantry on 4. Each mechanism repairs directly; the old tone-selection
puzzles are retired. The objectives can be completed in any order. The
Colossus is optional for this quest.

Return to Orin for **120 gold, 1,000 score and Magic Stone Warhammer**:

- +8 attack, rare, two handed, usable by all classes.
- **+30 maximum HP while equipped**, shown in equipment comparison.
- Equipping does not heal; unequipping clamps current HP to the reduced maximum.
- Weapon swaps, save/load and Royal Guard confiscation apply/remove the bonus
  consistently. No permanent base-HP increase is granted.
- With a full pack, Orin holds the hammer until a later conversation. Currency
  and equipment each pay once. The journal lists the equipment reward.

The Three Great Hunts retains Shardwarden's group on stage 4, now in Stonewake.
Existing hunt and quarry quest progress is retained.

## Art and compatibility

`tools/sprites/stonewake.py` uses the established pixelkit pipeline for the
168×144 entrance, five 48×48 enemy cells, three 72×72 crane poses in a 216×72
sheet, 96×72 workshop and four
24×24 machinery/supply props. These assets have transparent backgrounds and
use the same native pixel scale as existing buildings and the player. See
[approved concepts](stonewake-quarry-proposal.md),
[native comparison](images/stonewake-native-comparison.png),
[approach capture](images/stonewake-quarry-approach.png),
[workshop capture](images/stonewake-quarry-workshop.png),
[crane route comparison](images/stonewake-crane-routes.png),
[Colossus capture](images/stonewake-quarry-colossus.png), and
[atlas edit record](art-sources/world-map/stonewake.md).

Save version **194** adds the persistent unearned/reserved/claimed warhammer
state. Older active and cached Glassdeep maps are retiled in place, preserving
safe geometry, explored squares, enemy wounds/deaths, floor loot, quests,
portals and boss victories. Old optical props become quarry paving; optional
cranes are added across spare rock/water where they fit without closing any
existing walking squares. Unvisited floors use the new quarry generator. On the next fresh physical
expedition, old floors are replaced with full quarry layouts while maximum
stage, quests, equipment, currency and boss victories remain. Loading or
taking stairs never replaces an expedition underneath the player. Older return
anchors are retired when loading a save.
Retired prism windups and tone prompts are canceled safely. Previously completed
quests reserve the new warhammer with Orin without paying their gold again.

Save version **195** upgrades active and cached quarry cranes in place. Spare
water/rock becomes the isolated supply platform, and old shore stores move onto
it while retaining their claimed state. Floor loot, wounds, player position and
portal anchors remain intact. An older cut without enough spare space keeps its
two-state crossing until a fresh quarry layout replaces that legacy expedition.

The internal `GLASSDEEP`, `PRISM_SOVEREIGN`, `CRYSTAL_SPIDER`, `BLIND_STALKER`
and `SHARD_GOLEM` identifiers and JSON keys retain their old numeric meanings
for compatibility. New player-facing labels and runtime art use Stonewake.
The Iron Sentinel, quarry terrain and hammer metadata append new enum values;
no existing location, enemy, item or tile IDs are reassigned.

Crown Road North and the Catacombs structural overhaul are separate follow-up
change sets. Stormwatch/eastern Dawnmere development remains a later design.

Verification and visual review are recorded in
[implementation validation](art-sources/stonewake-proposal/validation.md).
