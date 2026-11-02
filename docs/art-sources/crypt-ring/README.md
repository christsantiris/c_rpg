# Crypt Ring: Chapter 1 Lich reward

Approved reward change, 2026-11-01. The Lich King now guarantees **Crypt Ring**
in place of Cryptblade. Every class can equip it in either accessory slot.
It grants exactly **+5 maximum HP and +5 maximum MP**, with no attack, defense
or critical-chance bonus. Equipping it does not restore current HP or MP.
Another ring may occupy the second accessory slot under the existing rules.

The ring is Uncommon, with base value 80 gold, matching the starter ring's
combined resource budget. It is a boss reward, not shop stock; the Goldsmith
accepts it for the normal 20-gold resale value. Its artwork reuses the existing
ring icon and Uncommon accessory palette.

Normal victory drops one ring on the ground. A full floor-loot list reserves
it in the Lich's reliquary; a full bag leaves it available. Both container
recovery and direct reliquary recovery give the ring. Previously unclaimed
rewards remain recoverable on revisiting, while claiming, dropping or selling
the reward cannot create a second copy. Existing Cryptblade definitions and
historical save behavior remain available for legacy/debug use.

The existing item metadata saves the ring's category, bonuses and equipped
location. The existing Lich reward field saves its unclaimed/claimed status.
No persistent field or new save migration is needed.

## Renderer inspection

These images use the actual SDL renderer at 1280×720:

- [Bag and item details](bag.png): Crypt Ring, +5 maximum HP, +5 maximum MP,
  all-class eligibility and the existing ring icon.
- [Equipped ring](worn.png): Accessory 1, the same exact bonuses after a fresh
  save/load, and the item excluded from bag capacity.
- [Game sidebar](sidebar.png): ring icon in Accessory 1; maximum HP and MP
  exceed the unchanged current values by five immediately after equipping.

The native fixture uses generated final dungeon floors for all three classes,
reduces only the boss's health to one and defeats it through a normal player
action. It checks one ring drop, no duplicate reliquary, pickup, equipment and
a fresh save/load. This validates reward behavior and presentation, not combat
balance.

## Validation

- Release game and test runner build successfully.
- Full regression suite: **9,654/9,654 assertions pass**.
- Lich reward tests cover melee, bow, Magic Arrow, Fireball and Frost Bolt kills,
  unclaimed floor-loot saving/loading, full loot and bag capacity, reliquary
  recovery, one-time claiming and recovery after a fresh expedition.
- Warrior, Mage and Rogue each equip the ring in both accessory slots. Tests
  check exactly five added HP/MP, unchanged current resources and attack/defense,
  coexistence with another ring, save/load and removal of only its own bonuses.
- Existing historical Cryptblade migration checks remain passing.
- `git diff --check` passes. Changes remain uncommitted for manual review.
