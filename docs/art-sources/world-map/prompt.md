# World map illustration

Generated with the built-in image generation tool using the user's handwritten
world sketch as a layout reference. Place names and connections were checked
against the game and README. The final image is `../../images/world-map.png`.
This is README artwork; it does not change generated game maps.

## Generation prompt

Use case: illustration-story
Asset type: finished landscape fantasy world map for the README of the C/SDL adventure game "The Castle of No Return".
Primary request: turn the user's rough handwritten world sketch into a beautifully drawn, readable illustrated atlas map. Image 1 is a geographical layout reference only; its photograph is sideways. Interpret the sketch upright with north at the top, Rosemoor in the northwest, Stillbury in the southwest, Ridgeshire in the northeast, and Oakhaven in the southeast. Create a new polished map, not a photograph of paper and not a flowchart.

Style/medium: traditional hand-drawn fantasy cartography, fine pen coastlines and contour hatching with restrained watercolor washes on warm ivory parchment, crisp dark readable cartographic lettering. Vivid but muted biome colors: forest green, teal marsh water, ochre dunes, blue-gray mountains, icy white high peaks, deep blue sea. Tiny detailed settlement, castle, cave and temple vignettes integrated naturally into terrain. Landscape aspect ratio about 3:2, high resolution. Tasteful thin frame, small north compass, title "THE CASTLE OF NO RETURN", subtitle "WORLD MAP". Labels must be legible when viewed in a README; avoid excessively small type.

Critical geography, as a north-up map, with approximate percentage positions (allow generous label room):
- One continuous mainland occupying the left and middle 85% of the map. Irregular rocky coastline especially along south and east. Blue sea along the southeast/right, including bays and small coastal islets.
- Rosemoor town around (26%, 30%), Stillbury town around (26%, 65%), Ridgeshire town around (70%, 30%), Oakhaven town around (70%, 65%).
- Castle of No Return, a conspicuous dark multi-towered stone castle, around (48%, 28%), connected west to Rosemoor and east to Ridgeshire by long winding roads. Label Rosemoor-to-castle road "Crown Road East" and castle-to-Ridgeshire road "Crown Road West" (these are intentional game names, relative to the departing towns).
- A broad dense "Haunted Forest" stretches horizontally between Stillbury and Oakhaven, around (47%, 65%); a woodland path runs through it between the towns.
- "Blackwater Swamp" occupies the vertical area between Rosemoor and Stillbury, around (26%, 47%), with reed beds, twisted trees, dark pools and marsh channels.
- "Goblin Mountains" runs vertically between Oakhaven and Ridgeshire, around (70%, 48%), many rugged medium-height peaks and a winding mountain pass joining the towns.
- "Dragonspine" stretches north of Ridgeshire around (70%, 14%): dramatically higher jagged snowcapped peaks than the Goblin Mountains. The height distinction must be obvious.
- "Ashen Hollow" east of Ridgeshire around (86%, 30%): volcanic basin, charcoal crags and restrained glowing lava.
- "Frostfell Wastes" north of Rosemoor around (26%, 13%): frozen ridges, snowy pines, icy lakes and crevasses.
- "Moonveil Gardens" west of Rosemoor around (11%, 30%): pale blossoms, lush green groves and garden ruins.
- "Sunscar Wastes" west of Stillbury around (11%, 63%): broad sandy dunes, sandstone mesas, scattered cacti and a tiny oasis.
- "Glassdeep Caverns" south of Stillbury around (24%, 83%): rocky cave entrance with visible blue crystals, small underground cutaway vignette.
- "Rook's Labyrinth" beside Stillbury, slightly southeast around (34%, 75%): small stone maze/ruined entrance.
- "Royal Catacombs" directly south of the castle around (48%, 41%): burial crypt entrance connected to castle. A separate small jail at the castle has a fine dotted underground "Escape Tunnel" to Ridgeshire; distinguish tunnel from surface roads.
- "Oakhaven Dungeon" east/northeast of Oakhaven around (80%, 56%): stone stair entrance in a rocky hill, connected only to Oakhaven.
- "Sunken Coast" south of Oakhaven around (67%, 82%): drowned ruins, tidal inlets, cliffs and coastal rocks.
- "Docks" east/southeast of Oakhaven around (83%, 71%) at the mainland coastline: a wooden pier and a small sailing boat. Use "Docks", NEVER "Harbor".
- "Ruined Isle" offshore to the southeast around (90%, 86%): a substantial lush rocky island separated from mainland by visible open sea. A weathered "Ruined Temple" stands on that island. A dotted sailing route connects the Docks to the island. A few small unlabelled offshore islets provide coastline variety.

