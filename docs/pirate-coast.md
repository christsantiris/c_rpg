# Pirate Coast

Pirate Coast is a five-stage optional adventure **south of Dawnmere**. Leave
through the south gate or follow the town's exterior road south. A safe,
single-tile cobblestone approach reaches a pirate harbor fort on the shore.
Open sea fills the southern view. Walking through the fort's arch begins stage
1; retreat and victory both return beside the fort, facing the road back to
Dawnmere. Entering Dawnmere from that road places the player by its south gate.

The eastern **Saltwind Mangroves** expansion is selected for the next change
set. It is not playable yet. Glassdeep Caverns stays south of Stillbury, and
Pirate Coast does not add or bypass a castle route.

## A dedicated generator

`src/game/pirate.c:pirate_generate()` constructs an 80×36 coastal map.
It does not call the dungeon, forest, swamp or Sunken Coast generators. Connected
clearings have narrow routes, loops and three plunder chests on side branches.
Sea, cliffs and barriers separate those routes instead of filling the map with
one open beach. A saved seed controls clearing shapes, route bends and initial
enemy placement, so revisiting does not reroll the terrain.

| Stage | Name | Layout | Entrance → continuation |
| --- | --- | --- | --- |
| 1 | Wreckers' Strand | Sandy coves cut through cliffs, a western loop, beach wrecks and secluded plunder pockets. | North → east |
| 2 | Cutlass Stockade | Defended palisade yards linked by staggered passages and a diagonal alternative route. | West → east |
| 3 | Smugglers' Piers | Wooden platforms, branching wharves and a loop around open harbor water. | West → north |
| 4 | Shipwreck Reach | Rounded wreck decks joined by narrow gangways, with an upper loop and side caches. | South → east |
| 5 | The Black Flagship | A ship-shaped deck split into compartments with alternating bulkhead openings and a stern fight. | West → east |

Every walkable tile belongs to the entrance's connected area. Enemy placement
uses that area, reserves clear arrivals, and never puts pirates inside sealed
wrecks. Railings leave openings where gangways enter wreck decks. Every chest
has a reachable approach. No water-level changes or spells are required.

Exits occupy the map's outer boundary, framed by blocked terrain and using the
existing coastal exit marker. They never float inside a clearing. Arrivals
stand one tile inside the matching boundary and face into the stage; walking
onto the marker or using stairs/exit from that arrival square travels onward
or back. Earlier stages allow backtracking, and entering the fort always starts
at stage 1. The stage-5 departure requires Captain Voss's defeat or the debug
passage override.

The minimap leaves sea, cliffs, palisades, railings and cargo blank. Explored
routes and exit/chest markers remain visible, producing a map of the actual
coastline and passages rather than a solid rectangle.

## Pirate enemies and combat

| Enemy | Role |
| --- | --- |
| Raider | A melee pirate who pursues the player through the coastal routes. |
| Crossbow Corsair | Advances to line up an orthogonal shot, then gives one turn of warning. |
| Buccaneer | A leather-coated swordsman who pursues the player and attacks with a curved cutlass. |
| Deck Brute | The toughest regular pirate, with high health, armor and melee damage. |
| Swashbuckler | A swordsman with two blades who closes the distance and attacks in melee. |
| Captain Voss | The flagship boss: 480 HP, 26 attack and 9 defense before player attacks. |

Numbered outlines show the exact threatened tiles and remaining enemy turns.
Targets stay fixed after being announced. Cargo and palisades block crossbow
bolts. Ordinary enemy freezing pauses warnings as well as movement. Raiders,
Buccaneers, Swashbucklers and Deck Brutes use melee combat.

Voss unleashes a magical **three-row blade wave** from his sword at range. Two ordinary moves can clear
its center row before impact. At close range he declares a **saber sweep**
around his position. Each attack has two warning turns followed by two recovery
turns with no extra captain attack. His status panel displays health, attack
countdown, freezing and the recovery opportunity. Blade waves pass through
cargo; their marked rows must be avoided.

Defeating Voss grants **200 gold, 2,200 score and 800 experience**, records him
in the boss journal, and opens the flagship departure. These victory rewards
do not repeat. Ordinary pirates need not be defeated to leave after victory.

## The Merchant's Lost Fortune

Inside Dawnmere's Merchant shop, speak to **Merchant Corvin** and accept his
quest. He gifts a treasure map, recorded in the quest journal. The map marks
his family's **silver-bound coffer on the northwest pier of Smugglers' Piers,
Pirate Coast stage 3**. The coffer is separate from the three ordinary plunder
chests. It stays sealed until the quest is accepted.

