# Building sprites

Each script in this folder draws town buildings as pixel art at native game
resolution (24 px per tile) and writes the BMP that the game loads from
`assets/images/`. The shared drawing helpers are in `pixelkit.py`: layers and
outlines, stone walls, battlements, cone and tiled roofs, timber framing,
windows, doors, props, and `save_sprite()`. That function writes a 32-bit
top-down BMP with alpha, the format `SDL_LoadBMP` reads with transparency.

## Changing a sprite

1. Edit the building's script.
2. Run `make sprites` (Python 3 with Pillow). It runs every script and rewrites
   every BMP.
3. Check the building in game.

The scripts are deterministic, so an unchanged script reproduces its BMP byte
for byte. After `make sprites`, `git status` shows only the sprites you
changed.

To add a building: write a new script that uses `pixelkit.py`, add it to the
`sprites` target in the `Makefile`, and load its BMP in
`src/renderer/renderer.c`.

New buildings must match the existing shops: use the shared textured roof,
shaded stone, timber, window, door and outline helpers at 24 pixels per tile.
Keep ordinary single doors about 13 pixels wide and 17–19 pixels tall. Review
new artwork beside an existing shop at the same scale, then check its
transparency, doorway alignment and signage in the game renderer.
Do not mark building artwork complete until both comparisons have been
inspected; automated gameplay tests do not verify visual quality.

## Sprites

| Script | Asset | Size | Where it is drawn |
|---|---|---|---|
| `castle.py` | `castle-of-no-return.bmp` | 408 × 240 (17 × 10 tiles) | Rosemoor, the moat footprint `TOWN_MOAT_*` |
| `tavern.py` | `tavern.bmp` | 168 × 120 (7 × 5) | Town 1, `TOWN_TAVERN_X/Y` |
| `inn.py` | `inn.bmp` | 168 × 120 (7 × 5) | Stillbury, `TOWN_INN_X/Y` |
| `blacksmith.py` | `blacksmith.bmp` | 120 × 96 (5 × 4) | Town 1, `TOWN_BLACKSMITH_X/Y` |
| `alchemist.py` | `alchemist.bmp` | 120 × 96 (5 × 4) | Town 1, `TOWN_ALCHEMIST_X/Y` |
| `harbor.py` | `harbor.bmp` | 120 × 96 (5 × 4) | Town 1, `TOWN_HARBOR_X/Y`, and enlarged on the harbor screen |
| `healer.py` | `healer.bmp` | 120 × 96 (5 × 4) | Stillbury, `TOWN_HEALER_X/Y` |
| `witch.py` | `witch-hut.bmp` | 120 × 96 (5 × 4) | Stillbury, `TOWN_WITCH_X/Y` |
| `labyrinth.py` | `labyrinth-entrance.bmp` | 144 × 72 (two 3 × 3 frames) | Forest approach, around `APPROACH_LABYRINTH_X/Y` |
| `dungeon.py` | `dungeon-entrance.bmp` | 72 × 72 (3 × 3) | OakHaven dungeon approach, around its area entrance |
| `temple.py` | `temple-entrance.bmp` | 168 × 144 (7 × 6) | Ruined Isle, central `ISLAND_TEMPLE_*` footprint |
| `apothecary.py` | `apothecary.bmp` | 120 × 96 (5 × 4) | Rosemoor, `TOWN_APOTHECARY_X/Y` |
| `adventurers_guild.py` | `adventurers-guild.bmp` | 168 × 120 (7 × 5) | Rosemoor, `TOWN_GUILD_X/Y` |
| `workshop.py` | `workshop.bmp` | 120 × 96 (5 × 4) | Ridgeshire, `TOWN4_WORKSHOP_X/Y` |
| `town_hall.py` | `town-hall.bmp` | 168 × 120 (7 × 5) | Ridgeshire, `TOWN4_HALL_X/Y` |
| `local_buildings.py` | `bakery.bmp` | 120 × 96 (5 × 4) | Oakhaven, southwest bakery plot |
| `local_buildings.py` | `butcher.bmp` | 120 × 96 (5 × 4) | Stillbury, southwest butcher plot |
| `local_buildings.py` | `monastery.bmp` | 168 × 120 (7 × 5) | Ridgeshire, southwest monastery plot |
| `local_buildings.py` | `spice-merchant.bmp` | 120 × 96 (5 × 4) | Rosemoor, southwest spice merchant plot |
| `crownroad_gate.py` | `crownroad-gate.bmp` | 72 × 72 (3 × 3) | Stillbury's north exit and Rosemoor's south exit |

