# World atlas acceptance checks

The [final atlas](../../images/world-map-final.png), embedded and linked
under **World Map** in the [main README](../../../README.md#world-map), is player-facing
game documentation. Its routes, geography and landmarks are part of the feature.
A successful image generation or gameplay test run does not establish that an
atlas revision is correct. Inspect the full image and enlarged details before
replacing it. Every geography change must update the image actually displayed
in README; a regional preview alone is insufficient. The current
[edit prompts and review](oakhaven-v2.prompt.md) supersede conflicting V1
instructions in the historical [prompt log](prompt.md).

`docs/images/world-map-final.png` is the permanent published map. Replace this
file with each finished revision, and keep README's image and click-through
link pointed at it. Revision-specific images and previews are working or
historical references; they are not README's source.

The current approved V2 scope is **Oakhaven, Haunted Forest, Goblin Mountains,
and Oakhaven's dungeon**. Most of the rest of the world will change. Retained
landmarks outside that scope are provisional context, not approved future-act
geography. Their notes below describe what this edit preserves; update them
when later geography is designed rather than treating the V1 layout as fixed.

Check the approved opening and preserve unrelated landmarks during this edit:

| Feature | Required appearance and connection |
| --- | --- |
| Stonewake Quarry | Replaces Glassdeep south of Stillbury within its southwest footprint; stepped limestone, timber crane and mine entry. Keep sandy beaches below it, backed by rocky cliffs. |
| Sunken Coast | Curved, sheltered C-shaped cove with drowned columns and its western shoreline intact. |
| Docks | Beside Sunken Coast; a shore-connected wooden wharf with attached mooring branches. Trace solid deck from shore to every tip at enlarged scale; water between connected branches must not sever them. V2 land access is undecided: no road from Oakhaven, its dungeon or Rook's Labyrinth. |
| Opening route | Stillbury → Goblin Mountains → Oakhaven → Haunted Forest → Dawnmere, with continuous roads meeting the towns. |
| Goblin Mountains | Red sandstone hoodoos, buttes and goblin fortifications between Stillbury and Oakhaven. The winding road passes through closely flanking canyon walls; no grassy bypass. No duplicate mountain destination between Dawnmere and Ridgeshire. |
| Oakhaven Dungeon | A single local destination directly south of Oakhaven, with a dead-end road returning to town. No onward road to Dawnmere or the shore. |
| Northern Rapids | River north of Oakhaven flows east to the sea through an estuary northeast of Dawnmere. No Oakhaven north road, bridge or ford. River headwaters remain east of the existing Stillbury–Blackwater road, preserving that route without a new crossing. |
| Town elevations | Dawnmere sits in open coastal lowland with a descent from the northern foothills before town; peaks must not surround its walls. Ridgeshire is the elevated city surrounded by mountains. |
| Saltwind Marsh | A restrained tidal inlet south of Dawnmere. Creeks meet the sea at the same level; no raised lake, cliff-bottom waterfall or giant delta. |
| Dawnmere roads | West connects to Oakhaven through Haunted Forest; east reaches Stormwatch; south reaches Saltwind. Each connection visibly meets town. No northern road to Ridgeshire. |
| Eastern regions | Stormwatch Headland lies east of lowland Dawnmere. Ironfang Mines is a compact adit in the mountain flank east of Ridgeshire, reached by its own eastern road. |
| Forests and northern route | Dense green mature Haunted Forest east of Oakhaven toward Dawnmere, with brown trunks and a mossy understory. Elderwood, Catacombs and Castle remain north of the river with their internal road; the southern Elderwood road ends at the north bank. |
| Northern volcano | Ashen Hollow north of Castle of No Return, with a clear terrain gap and no overlap with the complete dark castle silhouette. Dragonspine continues into the northeastern mountain flank. |
| Rook's Labyrinth | A separate landmark east of Oakhaven, outside Haunted Forest, on a short dead-end spur from its approach. No onward coastal road. |
| Deferred connections | No replacement road between Dawnmere and Ridgeshire, river crossing, provisional junction or Oakhaven–Coast access. Estuary remains an undecided Act 3 location; do not relocate Sunken Coast or Saltwind Marsh there. |
| Other destinations | All five towns and existing northern regions remain present once each, with Ruined Isle and Temple offshore. |

Check illustrated roads against `src/game/town_road.c` and
`src/game/approach.c`. Check landmark placement against the region documents.
Review the entire atlas after each edit, including regions the prompt asked to
preserve: localized prompts can still change unrelated terrain or erase roads.
The original `docs/images/world-map.png` remains a coastline reference.

Review at the native 1536×1024 scale, then enlarge the docks, mountain pass,
marsh and eastern headland, Stonewake/Sunken Coast, western swamp road and
both river banks. Specifically follow each road continuously between its
endpoints and each dock deck back to shore. Check that README's embedded
image and its click-through link resolve to the same current atlas file.
Replace the existing displayed asset when updating geography, and update the
README image URL's version to the first 12 characters of its SHA-256 checksum
so previews request the new image rather than reusing an earlier one.
Document visual checks separately from automated gameplay checks; neither
substitutes for the other.

Stormwatch Headland is a compact watchfort east of lowland Dawnmere, reached by a short eastern road; its label must remain separate from the southern Saltwind inlet.
