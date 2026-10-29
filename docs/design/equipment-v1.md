# Equipment v1 — approved design

## Starter catalog

All five starter items are Common and usable by every class. Shop prices use
existing rules: buying costs twice the base value; selling returns one quarter.

| Item | Type / category | Equipped bonus | Base value | Buy price | Intended shop |
| --- | --- | --- | ---: | ---: | --- |
| Leather Cap | Helmet | +1 defense | 40 | 80 gold | Blacksmith |
| Leather Gauntlets | Accessory / gauntlets | +1 attack | 60 | 120 gold | Blacksmith |
| Leather Bracers | Accessory / bracers | +1 defense | 40 | 80 gold | Blacksmith |
| Ring of Vigor | Accessory / ring | +10 maximum HP | 80 | 160 gold | Goldsmith, Oakhaven |
| Copper Amulet | Accessory / amulet | +10 maximum MP | 80 | 160 gold | Goldsmith, Oakhaven |

The Goldsmith is a new building in Oakhaven with its own jewelry shop. Rings and
amulets belong there, separate from the Blacksmith and Alchemist. The Blacksmith
primarily trades weapons and armor, including the starter cap, gauntlets and
bracers. All starter items are available at the first shop tier once stocked.
Additional accessory items and effects are deferred to v2.

## New characters

Each class starts with one poor weapon in the bag: Warrior — Rusty Sword,
Mage — Worn Staff, Rogue — Rusty Dagger. Preserve the existing manual equip
flow. No helmet, armor, accessory, second weapon or scroll is supplied. Helmets
and armor must be acquired. Existing characters keep their equipment.

## Equipment and bag rules

- Six equipment slots: Strong hand, Weak hand, Helmet, Armor, Accessory 1 and
  Accessory 2.
- A two-handed Strong hand weapon disables Weak hand. Its previous item returns
  to the bag; refuse the change if there is insufficient room.
- With a one-handed Strong hand weapon, Weak hand accepts a class-compatible
  one-handed weapon or shield. Preserve the existing half-strength off-hand
  weapon attack bonus.
- Rings may occupy both accessory slots, including two copies of the starter
  ring. Other accessory categories are limited to one equipped item each.
- Equipped items live outside the ten-space bag. Swapping uses the bag space
  vacated by the incoming item; unequipping requires free bag space.
- The sidebar shows six equipment slots, with the equipped spell in a separate
  section and the selected quick potion and remaining count below it.
- Preserve existing saves, equipment, wounds, inventory, quest progress and
  confiscated Royal Dungeon equipment throughout the storage migration.

## Quick potion

The default Drink Potion key is V, configurable through the existing Controls
screen. Only Health and Mana potions qualify. The player designates a default
potion type in inventory. Drinking uses the normal potion effects and turn cost;
using another potion through inventory's U remains available. Keep the selected
type when depleted and report that none remain, without switching types.
Persist the preference and key binding through save/load.

## Change-set sequence

1. **Catalog definitions and save metadata:** add helmet/accessory item types,
   the five factories, category limits and jewelry classification. Save version
   223 persists accessory categories everywhere items are serialized. Older
   items default to no accessory category. This change adds definitions only;
   the items are not yet stocked, equipped or assigned final artwork.
2. **Equipment storage and presentation:** separate worn items from bag storage,
   implement six usable slots and bonuses, migrate existing equipment and Royal
   Dungeon confiscation, and update inventory/sidebar controls and artwork.
   Save version 224 stores the six fixed worn slots separately from the bag in
   JSON. Runtime carried storage reserves entries 10–15 for those slots;
   `inventory_count` covers only bag entries 0–9. Legacy equipment moves out of
   the bag after historical item migrations, preserving already applied bonuses
   and wounds. Royal Dungeon evidence supports all sixteen carried items and
   remembers the original six slot assignments.

   Inventory: Tab or the heading switches Bag / Equipment; E or Enter equips
   a bag item; O equips a one-handed weapon to Weak hand; 1 / 2 equips an
   accessory to the chosen slot. E, Enter or X on a worn item returns it to
   the bag. U and D act on bag items. The workshop also lists both worn hands.
   Equipment swaps preserve current HP / MP and clamp them only when the final
   resource maximum falls. Removing Strong hand promotes a Weak hand weapon;
   a shield instead returns to the bag and requires an additional free space.

3. **Shop availability and Goldsmith:** stock the starter catalog and build the
   Oakhaven Goldsmith through the established pixelkit pipeline. Review the
   building beside existing town art and through the actual game renderer.
4. **Quick potion:** add the remappable V action, inventory selection, sidebar
   count and persistence, using the existing potion-use combat path.

Follow the repository's review and manual-commit pause after each change set.
