# Act 5 Crown Road label correction

Built-in imagegen edit of the canonical `docs/world-map.png`. Final SHA-256
prefix: `4ab1f192db91`. Both README image URLs use this checksum.

Exact prompt:

Use case: text-localization. Asset: supplied canonical 1536x1024 fantasy world map. Change ONLY TWO existing parchment road labels. The label physically WEST (left) of the castle between Rosemoor and the castle currently says 'Crown Road East': replace its text with EXACTLY 'Crown Road West'. The label physically EAST (right) of the castle between the castle and Ridgeshire currently says 'Crown Road West': replace its text with EXACTLY 'Crown Road East'. Match their existing typography, size and parchment placement. Preserve every other pixel as closely as possible: all roads and town/region positions, Ashen Hollow and Frostfell access from Rosemoor, Dragonspine and Ironfang Mines access from Ridgeshire, Castle Grounds sole northern first approach through Crown Road North / Catacombs / Blackwater, escape-only arrow to Stillbury, both Soulstone shrines, Stonewake and Labyrinth, the two escarpments, ACT V - PROVISIONAL legend, river and all southern geography, both white dashed island voyage and yellow dashed Saltwind skiff crossing, the title, border and compass. Do not redraw geography, add any route, remove any place, change another label or alter the image size. Two text replacements only.

The selected output was inspected against its predecessor. Crown Road West is
between Rosemoor and the castle; Crown Road East is between the castle and
Ridgeshire. The four regional branches, both shrines, escarpments, northern
castle route, escape-only return, two sea voyages and provisional legend remain.
