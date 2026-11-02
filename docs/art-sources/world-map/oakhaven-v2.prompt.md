# Oakhaven V2: main world atlas edit

Date: 2026-11-01.
Tool: built-in `image_gen.imagegen` using the imagegen skill, edit mode.
Edit target: `docs/images/world-map-giants-peaks.png`.
Geography and appearance reference: `docs/design/previews/oakhaven-approved-green-forest-v2.png`.
Output: `docs/images/world-map-oakhaven-v2.png`, also copied over the existing
README atlas asset, `docs/images/world-map-giants-peaks.png`.
Published final asset: `docs/images/world-map-final.png`, the permanent source
for README's embedded map and click-through link.

Current V2 design scope: Oakhaven, Haunted Forest, Goblin Mountains and
Oakhaven's dungeon. Other depicted regions are provisional context and are
expected to change in later steps. The prompts preserve their existing
illustration for this edit; they do not approve a future-act layout.

## Initial full-world edit prompt

Use case: precise-object-edit.
Asset type: the MAIN FULL-WORLD MAP embedded in the README for Castle of No Return.
Input images: Image 1 is the EDIT TARGET: the complete existing world atlas, whose full extent, northern regions, coastline, and overall cartographic style must be preserved. Image 2 is a GEOGRAPHY AND FOREST-APPEARANCE REFERENCE ONLY for the approved Oakhaven opening region. Integrate that approved opening into the FULL WORLD of image 1; do not return a cropped regional preview or paste an inset.

Primary request: revise the full atlas to show ALL approved Oakhaven V2 geography and retired travel connections. Keep the parchment border, title "THE CASTLE OF NO RETURN", subtitle "WORLD MAP", compass, 3:2 landscape framing, detailed painted fantasy cartography, serif labels and orange-and-white road convention of image 1.

Approved southern geography, strictly west to east:
STILLBURY — GOBLIN MOUNTAINS — OAKHAVEN — HAUNTED FOREST — DAWNMERE.
Keep Stillbury in the southwest and Oakhaven near the lower middle. Move Goblin Mountains entirely to the terrain between Stillbury and Oakhaven, replacing the old western Haunted Forest with red/orange sandstone canyon mountains: tall hoodoos, stepped buttes, goblin cave/fort, a narrow winding road through the rock formations. Both sides of this road must be flanked by canyon walls; no grassy bypass.
Move Haunted Forest entirely EAST of Oakhaven, along the road to Dawnmere. It must be lush, dense mature olive/moss/emerald broadleaf woodland with brown trunks and shaded green understory and occasional stone ruins, matching image 2, not bare black trees or gray dead forest. Move Dawnmere somewhat southeast if necessary to give the forest room: Dawnmere is low coastal land, not on a high cliff or enclosed by peaks. A continuous visible road connects Stillbury through the western mountains to Oakhaven and from Oakhaven through the eastern forest to Dawnmere. Branch Rook's Labyrinth from the Oakhaven-to-Haunted-Forest approach, in a small clearing outside the forest, EAST of Oakhaven; remove its old western location.

Move the single "Oakhaven Dungeon" landmark DIRECTLY SOUTH of Oakhaven. A short road leaves Oakhaven's south gate and ENDS at the dungeon stair/crypt entrance. It must not continue to Dawnmere or the shore. Remove its former eastern landmark/label and through-road. Oakhaven should have exactly three departing roads: west mountains, east forest, south dungeon.

Add a broad river immediately NORTH of the Stillbury–mountains–Oakhaven–forest–Dawnmere opening belt, flowing naturally downhill from western/northwestern highlands EAST to the sea northeast of Dawnmere. It must visibly separate Oakhaven from Elderwood and the castle-side northern continent. Use whitewater, boulders and rapids north of Oakhaven, exact label "Northern Rapids". Farther east it widens at sea level into a modest tidal estuary labeled "Estuary". The river must reach the open sea north of Dawnmere, not drop into the southern Sunken Coast cove. No bridge, ford, ferry or road across this river anywhere: later crossing locations are NOT decided.
Retain Elderwood Forest, Royal Catacombs and Castle of No Return north of the river with their existing internal south-to-north road, but terminate the Elderwood road at the NORTH bank of the rapids with a clear gap of water before Oakhaven. REMOVE Oakhaven's old north road.
REMOVE the old Dawnmere-to-Ridgeshire road and Goblin Mountains label from the northeastern mountains. The northern ridges may remain unlabelled natural terrain; do not create a new adventure or substitute road. Ridgeshire's south road must not cross the new river or reach Dawnmere.

