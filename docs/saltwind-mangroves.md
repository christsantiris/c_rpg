# Saltwind Mangroves — skiff pilot and area design

Status: the first exploration pilot is playable **south of Dawnmere**. It
includes the safe approach and landmark, four islands, physical skiff boarding,
paddling and docking, three one-time supply caches (15 gold each), the boundary
retreat and save/portal round trips. **Combat, the boss and stages 2–5 remain
planned.** The five-stage design below guides later changes. Pirate Coast and Merchant Corvin's
treasure-map quest are retired; this design does not carry them forward.
Dawnmere's east side remains reserved for a future area. Glassdeep Caverns
stays south of Stillbury.

## Identity and appearance

A saltwater mangrove delta of winding channels, tangled roots, muddy islands,
abandoned fishing platforms and lizardfolk settlements. Use warm silt and sand,
weathered timber, deep green foliage and dark teal water with a gentle animated shimmer.
Rounded banks and root clusters should break up the shoreline. Landforms need
recognizable silhouettes on the minimap, with water visible around the islands.

The entrance landmark is a cluster of mangroves surrounding a small landing
and a moored skiff. A single-tile road leads south from Dawnmere to the landing;
entering the landmark begins stage 1. Retreat returns to that same approach.
Interior stairs and landing transitions sit at the edges of terrain and built
structures, with clear onward/return labels. They never float in open ground.

## Skiff exploration pilot

Board a skiff with the existing contextual action key while beside a landing.
Use normal movement controls to paddle; interact at another landing to disembark.
A visible boat and rider, a short contextual prompt and distinct minimap marks
make the state clear. Skiffs are physical exploration vehicles, not destination
selection menus or instant ferries.

Each stage connects several islands by navigable channels. Some landings lead
onward, while others reveal optional ruins, guarded caches or alternate routes.
The starting landing always provides a skiff. A player can revisit an island
and recover the boat they left there. Boarding, paddling and disembarking each
consume a normal turn; failed moves and unavailable interactions do not.

For the pilot, normal combat takes place ashore. Landings provide clear space
to disembark, with enemies outside the arrival buffer. Do not add boat damage,
naval combat, fuel, tides or moving platforms in this first version. Channel
navigation and choosing where to land provide the new interaction. Mandatory
encounters and the boss remain on land, so boating cannot skip progression.

Persist the boat's position, facing, occupied state, stage and landing anchors
alongside exploration and encounters. A return portal must restore the same
boat/shore state, without leaving the player standing in deep water or creating
a second skiff. Return to Town still uses the last town actually visited.

Keep movement and docking rules separate from Saltwind's terrain generator so
they can later support Sunken Coast and another coastal adventure. Those areas
retain their current behavior until a separate change adopts the pilot.

## Enemies, from weakest to strongest

The following names and combat roles are proposed; numerical balance comes
with implementation.

| Enemy | Role and counterplay |
| --- | --- |
| Lizardfolk Scout | Fast knife fighter who retreats toward nearby allies; pursue carefully around roots. |
| Reed Spearhunter | Holds a straight approach with a warned spear thrust; step aside and close during recovery. |
| Marsh Spitter | Ranged venom attack with a visible target warning; use roots as cover. |
| Giant Mudback Turtle | Slow armored bruiser; a warned lunge leaves its head exposed for a short recovery. |
| Lizardfolk Scaleguard | Toughest regular enemy; shield and spear defend narrow island paths, with a recovery opening after its heavy attack. |
| Sszareth, Keeper of the Roots | Stage 5 lizardfolk chieftain; proposed boss below. |

Humanoid art follows the player and existing dark-elf scale. Turtles are visibly
larger creatures with footprints and collision matching what the player sees.
Avoid oversized humanoids and decorative barriers that trap spawned enemies.

Sszareth fights in a broad ruined clearing reached by skiff. A warned sweeping
spear attack and a committed charge reward movement around root cover. A charge
into a root buttress briefly exposes the boss. Below half health, a finite pair
of spearhunters arrives through visible land approaches. The arena always has
room to dodge and a reachable retreat landing. Exact rewards and any new quest
remain to be designed; the retired pirate quest is not reassigned here.

## Five distinct stages

| Stage | Layout and exploration focus |
| --- | --- |
| 1 — Saltwind Landing | Learn boarding and docking across four small islands; optional side landings and a clear retreat. |
| 2 — Splitroot Delta | Branching channels reconnect in a loop; scout alternate shores and find a ruined fishing platform. |
| 3 — Mudback Pools | Broad pools around turtle nesting islands; optional guarded ruins off the main route. |
| 4 — Stiltwater Village | Explore separate settlement platforms, root paths and several shore approaches to the same village. |
| 5 — Heartroot Sanctuary | A navigable outer ring surrounds the boss island and optional clearings; land to confront Sszareth. |

Use a dedicated generator built around a connected graph of waterways and
landings. Grow irregular landmasses around those connections, add foot routes
and optional branches, then validate movement in both modes. This should not
be a room chain with water painted around it. Require multiple meaningful
landings and a reconnecting branch on the exploration stages.

Validate that every exit, enemy and reward can be reached using legal boat and
foot movement, including the ability to disembark and fight. Blocking props
must leave an actual route onto platforms. Every generated stage has readable,
reachable return and onward transitions and useful minimap geography.

## Implementation acceptance checks

- Dawnmere's south gate and exterior road reach the mangrove landmark; no
  Pirate Coast route, boss, journal entry, shop NPC or treasure quest remains.
- Every fresh expedition starts at stage 1; ordinary enemies respawn according
  to the existing replay rules. Completed bosses and earned rewards persist.
- Seeded generation checks reach all required objects with the combined
  boat/foot movement model; shore-only enemies cannot spawn on isolated props.
- Save/load works while ashore, aboard, between stages and through a return
  portal. Boarding never duplicates a boat; leaving it never strands the player.
- Debug passage unlocking covers required travel and docking locks while
  leaving normal quest and boss progression independent.
- Inspect terrain, landings, humanoids, turtles and boat/rider together in the
  actual renderer at native scale. New art follows the established pixelkit
  pipeline and water treatment.

## Current pilot controls and persistence

Leave Dawnmere through its south exit and follow the single-tile road into the
mangrove landmark. Movement keys paddle while aboard. Stand on a timber landing
beside the skiff and use the bound Interact key (A by default) to board. Use it
again beside another landing to go ashore. Deep water blocks walking; a skiff
cannot move onto land, and gear can only be dropped ashore. Search the supplies
on the three outer islands with Interact. Their gold pays once across visits.

The north boundary on the starting island returns to the safe approach. Walk
north to Dawnmere's exterior, then into its southern side to enter beside the
south exit. Physical entry always begins on the starting shore with one skiff
at its landing. Reentering physically closes an older Saltwind portal; returning
through the portal resumes its existing boat or shore position instead.

Save version 190 stores the boat coordinates, facing and occupied state,
exploration, searched supplies and dropped items. Portals use the last town
actually entered. Older town saves gain the replacement southern connection;
old Pirate Coast saves first retire that content safely, then gain Saltwind.
The new locations and tiles skip the retired serialized IDs. See
[town roads](town-roads.md) for equipment recovery from removed coastal caches.

The illustrated atlas moves Dawnmere north to leave room for the mangroves
above the islands and widens Goblin Mountains around its town-to-town road.
See [native art and SDL previews](art-sources/saltwind/README.md).
