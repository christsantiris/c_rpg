# Stonewake Quarry

Stonewake replaces Glassdeep Caverns on the northern branch of Stillbury's
western bridge road, beside the optional Labyrinth. Cliffs block onward travel
to Rosemoor; this is a local adventure for equipment and experience. Walk into its limestone
quarry landmark to start stage 1; the rail ramps at the northern
and southern boundaries use the standard area travel controls. The final exit
returns to Stillbury's quarry approach even while the Quarry Colossus lives.
Boss victory and quest rewards remain separate. See [boss gates](boss-gates.md).
The atlas keeps the beaches and cliff-backed shoreline below the quarry,
Sunken Coast's sheltered cove and Oakhaven's connected branching docks.

The quarry is drawn at the Crown Roads' 1.5× zoom: the camera shows about 30 ×
18 tiles around the player. The Quarry Colossus acts within twelve tiles; the camera leans toward it to keep it on screen as its telegraphed windups begin.

## Five stages

| Stage | Name | Exploration and quest |
| --- | --- | --- |
| 1 | Outer Workings | Learn the connected yard/ramp layout and optional crane walkways. |
| 2 | Excavation Pits | Reclaim the guarded West Winch. |
| 3 | Buried Workshops | Reclaim the guarded Workshop Hoist. |
| 4 | Abandoned Galleries | Reclaim the guarded Main Gantry; Stone Giants join the roster. |
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
| Ogre | Shared melee bruiser from Giant’s Peaks. | 42 / 12 / 3 |
| Quarry Construct | Ambush melee guard with an iron-bound stone body. | 38 / 16 / 3 |
| Stone Troll | Slow, durable close combat threat. | 110 / 18 / 9 |
| Stone Giant | Shared heavy melee enemy introduced on stage 4. | 60 / 14 / 6 |
| Quarry Colossus | Stage 5 boss, roughly twice player height. | 275 / 24 / 8 |

These are base values; the existing region/player progression scaling still
applies. Fresh physical expeditions respawn ordinary enemies. Stair travel,
loading keep the current expedition's damage and deaths. Return to Town is a
one-way escape to the last town visited, with no return portal.
Boss victory and its guaranteed rewards remain one time.

The Construct, Stone Troll and Colossus use their established padded atlas cells.
Ogres and Stone Giants use their existing shared sprites at native player scale.
All five share the player's ground line. The [approved enemy roster](enemy-roster.md)
records their shared areas; older scale comparison images show the previous roster.

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
strongbox offers **one Potion of Strength or Potion of Intelligence**. Interact
beside it or use Pick Up while standing on it, then press 1 or 2. A full pack
leaves the choice pending. Quest equipment comes separately from Orin.

## Reclaim the Quarry

Surveyor Orin shelters with displaced quarry workers at the **workers’ camp on
Stillbury’s western quarry road**, beside the western Soulstone Shrine. Stillbury
needs Stonewake’s stone to repair its walls, roads and river landing. This is an
optional Act 4 recovery adventure; it never opens a road to Rosemoor or the castle.

Accept with Y, or explore first. Clear each machine’s defenders and press Action
beside the West Winch on stage 2, Workshop Hoist on 3 and Main Gantry on 4. The
mechanisms repair directly; legacy maps retain their existing tone interactions.
Repairs and the Colossus defeat count in either order, including before accepting
the quest. Defeating every ordinary enemy is unnecessary.

**All three repairs plus the Colossus defeat complete the quest immediately** and
release its active quest slot. Stillbury welcomes you again: civilians thank you,
its checkpoints allow passage, and it becomes a safe Return to Town destination
once revisited. The Crown’s historical record remains intact. Completion is
permanent, including when ordinary enemies return during a later expedition.

Return to Orin for **120 gold, 1,000 score and ONE equipment choice**:

| Key | Reward | Properties |
| --- | --- | --- |
| 1 | Magic Stone Warhammer | +8 attack, +30 maximum HP; rare, two handed, all classes. |
| 2 | Quarryman’s Bow | +9 attack, range 6, +20 maximum HP; rare, two handed, Rogue. |
| 3 | Prismweave Robes | +3 defense, +35 maximum MP, 12% spell cost reduction; Mage. |

Equipment bonuses apply only while equipped. Equipping does not heal; removing
HP equipment clamps current HP to the new maximum. Esc defers an equipment or
potion choice. Full packs leave the reward safely pending; choices, payment and
claims survive saving, and selling or dropping a reward never grants another.

The western shrine uses the same native shrine art, warlock and services as the
Oakhaven shrine. It works **before any quarry objective**: 200 gold resurrects a
fallen companion at full health; a level-10 Mage who knows Resurrection can use
the ritual circle for 80 MP and half health. The warlock also sells the existing
400-gold scroll. Both shrines remain available independently.

The Three Great Hunts retains Shardwarden's group on stage 4, now in Stonewake.
Existing hunt and quarry quest progress is retained.

## Art and compatibility

`tools/sprites/stonewake.py` uses the established pixelkit pipeline for the
168×144 entrance, five 48×48 enemy cells, three 72×72 crane poses in a 216×72
sheet, 96×72 workshop and four
24×24 machinery/supply props. These assets have transparent backgrounds and
use the same native pixel scale as existing buildings and the player. See
[approved concepts](stonewake-quarry-proposal.md).

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

The internal `GLASSDEEP`, `PRISM_SOVEREIGN`, `BLIND_STALKER` and `SHARD_GOLEM`
identifiers retain their old numeric meanings for compatibility. Retired creature
IDs are reserved; save version 222 migrates actors to the approved shared types
without resetting wounds, deaths, exploration, loot or quest progress.

Save version **249** adds persistent equipment and potion choices and their
pending dialogue. Older completed quarry quests keep their completion, currency
and earned warhammer; older boss victories keep their existing robes/potion loot
and never gain a second strongbox choice. Active and cached machinery is repaired
in place without resetting exploration, enemy wounds, loot or crane settings.
Orin moves out of Rosemoor’s Guild. New rewards apply to unfinished objectives.
Local saves are backed up before migration checks.

The canonical [world map](world-map.png) marks the western shrine beside the local
quarry/Labyrinth road. Its northern escarpment and sole northern route to the
castle are unchanged. Act V remains provisional.


See [quarry recovery validation and native screenshots](quarry-recovery/validation.md).
