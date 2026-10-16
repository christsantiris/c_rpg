# Goblin Mountains

The Goblin Mountains connect OakHaven's north gate and Ridgeshire's south mountain
gate and use an independent seven-level cache. Entering from OakHaven starts at
level 1; entering from Ridgeshire starts at level 7. Both routes climb toward
the Goblin King at Crown Peak on level 4, then descend in difficulty toward the
opposite town. Their black basalt, iron-red rock and ember-orange accents
separate the region visually from the dungeon and Haunted Forest. The Mountains
use seven authored route graphs rather than the Forest's clearing templates.

Wild Boars are deliberately excluded. The area is about an organized goblin
war host, its siege craft, guarded mines and enslaved cave creatures.

| Level | Region | Layout identity | Enemy roster |
| --- | --- | --- | --- |
| 1 | Blackrock Foothills | Open switchbacks and scout camps climbing to a north exit | Goblin Scouts |
| 2 | Raider Pass | Overlapping firing lanes leading east | Scouts and Goblin Archers |
| 3 | Cinder Mines | Braided mine routes descending to a south exit | Scouts, Archers, Bombers and Tunnel Spiders |
| 4 | Crown Peak | Twin approaches into the summit fortress and a southern descent | Full roster, Goblin Shamans and the Goblin King |
| 5 | Siege Camp | Split routes through abandoned positions; south pass | Scouts, Archers, Bombers and Tunnel Spiders |
| 6 | Collapsed Tunnels | Broken routes and side chambers leading east | Scouts and Goblin Archers |
| 7 | Ridgeshire Approach | Timber crossings and scout camps near the town | Goblin Scouts |

Approach stages do not require a full clear. Reaching the marked boundary pass
is enough to advance, whether it lies east, north or south. At the peak, only
the Goblin King blocks crossing to the other approach; retreat remains possible.
His defeat prompts discovery of a marked shortcut beside the defeated boss to
the opposite town. Press Enter to acknowledge the prompt; movement cannot
dismiss it, matching the forest and swamp. Players
can take it immediately or fight through the three remaining stages.

## Combat Roles

- **Goblin Scout:** basic melee pursuer.
- **Goblin Archer:** fires along clear rows and columns.
- **Goblin Bomber:** launches high-damage explosives along clear firing lines.
- **Tunnel Spider:** armored ambusher whose bite applies poison.
- **Cave Troll:** slow, high-health bruiser.
- **Hobgoblin Guard:** heavily armored frontline defender.
- **Goblin Shaman:** periodically heals a nearby wounded ally.
- **Goblin King:** a stationary two-phase encounter with fixed axe lanes,
  marked bombardment pockets and finite powder kegs at Crown Peak.

The route geometry and ground treatment vary by level. Foothill and ascent
stages use exposed basalt switchbacks. Raider Pass and Ridgeshire Approach contain
narrow timber bridge crossings. Cinder Mines and Collapsed Tunnels use dark
underground chambers, while Siege Camp and Crown Peak use paved
fortress courtyards and linked defensive positions.

## Terrain and Treasure

Stages 2 and 7 contain a weak bridge span that collapses after the player
crosses it. Stand beside the gap and press `A` to rebuild the bridge from either
side. The other mountain stages use open room routes without the old internal
toggle-gate chambers.

Stages 3–6 contain an optional rockfall passage. Press `A` beside either
end to expose it. The falling rocks deal `4 + difficulty` HP, but the chamber behind
them contains a one-time cache worth `15 + 4 × difficulty` gold. Difficulty follows
1, 2, 3, 8, 3, 2, 1, so hazards and rewards also decrease away from the peak. Stand on the cache
and press `A` to collect it. Bridge repairs, opened passages, claimed caches,
and minimap discoveries persist while the expedition is cached and through
save/load.

## Mountain Quest

Speak to Dain with `T` in **Ridgeshire's town center** to accept
**Recover the Treasure Map**. His quest places guarded Map Bearers on stages
**7, 6, and 5**, entered through Ridgeshire's south mountain gate. The Archer,
Bomber, and Shaman leaders respectively carry one fragment and travel with a themed warband. Defeating the
leader recovers the fragment automatically. All three objectives precede the
stage-4 Goblin King from Ridgeshire; entering from Oakhaven finds them after him.
Accepting starts a fresh mountain expedition, while a previously defeated
Goblin King remains dead and permanent shortcuts stay open. After victory,
take the peak shortcut to Oakhaven and the unlocked town roads home, or battle
through stages 3, 2, and 1.

Save version **113** preserves collected fragments and explored maps. Old
bearers on incorrect stages become regular enemies with their existing health,
positions, and death state. Missing Archer and Bomber fragments gain bearers on
stages 7 and 6 when visited; the Shaman remains on stage 5. Loot, boss victories,
and portal anchors remain intact.

Return all three fragments to Dain in Ridgeshire's town center for the one-time
reward of **60 gold and 400 score**.

Ridgeshire's east gate enters [Dragonspine](dragonspine.md) directly. Ilya
assigns its goblet quest in Oakhaven's Tavern. After the Goblin King's defeat, a separate south shortcut gate
at x=28 follows the safe High Pass to OakHaven's northeast road. Ridgeshire's
ordinary south mountain gate at x=20 stays open before and after the victory.
Return to Town anchors to the expedition's entry town, and its return portal
keeps that anchor if the mountains are later entered from the opposite town.

## Save Compatibility

Older saves migrate automatically. The old level 8 summit moves to level 4;
the former level 4 and level 7 approaches merge into level 7, preserving the
occupied or portal-linked map when needed. Character progress, boss victories,
partial map fragments, active loot, and portal coordinates remain intact. Old
map bearers become regular enemies, and missing fragments gain bearers on their
new stages when visited. Existing shortcut entrances relocate beside the saved
boss death position on load. Saving writes the current format with both mountain travel anchors.

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
victory still records the King's defeat, shield drop and shortcut discovery.
Surviving enemies do not block the mountain shortcut or either travel route.

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
