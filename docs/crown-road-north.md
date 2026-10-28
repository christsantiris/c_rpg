# Crown Road North

The northern castle connection runs from Oakhaven through Elderwood Forest and
Royal Catacombs, then across Crown Road North into Castle Grounds. Elderthorn
and the Grave Marshal gate their own sections from both directions; Crown Road
North has no additional boss gate. See [boss gates](boss-gates.md). The castle's south
gate enters the road from its northern end; its southern exit returns beside
the Catacombs landmark. The existing east and west Crown Roads retain their
routes and encounters.

This is one continuous outdoor road, with six occupied checkpoints and
reconnecting woodland trails. A single tile of stone paving marks the main
route. Barricades create firing lanes; side clearings and supply camps provide
alternate approaches. Both exits meet the map boundary and have destination
signs. Ordinary patrols do not lock an exit, so retreat and bypasses remain
possible.

The road is drawn at 1.5× zoom, the Royal Dungeon's tile height, so the camera
shows about 30 × 18 tiles around the player instead of 45 × 27. It is the first
encounter moved to the larger scale; `game_camera_zoom` in
`src/renderer/game_renderer.c` lists the zoomed areas.

## Patrols

Each checkpoint has fifteen enemies. The whole road contains 66 goblins and
24 wolves, using their existing sprites at their established scale.

| Enemy | Per checkpoint | Behavior |
| --- | ---: | --- |
| Hobgoblin Guard | 4 | Screens the ranged units. Supported guards commit to a facing when alerted and halve frontal weapon damage. Approach from the side or rear, freeze the guard, use spells, or eliminate its support. A cyan edge shows the shield facing. |
| Goblin Scout | 3 | Signals the formation and approaches from a flank. |
| Goblin Archer | 2 | Seeks a clear firing position, then marks a fixed square for one action before shooting. Sidestep the warning; the shot is followed by a recovery action. |
| Goblin Bomber | 1 | Marks a fixed cross-shaped blast for two actions. Walls and barricades clip the blast. Move twice to leave its center. |
| Goblin Shaman | 1 | Heals one wounded living squadmate by up to 10 HP every fourth active turn. Does not revive defeated enemies. |
| Blighted Wolf | 4 | Hunts as a pack, closes two tiles while distant, then occupies separate flanks before biting. Walkable terrain and other actors constrain every step. |

Spotting the player alerts the local formation. Distant formations stay at
their checkpoints until alerted. Freezing an enemy pauses its preparation and
recovery. Arrows and bombs keep their original targets while the player moves.

These patrols are tougher than their ordinary regional counterparts: after
shared progression scaling, regular enemies gain 40 HP, eight attack and three
defense, and grant twice their usual XP. The shorter castle route is intended
to be harder than Crown Road East or West. There is no additional boss or
quest requirement on this road.

## Exploration and replay

Six supply caches sit on the side trails. Defeat nearby guards, then use the
configured Interact key (`A` by default). A cache grants 25 gold and up to
20 HP once; its empty state persists.

Unlike other combat areas, the Crown Roads (North, East and West) never respawn
enemies. Patrols spawn on the first crossing; afterwards defeated enemies stay
defeated, survivors keep their wounds and positions, and a cleared road stays
clear. Physical re-entry cancels shots and bombs marked during the previous
crossing and moves any patrol off the entry tile. Exploration, claimed supplies,
and dropped equipment remain. Return
to Town is a one-way escape to the last town entered, with no return portal.

Save version **202** stores the road map, patrols, supplies, floor loot and
entry side. Older saves gain the route without resetting regional victories,
quests or equipment. Debug passage unlocks bypass the Elderwood and Catacombs
requirements without granting their victory credit.

[SDL renderer captures](art-sources/crown-road-north/README.md) show the patrols,
blast warning, boundary exit and renamed Dawnmere shop.
