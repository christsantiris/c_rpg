# Weapons

Weapons increase the player's attack while equipped. The game currently has
five melee weapons and one ranged weapon.

## Weapon Summary

| Weapon | Attack bonus | Buy value | Sell value | Ranged | Range | Two-handed flag |
| --- | ---: | ---: | ---: | --- | ---: | --- |
| Rusty Sword | +1 | 5 gold | 2 gold | No | - | No |
| Short Sword | +3 | 15 gold | 7 gold | No | - | No |
| Staff | +4 | 25 gold | 12 gold | No | - | Yes |
| Long Sword | +6 | 30 gold | 15 gold | No | - | No |
| Bow | +3 | 35 gold | 17 gold | Yes | 6 | No |
| Battle Axe | +10 | 50 gold | 25 gold | No | - | Yes |

Sell values are calculated using integer division as `value / 2`, so odd
values round down.

## Melee Weapons

Walking into an adjacent enemy performs a melee attack. Melee damage is:

```text
damage = max(1, player attack - enemy defense)
```

The equipped melee weapon's attack bonus is already included in the player's
attack value. Melee attacks play the standard attack sound effect.

### Rusty Sword

The weakest weapon, providing +1 attack. Warriors begin a new game with a
Rusty Sword in their inventory.

### Short Sword

A basic early-game upgrade providing +3 attack.

### Staff

Provides +4 attack and is marked as two-handed. Mages begin with a Staff in
their inventory. The Staff does not currently increase spell damage, reduce MP
costs, or otherwise modify casting.

### Long Sword

A strong general-purpose melee weapon providing +6 attack.

### Battle Axe

The strongest current melee weapon, providing +10 attack. It is marked as
two-handed.

## Bow

The Bow provides +3 attack and can fire up to six tiles in the player's last
movement direction. Rogues begin a new game with a Bow in their inventory.

Press `F` during play to fire an equipped ranged weapon. A valid Bow attack:

- Requires the Bow to be equipped.
- Requires a previous movement direction for aiming.
- Cannot be fired while any active enemy is adjacent, including diagonally.
- Has a minimum target distance of two tiles.
- Stops at the first enemy in its path.
- Cannot travel through walls or other non-walkable tiles.
- Has a 15% chance to deal 1.5 times normal damage.
- Does not consume ammunition.

Normal ranged damage is:

```text
damage = max(1, player attack - enemy defense)
```

Critical damage is calculated afterward using integer arithmetic:

```text
critical damage = normal damage * 3 / 2
```

The Bow's +3 bonus applies to ranged attacks. When the player bump-attacks an
adjacent enemy while the Bow is equipped, that bonus is removed from the melee
calculation. This keeps the Bow from functioning as a full-strength melee
weapon.

Bow attacks use a moving pixel-arrow effect and play
`assets/sfx/arrow.mp3` when a valid shot is fired.

## Equipping Weapons

1. Press `I` during play to open the inventory.
2. Select a weapon with the arrow keys.
3. Press `E` to equip it.
4. Press `I` or `Esc` to return to play.

Only one weapon can be equipped at a time. Equipping another weapon removes
the previous weapon's attack bonus before applying the new bonus. Dropping or
selling an equipped weapon also removes its bonus.

## Acquiring Weapons

The Blacksmith is the primary source of weapons and armor. Regular enemies do
not drop equipment; their item rolls contain only health potions, mana potions,
and spell scrolls. Existing gold-drop chances and amounts are unchanged.

Blacksmith inventory expands according to the number of regional bosses the
player has defeated:

| Bosses defeated | Blacksmith stock |
| ---: | --- |
| 0 | Basic weapons and armor |
| 1 | Uncommon weapons and armor |
| 2 | Rare equipment and the Magic Greatsword, Magic Staff, and Magic Longbow |
| 3-4 | Capstone armor |

Each regional boss instead guarantees one fixed thematic equipment reward:

| Boss | Reward |
| --- | --- |
| Lich King | Cryptblade |
| Forest Necromancer | Necromancer's Cloak |
| Goblin King | Goblin King's Greatsword |
| Drowned Queen | Tidecaller Robes |

Boss equipment is valuable but remains below the class capstone weapons. A
hero who cannot equip a boss reward can sell it to fund a Blacksmith purchase.
Treasure rooms and chests are planned as a later source of exploration rewards.

## Current Behavior Notes

- The two-handed flag is stored and saved for the Staff and Battle Axe, but it
  has no gameplay effect because the game has no hand or off-hand slots.
- Weapons do not have durability requirements.
- The Bow has unlimited ammunition; its balance comes from lower damage,
  limited range, minimum distance, and the adjacent-enemy firing restriction.
- Weapon eligibility is class-restricted. Equipment for another class can be
  retained and sold at the Blacksmith.
- Weapon attack bonuses affect the shared player attack statistic and continue
  to scale alongside attack gained from character levels.
