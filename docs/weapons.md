# Weapons, Armor, and Shields

Equipment is class-oriented and uses main-hand, off-hand, and armor slots.
Regular enemies never drop equipment. The Blacksmith and regional
bosses are the equipment sources.

## Equipment Rules

- Press `E` or `Enter` on a weapon in the inventory to equip it in the main hand.
- Press `E` or `Enter` on armor to equip it in the armor slot.
- Press `O` on a one-handed weapon to equip it in the off hand.
- Press `E` or `Enter` on a shield to equip it in the off hand.
- A one-handed main weapon may be paired with one one-handed weapon or shield.
- Two-handed weapons clear the off hand and prevent off-hand equipment.
- Removing a main weapon promotes an off-hand weapon to the main hand.
- Removing a main weapon also removes a shield because shields require a
  one-handed main weapon.
- Class restrictions apply in both hand slots and the armor slot.

An off-hand weapon contributes half its attack bonus, rounded up, and half its
critical chance. Its range, cleave, armor penetration, spell bonuses, and other
traits do not apply from the off hand.

## Workshop Sharpening

Enter the workshop in Ridgeshire and press your configured Talk key (default
`T`) while beside Garrick. His service screen lists your inventory.
Use Up/Down or click to select a weapon, then Enter or the sharpening button to
pay **50 gold** for **+1 weapon attack**. Press Esc to return to the workshop.

Swords (including greatswords), axes, and daggers qualify. Bows, staves, armor,
and other items do not. Each individual weapon can be sharpened only once;
another copy of the same weapon has its own allowance. Already sharpened weapons
are marked in the service screen and inventory. Insufficient funds and invalid
selections never charge gold.

An equipped main-hand weapon immediately adds 1 to displayed attack. Off-hand
weapons retain the normal half-attack contribution, rounded up, so their +1
weapon improvement can add either 0 or 1 to total attack. Unequipping and
re-equipping uses the upgraded bonus without stacking it again.

Sharpening does not change a weapon's name, class restrictions, traits, or shop
value. The improvement and its spent allowance remain attached when dropping,
picking up, saving, or restoring castle floor loot. Older saves initialize all
existing weapons as unsharpened without changing their attack or player progress.
The workshop offers this service; the Blacksmith buys and sells equipment.

## Weapon Catalog

The value column is the item's base value. The Blacksmith charges twice this
amount and pays one quarter when buying it back.

### Melee weapons by attack

Sorted by the weapon's base attack bonus, before sharpening. The bonus adds to
the character's attack; critical hits, cleave, and armor penetration also affect
actual damage. Staves are included because they strike in melee. The Demonic
Sword also has a ranged attack.

| Weapon | Attack | Hands | Class | Tier | Base value | Trait |
| --- | ---: | --- | --- | ---: | ---: | --- |
| Rusty Sword | +1 | One | Warrior | 1 | 10 | — |
| Dagger | +2 | One | Rogue | 1 | 90 | 25% critical chance |
| Short Sword | +3 | One | Warrior, Rogue | 1 | 40 | — |
| Staff | +4 | Two | Mage | 1 | 60 | +2 spell power, +10 maximum MP |
| Cryptblade | +5 | One | Warrior | Boss | 350 | 10% critical chance; Lich King reward |
| Demonic Sword | +6 | One | All | Boss | 180 | Range 2 with `F`; Swamp Demon reward |
| Long Sword | +6 | One | Warrior | 2 | 180 | — |
| Magic Dagger | +6 | One | Rogue | 3 | 500 | 40% critical chance |
| Runed Staff | +6 | Two | Mage | 2 | 300 | +4 spell power, +20 maximum MP, 5% cheaper spells |
| Sandstorm Staff | +7 | Two | Mage | Boss | 700 | +6 spell power, +25 maximum MP, 7% cheaper spells; Desert Pharaoh reward |
| Magic Staff | +9 | Two | Mage | 3 | 1,000 | +8 spell power, +35 maximum MP, 10% cheaper spells |
| Battle Axe | +10 | Two | Warrior | 2 | 275 | Ignores 25% of enemy defense |
| Magic Long Sword | +10 | One | Warrior | 3 | 550 | 20% critical chance |
| Greatsword | +12 | Two | Warrior | 2 | 400 | 50% cleave damage to adjacent enemies |
| Magic Battle Axe | +13 | Two | Warrior | 3 | 700 | Ignores 50% of enemy defense |
| Magic Greatsword | +18 | Two | Warrior | 3 | 1,100 | 75% cleave damage to adjacent enemies |

The Cryptblade's +5 attack and 10% critical chance make it an early Lich King
reward with room for later upgrades. Its one-handed grip permits a shield or
off-hand weapon. Existing saves update its attack while preserving sharpening
(+6 attack when sharpened). The Goblin King drops the Goblin King's Shield;
saved copies of his retired greatsword are converted to that shield.

### Ranged bows

Bows are excluded from the melee ranking: their attack bonus is removed when
striking adjacent enemies in melee.

