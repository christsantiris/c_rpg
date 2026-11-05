# Saltwind Marsh

Saltwind Marsh is a five-stage adventure **north of Dawnmere**, on the river
side of town. Dawnmere and Oakhaven share the south bank; Stillbury sits on
the opposite bank, at the former Elderwood location. The final atlas in README
shows the marsh and its downstream skiff passage below Northern Rapids.
Pirate Coast remains retired; unrelated later regions are provisional.

The marsh is drawn at the Crown Roads' 1.5× zoom: the camera shows about
30 × 18 tiles around the player. Venom spit and Sszareth's root charge reach six
tiles, so they start on screen.

Dawnmere is optional. Walk around the occupied town from the forest road, or
leave liberated Dawnmere through its north gate, then follow the exterior road
north and enter the mangrove-and-skiff landmark.
A new expedition starts at stage 1. Retreat returns south along this approach
to Dawnmere's north exterior. Defeating Sszareth opens stage 5's northern launch
into a playable river reach. Paddle to the opposite landing, dock, then follow
the bank approach north into Stillbury to begin Act 4. The same skiffs and
cached stages support the full return journey.

## Exploration and stages

| Stage | Name | Terrain and encounters |
| --- | --- | --- |
| 1 | Saltwind Landing | Four rounded muddy islands introduce skiff travel; scouts and spearhunters guard optional supplies. |
| 2 | Splitroot Delta | Channels loop between separated shores; spitters guard fishing supplies off the onward route. |
| 3 | Mudback Pools | Broad nesting islands and deep pools introduce armored turtles. |
| 4 | Stiltwater Village | Weathered settlement decks and a timber connection between platforms offer water and foot routes. |
| 5 | Heartroot Sanctuary | Outer clearings and a large root-covered island hold Sszareth and his scaleguards. |

Every stage has four timber landings, a physical skiff, three guarded supply
caches and clearly labelled boundary jetties. Optional islands branch away from
the onward route. Animated dark teal channels, warm silt, reeds and exposed root
clusters distinguish the marsh from the other coastal areas. Water and roots
block foot travel; timber and mud are walkable. Exits terminate the terrain at
map edges instead of floating in open ground.

Ordinary enemies do not seal stage departures or retreat. Sszareth seals only
the final river launch; his defeat permanently opens it, independently of quest
acceptance. Quest rewards still require their own objectives. See [boss gates](boss-gates.md).

## The Missing Spice Skiff

Talk to **Spice Merchant Corvin in Dawnmere's Spice Merchant shop** to accept
**The Missing Spice Skiff** after liberation. The shipment tried to avoid royal
confiscation; returning it is optional and never gates the crossing. Paddle to stage 2's northern side island and search
the labelled stranded skiff ashore with Interact (A by default). Its manifest
points to **stage 3's western island**, beneath an arched mangrove root. Reach
that island through its **east-facing timber landing**.

Defeat the Giant Crab and Lizardfolk guarding the cargo island, then recover
Corvin's sealed spice casket. Both manifest and casket are tracked as quest
discoveries outside the pack, including if found before acceptance or in the
opposite order. Bring both to Corvin for **150 gold and 1,000 score**, paid once.
Sszareth's defeat is independent of this delivery.

The quest keeps the existing shore combat and skiff controls. The stranded
skiff is scenery with a readable manifest; each stage's usable skiff remains at
its regular landing. Quest objects have local labels and contextual prompts,
with minimap markers appearing after discovery. Ordinary supplies stay separate.

Save version **205** retains the quest's acceptance, discoveries, and reward
alongside existing stage and boat records. Town portals preserve the same
encounter; fresh physical expeditions respawn ordinary guards while recovered
cargo stays empty. Older saves gain the quest without regenerating their maps
or resetting boats, loot, exploration, wounds, and previous quest progress.

## Physical skiff controls

