# Act 5 navigation foundation

After the northern Stillbury journey, Castle Grounds serves as the junction
for the last two regional hubs before the castle interior.

| From | Route | Destination | Regional adventures |
| --- | --- | --- | --- |
| Castle Grounds, west gate | Crown Road West | Rosemoor | Ashen Hollow through the west gate; Frostfell through the north gate. |
| Castle Grounds, east gate | Crown Road East | Ridgeshire | Dragonspine through the north gate; Ironfang Mines through the east gate. |

Both hubs retain their Crown Road return to Castle Grounds. The player can
visit either hub first; this navigation change introduces no new quest, boss
or reputation requirement. Stillbury’s sole first approach to the castle remains
Blackwater Swamp, Royal Catacombs and Crown Road North. The western quarry and
eastern coast remain local adventures, separated from these hubs by cliffs.

This establishes the routes for the final regional act. The next change sets
will define each hub’s storyline, NPC interactions, quests and rewards, followed
by the castle player experience and the final encounter with Malakar. Those
story and finale decisions remain provisional; the atlas keeps its Act V
provisional legend. This change stops for a manual commit before that work.

## Implementation

Rosemoor’s refresh logic preserves its Ashen Hollow gate instead of sealing it.
Its retired southern swamp connection stays closed. The action fallback also
enters Ashen Hollow rather than the retired Giant’s Peaks route. Frostfell and
Dragonspine now receive a northbound arrival at their first stage’s south edge,
with an inward path to the existing terrain. Later stage layouts retain their
existing winding directions. Ironfang retains its established indoor transitions.

Town gates, both Crown Road ends and regional stage exits have readable labels
and discovered markers. Labels stay within the playable viewport at small screen
sizes. Loot retains its record and metadata while transition tiles stay usable.

The displayed Crown Road names follow their compass position relative to the
castle. Historical numeric location IDs and patrol-cache keys remain unchanged,
so renaming the road does not reset its survivors, wounds or defeated patrols.

Save version **250** repairs active and cached first-stage entrances in place,
including saves from older versions. Old entrance tiles become ordinary ground;
actors, loot and existing exploration remain where they were. Quest progress,
equipment, boss victories and crane/terrain states are retained. No new persistent
fields are introduced. Local saves were backed up before migration validation.

See [validation and native screenshots](act5-navigation/validation.md) and the
[map edit prompt](act5-navigation/world-map-prompt.md). The canonical atlas remains
[world-map.png](world-map.png), linked and embedded in README with checksum
`4ab1f192db91`. Only the two lateral Crown Road labels change on the atlas.
