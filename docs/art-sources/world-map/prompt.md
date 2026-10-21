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
- "Dragonspine" stretches east of Ridgeshire into the northeast around (86%, 30%): dramatically higher jagged snowcapped peaks than the Goblin Mountains. The height distinction must be obvious.
- "Ashen Hollow" north of Ridgeshire around (70%, 14%): volcanic basin, charcoal crags and restrained glowing lava.
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
