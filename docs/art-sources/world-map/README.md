# World atlas acceptance checks

The [current atlas](../../images/world-map-ironfang.png) is player-facing
game documentation. Its routes, geography and landmarks are part of the feature.
A successful image generation or gameplay test run does not establish that an
atlas revision is correct. Inspect the full image and enlarged details before
replacing it. Record the final edit prompt and review in [prompt.md](prompt.md).

Preserve these features when changing any other part of the atlas:

| Feature | Required appearance and connection |
| --- | --- |
| Stonewake Quarry | Replaces Glassdeep south of Stillbury within its southwest footprint; stepped limestone, timber crane and mine entry. Keep sandy beaches below it, backed by rocky cliffs. |
| Sunken Coast | Curved, sheltered C-shaped cove with drowned columns and its western shoreline intact. |
| Docks | Outside Oakhaven beside Sunken Coast; a shore-connected wooden wharf with attached mooring branches. Trace solid deck from shore to every tip at enlarged scale; water between connected branches must not sever them. |
| Goblin Mountains | The winding Dawnmere–Ridgeshire road passes through closely flanking mountains on both sides. No grassy bypass valley. |
| Town elevations | Dawnmere sits in open coastal lowland with a descent from the northern foothills before town; peaks must not surround its walls. Ridgeshire is the elevated city surrounded by mountains. |
| Saltwind Marsh | A restrained tidal inlet south of Dawnmere. Creeks meet the sea at the same level; no raised lake, cliff-bottom waterfall or giant delta. |
| Dawnmere roads | West connects to Oakhaven through the dungeon road; north connects through the mountains; south reaches Saltwind. Each connection must visibly meet town. |
| Eastern regions | Stormwatch Headland lies east of lowland Dawnmere. Ironfang Mines is a compact adit in the mountain flank east of Ridgeshire, reached by its own eastern road. |
| Forests and northern route | Dark tangled Haunted Forest between Stillbury and Oakhaven; adjoining Elderwood north of Oakhaven has larger healthy emerald trees with golden highlights, followed by Catacombs and Castle. |
| Western volcanoes | Ashen Hollow west of Rosemoor beneath Frostfell. No garden destination or eastern Ridgeshire volcanic spur; Dragonspine continues into the northeastern mountain flank. |
| Rook's Labyrinth | A separate landmark beside the forest road, outside the forest. |
| Other destinations | All five towns and existing northern regions remain present once each, with Ruined Isle and Temple offshore. |

Check illustrated roads against `src/game/town_road.c` and
`src/game/approach.c`. Check landmark placement against the region documents.
Review the entire atlas after each edit, including regions the prompt asked to
preserve: localized prompts can still change unrelated terrain or erase roads.
The original `docs/images/world-map.png` remains a coastline reference.

Review at the native 1536×1024 scale, then enlarge the docks, mountain pass,
marsh and eastern headland, and Stonewake/Sunken Coast. Specifically follow
each road continuously between its endpoints and each dock deck back to shore.
Document visual checks separately from automated gameplay checks; neither
substitutes for the other.

Stormwatch Headland is a compact watchfort east of lowland Dawnmere, reached by a short eastern road; its label must remain separate from the southern Saltwind inlet.
