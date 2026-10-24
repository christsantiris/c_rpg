# Saltwind Marsh

Saltwind Marsh is a complete five-stage adventure **south of Dawnmere**.
The atlas shows a compact tidal inlet along the eastern rocky coast, leaving
cliffs and open sea prominent. Glassdeep Caverns is restored south of Stillbury.
The five-stage island layouts and skiff mechanics retain their full size.
Pirate Coast and its treasure quest remain retired.

Leave Dawnmere through the south gate, follow the exterior road and enter the
mangrove-and-skiff landmark. A new expedition always starts at stage 1. Both
retreat and the final departure lead back to this approach; walk north to
Dawnmere's exterior and enter its southern side to arrive beside the south gate.

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

Ordinary enemies do not block stage travel. Stage 5's departure stays sealed
until Sszareth is defeated; retreat through previous stages remains available.
The debug passage override opens the departure without recording a victory.

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
| Mudback Turtle | 3 | Slow, armored creature with a committed charge and an exposed recovery. |
| Lizardfolk Scaleguard | 4 | Armored warrior whose spear sweep gives two actions to escape. |
| Sszareth, Keeper of the Roots | 5 | Heavy spear sweep, committed charge and one finite half-health reinforcement wave. |

Humanoids match the existing player scale. Turtles have broader shells; the
chieftain has a taller crest. All six use the native pixelkit sprite atlas.
Amber ground outlines mark upcoming attacks. Move out of a thrust or charge
lane, step away from a sweep, and use roots as cover. Sszareth's charge leaves
his defense open during recovery; collision extends that opening. Below half
health he calls two spearhunters once. Summoned hunters grant no repeat loot.

Defeating Sszareth awards **200 gold, 2,400 score and 900 XP**, records his
boss-journal entry and opens the final departure. Gold and score pay once.
A later physical entry respawns ordinary enemies on visited stages while
preserving the boss victory, claimed supplies, exploration and dropped gear.
Backtracking and return portals preserve wounds, deaths, freeze and attack
warnings within the current expedition.

## Saves, migration and reuse

Save version **192** retains all five stage records introduced in version 191: skiff position and facing,
occupied state, exploration, supplies, loot and enemy combat state. Return to
Town goes to the last town actually entered; its return portal restores the
same stage and shore/skiff position. Bypassing a town does not change that
spell destination. Physical reentry closes an older marsh portal.

Version 190 pilot saves gain enemies and the onward jetty while retaining
existing discoveries, supplies, loot and boat occupancy. A boat or item covered
by changed terrain moves to the nearest valid square. Older town and approach
saves gain the relocated connections, with existing character, quest and boss
progress retained. Retired Pirate Coast migrations and equipment recovery
remain in place. Serialized location, tile and enemy IDs remain stable.

The skiff implementation stays separate from the stage layouts and shore
combat. Its use in Sunken Coast or another coastal region is a future change;
those areas retain their current movement rules.

See [town connections](town-roads.md), [coastal docks](harbor-and-ruined-isle.md)
and [native sprites and renderer captures](art-sources/saltwind/README.md).

Version 192 restores the original regional connections: Glassdeep south of
Stillbury, Saltwind south of Dawnmere. Version-191 saves keep their stage records,
boat, enemies, supplies, loot and boss progress. An active approach now returns
to the restored town; existing portals keep the town last visited when cast.
