# Difficulty Scaling

This document describes the current difficulty progression as implemented. The
dungeon floor and the player's character level are separate values and scale
different parts of the game.

## Dungeon Floors

The dungeon contains 25 floors (`MAX_DEPTH`). Entering a new, uncached floor
generates a new map and enemy roster. Previously visited floors are cached, so
returning to one restores its map, enemies, and cleared state instead of
generating a newly scaled version.

The player must defeat every active enemy on the current floor before using the
stairs down. Descending awards `floor * 100` score after advancing to the next
floor.

## Enemy Count

Each floor requests:

```text
enemy count = min(10 + dungeon floor, MAX_ENEMIES)
```

`MAX_ENEMIES` is currently 15, giving the following intended counts:

| Floors | Regular enemies | Bosses | Total enemies |
| --- | ---: | ---: | ---: |
| 1 | 11 | 0 | 11 |
| 2 | 12 | 0 | 12 |
| 3 | 13 | 0 | 13 |
| 4 | 14 | 0 | 14 |
| Non-boss floors 5-25 | 15 | 0 | 15 |
| Boss floors 5, 10, 15, 20, 25 | 14 | 1 | 15 |

## Enemy Composition

Enemy stats do not receive a per-floor multiplier. Difficulty increases by
replacing early creatures with stronger enemy types.

| Floors | Enemy distribution |
| --- | --- |
| 1-2 | 60% Skeleton, 40% Goblin |
| 3-4 | 30% Skeleton, 40% Goblin, 30% Zombie |
| 5-6 | 20% Goblin, 40% Zombie, 40% Orc |
| 7-8 | 20% Zombie, 40% Orc, 40% Troll |
| 9-10 | 20% Orc, 40% Troll, 40% Giant |
| 11-25 | 50% Troll, 50% Giant |

Regular enemy base stats are fixed:

| Enemy | HP | Attack | Defense | XP |
| --- | ---: | ---: | ---: | ---: |
| Skeleton | 10 | 3 | 0 | 8 |
| Goblin | 15 | 4 | 1 | 10 |
| Zombie | 22 | 6 | 1 | 14 |
| Orc | 25 | 7 | 2 | 20 |
| Troll | 40 | 10 | 4 | 30 |
| Giant | 60 | 14 | 6 | 50 |

Zombies move every other enemy turn. Other regular enemies move every turn.
All enemies attack when adjacent to the player, including diagonally. Damage is
`attack - defense`, with a minimum of 1.

## Boss Floors

Bosses are intended to spawn every five floors:

| Floor | Boss | HP | Attack | Defense | XP |
| --- | --- | ---: | ---: | ---: | ---: |
| 5 | Goblin King | 100 | 15 | 5 | 200 |
| 10 | Lich King | 200 | 25 | 10 | 400 |
| 15 | Demon Lord | 350 | 38 | 15 | 700 |
| 20 | Red Dragon | 500 | 55 | 22 | 1,200 |
| 25 | Tarrasque | 800 | 80 | 35 | 2,000 |

Bosses guarantee an equipment drop. The drop is either a level-appropriate
weapon or chain mail.

Bosses are spawned before regular enemies, reserving their place in the floor's
enemy capacity. Regular enemies then fill the remaining slots up to
`MAX_ENEMIES`, so every boss floor contains its scheduled boss.

## Traps and Map Generation

Each newly generated floor requests:

```text
trap count = min(2 + dungeon floor, 12)
```

This ranges from three traps on floor 1 to the maximum of 12 from floor 10
onward. Placement can yield fewer traps if a chosen tile is unsuitable.

The dungeon layout currently does not scale by floor. `map_generate()` accepts
the floor number but only uses it for trap count. Every floor otherwise uses
the same ranges of 6-10 rooms, room dimensions, and map dimensions.

## Player Progression

Enemies award fixed XP based on type. On gaining a character level:

- Maximum HP increases by 10.
- HP is completely restored.
- Attack increases by 2.
- Defense increases by 1, capped at half of the player's attack.
- The next XP requirement becomes `player level * 100`.

Damage and healing spells add `player level * 2` to their base effect. Player
level is capped at 50.

## Loot Progression

Non-boss enemies have a 20% gold-drop chance and a separate 5% item-drop
chance. Item tables change at floors 4 and 7:

- Floors 1-3 emphasize potions and basic scrolls.
- Floors 4-6 introduce weapons, leather armor, healing, and rare fireballs.
- Floors 7-25 increase weapon and fireball availability.

Weapon selection similarly advances from basic swords on floors 1-3, expands
on floors 4-6, and uses the full weapon pool from floor 7 onward.

## Current Curve Summary

Difficulty climbs most strongly through floor 10 due to rising enemy count,
stronger enemy composition, and additional traps. From floor 11 onward, the
regular encounter curve plateaus at 15 Trolls/Giants and 12 traps. Boss floors
are intended to provide the remaining major difficulty spikes, subject to the
known enemy-cap issue above.