Preserve the rest of the full world and all other named destinations once each: Frostfell Wastes; Giant's Peaks with monumental giant stone ruins west of Rosemoor; Rosemoor; Blackwater Swamp between Rosemoor and Stillbury with its road; Sunscar Wastes west of Stillbury; Stonewake Quarry south of Stillbury with its road, limestone terraces, timber crane, sandy beach and cliffs; Ashen Hollow compact volcano NORTH of the castle, with a clear gap and the complete imposing dark castle silhouette; Dragonspine in the northeastern snowy ranges; Crown Road East from Rosemoor toward the castle and Crown Road West between the castle and Ridgeshire; Ridgeshire; Ironfang Mines east of Ridgeshire with its own road; Escape Tunnel beside the castle; Elderwood Forest and Royal Catacombs on the northern bank.
Retain Stormwatch Headland EAST of lowland Dawnmere with its short road. Retain Saltwind Marsh SOUTH of Dawnmere, a restrained sea-level tidal inlet with the existing southern town road. Keep the Sunken Coast sheltered C-shaped cove and drowned columns in the far south, the shore-connected branching wooden Docks beside that cove, and offshore Ruined Isle with Ruined Temple and their existing dotted sea route. Do not draw any access road from Oakhaven or its dungeon to Sunken Coast or Docks: V2 Coast access is deferred. Do not relocate Sunken Coast or Saltwind Marsh into the new estuary: that Act 3 choice remains undecided.

Ensure every required label is spelled correctly, readable and placed on the correct landmark, without overlaps, duplicate labels or duplicate towns. The original atlas has obsolete geography: the instructions above explicitly supersede those southern placements and roads. Show the WHOLE WORLD, not only the opening region. Keep unrelated destinations, coastline character and the original art style as close to image 1 as the southern re-layout allows.

## Opening-region refinement prompt

Input: the first full-world edit, generated as `exec-18cfdc2e-c451-42d2-b8d9-fa47a1fa69e8.png`.

Use case: precise-object-edit. Edit the attached FULL WORLD ATLAS. The Oakhaven geography is now correct and must stay: Stillbury — red sandstone Goblin Mountains — Oakhaven — lush green Haunted Forest — Dawnmere; Oakhaven Dungeon directly south; Northern Rapids north; Estuary east. Preserve the entire 3:2 atlas, style, typography, all existing labels, all northern regions, Castle, Giant's Peaks, Stonewake Quarry, coast, complete docks, island, forests and mountains.
Correct ONLY these geographic and road artifacts:
1. Restore a CONTINUOUS ORANGE ROAD from Rosemoor SOUTH through Blackwater Swamp to Stillbury. The new river must NOT cut this existing western road. Move the river's western headwaters EAST to the small elevated foothills immediately northwest of Elderwood Forest and north of Goblin Mountains, EAST of the Blackwater-to-Stillbury road. Erase the obsolete water between Sunscar and Blackwater at far west; replace it with continuous grassy terrain and road. Route the swamp-to-Stillbury road down the WEST side of the river source and the WEST flank of Goblin Mountains so it never crosses water. The river begins at rocky springs northeast of that road, flows east through the Northern Rapids north of Oakhaven and onward to Estuary. Keep the whole river east of the western road. There is no bridge or ford.
2. Remove the dangling orange road SOUTH of Ridgeshire entirely: Ridgeshire has its west Crown Road and east Ironfang road, but currently no southern connection. Replace the former winding southern road with uninterrupted mountain/forest terrain. Keep the eastern Ironfang and western castle roads intact.
3. Remove the EXTRA UNLABELLED watchfort on the northern shore of Estuary in the upper-right at approximately x=1430,y=460. Replace it with ordinary rocky shore. Keep the ONE correctly LABELLED Stormwatch Headland to the RIGHT of Dawnmere, with its eastward road, at approximately x=1460,y=645. Exactly one headland watchfort.
4. Dawnmere must sit on LOW GENTLY SLOPING COASTAL GRASSLAND directly beside Saltwind Marsh, not a high cliff. Lower the exaggerated sheer white cliffs under Dawnmere to a low grassy bank at sea level, leave the town walls and roofs intact. Make its existing southern road connect visibly from the town's south gate into Saltwind Marsh. Keep the marsh in the lowland, and its creeks connected to the sea without waterfalls. Keep the east road to Stormwatch and west road through Haunted Forest.
5. Rook's Labyrinth is a SHORT DEAD-END SIDE SPUR branching from the Oakhaven-to-Haunted-Forest road. Keep its landmark and label east of Oakhaven, OUTSIDE the forest. Remove any orange road below/south of the Labyrinth or continuing toward Docks/Sunken Coast. Its ONLY road connection goes northwest to the forest approach. The Oakhaven Dungeon is likewise a dead end south of Oakhaven; neither landmark leads to the sea.
Preserve all other labels and features exactly, including no Oakhaven north road, no crossing of Northern Rapids, and no replacement road between Dawnmere and Ridgeshire. Output the complete corrected world atlas, not a regional crop.

