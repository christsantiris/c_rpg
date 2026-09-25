# Weapons, Armor, and Shields

Equipment is class-oriented and uses main-hand, off-hand, and armor slots.
Regular enemies never drop equipment. The Blacksmith and the four regional
bosses are the equipment sources.

## Equipment Rules

- Press `E` on a weapon in the inventory to equip it in the main hand.
- Press `E` on armor to equip it in the armor slot.
- Press `O` on a one-handed weapon to equip it in the off hand.
- Press `E` on a shield to equip it in the off hand.
- A one-handed main weapon may be paired with one one-handed weapon or shield.
- Two-handed weapons clear the off hand and prevent off-hand equipment.
- Removing a main weapon promotes an off-hand weapon to the main hand.
- Removing a main weapon also removes a shield because shields require a
  one-handed main weapon.
- Class restrictions apply in both hand slots and the armor slot.

An off-hand weapon contributes half its attack bonus, rounded up, and half its
critical chance. Its range, cleave, armor penetration, spell bonuses, and other
traits do not apply from the off hand.

## Weapon Catalog

The value column is the item's base value. The Blacksmith charges twice this
amount and pays one quarter when buying it back.

| Weapon | Attack | Hands | Class | Tier | Base value | Trait |
| --- | ---: | --- | --- | ---: | ---: | --- |
| Rusty Sword | +1 | One | Warrior | 1 | 10 | — |
| Short Sword | +3 | One | Warrior, Rogue | 1 | 40 | — |
| Staff | +4 | Two | Mage | 1 | 60 | +2 spell power, +10 maximum MP |
| Bow | +3 | Two | Rogue | 1 | 60 | Range 6 |
| Dagger | +2 | One | Rogue | 1 | 90 | 25% critical chance |
| Long Sword | +6 | One | Warrior | 2 | 180 | — |
| Battle Axe | +10 | Two | Warrior | 2 | 275 | Ignores 25% of enemy defense |
| Greatsword | +12 | Two | Warrior | 2 | 400 | 50% cleave damage to adjacent enemies |
| Longbow | +7 | Two | Rogue | 2 | 250 | Range 9 |
| Runed Staff | +6 | Two | Mage | 2 | 300 | +4 spell power, +20 maximum MP, 5% cheaper spells |
| Magic Long Sword | +10 | One | Warrior | 3 | 550 | 20% critical chance |
| Magic Battle Axe | +13 | Two | Warrior | 3 | 700 | Ignores 50% of enemy defense |
| Magic Dagger | +6 | One | Rogue | 3 | 500 | 40% critical chance |
| Magic Greatsword | +18 | Two | Warrior | 3 | 1,100 | 75% cleave damage to adjacent enemies |
| Magic Staff | +9 | Two | Mage | 3 | 1,000 | +8 spell power, +35 maximum MP, 10% cheaper spells |
| Magic Longbow | +13 | Two | Rogue | 3 | 1,200 | Range 12 and pierces every target in its path |
| Cryptblade | +8 | One | Warrior | Boss | 350 | 10% critical chance; Lich King reward |

Bows fire with `F` in the last movement direction. A target directly adjacent
to the player is too close; valid targets begin two tiles away. Arrows stop at
walls. The Bow and Longbow stop at the first target, while the Magic Longbow
can strike several targets on the same line. All bows have a fixed 15% ranged
critical chance.

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

Each of the four regional bosses guarantees a fixed thematic item:

| Boss | Reward |
| --- | --- |
| Lich King | Cryptblade |
| Necromancer | Necromancer's Cloak |
| Goblin King | Goblin King's Shield |
| Drowned Queen | Tidecaller Robes |

The Fallen Sun Guardian protects the buried temple treasure and does not drop
equipment. Boss victories remain permanent during repeat expeditions.
