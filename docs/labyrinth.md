# Labyrinth: Slay the Minotaur

Stillbury's **north gate** leads to the Stonewake Quarry approach. An optional
stone doorway stands on a short western spur beside that road. Its entrance
and signs are visible without accepting a quest. The main quarry route and
retreat to Stillbury remain available while the Minotaur lives.

Rook waits in **Stillbury's Tavern**, at (10,18), and offers **Slay the Minotaur**.
He says the King's soldiers send prisoners into the maze, never to return;
townsfolk believe the Minotaur consumes them. Accept with Y or decline with N.
The usual three-active-quest limit applies.

## Objective and rewards

Explore five maze floors and slay the Minotaur on floor 5. An accepted quest
completes at the moment of victory, paying the existing **40 gold and 500 score**
once and freeing its quest slot. No relic, rune collection or return hand-in
is needed. If the boss was defeated before meeting Rook, accepting his offer
acknowledges that victory immediately without occupying an active slot.

The boss retains its guaranteed **Labyrinth Aegis**: rare Warrior-only shield,
**+9 defense, 25% block, value 1,250**. A full floor-loot list reserves it in the
bull-marked cache. Use Action beside it or Pick Up on it. Full inventory leaves
it available; saving, leaving and returning retain the unclaimed reward.
Claiming once prevents replacement after selling or discarding it.

Victory also reveals **Labyrinth Intake Register** in the Evidence journal.
Prisoners were transferred from Stillbury customs under royal authority, with
authorization from **Malakar, Royal Arcanist**. The register orders reports to
his private office and forbids copies to the royal court. This implicates him
without revealing the King's control. The evidence requires no inventory slot
and is available regardless of quest acceptance.

## Maze and travel

The existing five maze layouts and ordinary enemies remain. Each intermediate
floor has one real downstairs and one upstairs; floor 5 has its entrance and
Minotaur arena. The rune switches, locked rune gate, ivory rook and false-stair
detours are removed. No ordinary enemy or puzzle blocks retreat.

The maze uses its established 1.5x scrolling camera and 43x25 floor bounds.
Readable stair labels and discovered minimap markers identify the next floor
or **QUARRY ROAD / STILLBURY**. Physical retreat from floor 1 returns beside the
same doorway on the quarry approach. Return to Town retains its normal one-way
behavior to the last visited town.

## Minotaur combat

The existing encounter retains **120 base HP, 18 attack, 7 defense and 250 XP**,
with regional progression scaling. Above half health it alternates warned
charges and frontal axe sweeps. Below half health it uses paired charges and a
wider axe arc. Every signature leaves a recovery action without an extra
adjacent attack. Warnings retain their announced positions when the player
moves; freeze pauses their countdowns.

Baiting a charge into stone or either reinforced pillar gives two recovery
actions. A first pillar impact cracks it; a later Bloodied Bull impact breaks
it into walkable rubble. Ordinary walls, stairs and reward caches stay intact.
Pillar state and living boss wounds, warnings and positions persist on retreat
and re-entry. The existing boss victory and loot remain one time.

## Existing saves and scope

Save version **242** stores the new route and expanded evidence read mask using
existing quest, boss, reward and map state. Active and cached old mazes lose
only the retired puzzle and false-stair tiles; maps are repaired in place.
Exploration, enemy wounds/deaths, active floor loot, equipment and earned
victories remain. Historical rune fields and tile IDs are retained solely for
loading old saves. Old expeditions now retreat to Stillbury's quarry road.
Rook is restored in saved tavern interiors, and saved Stillbury exteriors gain
the north road without resetting exploration or loot.

The former Oakhaven/Haunted Forest entrance stays retired. No coastal-route,
Elderwood/Blackwater relocation or Elderwood story clue is implemented here.
The canonical README atlas shows the quarry road and optional labyrinth spur.