Roads should communicate the sketch's travel connections through terrain without drawing rectangular region boxes. Do not invent extra towns, landmarks, named seas or destinations. Keep every named place exactly once, aside from the title. Clear readable text, no overlapping labels, no text on top of buildings. Terrain should occupy the whole geographic space organically rather than disconnected icons on blank paper. Main towns and castle have the strongest label hierarchy. No characters, no monsters, no game UI, no level counts. Tiny footer: "Illustrated geography — not to scale".

## Label correction prompt

Edit this finished world map with exactly one localized addition: add the geographic label "Ruined Isle" in the open blue sea immediately beneath the large temple island in the lower right, above the bottom map frame. Use a small warm parchment label and crisp dark cartographic serif lettering consistent with the existing place labels, readable and centered below the island without obscuring its coast. The existing "Ruined Temple" label must remain on the island as it is: these are two separate names, an island and its landmark. Preserve ALL other artwork, labels, positions, routes, colors, textures, dimensions, typography, the volcanoes of Ashen Hollow, and snowy Dragonspine exactly. Do not redraw, recolor, crop or recompose the map. Only add this one missing label.


## Ridgeshire geography swap

Edited with the built-in image generation tool using the previous finished map
as the edit target (`precise-object-edit`). Final prompt:

> Use case: precise-object-edit. Edit target: the supplied finished fantasy world map. Make exactly one geographic swap in its upper-right quadrant: Dragonspine must be NORTH of Ridgeshire, occupying the current Ashen Hollow volcanic region directly above the town, with towering icy snow-covered mountain peaks and snowy pines. Ashen Hollow must be EAST of Ridgeshire, occupying the current Dragonspine snowy region on the far upper-right mainland, with charcoal volcanic mountains, glowing lava and ash. Move the actual terrain and the exact labels together, not just the labels. Remove volcanoes from the northern Dragonspine region and remove snowy peaks from the eastern Ashen Hollow region. Frostfell Wastes in the northwest and Dragonspine in the northeast must now be the two northernmost cold regions. Keep Ridgeshire in exactly its current position. Make the northern snowy Dragonspine taller than Goblin Mountains. Preserve every other region, town, castle, road, coastline, island, label, parchment texture, frame, title and compass from the original. Especially preserve Frostfell Wastes, Crown Roads, Castle of No Return, Royal Catacombs, Escape Tunnel, Goblin Mountains, Oakhaven Dungeon, Docks, Ruined Temple and Ruined Isle. Do not add destinations or omit labels. Preserve the landscape 3:2 composition and original fine ink/watercolor cartographic style. Final exact text: 'Dragonspine' over the northern icy peaks above Ridgeshire; 'Ashen Hollow' over the volcanic basin east of Ridgeshire. All other existing text unchanged. Return the edited map.

Reviewed the two moved regions and all original labels. The edited asset
replaces `docs/images/world-map.png`.

## Current mountain route (save version 171)

The illustration above predates Dawnmere. For its next geography revision,
place Dawnmere directly east of Oakhaven and south of Ridgeshire. Put Goblin
Mountains north of Dawnmere beside the straight road to Ridgeshire. The dungeon
stands beside the west–east Oakhaven–Dawnmere road. Both buildings unlock their
through road on boss victory. Oakhaven's future northern side region is
Windscar Highlands (not yet playable). See the live connection diagram in
[the expansion document](../../five-town-expansion.md).