## Final northern-road refinement prompt

Input: the opening-region refinement, generated as `exec-6fea2aa7-0f37-4bae-86ae-e39b8ca1bcb8.png`.

Use case: precise-object-edit. Make one final, narrowly localized correction to this FULL WORLD atlas. Preserve ALL southern V2 geography, river, headwaters, towns, roads, forests, mountains, labels, shorelines and every other part unchanged. Keep the exact full-world framing, 1536x1024 size, parchment border and painting style.
Change ONLY the central-northern Crown roads and the Ironfang Mines entrance:
- Restore a thin, CONTINUOUS ORANGE-WHITE ROAD from ROSEMOOR'S EAST GATE to the WEST GATE of CASTLE OF NO RETURN, through the grassy strip under the existing "Crown Road East" label. There is currently a gap between Rosemoor and the short road at the castle's west flank. Join those endpoints unmistakably through the foothills, without going north to the volcano. Preserve the separate existing Ashen Hollow road branching around the castle's western perimeter and going north.
- Restore a thin, CONTINUOUS ORANGE-WHITE ROAD from the CASTLE'S EAST GATE to RIDGESHIRE'S WEST GATE, through the foothill strip below the existing "Crown Road West" label. Keep this road visually separate from the WHITE DOTTED Escape Tunnel path farther south; preserve that dotted path and its label.
- At the endpoint of the existing orange road EAST of Ridgeshire, immediately northwest of the "Ironfang Mines" label, restore the recognizable small TIMBER-FRAMED DARK MINE ADIT set into the mountain face, with a few mining crates. The orange road must terminate at the mine entrance, not blank rock. Do not change the mine's region or label.
Keep all letters of the existing labels unchanged. Do not move any towns or any other landmark; no new destinations, no other connections. In particular keep the river uncrossed, Ridgeshire without a south road, all three Oakhaven roads, west Goblin Mountains, east green Haunted Forest, the short dead-end Dungeon and Labyrinth paths, and the Stillbury–Blackwater road exactly as they are. Output the entire complete atlas.

## Selected output and visual review

Selected built-in output: `exec-2b1113c1-86c7-429f-80b6-e8d15b477767.png`,
copied into `docs/images/world-map-oakhaven-v2.png` (1536×1024, RGB).
The same pixels replace `world-map-giants-peaks.png`, the existing tracked atlas
file shown by the original README preview. The previous version is preserved
in Git history. README embeds and links this replaced asset with an image
checksum in its URL to request a fresh image.
The selected image is also published at `docs/images/world-map-final.png`.
README now always uses that permanent filename; future finished revisions
replace its contents and update only the URL's checksum value.

- Reviewed the full atlas and enlarged opening, coast and northern landmarks.
- Confirmed Goblin Mountains west of Oakhaven, mature green Haunted Forest
  east, and the single local dungeon south with a dead-end return road.
- Confirmed northern rapids separate Oakhaven from Elderwood, flowing east
  into the estuary without a crossing. The western swamp road skirts the
  river headwaters, preserving the existing Stillbury route.
- Removed the old western Haunted Forest, eastern mountain destination,
  eastern dungeon/through-road, Oakhaven north road and Dawnmere north road.
- Rook's Labyrinth sits outside the eastern forest on a short side spur;
  no road continues from it or the dungeon to Sunken Coast.
- README embeds and links the selected full-world file. Its caption explicitly
  marks the rest of the world as provisional; later geography is not approved
  by this image. No additional gameplay changes belong to this correction.