| Weapon | Ranged attack | Hands | Class | Tier | Base value | Trait |
| --- | ---: | --- | --- | ---: | ---: | --- |
| Bow | +3 | Two | Rogue | 1 | 60 | Range 6 |
| Longbow | +7 | Two | Rogue | 2 | 250 | Range 9 |
| Krakenbone Bow | +10 | Two | Rogue | Boss | 700 | Range 10; Polar Kraken reward |
| Magic Longbow | +13 | Two | Rogue | 3 | 1,200 | Range 12 and pierces every target in its path |

Bows fire with `F` in the last movement direction. A target directly adjacent
to the player is too close; valid targets begin two tiles away. Arrows stop at
walls. The Bow and Longbow stop at the first target, while the Magic Longbow
can strike several targets on the same line. All bows have a fixed 15% ranged
critical chance.

Rogues start with 100 arrows and can carry at most 100 in a shared quiver. Each
fired bow shot consumes one arrow, including misses and shots stopped by walls.
A piercing arrow consumes only one regardless of how many enemies it hits.
Rejected shots (no aim, no equipped bow, or an adjacent target) consume none.
At zero arrows, ranged bow attacks stop; buy more or equip a melee weapon such
as a dagger. Melee attacks, spells, and the Demonic Sword do not use arrows.
If no dagger is available, an empty bow still permits melee hits against adjacent
enemies, using the existing melee damage without the bow's attack bonus.

The Blacksmith stocks 20-arrow bundles for 10 gold at every stock tier. Purchases
go directly into the quiver, including with a full inventory. If fewer than 20
spaces remain, only the missing arrows are purchased at a proportional price
rounded up; a full quiver cannot be charged. The character panel shows the
remaining count. Switching bows, travel, and saving never refill the quiver.
Older Rogue saves receive 100 arrows once when migrated to save version 103,
with all existing progress retained.

The Demonic Sword also uses `F`, but its magic reaches one or two tiles, stops
at the first enemy or wall, and costs no mana. It keeps its full +6 attack bonus
in melee.

## Armor Catalog

| Armor | Defense | Class | Tier | Base value | Trait |
| --- | ---: | --- | ---: | ---: | --- |
| Leather Armor | +2 | All | 1 | 60 | — |
| Chain Mail | +3 | Warrior | 1 | 80 | — |
| Apprentice Robes | +1 | Mage | 1 | 60 | +10 maximum MP |
| Scale Mail | +5 | Warrior | 2 | 275 | +10 maximum HP |
| Studded Leather | +4 | Rogue | 2 | 250 | 5% evasion |
| Runed Robes | +2 | Mage | 2 | 275 | +25 maximum MP |
| Plate Armor | +8 | Warrior | 3 | 650 | +25 maximum HP |
| Ranger Cloak | +5 | Rogue | 3 | 600 | 10% evasion |
| Enchanter Robes | +3 | Mage | 3 | 675 | +40 maximum MP, 10% cheaper spells |
| Magic Plate | +11 | Warrior | 4 | 1,200 | +50 maximum HP |
| Shadow Armor | +7 | Rogue | 4 | 1,100 | 15% evasion |
| Archmage Robes | +4 | Mage | 4 | 1,250 | +60 maximum MP, 20% cheaper spells |
| Necromancer's Cloak | +5 | Rogue | Boss | 450 | 8% evasion; Necromancer reward |
| Tidecaller Robes | +3 | Mage | Boss | 500 | +30 maximum MP, 8% cheaper spells; Drowned Queen reward |
| Dragon Scale Mantle | +4 | All | Boss | 1,400 | +20 maximum HP; Red Dragon reward |

Evasion is checked before damage is applied. Staff and robe casting discounts
stack, up to a combined reduction of 50%.

## Shields

Every successful shield block reduces the remaining damage by 50%. Shields are
usable by the listed classes and require a one-handed main weapon.

| Shield | Defense | Block chance | Class | Tier | Base value |
| --- | ---: | ---: | --- | ---: | ---: |
| Buckler | +2 | 10% | Warrior, Rogue | 1 | 75 |
| Kite Shield | +4 | 15% | Warrior | 2 | 300 |
| Tower Shield | +7 | 20% | Warrior | 3 | 700 |
| Magic Shield | +9 | 25% | Warrior | 3 | 1,250 |
| Goblin King's Shield | +5 | 15% | Warrior, Rogue | Boss | 500 |

## Blacksmith Progression and Boss Rewards

The Blacksmith starts at Tier 1. Defeating one regional boss unlocks Tier 2,
two bosses unlock Tier 3, and three bosses unlock Tier 4 armor. The Blacksmith
buys only weapons, armor, and shields; potions, scrolls, and tomes belong at the
Alchemist.

Each of the five regional bosses guarantees a fixed thematic item:

| Boss | Reward |
| --- | --- |
| Lich King | Cryptblade |
| Necromancer | Necromancer's Cloak |
| Goblin King | Goblin King's Shield |
| Drowned Queen | Tidecaller Robes |
| Swamp Demon | Demonic Sword |

The Fallen Sun Guardian protects the buried temple treasure and does not drop
equipment. Boss victories remain permanent during repeat expeditions.

### Gravekeeper's Mantle

The Grave Marshal in the Royal Catacombs drops this rare armor for every class.
It grants +6 defense and +20 maximum health. Extinguish the boss chamber's two
braziers to remove its damage protection. An unclaimed mantle remains
recoverable on returning to the final catacomb floor.
