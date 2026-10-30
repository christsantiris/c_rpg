# Road scenery

The safe approach roads between towns and adventures pass through the terrain
of the region they lead to. Scenery is impassable and grows beyond a verge of
2–4 tiles. On most roads it fills the rest of the map; light woodland and
boulder fields leave open grass between them. Landmarks stand in clearings, and
the road opens into a wider clearing at the town it leaves. A road guarded by a
checkpoint narrows to a single-tile verge, with solid scenery, as it reaches the
checkpoint.

| Road | Scenery |
| --- | --- |
| Oakhaven ↔ Stillbury (Haunted Forest) | Haunted trees and grass verges |
| Oakhaven ↔ Castle Grounds (northern road) | Elderwood trees, matching Crown Road North; opens at both ends |
| Stillbury ↔ Rosemoor (Blackwater Swamp) | Bog and dead trees; mud verges |
| Dawnmere ↔ Ridgeshire (Goblin Mountains) | Rock walls and a grassy pass |
| Ridgeshire to Dragonspine and Ironfang Mines | Rock walls and a grassy pass |
| Rosemoor to Frostfell Wastes | Green woods up to a ragged tree line, then snowy ice fields with pines; snow settles along the path first |
| Stillbury to Sunscar Wastes | Green woods, then sandstone outcrops, then sand and mesas with cacti |
| Castle Grounds to Ashen Hollow | Green woods, ash on the verge, charred dead trees, then basalt split by lava pools |
| Oakhaven ↔ Dawnmere (Dungeon) | Light woodland: clumps of Elderwood trees in open grass |
| Stillbury to Stonewake Quarry | Open grass gathering stone flecks and a few boulders near the quarry |
| Dawnmere to Stormwatch Headland | Grass turning to gravel strewn with boulders near the headland |
| Rosemoor to Giant's Peaks | Open grass gathering stone flecks and a few boulders near the peaks, like Stonewake |

The Frostfell, Sunscar, Ashen, Stonewake, Giant's Peaks and Stormwatch roads change gradually
from the town toward the landmark; the landmark's terrain continues beyond it.
Patches of the next terrain mix into the previous one, except that Frostfell's
woods end in a single tree line at the frozen ground. Snow, sand, ash or
stone first appears as flecks on the grass. Except near Stonewake and Giant's
Peaks, the ground then turns over completely. That ground is drawn over ordinary road-side grass, so no regional
floor hazards apply on the road, and boulders are scenery rather than Dragonspine's
pushable boulders. The Sunken Coast road stays open grass with its path and
shore.

`approach_grow_scenery()` in `src/game/approach.c` grows the scenery after each
road's layout is built, and `approach_scenery()` lists each road's scenery by
progress from the town (0) to the landmark (100). Every open tile stays
reachable from the road.

## Saves

A save made on one of these roads before its scenery existed still has open
grass. Loading it rebuilds the road with its scenery and keeps exploration, money and quests.
A player or loot inside the new scenery moves back onto the road. No save
version change is needed: a road without any scenery is detected on load.