### Castle of No Return

The 15 × 8 castle plus its one-tile moat ring. The drawbridge covers tile
(20, 11), directly below the entrance. Transparent pixels, meaning the grass
banks and the ground behind the castle, show the town floor drawn underneath.

- Dark charcoal-violet stone with cracks, grime streaks and waterline slime.
- Needle spires in black-crimson slate with hooked eaves and iron finials; the
  central keep spire is the tallest point.
- Blood-red lit windows and tattered black banners with red skull sigils.
- A black-and-crimson shield with a downward iron sword above the gate.
- A spiked portcullis, iron braziers, and a chained drawbridge flanked by skull
  pikes.
- A murky moat with grass banks and dead reeds, and a stone quay along the
  plaza.

### Tavern

East of the Blacksmith, with its door on `TOWN_TAVERN_DOOR_X/Y` opening onto
the square.

- A red clay tile roof with a ridge line, a central cross gable, two dormers
  and a stone chimney.
- A half-timbered upper floor: cream plaster, dark beams and diagonal braces,
  and warm lit windows with plank shutters.
- A stone ground floor with timber posts, two shuttered windows, an arched
  plank door in a stone surround, and stone steps.
- Two iron lanterns by the door and a hanging beer-mug sign on the left.
- Barrels, a crate and a bench along the front.

### Inn

East of the Healer, with its double door on `TOWN_INN_DOOR_X/Y` opening onto
the square. It shares the tavern's timber-and-plaster style but has its own
design.

The double door is 19 pixels tall to match the shops' doorway proportions; its
threshold and map entrance stay in the same position.

- Twin front gables and a hipped middle roof in blue-grey slate, with a
  sandstone chimney.
- A whitewashed upper floor with dark timber framing, small attic windows, and
  shuttered windows with flower boxes.
- A covered balcony with balusters across the middle, in front of three lit
  windows.
- A sandstone ground floor with timber posts, a double plank door under a lit
  fanlight, stone steps, and two iron lanterns.
- A hanging navy sign with a gold crescent moon and star.
- A horse trough and hitching post, flower pots by the door, and stacked hay
  bales.

### Blacksmith

The door is on tile (13, 10).

- A front-facing gable in dark slate with a stone chimney.
- A half-timbered upper floor with cream plaster and a lit, shuttered window.
- A stone ground floor with an arched plank door in a stone surround and steps.
- A slate lean-to over the forge: a stone hearth with a glowing fire that warms
  the nearby stones, hanging tongs and hammers, and an anvil on a stump.
- An anvil sign, a lantern, a firewood stack, a crate and a barrel.

### Alchemist

The door is on tile (31, 10).

- A broad gable in teal shingles whose long right slope shelters a herb stall,
  and a stone chimney.
- A mauve plaster and dark timber upper floor, with a violet-lit window holding
  a potion flask.
- A shop window of glowing green potions under a small teal awning, which
  casts a green light on the stone nearby.
- A stone ground floor with an arched plank door, steps, and a keg on each side.
- An open stall with hanging herb bundles and a shelf of bottles, a crate, a
  lantern, and a hanging green flask sign.

### Harbor

In the south-east corner of Town 1. The dock crosses row
`TOWN_HARBOR_ENTRANCE_Y`, where the harbor road ends. The harbor screen draws
the same image enlarged by a whole-number scale.

- A harbour house with a red clay gable roof, a stone chimney, a half-timbered
  upper floor with a lit shuttered window, and a stone ground floor with an
  arched door.
- A plank dock on rope-wrapped pilings, with a barrel, a crate, a coil of rope
  and a lantern on a bracket.
- A crane post whose hook hangs over a small sailboat with a cream sail and
  rigging.
- Harbour water with an irregular edge and ripples, in the coast palette.

### Healer

The door is on `TOWN_HEALER_DOOR_X/Y`.

- A front-facing gable in sage-green shingles with a stone chimney.
- A half-timbered upper floor with cream plaster and a lit, shuttered window.
- A stone ground floor with an arched plank door, stone steps, and a lantern on
  each side of the door.
- A sage lean-to over a herb stall: a small lit window, hanging lavender and
  white herb bundles, a shelf of bottles, and a table with a mortar.
- A green sign with an ivory healing cross, a lantern, and three planters of
  herbs and flowers.

### Witch's hut

The door is on `TOWN_WITCH_DOOR_X/Y`.

- A wide gable roof and lean-to in purple shingles, patched with moss, and a
  mossy stone chimney.