## Forest adventure revision (save version 172)

Keep the Haunted Forest geographically between Oakhaven and Stillbury. Its new
haunted-tree entrance stands beside a straight west–east town road, which opens
after defeating the Necromancer. Both towns enter Forest 1; the adventure has
five levels and no separate shortcut to the grove.

## Five-town geography and pathway entrances (save version 175)

Edited with the built-in image generation tool. Output copied to
`docs/images/world-map.png`; reviewed all five town labels, the two northern
cold regions, Dungeon between Oakhaven and Dawnmere, and Goblin Mountains
between Dawnmere and Ridgeshire. Previous prompts above describe historical
layouts. Final edit prompt:

```text
Use case: precise-object-edit
Asset type: finished illustrated fantasy atlas for the README of The Castle of No Return.
Input image: edit target, the existing finished world map. Preserve its detailed pen-and-watercolor parchment cartography, title, border, compass, palette, terrain style, and all destinations. Recompose the southern half to add exactly one fifth town, Dawnmere, and correct the roads. High-resolution landscape 3:2.
Mainland geography: Rosemoor northwest, Stillbury southwest, Castle of No Return north-center, Ridgeshire northeast. Move Oakhaven to south-center, east of Stillbury. Put the NEW town Dawnmere directly EAST of Oakhaven and directly SOUTH of Ridgeshire, on mainland with room for town and road labels.
Critical playable connections, clear continuous thin roads:
Stillbury — Haunted Forest — Oakhaven, west/east.
Stillbury — Blackwater Swamp — Rosemoor, south/north.
Oakhaven — Oakhaven Dungeon — Dawnmere, west/east. The small evil crypt stands BESIDE the straight road between these towns, which continues past it to Dawnmere.
Dawnmere — Goblin Mountains — Ridgeshire, south/north. Move Goblin Mountains off the old Oakhaven/Ridgeshire connection onto this new eastern Dawnmere/Ridgeshire road; a mountain entrance stands beside this road.
No direct road from Oakhaven to Ridgeshire. Five towns total, each exactly once, all named legibly: Rosemoor, Stillbury, Oakhaven, Dawnmere, Ridgeshire. Dawnmere should be prominent and spelled D-A-W-N-M-E-R-E.
Preserve northern geography: Frostfell Wastes north of Rosemoor; snowy Dragonspine north of Ridgeshire. These are the two northernmost cold regions. Volcanic Ashen Hollow EAST of Ridgeshire.
Preserve other destinations and connections: Moonveil Gardens west of Rosemoor; Sunscar Wastes west of Stillbury; Glassdeep Caverns south of Stillbury; Rook's Labyrinth beside Stillbury; Royal Catacombs south of Castle. Crown Road East goes Rosemoor→Castle; Crown Road West goes Ridgeshire→Castle (intentional names relative to departing towns). A dotted underground Escape Tunnel connects castle jail to Ridgeshire. Sunken Coast south of Oakhaven; Docks southeast of Oakhaven on a coast inlet, with dotted sailing route to offshore southeastern Ruined Isle and its Ruined Temple.
Redesign the coastline as needed to fit Dawnmere east of Oakhaven without crowding. Mainland roads stay on land. Island separated by open sea. Terrain organically fills the map. Keep Castle visually central and dominant. Each named place once, readable dark serif labels with no overlaps. No extra destinations, no Windscar Highlands (not playable), no level numbers, no UI, no monsters. Title THE CASTLE OF NO RETURN, subtitle WORLD MAP; footer Illustrated geography — not to scale.
```

## Labyrinth relocation (save version 176)

The built-in image generation tool moved the labyrinth vignette and label from
Stillbury's outskirts onto the Haunted Forest road. Reviewed the new spur,
all five towns, and preserved regional connections. Output replaces
`docs/images/world-map.png`. Final edit prompt:

```text
Use case: precise-object-edit. Edit target: this finished FIVE-TOWN fantasy world map. Make exactly one localized geographic relocation: move the existing stone maze vignette AND its label "Rook's Labyrinth" away from the southern/eastern outskirts of Stillbury. Remove its old vignette and label, restoring natural grass and forest there while preserving the nearby Glassdeep road.
Place Rook's Labyrinth in a small clearing BESIDE THE ROAD THROUGH HAUNTED FOREST, between Stillbury and Oakhaven, close to the Haunted Forest landmark. Prefer above the woodland road just west of Oakhaven, with a tiny spur from that road to the stone maze, enough room for a legible label. This is an entrance beside the forest path, not a destination in Stillbury's town. Its maze drawing should be modest and integrated with the forest terrain. There must be exactly one label "Rook's Labyrinth" and one corresponding stone maze. Do not add quest text or a lock icon.
Preserve ALL five towns (Rosemoor, Stillbury, Oakhaven, Dawnmere, Ridgeshire), the exact Oakhaven–Dungeon–Dawnmere road, the Dawnmere–Goblin Mountains–Ridgeshire road, every other destination and label, all northern snowy peaks, Ashen Hollow volcanoes, castle, Crown Roads, coast, Docks, Ruined Isle, Ruined Temple, title, compass, parchment border, colors, textures, typography and dimensions. No other geographic changes. Keep Haunted Forest label readable and do not obscure either town with the moved labyrinth. Return the complete edited map.
```


## Labyrinth outside the forest (town-exterior revision)

Edited with the built-in image generation tool, replacing
`docs/images/world-map.png`. The earlier woodland placement was incorrect.
The labyrinth now stands in open grass south of the Stillbury–Oakhaven road,
with its own short spur; the Haunted Forest remains north of that road.
Reviewed all five towns, other labels, routes and coastlines after the edit.
Final edit prompt:

```text
Use case: precise-object-edit. Edit this finished fantasy atlas. Make one localized correction only: Rook's Labyrinth is NOT INSIDE the Haunted Forest. Move the small stone maze drawing and its exact label "Rook's Labyrinth" to open GRASSLAND SOUTH OF the Stillbury–Oakhaven road, near its midpoint. The woodland and Haunted Forest label must be NORTH of that road, with a clear visible separation between trees and labyrinth. Clear a grassy roadside meadow for the labyrinth on the SOUTH side of the road; connect its entrance with a short spur from the road. Restore trees where the previous labyrinth was north of the road. There is exactly one labyrinth and one label, and no forest trees enclosing the labyrinth. Preserve the road between Stillbury and Oakhaven, all FIVE towns and names (Stillbury, Rosemoor, Oakhaven, Dawnmere, Ridgeshire), all other destinations, coastlines, mountains, Crown Roads, castle, label typography, title, compass, border, palette, illustration style, layout and 3:2 landscape format. Keep the dungeon between Oakhaven and Dawnmere, Goblin Mountains between Dawnmere and Ridgeshire, Dragonspine north of Ridgeshire, and Ashen Hollow east of Ridgeshire unchanged. Do not add destinations, do not move towns. Return the complete corrected map. This is a precise correction of geography, not a redesign.
```

## Elderwood and the northern castle road

Edited with the built-in imagegen tool. The selected result replaces the canonical
`docs/images/world-map.png`; README continues to reference this path.
Reviewed the five town names, both Crown Roads, Elderwood's northern position,
the Catacombs landmark on that road, the labyrinth outside Haunted Forest,
Glassdeep south of Stillbury and the island/coast geography.

Final edit prompt:

