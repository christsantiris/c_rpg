# Weapons, Off-Hand Equipment, and Shields

Weapons are class-oriented equipment with different attack ranges, hand
requirements, and combat traits. Regular enemies do not drop equipment. The
Blacksmith and guaranteed boss rewards are the primary equipment sources.

## Equipment Rules

The player has a main-hand slot and an off-hand slot:

- Press `E` on a weapon to equip it in the main hand.
- Press `O` on a one-handed weapon to equip it in the off hand.
- Press `E` on a shield to equip it in the off hand.
- A one-handed main weapon may be paired with one off-hand weapon or shield.
- Two-handed weapons clear the off hand and prevent off-hand equipment.
- Removing a main weapon promotes an off-hand weapon to the main hand.
- Removing a main weapon unequips a shield because shields require a weapon.
- Class restrictions apply to main-hand weapons, off-hand weapons, and shields.

An off-hand weapon contributes half of its attack bonus, rounded up, and half
of its critical chance. Its other weapon traits do not apply. This makes dual
wielding an offensive choice without doubling the power of a full weapon.

Shields add defense and have a chance to block half of the damage remaining
after normal defense. Their progression is:

| Shield | Defense | Block chance | Classes | Blacksmith tier |
| --- | ---: | ---: | --- | ---: |
| Buckler | +2 | 10% | Warrior, Rogue | 1 |
| Kite Shield | +4 | 15% | Warrior | 2 |
| Tower Shield | +7 | 20% | Warrior | 3 |
| Magic Shield | +9 | 25% | Warrior | 3 |

## Weapon Roles

- Swords are dependable weapons; magical swords add critical chance.
- Axes are two-handed and ignore part of enemy defense.
- Bows are two-handed ranged weapons. The Magic Longbow is the Rogue's
  strongest weapon and pierces targets.
- Staves are two-handed Mage weapons that improve spell power.
- Daggers trade base attack for stronger melee critical chances and work well
  as Rogue off-hand weapons.
- Greatswords are two-handed Warrior weapons with cleave. The Magic
  Greatsword is the Warrior's strongest weapon.

## Acquisition and Progression

Regular enemies drop gold, potions, and scrolls, but never weapons or armor.
The Blacksmith begins with basic stock, unlocks uncommon equipment after one
boss victory, and unlocks rare equipment after two boss victories. The most
powerful class weapons are expensive Tier 3 purchases.

Each regional boss currently guarantees one thematic equipment reward. A
reward that the current class cannot equip can be sold to the Blacksmith.
Treasure rooms and chests are planned as a later exploration reward source.

Equipment metadata, both hand indices, shield block properties, and adjusted
player combat totals are preserved by save/load. Saves from before the
off-hand feature discard the formerly unused off-hand index during migration.
