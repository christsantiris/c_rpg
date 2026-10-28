# Ironfang Mines art and validation

Runtime assets use `tools/sprites/ironfang.py` and the established pixelkit
pipeline. The entrance is 168×144; its doorway matches a walking character.
The rock uses irregular slate faces, chipped ledges, rust-coloured ore seams
and sparse moss. Iron-strapped timber supports frame a recessed rail tunnel,
with paired lanterns, an ore cart and an ore hoist beside it. Transparent
edges preserve the approach terrain, and the doorway keeps its existing anchor.
The Goblin Mine Boss is 24×24 and derives from the existing Hobgoblin artwork,
with a mining helmet, lamp, and pick. Seven regular enemies reuse their existing
native sprites. There is no rescaling of the player or other enemy families.

Actual SDL renderer captures:

- [Entrance beside the existing blacksmith, mountain entrance, player, and boss](ironfang-native.png).
- [Ridgeshire east gate](ironfang-ridgeshire.png) and [mine approach](ironfang-approach.png).
- [Abandoned Workings](stage-1.png), [Goblin Warrens](stage-2.png),
  [Flooded Galleries](stage-3.png), [Deep Ironworks](stage-4.png), and
  [The Rootshaft](stage-5.png).
- [Goblin Mine Boss rockfall warning](ironfang-boss-game.png).

Reviewed the entrance at native scale beside the existing building and player,
then in its actual approach. The actor sprites preserve visible feet and
transparent terrain underlays. Map, cart, bed, forge, lamp, and warning rendering
were inspected in the game renderer; gameplay assertions alone do not establish
visual quality.

## World atlas

The built-in **imagegen** tool edited the previous atlas to add the fortified
mine in the mountain flank east of Ridgeshire. The final project asset is
[world-map-ironfang.png](../../images/world-map-ironfang.png); the previous atlas
remains available unchanged. The exact final prompt is [atlas-prompt.txt](atlas-prompt.txt).

Reviewed the full map, then enlarged the mine and southern coast. The new road
meets Ridgeshire and the mine. The northern cold mountain belt and western Ashen
Hollow remain. Stonewake's southern beaches retain their cliff backdrop;
Sunken Coast retains its sheltered curved cove and drowned columns. The dock
deck remains connected from Oakhaven's shore to its mooring branches. Saltwind
remains a compact sea-level inlet, and Stormwatch remains east of lowland
Dawnmere. All five towns, castle, woods, Catacombs, separate Labyrinth, island,
and temple remain present.

## Verification

- Release and Debug builds complete without new compiler warnings.
- Gameplay/save suite: **7,956 / 7,956 assertions pass**.
- Generation checks cover **32 seeds × five stages**: both exits, enemy walking
  reachability, interaction access, excavation pockets, salvage access, boss
  reservation, and absence of overlapping actors.
- Cart checks cover bound Interact, arrow movement, speed, switch direction,
  broken-track repair, water-only crossings, disembarking, save/load, and
  recovering a cart after abandoning a portal.
- Boss checks cover fixed rockfall warnings, recovery, a finite guard pair,
  saved ore warnings, cart damage/interruption, victory, and debug departure.
- Version 199 town saves and version 196 exterior-road saves gain the east road
  while retaining existing progress and floor loot.
- The SDL application suite verifies all eight enemy sprites, texture sizes,
  actual input dispatch, gate style, and debug passage repairs.
- AddressSanitizer and UndefinedBehaviorSanitizer pass the targeted mine suite.

One repeated full SDL run encountered the existing intermittent Stormwatch
Matriarch spawn overlap. The final full run and standalone Ironfang checks pass;
this change does not alter Stormwatch spawning.

Crown Road North and the optional region north of the castle remain separate
follow-up work.