- A half-timbered gable with a purple-glowing window, a flower box, trailing
  ivy and a hanging gold crescent charm.
- A stone ground floor with an arched plank door, stone steps and two lanterns.
- A plaster side wing with a shelf of potions, and a crescent-moon sign on a
  bracket.
- A lean-to potion stall: hanging herbs, a bubbling cauldron over a fire that
  casts a violet glow, a table of bottles and a lantern.
- Red and violet mushrooms around the base.

### Labyrinth entrance

Two 72 × 72 frames side by side: sealed on the left, open on the right. The
game draws a frame from one tile left of and two tiles above the entrance tile.
If the file is missing, it falls back to its older rectangle drawing.

- A mossy stone archway with stepped coping, a stepped arch over a deep
  doorway, and worn steps at its foot.
- A brass-framed ivory rook crest above the arch.
- Buttresses on both sides with iron sconces and climbing ivy.
- Sealed: an iron portcullis with brass rivets and a brass lock plate, and
  unlit sconces.
- Open: the portcullis raised, steps descending into the dark, and lit torches.

### Apothecary

On the north edge of the Rosemoor square, with its door opening directly onto
the plaza. If the file is missing, the game draws the alchemist sprite in its
place.

- A steep front gable in dark plum shingles with a stone chimney.
- A cream plaster and dark timber upper floor with three lit windows and flower
  strips under the lower two.
- A stone shop front with two lamplit display windows. Their shelves hold red
  healing, blue mana, orange strength and violet intelligence potions, and they
  warm the stone around them.
- An arched plank door with stone steps, a lantern, and a green sign with a
  brass mortar and pestle.
- A crate of bottles and two herb planters.

### Adventurer's Guild

The Guild stands on the north-west edge of the Rosemoor square, with its front
door opening directly onto the plaza.

The oak door is about 19 pixels tall after the sprite's 75% reduction, matching
the shops' doorway proportions without moving the entrance.

- A broad slate gable with a gold crossed-swords crest.
- A stone-and-timber front with two lit windows.
- An arched oak entrance and a hanging sign marked GUILD.

### Crown Road gate

Drawn from the tile left of `CROWNROAD_X`, so its archway lines up with the
road. The archway is transparent, so the road drawn underneath shows through.
If the file is missing, the game falls back to the older rectangle drawing.

- A weathered royal gatehouse in pale stone: two crenellated towers with arrow
  slits, string courses and plinths, joined by a lower crenellated wall.
- A pointed arch over the road, with the portcullis raised into the passage.
- A cracked gold crown relief above the arch, for the fallen Crown Road.
- Torn crimson banners edged in gold with small crowns, a torch on each side of
  the arch, and ivy up the outer tower walls.

### Butcher shop

Stillbury's butcher uses the same roof, masonry and timber detailing as the
healer and blacksmith, with its existing 5 × 4 tile footprint and doorway.
It has a burgundy shingle roof, a lit dormer, striped awnings, hanging ham,
ribs and sausage links, a chopping counter, a warm display window, and a
cleaver sign. Crates, a barrel, herbs and a metal jug fill out the frontage.

### Monastery

Ridgeshire's monastery uses a 7 × 5 tile footprint, with its entrance centered
on the existing doorway tile. Its chapel and separate bell tower give it a
different silhouette from the Town Hall.

- A steep slate chapel roof and a taller bell tower with a polished brass bell.
- Shaded limestone masonry, carved cornices, buttresses and an oak door in a
  layered stone arch.
- A circular stained-glass rose window and narrow windows with colored panes
  and lead tracery.
- Blue altar banners, warm lanterns, climbing ivy, planters and a small shaded
  cloister with a wooden bench.

## Source art that is not generated

The island sprite sheets (`island-sprites.bmp` and `island-ship.bmp`) are not
drawn by these scripts. Their original high-resolution images, which the
BMPs were converted from, are in `docs/art-sources/`.

The Temple enemy sheet is imported from its refined source with:

```sh
python3 tools/sprites/temple_enemies.py
```

The importer uses `pixelkit.save_sprite()` to write the alpha BMP at native
display resolution. It packs nine 28×28 enemy frames and two 40×40 Guardian
poses into a 192×144 atlas. Each pose has at most 32 opaque colors and a clear
transparent edge. Keep the atlas and the source rectangles in `draw_enemy()`
and `draw_fallen_sun_guardian_broken()` in `src/renderer/sprites.c` together.
The source, exact imagegen prompts, and review images are documented in
[`docs/art-sources/enemy-batch-15/README.md`](../../docs/art-sources/enemy-batch-15/README.md).