Stand beside the silver coffer and use **Interact** to recover it. If a pirate
is standing on it, defeat that guard first. The map and recovered coffer are
quest records, so they require no inventory slots. Acceptance preserves terrain,
enemy wounds, loot and chest claims from earlier visits. Captain Voss's defeat
is not required; the player can backtrack or use Return to Town.

Return to Corvin in the same shop for **100 gold and a Mariner's Cloak**. The
cloak grants **3 defense and 5% evasion**, fits the armor slot, and can be worn
by warriors, rogues and mages. If the pack is full, Corvin holds both rewards
until one slot is available. The bounty can be claimed once. Completion moves
the quest to the journal's completed tab.

Save version **186** preserves unaccepted offers, the gifted map, recovered
coffer and claimed bounty. Older saves begin with this quest unassigned and
Dawnmere saves gain the merchant doorway in place. Existing expedition progress
and equipment remain intact. See [quest offer](art-sources/pirate-coast/merchant-offer.png),
[quest map](art-sources/pirate-coast/merchant-journal.png), and
[the coffer in stage 3](art-sources/pirate-coast/merchant-coffer.png).

## Plunder and presentation

Stand beside a chest and use **Interact** to claim `25 + 5 × stage` gold.
Each chest pays once. The stage panel shows **PLUNDER n/3**. No inventory slot
is needed for coins. The region uses original pirate enemy sprites, a black-sailed
harbor-fort entrance with arrow slits, timber decking, hull rails and numbered
attack warnings.
The approach uses `travel-loop.mp3`; combat uses `dungeon.mp3`.

Beach headlands and inlet corners have rounded pixel contours with a clean
sand-to-water boundary. Water highlights shift and brighten on a gentle cycle.
This animation uses real time and continues while the player stands still.
The effect belongs to the Pirate Coast renderer; collision tiles and saved
layout seeds retain the connected routes described above. See the
[animated SDL capture](art-sources/pirate-coast/pirate-water.gif).

Art is generated through `tools/sprites/pirate.py` using the existing pixelkit
stonework, roof tiles, outlines and harbor textures. Run that script, or
`make sprites`, to regenerate the two runtime BMP assets. See the
[art comparison and actual SDL captures](art-sources/pirate-coast/README.md).

## Persistence and testing

Save version **184** added a separate Pirate Coast cache and victory flag. Each
stage stores its seed, exploration, chest claims, enemies and dropped loot.
Enemy wounds, deaths, frozen turns and announced attacks survive backtracking,
retreat, town portals and save/load. Return to Town still visits the last town
actually entered and retains the same expedition for the return portal.

Older saves gain empty Pirate Coast state. Saved Dawnmere interiors gain the
south gate; saved Dawnmere bypass roads gain the matching town entrance and
southern continuation in place. Player position, equipment, gold, quests,
existing regional victories and jail recovery progress are preserved. Existing
location, tile and enemy IDs are unchanged; new IDs are appended. The new boss
uses a dedicated flag rather than shifting the old integer boss mask.

Save version **185** replaces the original open layouts. Existing active and
cached Pirate Coast stages are regenerated from their saved seeds. Player,
living enemies, loot and Pirate Coast portal anchors are relocated to reachable
ground when needed. Enemy health, deaths, freeze, equipment upgrades, chest
claims, exploration, gold and victories are retained. Relocated attacks are
cancelled so a shifted warning cannot hit without being announced again. New
saves retain the new positions and pending attacks without repeating migration.
Loading also updates renamed pirate enemies in active and cached stages.
Obsolete ranged attacks on Buccaneers and Swashbucklers are cancelled while
their health, freeze and defeated state are preserved.

`tests/test_pirate.c` checks 500 generated stages for connected ground, boundary
exits and chest approaches, and 500 populated stages for reachable, unique enemy
spawns away from arrivals. It also checks distinct terrain, physical town
travel, chest claims, warnings, cover, captain defeat, backtracking, upgraded
loot, portals and save migrations. The SDL debug test reads minimap pixels to
check that every Pirate Coast barrier stays blank while floors and exits remain
distinct. It also compares consecutive rendered frames to check rounded beach
and inlet corners, and moving water while idle. Fixtures use heap storage; the older Queen encounter fixture also
uses heap storage to keep the expanded binary within the macOS static-data limit.

`tests/test_pirate_quest.c` exercises the complete shop/map/coffer/bounty loop,
including decline and re-offer, revisiting an explored stage, full inventory,
portal return, dropped loot, all-class equipment, repeated conversations,
save/load at each quest state, old-save doorway migration and invalid-state
rejection.