Walk onto a landing beside the skiff and use **Interact** (`A` by default) to
board. Movement keys paddle through deep water. Interact beside another timber
landing to disembark; the skiff stays on the adjacent water tile for the return
trip. The starting shore always supplies one skiff per stage. The contextual
prompt, visible rider and minimap marker show whether the player is aboard.

Combat takes place ashore. Arrival spaces stay clear of enemy spawns and
movement. While aboard, shore encounters pause and offensive spells and ranged
or melee attacks are unavailable. Healing and Return to Town remain available.
There is no boat damage, fuel or naval combat. Water cannot be entered on foot,
and gear can only be dropped ashore. Failed movement consumes no turn.

Search supplies with Interact after defeating nearby guards. Each cache pays
once: 15 gold on stage 1, then 20, 25, 30 and 35 gold on stages 2–5. Claimed
caches remain empty on later expeditions. Boarding, docking and successful
searches consume an action.

## Enemies and boss

| Enemy | First stage | Combat role |
| --- | ---: | --- |
| Lizardfolk Scout | 1 | Light melee hunter. |
| Reed Spearhunter | 1 | Warns before a two-square straight thrust. |
| Marsh Spitter | 2 | Warns before spitting at a fixed target; roots stop its sightline. |
| Giant Crab | 3 | Reuses the armored Sunken Coast crab; moves every other turn and attacks in melee. |
| Lizardfolk Scaleguard | 4 | Armored warrior whose spear sweep gives two actions to escape. |
| Sszareth, Keeper of the Roots | 5 | Heavy spear sweep, committed charge and one finite half-health reinforcement wave. |

Humanoids match the existing player scale. Turtles have broader shells; the
chieftain has a taller crest. All six use the native pixelkit sprite atlas.
Amber ground outlines mark upcoming attacks. Move out of a thrust or charge
lane, step away from a sweep, and use roots as cover. Sszareth's charge leaves
his defense open during recovery; collision extends that opening. Below half
health he calls two spearhunters once. Summoned hunters grant no repeat loot.

Defeating Sszareth awards **200 gold, 2,400 score and 900 XP**, records his
boss-journal entry and unlocks the final river departure. Gold and score pay once.
A later physical entry respawns ordinary enemies on visited stages while
preserving the boss victory, claimed supplies, exploration and dropped gear.
Backtracking and saves preserve wounds, deaths, freeze and attack warnings
within the current expedition.

## Saves, migration and reuse

Save version **236** adds the eastern story state and river boat while retaining
all five stage records introduced in version 191: skiff position and facing,
occupied state, exploration, supplies, loot and enemy combat state. Return to
Town is a one-way escape to the last safe town actually entered and leaves no
return portal. Bypassing a town does not change that destination. Physical
re-entry resets the skiff to the entry landing so the player cannot be stranded.

Version 190 pilot saves gain enemies and the onward jetty while retaining
existing discoveries, supplies, loot and boat occupancy. A boat or item covered
by changed terrain moves to the nearest valid square. Older town and approach
saves gain the relocated connections, with existing character, quest and boss
progress retained. Retired Pirate Coast migrations and equipment recovery
remain in place. Serialized location, tile and enemy IDs remain stable.

The skiff implementation stays separate from the stage layouts and shore
combat. Its use in Sunken Coast or another coastal region is a future change;
those areas retain their current movement rules.

See [town connections](town-roads.md) and [coastal docks](harbor-and-ruined-isle.md).

Historical note: version 192 restored the former regional connections: Glassdeep south of
Stillbury, Saltwind south of Dawnmere. Version-191 saves keep their stage records,
boat, enemies, supplies, loot and boss progress. An active approach now returns
to the restored town; existing portals keep the town last visited when cast.

Version 236 supersedes those town connections. It repairs the active and cached
first/final-stage terrain in place, retaining exploration, wounds, drops,
quest discoveries, claimed supplies and Sszareth's defeat. The river boat,
boarding state, discovered shores and completed passage are saved separately.
Occupied Dawnmere and wanted Stillbury never replace the safe-town anchor;
liberated Dawnmere does. The Crown's wider Stillbury story is deferred to Act 4.
