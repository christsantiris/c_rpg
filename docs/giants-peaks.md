# Giant's Peaks

**Giant's Crown** is assigned by Historian Liora at the Sunscar caravan camp.
Scout Garran separately assigns **Break the Jarl's Blockade** there.

Giant's Peaks follows Sunscar Wastes. Beyond the Sunscar pyramid entrance, the
desert road turns north, signed **N: GIANT'S PEAKS**. It leaves
the map's north edge onto the Giant's Peaks road, which climbs from sand into
grass that grows rocky toward the peaks, passing the ruined giant hall.
Leaving the first stage, or completing the fifth, returns to that road outside
the southern entrance returns to the desert road. Leaving the northern court
reaches the road across Giant's Bridge, then bends east to Stillbury's west
gate. There is no exterior road bypassing the five stages.
Rosemoor's west gate no longer
leads here. This optional five-stage region rises from grassy western foothills
into the frost-covered remains of a giant kingdom.

The peaks are drawn at the Crown Roads' 1.5× zoom: the camera shows about
30 × 18 tiles around the player. Hill Giant boulders and the Last Jarl's maul
reach six tiles, so every warned strike starts on screen.

| Stage | Location | Exploration |
| --- | --- | --- |
| 1 | Terraced Foothills | Rounded grassy terraces, broken monuments, and several reconnecting approaches |
| 2 | The Fallen Village | Giant-sized stone houses and connected village courts |
| 3 | Broken Aqueduct | A ruined aqueduct divides the central court; both outer approaches reconnect |
| 4 | Winter Halls | Snow-covered outer courts surround the surviving stone halls |
| 5 | The Jarl's Court | A ruined royal precinct with pillars, flanking approaches, and the Last Jarl |

Southern boundary paths retreat; northern boundary paths advance. Intermediate
travel and retreat remain available while enemies live. Defeat the Last Jarl to
leave through the opposite end of the pass and open onward passage permanently.
The Jarl carries the quest crown. No quest acceptance is required to fight
him or open the bridge. See [boss gates](boss-gates.md).

From Stillbury, the bridge itself is freely crossable. The southern trail enters
the Jarl's Court at stage 5, facing south. Stage geography stays fixed: travelers
must descend through stages 4, 3, 2 and 1 to reach the Sunscar road. A living
Jarl blocks that final southern departure, while retreat north to Stillbury
remains available. From Sunscar, the reverse journey visits stages 1–5 before
the northern departure. Defeating the Jarl opens either departure, but both
journeys still cross all five stages.

Save version 239 records the expedition's entry side. Existing saves keep their
southern entry, cached stages, discoveries, loot, enemy wounds and boss victory.

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

## Giant's Crown

Liora at the caravan camp asks the player to recover the crown directly from the
Last Jarl and deliver it to **Antiquarian Edda beside the Stillbury end of
Giant's Bridge, outside the checkpoint**. No inscription, royal seal or vault
key is required. The crown is recovered automatically on his defeat, including
when the quest has not yet been accepted, and uses no pack space.

Edda grants **220 gold, 1,600 score, and a Stoneguard Shield** once. This Warrior
shield grants **+8 defense, 20% block chance, 50% blocked damage reduction, and
+30 maximum HP** while equipped. She reserves it if the pack is full.
Garran's boss quest completes immediately on the Jarl's defeat, with no camp
return. The Jarl's normal boss reward is unchanged. See [western quests](act3-western.md).

## Expeditions and saves

Backtracking preserves wounds, deaths, floor loot, exploration, broken masonry,
and searched coffers. A fresh physical entry starts at stage one and respawns
ordinary enemies on visited stages while retaining discoveries, loot, and boss
victory. Return to Town is a one-way escape to the last visited town and leaves
no return portal.

Save version **203** stores all five compact stage caches, terrain, actors,
floor loot, pending attacks, and Jarl victory. Older saves gain the new region
and castle perimeter connection without resetting existing Ashen Hollow stages,
quests, inventory, character statistics, or boss progress. Saved active and
cached giant names normalize without changing their statistics or wounds.

Save version **240** retains previous crown discoveries and claimed or reserved
rewards. The Jarl now supplies the crown directly; the old warded vault becomes
ordinary optional treasure with breakable masonry. Live and cached maps are
repaired in place, preserving loot and an escape route from an already open room.
