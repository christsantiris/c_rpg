# Regional Difficulty Scaling

The castle dungeon is a short, self-contained undead region. The Haunted Forest,
Goblin Mountains, and Sunken Coast maintain their own progression rather than
extending dungeon depth.

## Region Order Scaling

New enemies scale from the number of **other** regional bosses defeated. This
order tier ranges from 0 for the first region to 3 for the fourth. Each region
keeps its own eight-stage progression and encounter count; the tier changes
enemy strength when a floor is generated, not on every player level-up.

| Prior bosses | Regular enemy HP | Regular attack | Boss HP | Boss attack | Enemy XP |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | Base | Base | Base | Base | Base |
| 1 | +45% | +7 | +30% | +6 | +10% |
| 2 | +90% | +14 | +60% | +12 | +20% |
| 3 | +135% | +21 | +90% | +18 | +30% |

Each tier also adds 1 defense. The one-HP Illusion remains at one HP. Player
level adds a smaller step at levels 9 and 17, capped there: each step adds 10%
HP and 2 attack. Every region draws its regular enemy mix from three stages
deeper per prior boss, without changing the actual map stage or encounter count.
Gold drops, equipment rewards, and traps are unchanged.

The scaled stats are stored on each spawned enemy. Cached floors and saved
enemies retain their stats when revisited or loaded; generating a fresh floor
uses the boss victories and player level at that time.

## Dungeon Structure

The dungeon contains eight floors (`DUNGEON_DEPTH`). New floors generate a map and
enemy roster. Previously visited floors are cached, preserving their map,
enemies, and cleared state.

Floors 1-7 use down stairs and do not require every enemy to be defeated.
Descending awards `new floor * 100` score.

Floor 8 is the Lich King's finale. Clearing it changes the down stairs into a
glowing return passage. Using that passage returns the player directly to the
north road in town, preserves dungeon progress, awards completion score, and
never creates a ninth dungeon floor.

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
| 5 | 15 | 0 | 15 |
| 6 | 15 | 0 | 15 |
| 7 | 15 | 0 | 15 |
| 8 | 14 | 1 | 15 |

## Undead Composition

Goblins and the Goblin King are reserved for a future mountain region.

| Floor | Enemy distribution |
| --- | --- |
| 1 | 100% Skeleton |
| 2 | 50% Skeleton, 30% Zombie, 20% Crypt Bat |
| 3 | 30% Skeleton, 35% Zombie, 20% Crypt Bat, 15% Wraith |
| 4 | 20% Skeleton, 35% Zombie, 15% Crypt Bat, 20% Wraith, 10% Crypt Conjurer |
| 5–7 | 15% Skeleton, 30% Zombie, 15% Crypt Bat, 20% Wraith, 20% Crypt Conjurer |
| 8 | 15% Skeleton, 30% Zombie, 15% Crypt Bat, 20% Wraith, 20% Crypt Conjurer, plus the Lich King |

| Enemy | HP | Attack | Defense | XP |
| --- | ---: | ---: | ---: | ---: |
| Skeleton | 10 | 3 | 0 | 8 |
| Zombie | 22 | 6 | 1 | 14 |
| Crypt Bat | 7 | 4 | 0 | 10 |
| Wraith | 18 | 7 | 2 | 22 |
| Crypt Conjurer | 24 | 7 | 1 | 35 |

Enemy roles:

- Skeletons are predictable melee pursuers.
- Zombies move every other enemy turn but are more durable.
- Crypt Bats move up to two tiles per turn but do not attack after their second
  movement step.
- Wraiths obey normal wall collision. Their melee attack ignores half of player
  defense and drains up to 3 MP.
- Crypt Conjurers fire a defense-piercing bolt along a clear orthogonal line every
  second turn. Every fourth turn they first attempt to revive a fallen Skeleton
  in an open tile beside them.

All enemies use adjacent melee attacks, including diagonally. Ordinary damage
is `attack - defense`, with a minimum of 1.

## Final Boss

| Floor | Boss | HP | Attack | Defense | XP |
| --- | --- | ---: | ---: | ---: | ---: |
| 8 | Lich King | 140 | 18 | 6 | 400 |

The Lich King is spawned before regular enemies, reserving its place under the
enemy cap. Once engaged, it holds position in the chamber and launches a
necrotic bolt every other turn, with a visible warning on its recovery turn.
It guarantees either a level-appropriate weapon or Chain Mail.

## Traps and Maps

```text
trap count = 2 + dungeon floor
```

This gives three traps on floor 1 and ten on floor 8. Placement can yield
fewer traps when a selected tile is unsuitable.

Ordinary traps stay hidden until stepped on. Visible plates are reserved for
reward or route choices, such as the mountain fort gate with its alternate
buried passage. Previously revealed traps in saved games remain visible.

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

Non-boss enemies have a 10% gold-drop chance and a separate 5%
item-drop chance. Item drops contain only health potions, mana potions, and
spell scrolls; regular enemies never drop weapons or armor. Deeper stages shift
the scroll mix toward Heal and Fireball.

Shop purchase prices are 200% of an item's base value. Shops pay
25% of base value, rounded down, when buying an item from the player. These prices apply to
both Blacksmith and Alchemist stock; existing item values in saved games stay
unchanged.

Regional bosses guarantee fixed thematic equipment rewards. Boss victories
also expand the Blacksmith's inventory. Uncommon stock unlocks after one boss,
the strongest class weapons unlock after two bosses, and capstone armor unlocks
after three bosses. Enemy coin amounts, including boss purses, are halved and
rounded down. The four quests award 40, 60, 70, and 80 gold respectively.
Crypt caches grant `10 + 2 × floor` gold, mountain caches grant
`15 + 4 × stage`, and Coast caches grant `20 + 4 × stage`.

## Current Curve Summary

Difficulty rises over eight floors through increasing enemy count, new tactical
roles, and additional traps. Bats add speed, Wraiths undermine heavy armor and
mana reserves, and Crypt Conjurers create ranged pressure and target priority. Floor
8 combines the complete roster with the Lich King before returning the player
to town.