The Castle enemy and boss sheets are imported with:

```sh
python3 tools/sprites/castle_enemies.py
```

The importer uses `pixelkit.save_sprite()` to write two native atlases: five
24×26 regular frames in a 96×64 sheet and five 36×36 boss poses in a 120×80
sheet. Boss poses include the Castellan, Royal Arcanist, and all three Veyr
forms. Frames have at most 32 opaque colors and clear transparent borders.
Keep both BMPs and the source rectangles in `castle_draw_enemy()` in
`src/renderer/castle_renderer.c` together. Sources, exact imagegen prompts,
and renderer previews are in
[`docs/art-sources/enemy-batch-16/README.md`](../../docs/art-sources/enemy-batch-16/README.md).

The fortified Goblin Mountains entrance is generated with
`python3 tools/sprites/mountain.py`. It uses pixelkit layers, shaded rock facets,
stonework and timber at the established 24px tile scale. The runtime BMP spans
7×6 tiles; its south-facing cave door anchors the approach collision footprint.

The Haunted Forest entrance uses `python3 tools/sprites/forest.py` and the same
pixelkit pipeline. Its 7×6 tile sprite frames a south-facing woodland passage
with ancient trunks, clustered foliage, hanging moss and warning charms.
Comparison and actual-renderer previews are in
[`docs/art-sources/forest-entrance/README.md`](../../docs/art-sources/forest-entrance/README.md).


### Regional pathway entrances

`make sprites` includes all regional entrance generators. Forest, mountain and
swamp landmarks are 168×144 pixels; the same size is used for the coast,
Sunscar, Frostfell, Moonveil, Ashen, Glassdeep and Royal Catacombs entrances.
Dragonspine is 168×216 pixels to accommodate its taller snowy peak and dragons.
Every asset uses pixelkit at native 24px tile scale and a south-facing doorway.

The generators are `coast.py`, `desert.py`, `frostfell.py`, `moonveil.py`,
`dragonspine.py`, `ashen.py`, `stonewake.py` and `catacombs.py`. Each writes
`assets/images/<name>-entrance.bmp`. The side-area approach metadata in
`src/game/approach.c` specifies each footprint's height; the renderer loads
textures in `src/renderer/renderer.c` and anchors them in `draw_area_entrance()`.

Each `docs/art-sources/<name>-entrance/` folder includes a native-scale comparison
with the forest entrance and Inn, and a preview captured through the actual SDL
renderer. Save version 175 converts old corridor approaches without resetting
combat maps, quest progress, character state or loot. Castle, secret tunnel and Crown Roads retain their existing layouts. The
existing sealed/open labyrinth sprite now stands beside the forest entrance;
its maze floors are unchanged.


### Town exteriors

`towns.py` creates the five 408×360 pixel town sprites: `oakhaven-town.bmp`,
`stillbury-town.bmp`, `rosemoor-town.bmp`, `ridgeshire-town.bmp` and
`dawnmere-town.bmp`. It runs after individual buildings in `make sprites`, using
pixelkit stonework, timber framing, textured plaster and tiled roofs. Dense roof
clusters sit inside a rounded curtain wall with projecting towers, following the
user's medieval hill-town reference. The entire connected side is an entrance
trigger; exterior artwork has no target gate. Interior gates remain exits.

`src/game/town_road.c` defines the footprint, directional sprite entrances and bypass
roads; `src/renderer/town_road_renderer.c` anchors the sprite and direction labels.
See [art review and SDL previews](../../docs/art-sources/town-exteriors/README.md).

### Elderwood

`elderwood.py` creates the living forest entrance and five creature sprites using
pixelkit. See [native comparisons and SDL previews](../../docs/art-sources/elderwood/README.md).

Saltwind entrance trees, root props and the four-direction skiff atlas use
`python3 tools/sprites/mangrove.py`. See the native comparison and SDL captures
in [Saltwind art sources](../../docs/art-sources/saltwind/README.md).

### Stonewake Quarry

Run `python3 tools/sprites/stonewake.py` to reproduce the transparent entrance,
crane, workshop, repair/supply props and five enemy cells. It uses pixelkit
stone, timber, textured roofs and outlined layers at 24 pixels per game tile.
The crane sheet contains three 72×72 cells for raised, crossing and salvage
settings; its load height changes with the selected route.
The old Glassdeep script/art remains a historical source and is no longer
loaded by the game. See [the region guide](../../docs/stonewake-quarry.md) for
stages, mechanics and the native/SDL visual checks.
