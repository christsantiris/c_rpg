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
| Dawnmere | Mountains / Ridgeshire road | Closed | Closed | Dungeon / Oakhaven road |

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
