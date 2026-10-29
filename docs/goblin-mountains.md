# Goblin Mountains

The Goblin Mountains stand beside a straight, single-tile cobblestone road
between **Dawnmere's north gate** and **Ridgeshire's south gate**. The road
climbs a grassy pass between rock walls that open into a meadow at its town end. Enter the
large fortified mountain cave from either town to start at **level 1** and
fight upward through five levels. Mountain levels never play in reverse from
Ridgeshire. Ordinary backtracking within a climb remains available.

The mountains are drawn at the Crown Roads' 1.5× zoom: the camera shows about 30
× 18 tiles around the player. The Goblin King acts within twelve tiles; the camera leans toward him to keep him on screen as his warned attacks begin.

The cave opens at the foot of a stepped red sandstone massif, flanked by a
leaning needle and a squat butte, sunlit orange on their western faces and
violet in shadow. The range
fills a tapered 7×9 tile footprint; save version **218** moves anyone saved on
its newly raised rows onto the road.

| Level | Region | Layout identity | Enemy roster |
| --- | --- | --- | --- |
| 1 | Blackrock Foothills | Open switchbacks and scout camps | Goblin Scouts |
| 2 | Raider Pass | Timber bridges and overlapping firing lanes | Scouts and Archers |
| 3 | Cinder Mines | Braided underground routes | Scouts, Archers, Warchiefs and Giant Spiders |
| 4 | Siege Camp | Split routes and fortified positions | Goblin warbands |
| 5 | Crown Peak | Summit fortress | Full roster and the Goblin King |

Intermediate levels do not require a full clear. The living Goblin King blocks
the level-5 summit exit. Defeat him, collect the loot, and use that exit to
emerge onto the road. Walk **north to Ridgeshire** or **south to Dawnmere**.
The through road opens permanently on victory. There is no mountain shortcut,
boss branch, travel menu, or direct teleport to town. Retreat at level 1 returns
to the cave approach for the town used to enter.

## Combat Roles

- **Goblin Scout:** basic melee pursuer.
- **Goblin Archer:** fires along clear rows and columns.
- **Goblin Warchief:** armored melee leader that advances with the warband.
- **Giant Spider:** armored ambusher whose bite applies poison.
- **Cave Troll:** slow, high-health bruiser.
- **Hobgoblin Guard:** heavily armored frontline defender.
- **Goblin Shaman:** periodically heals a nearby wounded ally.
- **Goblin King:** a stationary two-phase encounter with fixed axe lanes,
  marked bombardment pockets and finite powder kegs at Crown Peak.

The route geometry and ground treatment vary by level. Foothill and ascent
stages use exposed basalt switchbacks. Raider Pass contains
narrow timber bridge crossings. Cinder Mines uses dark
underground chambers, while Siege Camp and Crown Peak use paved
fortress courtyards and linked defensive positions.

## Terrain and Treasure

Stage 2 contains a weak bridge span that collapses after the player
crosses it. Stand beside the gap and press `A` to rebuild the bridge from either
side. The other mountain stages use open room routes without the old internal
toggle-gate chambers.

Stages 3–5 contain an optional rockfall passage. Press `A` beside either
end to expose it. The falling rocks deal `4 + difficulty` HP, but the chamber behind
them contains a one-time cache worth `15 + 4 × difficulty` gold. Difficulty follows
1, 2, 3, 4, 8, increasing toward the summit. Stand on the cache
and press `A` to collect it. Bridge repairs, opened passages, claimed caches,
and minimap discoveries persist while the expedition is cached and through
save/load.

## Mountain Quests

Dain assigns **Recover the Treasure Map** in Dawnmere's Alehouse. The
Archer, Warchief and Shaman Map Bearers occupy **levels 2, 3 and 4**, respectively,
with themed guards. Collected fragments do not use inventory space. Return all
three to Dain for **60 gold and 400 score**.

Marshal Veyra's **Watchfires of Ridgeshire** still spans three regions:
**Goblin Mountains 5**, **Ashen Hollow 3** and **Dragonspine 3**. Clear each
fire's defenders and press `A` beside it. The mountain fire occupies an ordinary
clearing before the King's fortress; killing the regional bosses is optional.

