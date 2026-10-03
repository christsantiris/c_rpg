# Building sprites

Each script in this folder draws one town building as pixel art at native game
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

## Sprites

| Script | Asset | Size | Where it is drawn |
|---|---|---|---|
| `castle.py` | `castle-of-no-return.bmp` | 408 × 240 (17 × 10 tiles) | Town 3, the moat footprint `TOWN_MOAT_*` |
| `tavern.py` | `tavern.bmp` | 168 × 120 (7 × 5) | Town 1, `TOWN_TAVERN_X/Y` |
| `inn.py` | `inn.bmp` | 168 × 120 (7 × 5) | Stillbury, `TOWN_INN_X/Y` |
| `blacksmith.py` | `blacksmith.bmp` | 120 × 96 (5 × 4) | Town 1, `TOWN_BLACKSMITH_X/Y` |
| `alchemist.py` | `alchemist.bmp` | 120 × 96 (5 × 4) | Town 1, `TOWN_ALCHEMIST_X/Y` |
| `harbor.py` | `harbor.bmp` | 120 × 96 (5 × 4) | Town 1, `TOWN_HARBOR_X/Y`, and enlarged on the harbor screen |
| `healer.py` | `healer.bmp` | 120 × 96 (5 × 4) | Stillbury, `TOWN_HEALER_X/Y` |
| `witch.py` | `witch-hut.bmp` | 120 × 96 (5 × 4) | Stillbury, `TOWN_WITCH_X/Y` |
| `labyrinth.py` | `labyrinth-entrance.bmp` | 144 × 72 (two 3 × 3 frames) | Stillbury, around `TOWN_LABYRINTH_X/Y` |
| `apothecary.py` | `apothecary.bmp` | 120 × 96 (5 × 4) | Town 3, `TOWN_APOTHECARY_X/Y` |
| `crownroad_gate.py` | `crownroad-gate.bmp` | 72 × 72 (3 × 3) | Stillbury's north exit and Town 3's south exit |

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

East of the Town 3 square, with its door on `TOWN_APOTHECARY_DOOR_X/Y`. If the
file is missing, the game draws the alchemist sprite in its place.

- A steep front gable in dark plum shingles with a stone chimney.
- A cream plaster and dark timber upper floor with three lit windows and flower
  strips under the lower two.
- A stone shop front with two lamplit display windows. Their shelves hold red
  healing, blue mana, orange strength and violet intelligence potions, and they
  warm the stone around them.
- An arched plank door with stone steps, a lantern, and a green sign with a
  brass mortar and pestle.
- A crate of bottles and two herb planters.

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

## Source art that is not generated

The island and temple sprite sheets (`island-sprites.bmp`, `island-ship.bmp`
and `temple-enemies.bmp`) are not drawn by these scripts. Their original
high-resolution images, which the BMPs were converted from, are in
`docs/art-sources/`.
