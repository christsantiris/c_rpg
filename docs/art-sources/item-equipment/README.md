# Item and equipment sprite review

The item/equipment pass covers all **56 current item constructors**, the
legacy/generic weapon and shield icons, and all six equipped spell emblems.
The artwork is native C pixel art rather than imported bitmap sheets.

## Changes

- Weapons have continuous blade contours, fuller inlays, wrapped grips,
  curved bows and strings, forged axe heads, and faceted staff crystals.
  Greatswords have a visibly broader blade than long swords and daggers.
- Armor icons have separate necklines, shoulders, sleeves, and hems. Chain
  rings, overlapping scales, leather studs, cloak folds, and robe embroidery
  distinguish the families at the existing 24-pixel size.
- Shields now have round, pointed, and tower silhouettes, metal rims, bosses,
  rivets, and separate magical crests.
- Potions have glass shoulders, corks, highlights, and colored liquid. Scrolls,
  tomes, maps, and arrows have their own inventory icons.
- The equipped navigation grid uses the same refined art in its existing
  main-hand, off-hand, armor, and spell slots. Slot frames have restrained bevels.
- Inventory rows and both equipment comparison columns show matching icons
  without displacing the existing item names, stats, selection arrows or labels.
- Held weapons have blade edges, grip wraps, shaft bands and curved bow limbs.
  Worn shields use distinct shapes. Equipped armor shows chain, scale, stud,
  and robe details while preserving the existing class bodies and facing order.
- Named rewards have recognizable finishes: Cryptblade, Krakenbone Bow,
  Necromancer's Cloak, Gravekeeper's Mantle, Dragon Scale Mantle,
  Tidecaller Robes, and Goblin King's Shield. These use their existing names and
  saved visual IDs; no persistent fields were added.

## Source and review images

The 29 native silhouettes are authored in
[`item_icon_art.h`](../../../src/renderer/item_icon_art.h), with palettes and
metadata selection in [`item_icons.c`](../../../src/renderer/item_icons.c).
Player equipment details are in the existing equipped-weapon, shield, and
body functions in [`sprites.c`](../../../src/renderer/sprites.c).

- [Equipped navigation grid before/after](equipped-grid.png).
- [Weapon icons before/after](weapons.png).
- [Armor and shield icons before/after](armor-shields.png).
- [Consumable and utility icons](consumables.png). The old inventory used text
  rows without these item icons, so its previous-art columns are blank.
- [Equipped spell icons before/after](spell-icons.png).
- [Inventory and equipment comparisons](inventory.png).
- [Equipment in the game renderer](equipped-game.png).
- [Selected equipped loadouts before/after in four directions](worn-equipment.png).
- [Every held weapon in four directions](all-worn-0.png).
- [Every worn armor definition in four directions](all-worn-1.png).
- [Every shield in four directions](all-worn-2.png).

Icon comparisons include native-size views and nearest-neighbor enlarged
views. Worn-equipment galleries are ordered south, east, north, west. These are
controlled renderer fixtures rather than gameplay or shop-purchase playthroughs.

## Validation

Release and Debug builds passed. A temporary AddressSanitizer/UndefinedBehaviorSanitizer
renderer harness covered all 56 constructors, all equipment definitions in all
four directions, seven representative loadouts, inventory rows and comparison
panels, and the equipped navigation grid. Icon draws fit within 24×24 pixels,
preserve transparent terrain, and leave SDL viewport, scale, clip and blend
state intact. Native icon and shield row lengths and palette keys were checked.
Legacy/generic visual IDs, gold, empty items, null equipment pointers, and
invalid spell IDs were also exercised. All twelve unequipped class/facing views
remain pixel-for-pixel identical.

Combat stats, item identity and equipment rules, animation timing, and save
formats are unchanged. Existing saves remain compatible. World props are the
next separate art pass.

## Item coverage

### Weapons (20)

- [x] `item_make_rusty_sword()`
- [x] `item_make_short_sword()`
- [x] `item_make_long_sword()`
- [x] `item_make_demonic_sword()`
- [x] `item_make_magic_long_sword()`
- [x] `item_make_battle_axe()`
- [x] `item_make_magic_battle_axe()`
- [x] `item_make_staff()`
- [x] `item_make_runed_staff()`
- [x] `item_make_magic_staff()`
- [x] `item_make_sandstorm_staff()`
- [x] `item_make_bow()`
- [x] `item_make_longbow()`
- [x] `item_make_magic_longbow()`
- [x] `item_make_krakenbone_bow()`
- [x] `item_make_dagger()`
- [x] `item_make_magic_dagger()`
- [x] `item_make_greatsword()`
- [x] `item_make_magic_greatsword()`
- [x] `item_make_cryptblade()`

### Armor and robes (16)

- [x] `item_make_leather_armor()`
- [x] `item_make_chain_mail()`
- [x] `item_make_scale_mail()`
- [x] `item_make_plate_armor()`
- [x] `item_make_magic_plate()`
- [x] `item_make_studded_leather()`
- [x] `item_make_ranger_cloak()`
- [x] `item_make_shadow_armor()`
- [x] `item_make_apprentice_robes()`
- [x] `item_make_runed_robes()`
- [x] `item_make_enchanter_robes()`
- [x] `item_make_archmage_robes()`
- [x] `item_make_necromancer_cloak()`
- [x] `item_make_gravekeeper_mantle()`
- [x] `item_make_dragon_scale_mantle()`
- [x] `item_make_tidecaller_robes()`

### Shields (5)

- [x] `item_make_buckler()`
- [x] `item_make_kite_shield()`
- [x] `item_make_tower_shield()`
- [x] `item_make_magic_shield()`
- [x] `item_make_goblin_king_shield()`

### Consumables and utility items (15)

- [x] `item_make_health_potion()`
- [x] `item_make_mana_potion()`
- [x] `item_make_arrows()`
- [x] `item_make_strength_potion()`
- [x] `item_make_intelligence_potion()`
- [x] `item_make_scroll_magic_arrow()`
- [x] `item_make_scroll_fireball()`
- [x] `item_make_scroll_heal()`
- [x] `item_make_scroll_return_to_town()`
- [x] `item_make_treasure_map()`
- [x] `item_make_magic_arrow_tome()`
- [x] `item_make_fireball_tome()`
- [x] `item_make_heal_tome()`
- [x] `item_make_scroll_frost_bolt()`
- [x] `item_make_scroll_teleport()`

All six spell emblems: Magic Arrow, Fireball, Heal, Return to Town, Frost Bolt, and Teleport.