Return to Town is a one-way escape to the last town visited, independently of
the mountain entrance used. It leaves no return portal.

## Save Compatibility

Save version **171** moves Oakhaven mountain entrance and portal anchors to
Dawnmere. Saved mountain approaches become the new road. Legacy peak level 4
becomes level 5; level 5 becomes level 4; the former outer slopes merge into
levels 2 and 3, prioritizing occupied and portal-linked maps. Older eight-level
saves first pass through their existing migration. Wounded actors, active loot,
exploration, inventory, collected fragments, lit watchfires and boss victories
remain. Pending quest encounters move to their new levels. Retired shortcut
markers become ordinary floor. A portal already in Oakhaven remains usable
for that saved visit, so moving its future anchor cannot strand the player.

## Crown Peak Boss Encounter

The Goblin King keeps his original baseline 180 HP, 20 attack, 8 defense and
500 experience, with existing regional scaling. He remains dormant until the
player enters the fortress or damages him from range. He stays in the arena;
retreat beyond his range pauses a pending warning without healing him.

- **Fortress Chief, above half health:** thrown axes mark a fixed lane for one
  complete player action, then resolve and leave a recovery turn. Sidestep to
  dodge. Walls and living enemies can block the axe; removing a blocker never
  extends the warning beyond its original endpoint. Axes can light unlit kegs
  along their announced path.
- **Powder King, at/below half health:** alternate axe throws with two-turn
  bombardments. Up to two fixed floor pockets show every affected tile. Each
  pocket covers its center and four cardinal neighbors; two ordinary moves can
  escape. Bombs deal one hit and leave no lingering damage or blocked floor.
- Every signature has a recovery opening with no extra adjacent boss attack.
  Cross half health during an existing warning and that attack keeps its
  original shape; the new phase is announced before its next preparation.
  Freezing the King pauses his pending signature and preserves its targets.

Three finite powder kegs use native pixel art with wooden staves, iron hoops,
powder marks and burning fuses. Interact on or beside an unlit keg with contextual
**`A` by default** to prime it. No class, item, ammunition or quest is required.
The interaction's enemy phase leaves two complete player actions before the
blast. An axe also starts a two-turn fuse. Numbered outlines show the exact
**3×3 blast area**. Move two tiles from the center to get clear.

A keg blast damages the player for `max(4, 20 - defense / 2)` before normal
block/evasion checks, deals 30 damage to ordinary enemies and one-sixth of the
King's maximum health to him. Overlapping blasts hit each actor once per turn.
A blast catching the King cancels his current signature and grants two complete
recovery turns. Blasts also remove nearby incoming bomb pockets. Kegs remain
walkable, spent kegs remain spent, loot covering a keg is preserved, and a full
loot list cannot erase an earned boss reward. Active fuses continue while the
King is frozen; he waits for them before preparing another signature footprint.
His signature warnings and keg fuses display separately. If both affect a tile,
its label shows **attack turns / keg turns**.

Keg kills use ordinary enemy death, experience, quest and loot handling. A keg
victory still records the King's defeat, shield drop and road unlock.
Surviving ordinary enemies do not block the summit exit or the town road.

The guaranteed **Goblin King's Shield** retains +3 defense, 12% block and its
Warrior/Rogue restriction. It normally drops as floor loot. A full floor list
reserves it in a labeled fortress reliquary without deleting another item.
An earned, unclaimed shield is restored there after a floor refresh. Interact
beside the reliquary or press `P` on it; a full inventory leaves it available.
First claim is permanent, so later dropping or selling it creates no replacement.

Save version **126** stores shield claim state along with the existing saved
phase, attack countdown, targets and cached maps. Keg fuses and spent state are
saved as tiles, including item underlays. Older living encounters gain kegs in
place and restart old unmarked timings with a visible warning, preserving boss
health, deaths, player position, exploration, loot, portal anchors and quests.
Previous victories remain defeated; only a known current legacy floor shield
establishes an unclaimed reward.

See the [encounter screenshots and play-testing guide](art-sources/goblin-king-encounter/README.md).
