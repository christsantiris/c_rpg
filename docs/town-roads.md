# Towns beside the regional roads

Oakhaven has a safe riverbank visit through its north gate as well as its three Act 1 adventure roads.
The north gate leads to the **south bank of Northern Rapids**, where a broken
ferry landing, snapped rope and damaged boat mark an impassable river. Walk
south along the short path to return through Oakhaven's north gate. Boss
victories never open this crossing. The actual river tiles remain blocked
across the entire northern boundary. The README's [final atlas](world-map.png)
shows this local dead end. Other towns and later connections below are legacy,
provisional context pending later V2 design.

All five towns have dense medieval settlement sprites and safe, signed roads leading to them.
Players arriving from a regional approach or Crown Road can enter town or walk
around it across open grass to another connection; there is no paved ring road. Bypassing town does not visit shops, accept
quests, heal the player or unlock regional boss roads.

Walk into a connected cardinal side of the sprite to enter town; there is no
exterior gate to find. Contact along that side, including away from the center
of the road, enters beside the corresponding interior exit gate. North enters
facing south, east facing west, south facing north, and west facing east.
Leaving through an interior gate puts the player just
outside the matching side of the sprite. From there, follow the road to a region or
walk around town to another road. Gates are exits from town. Regional road boundaries are open road
continuations, without a second gate. Streets and services inside all five towns retain their layouts.

| Town | North | East | South | West |
| --- | --- | --- | --- | --- |
| Oakhaven | Northern Rapids / broken ferry (dead end) | Haunted Forest | Oakhaven Dungeon | Goblin Mountains |
| Stillbury | Swamp / Rosemoor road | Haunted Forest / Oakhaven road | Stonewake Quarry | Sunscar Wastes |
| Rosemoor | Frostfell Wastes | Crown Road East / Castle | Swamp / Stillbury road | Ashen Hollow |
| Ridgeshire | Dragonspine | Ironfang Mines | Mountains / Dawnmere road | Crown Road West / Castle |
| Dawnmere | Mountains / Ridgeshire road | Stormwatch Headland | Saltwind Marsh | Dungeon / Oakhaven road |

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
reference.

The former northern through road from Oakhaven is disconnected in V2. Its
legacy Elderthorn and Grave Marshal gates remain part of later-region code;
see [boss gates](boss-gates.md). They do not create a crossing at the rapids.
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
free in town. Recovery stays suspended in adventures and the Royal Dungeon. An active pirate floor's loot is relocated to Dawnmere and is not recovered
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

Save version 194 replaces Glassdeep with Stonewake Quarry in the same southern
Stillbury location. Its approach and town exit retain their IDs and roads.
See [Stonewake](stonewake-quarry.md) for cranes, stages and migration details.

Dawnmere's resident shop is labelled **Spice Merchant**; its resident is
**Spice Merchant Corvin**. The northern castle approach now crosses
[Crown Road North](crown-road-north.md) between Royal Catacombs and Castle
Grounds.

Save version **221** rebuilds a town exterior saved with the former paved ring
road, keeping the player's position, exploration and any dropped items.
