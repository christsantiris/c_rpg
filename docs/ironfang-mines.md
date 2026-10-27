# Ironfang Mines

Leave **Ridgeshire through its east gate**, follow the road, and walk into the
fortified mine sprite. Ironfang is an optional five-stage underground adventure.
It begins at stage 1 on every physical entry. Retreat through the southern
boundary stairs; advance through the northern stairs. Ordinary enemies never
lock intermediate travel. The Goblin Mine Boss guards the final departure.

| Stage | Working | Layout and discoveries |
| --- | --- | --- |
| 1 | Abandoned Workings | Broad excavated chambers, looping galleries, a lamp-lit cart landing, and abandoned side pockets |
| 2 | Goblin Warrens | Connected sleeping quarters and defended intersections, with several approaches around the occupied chambers |
| 3 | Flooded Galleries | Natural chambers and underground pools, dry perimeter routes, and rail crossings above the water |
| 4 | Deep Ironworks | Linked forge rooms and rectilinear workshops around the haulage network |
| 5 | The Rootshaft | Natural crystal-bearing rock surrounding the Mine Boss's excavation gallery |

## Exploration and controls

Use **Interact** (A by default, or the configured Move Left/Interact key) on or
cardinally beside the highlighted object. Arrow keys always remain available
for movement.

- **Lamps:** repair an unlit lamp to illuminate nearby floor and reveal the
  surrounding workings on the minimap. The player always carries a local light;
  enemy sprites remain visible in the viewport even outside fixed lamp light.
  There is no fuel requirement.
- **Minecart:** Interact beside the parked cart to board. Movement follows the
  physical rails and covers up to two clear straight tiles per action. The cart
  stops before enemies, gaps, endpoints, and junctions. Interact again to step
  onto adjacent solid ground. It cannot unload the player into water.
- **Junction:** the lever beside the central T junction selects the east or north
  branch. Disembark to operate it. Both approaches obey the selected points;
  arrows cannot steer through a disconnected turnout.
- **Broken track:** repair the missing eastern span before driving across it.
  Foot paths provide a permanent alternative through the galleries.
- **Salvage spur:** ride beyond the eastern landing over the flooded trestle to
  reach a small isolated cargo alcove. Disembark, search the cache, and ride back.
  This water crossing cannot be walked.
- **Excavation:** break the cracked rock beside an abandoned western working to
  reveal an optional treasure pocket. No tool, class, consumable, or quest is
  required. Side caches award 20–40 gold according to depth, once each.

Timing is turn based. Minecart travel, repairs, and interactions use ordinary
actions. Mine warnings advance with enemy phases, including after ordinary
enemies have been cleared; standing idle does not advance them.

## Enemies and boss

The mine reuses the existing **Goblin Scout, Goblin Archer, Goblin Bomber,
Tunnel Spider, Cave Troll, Hobgoblin Guard, and Goblin Shaman** types and artwork.
Later stages introduce the more heavily defended groups. Enemies spawn only in
workings reachable on foot from the entrance; none occupy the isolated salvage
alcove or impassable water. Ordinary sprites keep their established native scale.

The **Goblin Mine Boss** occupies the fifth stage's northern gallery. Its new
sprite uses the established 24-pixel Hobgoblin silhouette, mining helmet, lamp,
and pick. Its encounter includes:

- A rockfall cross locked to the announced position, with two actions to escape.
- A warned cart charge along the fixed rail through the boss gallery.
- A one-action adjacent pick sweep, followed by a recovery opening.
- One finite pair of Hobgoblin Guards called at half health. They grant no gold
  or experience. Further turns do not create additional groups.

The nearby ore winch releases one of **three loaded carts per expedition**. The marked gallery
leaves two complete actions to get clear. A cart deals one-fifth of the boss's
maximum HP, cancels its pending attack, and exposes a recovery opening. It also
hits ordinary enemies and the player on the marked track. Emptying the hopper
never makes victory impossible: normal weapons and spells remain effective.
Defeating the boss grants **180 gold and 1,800 score**, records its journal entry,
and permanently opens the final departure. Both departures return to the mine
approach east of Ridgeshire.

## Expeditions, portals, and saves

Backtracking retains wounded enemies, loot, lamp repairs, rail repairs, cart
position, switch direction, and searched caches. A fresh physical expedition
restores ordinary enemies on previously visited stages while retaining explored
terrain and completed infrastructure. Each cart returns to its western landing,
so abandoning a portal cannot strand it across water. A living boss regains its
three ore loads for the new attempt. Boss victory and claimed side caches stay
recorded. Return to Town uses the last town actually visited; its portal resumes
the exact mine stage, position, passenger state, and encounter.

Save version **200** serializes all five compact stage caches, actors, floor
loot, explored terrain, cart and junction state, finite ore loads, pending ore
warning, and boss victory. Version 199 and older saves gain the eastern
Ridgeshire connection without regenerating existing adventures or resetting
character progress. Debug shortcuts open the final departure and repair the
optional track and rock obstruction without granting the boss victory.

## Break the War Machine

Talk to Marshal Veyra in Ridgeshire's Town Hall and press **2** to select this
quest, then Y to accept. **1** returns to her Watchfires conversation. Both
quests can be active together and retain separate progress and rewards.

Read the shipment orders on the desk in stage 2's northwest side office. In
stage 4, repair the eastern rail and set the junction east. Ride the minecart
over the flooded trestle to the isolated supply platform, then disembark and
operate the loading winch beside the ordinary salvage cache. Suspended weapons
crates fall into the flooded shaft, leaving the gantry empty. The cart's return
route remains intact; searching the ordinary cache earlier does not affect the
quest. The falling crates and splash are cosmetic and never advance enemies
while the player stands still.

Defeat the Goblin Mine Boss on stage 5 and recover his command seal from the
coffer in his gallery. An existing victory counts. Return to Veyra and select
the Ironfang topic for **160 gold and 1,000 score**, once, separately from the
boss reward. Discoveries need no pack slots and retain credit before acceptance.

Save version **207** adds the quest without regenerating old mines or changing
cart position, track repairs, lamps, switches, explored terrain, wounded actors,
loot, portals, previous victories, or other quests. Sabotage persists through
backtracking, portals, and fresh expeditions. Normal enemies continue to respawn
on physical re-entry. Crown Road North retains its existing encounters.