```text
Use case: precise-object-edit. Asset type: existing illustrated RPG world map. Edit target: the provided map. Preserve the parchment frame, title, art style and ALL existing towns, other regions, labels and routes in exactly their present positions. Add the new northern route from Oakhaven directly north through the central forest to Castle of No Return. The south half of this central woodland immediately NORTH of Oakhaven must have a clearly legible parchment label 'Elderwood Forest'. Keep 'Haunted Forest' as the distinct forest WEST of Oakhaven between Oakhaven and Stillbury, shift only that forest label left within its forest to avoid overlap. Draw a winding thin road from Oakhaven north through Elderwood Forest, then past the Royal Catacombs landmark, then north to Castle. Royal Catacombs is a separate stone crypt entrance ALONG this northern road, north of Elderwood and south of Castle; preserve the crypt icon and exact label 'Royal Catacombs', slightly relocate it on this road if needed. The geography should read Oakhaven -> Elderwood Forest -> Royal Catacombs -> Castle along a south to north road. Rook's Labyrinth must remain OUTSIDE Haunted Forest beside the Oakhaven-Stillbury road. Glassdeep Caverns remains south of Stillbury. Dawnmere remains east of Oakhaven, Goblin Mountains north of Dawnmere, Ridgeshire north of mountains, Ashen Hollow east of Ridgeshire and Dragonspine north of Ridgeshire. Do not introduce any additional locations or remove any labels. Retain everything else, including the two Crown Roads, Castle, Ruined Isle, Sunken Coast, Docks, the dungeon east of Oakhaven and five towns. Clearly spaced readable map labels. Landscape aspect ratio same as original.
```

## Saltwind landing atlas (save version 190)

Current README atlas: `docs/images/world-map-saltwind.png`.
The earlier `world-map.png` remains the pre-Saltwind reference. The Pirate Coast
atlas has been removed along with that region and its merchant quest.

Edited with the built-in imagegen tool from `world-map.png`. Reviewed Dawnmere
shifted north, Saltwind Mangroves south of town and above the offshore islands,
and the widened Goblin Mountains enclosing the Ridgeshire road on both sides.
The northern Elderwood/Catacombs route, separate forest/Labyrinth entrances,
Glassdeep south of Stillbury and all other towns remain visible. Saltwind's
first skiff exploration landing is playable; its later stages are planned.

Final prompt:

> Edit target: attached existing illustrated world atlas for The Castle of No Return. Preserve its parchment border, watercolor pen-and-ink fantasy cartography, title, compass, exact existing names and all unrelated geography. Make ONLY these requested geographic edits in the southeastern mainland. (1) Move the walled town labelled exactly 'Dawnmere' a modest distance NORTH, keeping it east of Oakhaven and south of Ridgeshire, and adjust its connected roads. Make ample mainland space directly SOUTH of Dawnmere and NORTH of the Ruined Isle and Ruined Temple offshore. Fill that new coastal space with an expansive green saltwater mangrove delta attached to the mainland: winding teal tidal channels, irregular muddy islands, tangled roots and a tiny moored wooden skiff. Label it exactly 'Saltwind Mangroves' with a readable dark cartographic serif label. Add a short road south from Dawnmere to the delta. Do not add Pirate Coast, pirates, black-sailed ships, pirate names or treasure quest landmarks. (2) Widen the Goblin Mountains range between Dawnmere and Ridgeshire across the full width of their connecting north-south road. Clearly draw the road from Dawnmere to Ridgeshire passing THROUGH a mountain pass within this range; the path must be visibly enclosed by mountains to both its east and west, not skirt alongside the eastern edge. Keep its exact 'Goblin Mountains' label readable. Preserve the Oakhaven Dungeon on the west-east Oakhaven-Dawnmere road as a separate landmark. Preserve every other region and road exactly in their current relationships: Elderwood Forest north of Oakhaven leading to separate Royal Catacombs then Castle; Haunted Forest west of Oakhaven; Rook's Labyrinth outside the forest beside its road; Glassdeep Caverns south of Stillbury; all five towns; Frostfell Wastes and Dragonspine in the far north; Ashen Hollow east of Ridgeshire; both Crown Roads; Sunken Coast, Oakhaven Docks, Ruined Isle and Ruined Temple south offshore. Maintain landscape 3:2 framing, matching the original map. Do not invent new settlements or rename existing places.
