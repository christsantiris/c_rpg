# Towns beside the regional roads

All five towns have dense medieval settlement sprites and safe, signed roads around them.
Players arriving from a regional approach or Crown Road can enter town or walk
around it to another connection. Bypassing town does not visit shops, accept
quests, heal the player or unlock regional boss roads.

Walk into a connected cardinal side of the sprite to enter town; there is no
exterior gate to find. Contact along that side, including away from the center
of the road, enters beside the corresponding interior exit gate. North enters
facing south, east facing west, south facing north, and west facing east.
Leaving through an interior gate puts the player just
outside the matching side of the sprite. From there, follow the road to a region or
around town. Gates are exits from town. Regional road boundaries are open road
continuations, without a second gate. Streets and services inside all five towns retain their layouts.

| Town | North | East | South | West |
| --- | --- | --- | --- | --- |
| Oakhaven | Elderwood / Catacombs / Castle road | Dungeon / Dawnmere road | Sunken Coast | Haunted Forest / Stillbury road |
| Stillbury | Swamp / Rosemoor road | Haunted Forest / Oakhaven road | Glassdeep Caverns | Sunscar Wastes |
| Rosemoor | Frostfell Wastes | Crown Road East / Castle | Swamp / Stillbury road | Moonveil Gardens |
| Ridgeshire | Dragonspine | Ashen Hollow | Mountains / Dawnmere road | Crown Road West / Castle |
| Dawnmere | Mountains / Ridgeshire road | Closed | Saltwind Marsh | Dungeon / Oakhaven road |

The Lich King, Necromancer, Swamp Demon and Goblin King still control their
regional through roads. Town bypasses connect their existing approaches and
never cross a locked regional road. Enemy adventures retain their depth,
encounters, quests and entrance behavior. The castle retains its own grounds.

New characters still start in Oakhaven. Return to Town uses the last town
actually entered. Passing around a town leaves that destination unchanged.
Return portals preserve their expedition anchors and physical entrance towns. Narrative
arrivals, including Tomas's escape-tunnel return, retain their existing events.
Physical travel along the regional roads uses the new town exteriors.

## Saves and artwork

Save version 178 adds the appended `LOCATION_TOWN_ROAD`, cardinal sprite entrance
and road-exit tiles, and the serialized `road_town` identity. Exterior saves
retain player position, facing, exploration and dropped loot; combat caches,
quests, inventory, gold, boss victories and portals remain independent.

Older saves load in their existing town or adventure without moving the player
or resetting progress. Their next physical town arrival/departure uses the
exterior. Town names and tile/location IDs retain their existing serialized
values. Missing exterior identity in new saves or an unrelated location used
as that identity is rejected.

Save version 179 narrows Dawnmere's interior roads to one tile and gives its
plaza a regular rectangular outline. Older Dawnmere saves update their paving
in place, preserving exploration, dropped loot, portals and player progress.

Save version 180 records `debug_passages_unlocked` for the debug shortcut
launcher. It bypasses road, dock, sailing, quest, door and puzzle passage locks
without assigning quests or recording boss victories. Normal older saves
default to normal locks; loading them through the launcher enables the override.

Artwork uses the established pixelkit textures, timber framing, tiled roofs,
stonework and doorway details at native resolution. A rounded curtain wall,
projecting towers and clustered houses follow the user's medieval hill-town
reference. See the [five-town sprite comparison and renderer previews](art-sources/town-exteriors/README.md).

The northern Oakhaven road opens in two steps: Elderthorn, then the Grave Marshal.
See [Elderwood](elderwood-forest.md) for travel and save migration details.

Save version 189 retires Pirate Coast and Corvin's treasure quest. Saves from
versions 184–188 on the removed approach, in the adventure or in Corvin's room
return safely to Dawnmere. Saved Dawnmere interiors and exteriors replace the
southern pirate connection with Saltwind. That version also removed the merchant
doorway; version 193 restores the shop independently of the retired quest. Exploration, facing and
reachable dropped items remain in place where possible; items on removed
terrain move to safe ground. Pirate return portals close, while unrelated
portals and regional progress remain intact. Earned gold and equipment remain.

Items left in retired coastal caches return to the pack. If it is full, the
remaining items stay serialized in a recovery queue and return as slots become
free in town. Recovery stays suspended in adventures and jail. An active pirate floor's loot is relocated to Dawnmere and is not recovered
again from its duplicate cache. The retired quest and encounter state are not
written into new saves. Serialized location IDs 44–45, approach index 17, the
six appended pirate enemy IDs and sixteen appended tile IDs remain reserved
for migration; new content must not reuse them.

Save version 193 restores Dawnmere's merchant through the same building system
as the other four towns. The original building art and footprint stay in place.
A single cobblestone tile joins the south-facing door to the west road; the
rectangular plaza and single-tile streets retain their shape. Corvin waits in
the interior and speaks when the player presses Talk nearby. He currently gives
Saltwind directions; the future marsh quest has no objectives or rewards yet.
The shop uses Dawnmere's music and returns the player to its doorstep on exit.

The interior appends location ID 48; retired IDs 44–45 remain reserved. Older
Dawnmere saves regain the doorway and approach without replacing the town map.
Dropped items keep their correct ground beneath them, including items on the
new path, and exploration, quests and return portals survive. Saves inside the
shop retain the interior, player position and dialogue. Regression tests walk
through all five resident buildings and cover version 192's broken merchant.
See the [renderer captures](art-sources/saltwind/README.md) and
[atlas acceptance checks](art-sources/world-map/README.md).

Save version 192 restores Glassdeep Caverns south of Stillbury and
[Saltwind Marsh](saltwind-marsh.md) south of Dawnmere. The marsh retains all five
stages, enemies, boss and physical skiff exploration introduced in version 191.
Existing region records, skiffs, discoveries, loot, quests, boss victories and
return portals survive the relocation. Physical retreat follows the restored
road; return portals still use the town last visited when they were cast.

Rowan and the docks remain on the Sunken Coast approach outside Oakhaven.
The Drowned Queen still unlocks dock access; a map or previous voyage still
unlocks sailing. The atlas uses a compact eastern marsh inlet and a small
branching wharf, preserving the rocky coastline and broad open sea.
