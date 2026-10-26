# Giant's Peaks

The planned quest for this area is titled **Giant's Crown**.

Leave **Rosemoor through its west gate**, follow the single-tile road, and enter
the ruined giant hall. This optional five-stage region rises from grassy western
foothills into the frost-covered remains of a giant kingdom. Ashen Hollow lies
north of Castle Grounds, reached around the castle's western perimeter.

| Stage | Location | Exploration |
| --- | --- | --- |
| 1 | Terraced Foothills | Rounded grassy terraces, broken monuments, and several reconnecting approaches |
| 2 | The Fallen Village | Giant-sized stone houses and connected village courts |
| 3 | Broken Aqueduct | A ruined aqueduct divides the central court; both outer approaches reconnect |
| 4 | Winter Halls | Snow-covered outer courts surround the surviving stone halls |
| 5 | The Jarl's Court | A ruined royal precinct with pillars, flanking approaches, and the Last Jarl |

Southern boundary paths retreat; northern boundary paths advance. Intermediate
travel remains available while ordinary enemies live. The Last Jarl guards the
final departure, which returns to the western approach after victory.

| Enemy | First stage | Role |
| --- | --- | --- |
| Ogre | 1 | The region's weakest enemy; a slow, warned heavy strike |
| Cave Troll | 1 | Recurring Goblin Mountains and Ironfang bruiser |
| Hill Giant | 2 | Throws a boulder at a fixed, visibly warned target |
| Stone Giant | 3 | Recurring Dragonspine giant, with tougher regional statistics |
| Stone Troll | 3 | Recurring armored Stonewake creature |
| Frost Giant | 4 | Recurring Frostfell giant inhabiting the upper ruins |
| The Last Jarl | 5 | A giant ruler whose maul threatens a larger marked area |

**Stone Giant** replaces the former generic giant name throughout the game;
**Frost Giant** replaces Ice Giant. Their saved numeric type IDs are unchanged.
The new Ogre, Hill Giant, and Jarl artwork derives from the existing native
24-pixel giant silhouette. Existing enemies retain their established artwork.

## Heavy strikes and exploration

Heavy strikes lock their target when announced. Amber ground markings show
**2**, then **1**, before impact. Moving away avoids damage; the attack does not
follow the player. Each impact leaves two actions of recovery. Freeze pauses
the enemy's attack countdown. Idle animation does not advance combat.

Strikes shatter cracked masonry and freestanding pillars within their marked
area. Baiting an enemy beside a sealed chamber opens a new exploration route;
destroying pillars also removes cover. Permanent main routes remain accessible
without destroying anything. Enemies never spawn inside sealed treasure rooms.
Every class can also clear cracked masonry with **Interact**, so defeating a
nearby giant cannot make treasure inaccessible.

Use the configured **Interact** key (**A** by default) on or cardinally beside
a stone coffer to search it. Each pays **25 + 5 × stage** gold once. The Last
Jarl grants **200 gold and 2,200 score** once and appears in the boss journal.

## Expeditions and saves

Backtracking preserves wounds, deaths, floor loot, exploration, broken masonry,
and searched coffers. A fresh physical entry starts at stage one and respawns
ordinary enemies on visited stages while retaining discoveries, loot, and boss
victory. Return to Town goes to the last visited town; its portal resumes the
same stage and encounter without refreshing enemies.

Save version **203** stores all five compact stage caches, terrain, actors,
floor loot, pending attacks, and Jarl victory. Older saves gain the new region
and castle perimeter connection without resetting existing Ashen Hollow stages,
quests, inventory, character statistics, or boss progress. Saved active and
cached giant names normalize without changing their statistics or wounds.
