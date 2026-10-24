# Atlas regression repair (save version 193)

Final asset: `../../images/world-map-coastal-regions.png` (1536×1024).
Generated with the built-in image tool. The version 192 atlas remains the edit
base; the original `world-map.png` remains the reference for the beaches, cliffs
and sheltered cove. Earlier dock-only candidates were rejected because a water
gap remained or the upper mooring arm disappeared.

The selected revision was inspected at full size and enlarged. The main wharf
and both mooring branches share continuous timber back to shore. The northern
road winds between closely flanking peaks. Saltwind's tidal creeks meet the sea
without a waterfall or elevated terrace. The headland east of Dawnmere remains
uncrowded for the planned area; no new destination or game connection is added.

A candidate lost the dungeon-to-Dawnmere connection; the final repair restores
that continuous west road while retaining the south road to Saltwind and north
road through the mountains. Compared the final southwest coast against the
original atlas side by side: Glassdeep remains below Stillbury, with pale beaches
below it and cliffs behind, and Sunken Coast retains its curved sheltered cove.
All five town names, the separate Labyrinth, both forests, northern castle route,
other regions and offshore island remain present.

These are visual acceptance checks, separate from gameplay tests. Follow the
[review contract](README.md) for subsequent edits; inspect preserved features
as well as the requested change. Merchant doorway/NPC movement, interior
save/load and version 192 map repair are tested in `tests/test_town_life.c`.

## Mountain pass, sea-level marsh and dock continuity prompt

```text
Edit IMAGE 1, the full 1536x1024 fantasy atlas. IMAGE 2 is a magnified detail solely to show the FAILED DOCK CONNECTION that MUST be fixed, not a second map. Make ONLY these three bounded geographic corrections; preserve everything else. (1) GOBLIN MOUNTAINS: north of Dawnmere and south of Ridgeshire, replace the broad grassy eastern valley where the road currently bypasses the mountain range with dense jagged gray MOUNTAINS. The north-south road must wind THROUGH the MIDDLE of the range, flanked closely by substantial peaks on BOTH sides for its whole length. Narrow stony zigzag mountain pass, never a wide green valley or a road beside peaks. Reposition the Goblin Mountains label onto the mountains near the road if necessary, keep both town names untouched. Peaks tightly enclose the route from Dawnmere up toward Ridgeshire, matching the existing gray stone mountain artwork, snowy tips and pine detail. (2) SALTWIND MARSH: ONLY the little inlet immediately south of Dawnmere. Lower its mangrove trees, tiny skiffs, muddy banks and teal waterways down to SEA LEVEL. Tidal creeks must open directly into the surrounding sea with continuous flat water surface: NO sheer cliff edge beneath the marsh, NO waterfall, NO perched lake or elevated terrace. Keep it SMALL, about its existing footprint, as a low coastal creek mouth at the base of surrounding rocky coast. Move its shoreline gently south if needed, retaining the exact 'Saltwind Marsh' label. Keep grass/low scrub headland directly EAST of Dawnmere between town and the eastern shore open and uncrowded for a future region. No new destination, no new label, no expansion replacing sea or existing regions. (3) BROKEN DOCK: IMAGE 2 shows upper-left shore wharf and lower-right main wharf separated by water. Fill the little missing timber connection between the lower tip of the SHORE-attached platform (near pixel 962,724 in IMAGE1) and the upper-left end of the MAIN pier (near pixel 962,737). Draw an obvious plank-deck CONNECTOR joining these two ends into ONE SOLID CONTINUOUS WALKABLE DOCK with uninterrupted wood and supporting piles. Inspect it: can someone walk from the shore to the very far tip WITHOUT crossing water? They must be able to. Preserve both branching mooring fingers, the wharf size, Docks label and dotted voyage line. No floating platform, no detached dock, no boat. PROTECTED FEATURES MUST REMAIN IDENTICAL: all of the lower-left Glassdeep Caverns terrain south of Stillbury, pale sandy BEACHES BELOW GLASSDEEP with cliffs BEHIND THEM, the C-SHAPED CURVING SUNKEN COAST COVE, its inland bay and drowned columns, existing western coast, Oakhaven, Rook's Labyrinth outside the forest, both forests and northern castle route, all five towns, other northern regions, Ruined Isle/Temple, all other labels, roads, frame/title/compass, existing watercolor style and image dimensions. Do not regenerate or rearrange the atlas. Only mountain-pass enclosure, sea-level compact marsh, and an actual wooden dock connector.
```

## Final road connection and attached mooring arm prompt

```text
LOCAL ATLAS REGRESSION REPAIR. Edit attached map, keep its new dense mountain pass and sea-level Saltwind Marsh EXACTLY. Two tiny local corrections only. A: RESTORE THE MISSING OAKHAVEN-DUNGEON-DAWNMERE ROAD CONNECTION. The glowing east-west road from Oakhaven past Oakhaven Dungeon currently ends/turns into Saltwind Marsh near x1105,y650. It MUST visibly connect to Dawnmere as well. Add a continuous narrow winding trail from that junction near (1105,645), curving NORTH along the western foot of the gray mountains around (1110,585), then joining the existing road just WEST OF DAWNMERE around (1145,525). Make it physically continuous into Dawnmere's west entrance. Preserve the separate SOUTH road from Dawnmere to Saltwind Marsh. Preserve the existing NORTH road through Goblin Mountains toward Ridgeshire, tightly enclosed by peaks, no grassy valley. This repairs an existing connection only, no new area, road signs or labels. B: The docks are now connected but lost their characteristic second mooring arm. Keep the long main pier and shore attachment intact. Add/restore a modest SHORT SECOND WOODEN MOORING PIER branching from the SHORE-END of the main wharf near (942,709), extending diagonally southeast to about (987,724), on support posts, ending above and parallel to the long main pier, with visible water between these two arms. It must be visibly ATTACHED by solid timber at its base to the main wharf so both arms can be walked from shore without crossing water. Think a compact branching wooden marina wharf, not a lone straight bridge and not two disconnected platforms. Keep the Docks label clear. No boats. NO OTHER EDITS. Absolutely preserve Glassdeep below Stillbury, beaches and cliffs below Glassdeep, C-shaped Sunken Coast cove and drowned columns, Oakhaven, Dawnmere, sea-level compact marsh and its trees/boats/label, open headland EAST of Dawnmere reserved for future area, Labyrinth, both forests, all other roads/towns/regions/labels, islands, typography, colors, frame and dimensions. Do not add peaks below Dawnmere or change terrain. All edits limited to adding the missing connecting trail west of Dawnmere and a short attached mooring arm at Docks.
```
