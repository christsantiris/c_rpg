# Dungeon Difficulty Scaling

The castle dungeon is a short, self-contained undead region. Future regions
such as the Haunted Forest maintain their own progression rather than extending
dungeon depth.

## Dungeon Structure

The dungeon contains five floors (`MAX_DEPTH`). New floors generate a map and
enemy roster. Previously visited floors are cached, preserving their map,
enemies, and cleared state.

The player must defeat every active enemy before using an exit. Floors 1-4 use
down stairs. Descending awards `new floor * 100` score.

Floor 5 is the Lich King's finale. Clearing it changes the down stairs into a
glowing return passage. Using that passage returns the player directly to the
north road in town, preserves dungeon progress, awards completion score, and
never creates a sixth dungeon floor.

The Lich King begins inside a sealed, single-entrance chamber behind a locked
door. No regular enemy can spawn in that room, and the Lich remains completely
dormant until the player unlocks the door and enters the chamber. A golden key
is always visible at the center of the penultimate room. Stand on the key and
press `P` to collect it; walking into the locked door then consumes the key and
opens the boss room. The player does not need to clear every regular enemy.

Every new character starts with a Scroll: Return to Town. Learning and casting
its zero-MP spell from the dungeon opens a portal in town and transports the
player there. Entering that portal returns the player to the exact dungeon tile
where the spell was cast, then closes the portal. This lets the player prepare
for the boss without replaying the dungeon route.

## Enemy Count

```text
enemy count = min(10 + dungeon floor, MAX_ENEMIES)
```

`MAX_ENEMIES` is 15:

| Floor | Regular enemies | Bosses | Total enemies |
| --- | ---: | ---: | ---: |
| 1 | 11 | 0 | 11 |
| 2 | 12 | 0 | 12 |
| 3 | 13 | 0 | 13 |
| 4 | 14 | 0 | 14 |
| 5 | 14 | 1 | 15 |

## Undead Composition

Goblins and the Goblin King are reserved for a future mountain region.

| Floor | Enemy distribution |
| --- | --- |
| 1 | 100% Skeleton |
| 2 | 50% Skeleton, 30% Zombie, 20% Crypt Bat |
| 3 | 30% Skeleton, 35% Zombie, 20% Crypt Bat, 15% Wraith |
| 4 | 20% Skeleton, 35% Zombie, 15% Crypt Bat, 20% Wraith, 10% Necromancer |
| 5 | 15% Skeleton, 30% Zombie, 15% Crypt Bat, 20% Wraith, 20% Necromancer, plus the Lich King |

| Enemy | HP | Attack | Defense | XP |
| --- | ---: | ---: | ---: | ---: |
| Skeleton | 10 | 3 | 0 | 8 |
| Zombie | 22 | 6 | 1 | 14 |
| Crypt Bat | 7 | 4 | 0 | 10 |
| Wraith | 18 | 7 | 2 | 22 |
| Necromancer | 24 | 7 | 1 | 35 |

Enemy roles:

- Skeletons are predictable melee pursuers.
- Zombies move every other enemy turn but are more durable.
- Crypt Bats move up to two tiles per turn but do not attack after their second
  movement step.
- Wraiths obey normal wall collision. Their melee attack ignores half of player
  defense and drains up to 3 MP.
- Necromancers fire a defense-piercing bolt along a clear orthogonal line every
  second turn. Every fourth turn they first attempt to revive a fallen Skeleton
  in an open tile beside them.

All enemies use adjacent melee attacks, including diagonally. Ordinary damage
is `attack - defense`, with a minimum of 1.

## Final Boss

| Floor | Boss | HP | Attack | Defense | XP |
| --- | --- | ---: | ---: | ---: | ---: |
| 5 | Lich King | 140 | 18 | 6 | 400 |

The Lich King is spawned before regular enemies, reserving its place under the
enemy cap. Once engaged, it holds position in the chamber and launches a
necrotic bolt every other turn, with a visible warning on its recovery turn.
It guarantees either a level-appropriate weapon or Chain Mail.

## Traps and Maps

```text
trap count = 2 + dungeon floor
```

This gives three traps on floor 1 and seven on floor 5. Placement can yield
fewer traps when a selected tile is unsuitable.

Layouts use 6-10 rooms with fixed room-size ranges. Floor number affects traps
and encounters, but not room geometry.

## Player Progression

On gaining a character level:

- Maximum HP increases by 10 and HP is restored.
- Attack increases by 2.
- Defense increases by 1, capped at half the player's attack.
- The next XP requirement becomes `player level * 100`.

Damage and healing spells add `player level * 2` to their base effect. Player
progress persists across future regions.

## Loot Progression

Non-boss enemies have a 20% gold-drop chance and a separate 5% item-drop
chance:

- Floors 1-3 emphasize potions and basic scrolls.
- Floors 4-5 introduce weapons, Leather Armor, healing, and rare Fireballs.
- The weapon pool expands to Short Swords, Long Swords, and Bows on floors 4-5.

## Current Curve Summary

Difficulty rises over five floors through increasing enemy count, new tactical
roles, and additional traps. Bats add speed, Wraiths undermine heavy armor and
mana reserves, and Necromancers create ranged pressure and target priority. Floor
5 combines the complete roster with the Lich King before returning the player
to town.
