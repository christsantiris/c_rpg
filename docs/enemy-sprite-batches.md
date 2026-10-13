# Enemy sprite update checklist

The current roster contains **95 enemy types in 16 batches**. Each `EnemyType` from `src/game/enemy.h` appears exactly once below. Additional boss forms, named hunts, quest enemies, and shared-renderer dependencies are tracked separately.

Progress: **77/95 enemy types completed (batches 1–14)**. Completed boxes have passed artwork and renderer review. Complete one batch at a time, then wait for the next batch request. Mark a sprite complete after its artwork and native-size renderer review pass.

## Batch overview

| Batch | Group | Enemy types | Complete |
|---|---|---:|---|
| 1 | Core dungeon enemies | 6 | Yes |
| 2 | Crypt enemies | 3 | Yes |
| 3 | Haunted Forest | 7 | Yes |
| 4 | Goblin Mountains | 8 | Yes |
| 5 | Sunken Coast | 8 | Yes |
| 6 | Blackwater Swamp and Labyrinth | 5 | Yes |
| 7 | Dragonspine and Crown Roads | 4 | Yes |
| 8 | Frostfell | 7 | Yes |
| 9 | Sunscar Wastes | 6 | Yes |
| 10 | Moonveil Gardens | 5 | Yes |
| 11 | Ashen Hollow | 4 | Yes |
| 12 | Glassdeep Caverns | 4 | Yes |
| 13 | Royal Catacombs | 5 | Yes |
| 14 | Original dungeon bosses | 5 | Yes |
| 15 | Ruined Isle and Temple | 10 | No |
| 16 | Castle of No Return | 8 | No |
| **Total** | | **95** | |

## Sprite checklist

### Batch 1: Core dungeon enemies (6)

- [x] Skeleton — `ENEMY_SKELETON`
- [x] Goblin — `ENEMY_GOBLIN`
- [x] Zombie — `ENEMY_ZOMBIE`
- [x] Orc — `ENEMY_ORC`
- [x] Troll — `ENEMY_TROLL`
- [x] Giant — `ENEMY_GIANT`

Reviewed with the actual SDL renderer: native 24×24 sprites, transparent terrain, tile bounds, SDL scale/viewport/clip preservation, frozen overlays, Dain’s fragment marker, all five existing catacomb overlays, and the castle’s missing-texture Orc fallback. Release and Debug builds and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-1/before-after.png).
- [Dungeon renderer preview](art-sources/enemy-batch-1/dungeon.png).
- [Shared catacomb overlay comparison](art-sources/enemy-batch-1/catacombs.png).

The dungeon scene uses a controlled room fixture so all six enemies can be compared on the same terrain. Catacomb identities remain scheduled for batch 13; this batch checks their compatibility with the new Skeleton body.

### Batch 2: Crypt enemies (3)

- [x] Crypt Bat — `ENEMY_CRYPT_BAT`
- [x] Wraith — `ENEMY_WRAITH`
- [x] Crypt Conjurer — `ENEMY_CRYPT_CONJURER`

Reviewed at native 24×24 size through the actual SDL renderer on dungeon terrain, including a burial-seal fixture. Also inspected the Bat and Wraith in the catacombs, the Wraith beside the existing Grave Archer and on swamp terrain, and frozen overlays. Tile bounds, terrain transparency, SDL scale/viewport/clip preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer checks passed. All six batch 1 sprites still render pixel-for-pixel identically.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-2/before-after.png).
- [Dungeon and burial-seal renderer preview](art-sources/enemy-batch-2/dungeon.png).
- [Catacomb renderer preview](art-sources/enemy-batch-2/catacombs.png).

Scene previews use controlled room fixtures to compare the artwork on its actual terrain. Watchers’ Memorial Guard identities remain tracked for batch 13; this batch reviews the shared Wraith appearance.

### Batch 3: Haunted Forest (7)

- [x] Pixie — `ENEMY_PIXIE`
- [x] Blighted Wolf — `ENEMY_BLIGHTED_WOLF`
- [x] Giant Spider — `ENEMY_GIANT_SPIDER`
- [x] Dark Elf — `ENEMY_DARK_ELF`
- [x] Giant Wurm — `ENEMY_GIANT_WURM`
- [x] Forest Troll — `ENEMY_FOREST_TROLL`
- [x] Forest Necromancer — `ENEMY_FOREST_NECROMANCER`

Reviewed all seven sprites at native 24×24 size through the actual SDL renderer on forest terrain, including Alder’s bound Warden setting and the Forest Necromancer’s boss flag. Also inspected the shared Blighted Wolf on Crown Road East and West, plus frozen overlays. Tile bounds, terrain transparency, SDL scale/viewport/clip preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer checks passed. All nine sprites from batches 1–2 remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-3/before-after.png).
- [Forest renderer preview](art-sources/enemy-batch-3/forest.png).
- [Blighted Wolves on Crown Road East](art-sources/enemy-batch-3/crownroad-east.png).

Scene previews use controlled clearing and road fixtures to compare character readability on the actual terrain.

### Batch 4: Goblin Mountains (8)

- [x] Goblin Scout — `ENEMY_GOBLIN_SCOUT`
- [x] Goblin Archer — `ENEMY_GOBLIN_ARCHER`
- [x] Goblin Bomber — `ENEMY_GOBLIN_BOMBER`
- [x] Tunnel Spider — `ENEMY_TUNNEL_SPIDER`
- [x] Cave Troll — `ENEMY_CAVE_TROLL`
- [x] Hobgoblin Guard — `ENEMY_HOBGOBLIN_GUARD`
- [x] Goblin Shaman — `ENEMY_GOBLIN_SHAMAN`
- [x] Mountain Goblin King — `ENEMY_MOUNTAIN_GOBLIN_KING`

Reviewed all eight sprites at native 24×24 size through the actual SDL renderer on mountain terrain, including the King’s boss flag, Dain’s three Map Bearer borders, watchfire context, and frozen overlays. Also inspected the shared Hobgoblin Guard on Crown Road East, Goblin Archer in Dragonspine, and Tunnel Spider/Smuggler Bodyguard through the escape-tunnel renderer. Tile bounds, terrain transparency, SDL scale/viewport/clip preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer checks passed. All 16 sprites from batches 1–3 remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-4/before-after.png).
- [Mountain renderer preview with Dain’s markers](art-sources/enemy-batch-4/mountains.png).
- [Shared Hobgoblin Guard on Crown Road East](art-sources/enemy-batch-4/crownroad-east.png).

Scene previews use controlled room and road fixtures to compare character readability on the actual terrain. The comparison includes a margin around the native tile so the old Shaman’s overhanging staff light remains visible; the new sprites fit within 24×24 pixels.

### Batch 5: Sunken Coast (8)

- [x] Illusion — `ENEMY_ILLUSION`
- [x] Merfolk — `ENEMY_MERFOLK`
- [x] Siren — `ENEMY_SIREN`
- [x] Giant Crab — `ENEMY_GIANT_CRAB`
- [x] Animated Statue — `ENEMY_ANIMATED_STATUE`
- [x] Water Elemental — `ENEMY_WATER_ELEMENTAL`
- [x] Sea Serpent — `ENEMY_SEA_SERPENT`
- [x] Drowned Queen — `ENEMY_DROWNED_QUEEN`

Reviewed all eight sprites at native 24×24 size through the actual SDL renderer on coast stone and shallow water, including beacon/cache context and the Drowned Queen. Also inspected Animated Statues beside Giant Spiders on Rook’s labyrinth terrain, frozen overlays, health bars, ranged projectile travel/impact, and melee damage feedback. Tile bounds, terrain transparency, SDL scale/viewport/clip preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 24 sprites from batches 1–4 remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-5/before-after.png).
- [Coast renderer preview with beacons and cache](art-sources/enemy-batch-5/coast.png).
- [Shared Animated Statue in Rook’s labyrinth](art-sources/enemy-batch-5/labyrinth.png).

Scene previews use controlled room fixtures to compare character readability on the actual terrain. These are artwork checks rather than quest playthroughs. Transparent gaps provide the Illusion’s broken contour and Water Elemental’s open wave interior; combat, collision footprints, saved enemy types, and animations are unchanged.

### Batch 6: Blackwater Swamp and Labyrinth (5)

- [x] Giant Rat — `ENEMY_GIANT_RAT`
- [x] Bandit — `ENEMY_BANDIT`
- [x] Vampire — `ENEMY_VAMPIRE`
- [x] Swamp Demon — `ENEMY_SWAMP_DEMON`
- [x] Minotaur — `ENEMY_MINOTAUR`

Reviewed all five sprites at native 24×24 size through the actual SDL renderer: the four swamp enemies on bog terrain, and the Minotaur beside Animated Statues and Giant Spiders in Rook’s labyrinth. Also inspected the Vampire Captor beside the rescued daughter, Bandit/Giant Rat/Tunnel Spider in the escape tunnel, the Bandit on Crown Road East, frozen overlays, health bars, and melee damage feedback. Tile bounds, terrain transparency, SDL scale/viewport/clip preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 32 sprites from batches 1–5 and the original Demon Lord remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-6/before-after.png).
- [Swamp renderer preview](art-sources/enemy-batch-6/swamp.png).
- [Minotaur in Rook’s labyrinth](art-sources/enemy-batch-6/labyrinth.png).

Scene previews use controlled room and road fixtures for artwork review rather than quest playthroughs. The Swamp Demon now has its own mossy body, broken horns, and hanging bog growth. Combat, collision footprints, saved enemy types, and animations are unchanged.

### Batch 7: Dragonspine and Crown Roads (4)

- [x] Drake — `ENEMY_DRAKE`
- [x] Fire Elemental — `ENEMY_FIRE_ELEMENTAL`
- [x] Road Archer — `ENEMY_ROAD_ARCHER`
- [x] Horseman — `ENEMY_HORSEMAN`

Reviewed all four sprites at native 24×24 size through the actual SDL renderer: Drake and Fire Elemental on Dragonspine terrain, and Road Archer/Horseman on both Crown Roads. Also inspected the existing Goblin Scout, Goblin Archer, and Giant beside the regional enemies and watchfire; the Smuggler Archer beside tunnel enemies; frozen overlays; health bars; melee damage feedback; and Road Archer projectile travel/impact. Tile bounds, terrain transparency, SDL scale/viewport/clip preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 37 sprites from batches 1–6 and the original Red Dragon remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-7/before-after.png).
- [Dragonspine renderer preview with watchfire context](art-sources/enemy-batch-7/dragonspine.png).
- [Crown Road East renderer preview](art-sources/enemy-batch-7/crownroad-east.png).

Scene previews use controlled room and road fixtures for artwork review rather than quest playthroughs. The Drake keeps its blue-green scales with defined wing membranes and talons; the Fire Elemental has curved flames and a darker core; the road enemies retain crimson clothing with clearer equipment and horse/rider separation. Combat, collision footprints, saved enemy types, and animations are unchanged.

### Batch 8: Frostfell (7)

- [x] Ice Wolf — `ENEMY_ICE_WOLF`
- [x] Frost Archer — `ENEMY_FROST_ARCHER`
- [x] Yeti — `ENEMY_YETI`
- [x] Frost Wraith — `ENEMY_FROST_WRAITH`
- [x] Ice Golem — `ENEMY_ICE_GOLEM`
- [x] Ice Giant — `ENEMY_ICE_GIANT`
- [x] Polar Kraken — `ENEMY_POLAR_KRAKEN`

Reviewed the six native 24×24 sprites on snow and slick ice, including Rimefang’s label and hunt markers, expedition journal/Fen context, frozen overlays, health bars, melee damage feedback, and Frost Archer projectile travel/impact. Reviewed the Polar Kraken texture and fallback at their 96×96 display size, including targeted warnings, single sweeps, enraged double sweeps, and recovery cues. The fallback now matches the texture’s display footprint. Tile bounds, transparent terrain, SDL scale/viewport/clip/blend preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 41 sprites from batches 1–7 remain pixel-for-pixel identical.

- [Regular enemy comparison at native and enlarged sizes](art-sources/enemy-batch-8/before-after.png).
- [Kraken texture and fallback comparison](art-sources/enemy-batch-8/kraken-before-after.png).
- [Frostfell renderer preview](art-sources/enemy-batch-8/frostfell.png).
- [Kraken warning, sweep, enraged phase, and recovery](art-sources/enemy-batch-8/kraken-phases.png).
- [Kraken fallback on lake terrain](art-sources/enemy-batch-8/kraken-fallback.png).
- [Kraken source, pipeline, and exact imagegen edit prompt](art-sources/enemy-batch-8/README.md).

Scene previews use controlled snow, ice, quest, and lake fixtures through the actual SDL renderer rather than quest playthroughs. The Kraken source was refined with the built-in imagegen tool and converted with the existing `tools/sprites/polar_kraken.py` pipeline. Combat, collision footprints, saved enemy types, and animations are unchanged.

### Batch 9: Sunscar Wastes (6)

- [x] Scarab — `ENEMY_SCARAB`
- [x] Viper — `ENEMY_VIPER`
- [x] Mummy — `ENEMY_MUMMY`
- [x] Djinn — `ENEMY_DJINN`
- [x] Golem — `ENEMY_GOLEM`
- [x] Desert Pharaoh — `ENEMY_DESERT_PHARAOH`

Reviewed all six sprites at native 24×24 size through the actual SDL renderer on Sunscar sand, including Dunehex’s label and hunt markers, desert lamp context, and the Pharaoh at normal and low health. Also inspected frozen overlays, health bars, melee damage feedback, and Djinn/Pharaoh projectile travel and impact. Tile bounds, transparent terrain, SDL scale/viewport/clip/blend preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 48 sprites from batches 1–8 remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-9/before-after.png).
- [Sunscar renderer preview](art-sources/enemy-batch-9/sunscar.png).
- [Dunehex and Mummy hunt group](art-sources/enemy-batch-9/dunehex.png).

Scene previews use controlled desert room fixtures for artwork review rather than quest playthroughs. The Scarab has an oval shell and connected jointed legs; the Viper has a continuous coil and scale bands; the Mummy has diagonal wraps and loose strips; the Djinn has a curling smoke tail; the Golem has carved sandstone joints; and the Pharaoh has a striped headdress and connected crook grip. Combat, collision footprints, saved enemy types, and animations are unchanged.

### Batch 10: Moonveil Gardens (5)

- [x] Fey Trickster — `ENEMY_FEY_TRICKSTER`
- [x] Giant Moth — `ENEMY_GIANT_MOTH`
- [x] Living Flower (displayed as Carnivorous Flower) — `ENEMY_LIVING_FLOWER`
- [x] Thorn Guardian — `ENEMY_THORN_GUARDIAN`
- [x] Thorn Regent — `ENEMY_THORN_REGENT`

Reviewed all five sprites at native 24×24 size through the actual SDL renderer on Moonveil terrain, including the Moonseed, Moonwater, and Mooncircle guardians beside their objectives, and the Thorn Regent at normal and low health. Both existing wing-flutter phases were reviewed for the Trickster and Moth; the original 350-ms timing and opposing upper/lower Moth wing motion remain. Also inspected frozen overlays, health bars, melee damage feedback, and Trickster/Flower/Regent projectile travel and impact. Tile bounds in both phases, transparent terrain, SDL scale/viewport/clip/blend preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 54 sprites from batches 1–9 remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-10/before-after.png).
- [Both existing wing-flutter phases](art-sources/enemy-batch-10/flutter.png).
- [Moonveil renderer preview](art-sources/enemy-batch-10/moonveil.png).
- [Moonseed guardians and objective](art-sources/enemy-batch-10/moonseed.png).

Scene previews use controlled garden and quest fixtures for artwork review rather than quest playthroughs. The Trickster has a pointed hat, wing veins, and a discrete conjured light; the Moth has scalloped wings and eye-spots; the Flower has layered petals, irregular teeth, and a bending stem; the Guardian has interlocking branches and rooted feet; and the Regent has a spreading crown and trailing root mantle. Combat, collision footprints, saved enemy types, and animation rules are unchanged.

### Batch 11: Ashen Hollow (4)

- [x] Cinder Imp — `ENEMY_CINDER_IMP`
- [x] Ash Hound — `ENEMY_ASH_HOUND`
- [x] Obsidian Guardian — `ENEMY_OBSIDIAN_GUARDIAN`
- [x] Cinder Lord — `ENEMY_CINDER_LORD`

Reviewed all four sprites at native 24×24 size through the actual SDL renderer on Ashen Hollow terrain, including the mechanism and furnace Emberforge guardians, Ashen watchfire defenders, and the Cinder Lord at normal and low health. Both existing ember phases were reviewed for the Imp, Hound, and Lord; the original 350-ms timing and one-pixel motion remain, and the Obsidian Guardian stays static. Also inspected frozen overlays, health bars, melee damage feedback, and Cinder Lord projectile travel/impact. Tile bounds in both phases, transparent terrain, SDL scale/viewport/clip/blend preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 59 sprites from batches 1–10 remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-11/before-after.png).
- [Both existing ember phases](art-sources/enemy-batch-11/embers.png).
- [Ashen Hollow renderer preview](art-sources/enemy-batch-11/ashen.png).
- [Emberforge furnace guardians](art-sources/enemy-batch-11/emberforge.png).

Scene previews use controlled volcanic and quest fixtures for artwork review rather than quest playthroughs. The Imp has curved horns, a pointed grin, folded wings, and a tail; the Hound has a defined jaw, staggered legs, charred fur, and connected shoulder flames; the Guardian has angular obsidian plates and a recessed molten core; and the Lord has a distinct crown, layered mantle, and separate flame staff. Combat, collision footprints, saved enemy types, and animation rules are unchanged.

### Batch 12: Glassdeep Caverns (4)

- [x] Crystal Spider — `ENEMY_CRYSTAL_SPIDER`
- [x] Blind Stalker — `ENEMY_BLIND_STALKER`
- [x] Shard Golem — `ENEMY_SHARD_GOLEM`
- [x] Prism Sovereign — `ENEMY_PRISM_SOVEREIGN`

Reviewed all four sprites at native 24×24 size through the actual SDL renderer on Glassdeep terrain, including Shardwarden’s hunt label and markers, all three resonator guardian types, and the Prism Sovereign at normal and low health. Both existing scepter-pulse phases were reviewed; the original 350-ms timing and one-pixel expansion remain. Also inspected frozen overlays, health bars, melee damage feedback, and the Sovereign’s beam warning, travel, and impact. Tile bounds in both phases, transparent terrain, SDL scale/viewport/clip/blend preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 63 sprites from batches 1–11 remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-12/before-after.png).
- [Both existing scepter-pulse phases](art-sources/enemy-batch-12/pulse.png).
- [Glassdeep renderer preview](art-sources/enemy-batch-12/glassdeep.png).
- [Shardwarden and Stalker hunt group](art-sources/enemy-batch-12/shardwarden.png).
- [Sovereign beam warning](art-sources/enemy-batch-12/beam-warning.png).

Scene previews use controlled cavern, hunt, and resonator fixtures for artwork review rather than quest playthroughs. The Spider has jointed crystal legs and a faceted abdomen; the Stalker has an eyeless ridged face, hunched spine, and hooked fingers; the Golem has separated slabs and a recessed prism core; and the Sovereign has a distinct segmented silhouette, crown, and faceted scepter. Combat, collision footprints, saved enemy types, and animation rules are unchanged.

### Batch 13: Royal Catacombs (5)

- [x] Ancient Skeleton — `ENEMY_ANCIENT_SKELETON`
- [x] Bone Sentinel — `ENEMY_BONE_SENTINEL`
- [x] Grave Archer — `ENEMY_GRAVE_ARCHER`
- [x] Bone Cantor — `ENEMY_BONE_CANTOR`
- [x] Grave Marshal — `ENEMY_GRAVE_MARSHAL`

Reviewed all five sprites at native 24×24 size through the actual SDL renderer on Royal Catacombs terrain, including Soldiers’, Watchers’, and Choir Memorial guard pairs. Also inspected the corpse revival marker, revived enemies, the Marshal’s sweep warning and low-health appearance, frozen overlays, health bars, melee damage feedback, and Archer/Cantor projectile travel and impact. Tile bounds, transparent terrain, SDL scale/viewport/clip/blend preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 67 sprites from batches 1–12, including the original dungeon Skeleton, remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-13/before-after.png).
- [Royal Catacombs renderer preview](art-sources/enemy-batch-13/catacombs.png).
- [Memorial guards, revival cues, and Marshal sweep warning](art-sources/enemy-batch-13/encounters.png).
- [Frozen overlays, damage, projectiles, and low-health Marshal](art-sources/enemy-batch-13/effects.png).

Scene previews use controlled catacomb and memorial fixtures for artwork review rather than quest playthroughs. The Ancient Skeleton has weathered bones, a broken helmet, and a rusted sword; the Sentinel has plated armor and a reinforced shield; the Archer has a hood, curved bow, and quiver; the Cantor has layered violet vestments and a ritual staff; and the Marshal has a crested helmet, crimson mantle, and ornate polearm. Each now uses dedicated artwork instead of the shared dungeon Skeleton. Combat, collision footprints, saved enemy types, and animation rules are unchanged.

### Batch 14: Original dungeon bosses (5)

- [x] Goblin King — `ENEMY_GOBLIN_KING`
- [x] Lich King — `ENEMY_LICH_KING`
- [x] Demon Lord — `ENEMY_DEMON_LORD`
- [x] Red Dragon — `ENEMY_RED_DRAGON`
- [x] Tarrasque — `ENEMY_TARRASQUE`

Reviewed all five sprites at native 24×24 size through the actual SDL renderer on dungeon terrain, including the Lich King beside his two Skeleton guards. Also inspected the Red Dragon on Dragonspine hoard terrain beside the existing Drake and Fire Elemental, all five bosses at low health, frozen overlays, health bars, melee damage feedback, and Lich/Dragon projectile travel and impact. Tile bounds, transparent terrain, SDL scale/viewport/clip/blend preservation, Release and Debug builds, and AddressSanitizer/UndefinedBehaviorSanitizer renderer checks passed. All 72 sprites from batches 1–13 remain pixel-for-pixel identical.

- [Before/after comparison, including native-size views](art-sources/enemy-batch-14/before-after.png).
- [Dungeon renderer preview](art-sources/enemy-batch-14/dungeon.png).
- [Red Dragon on Dragonspine terrain](art-sources/enemy-batch-14/dragonspine.png).
- [Lich guards, frozen overlays, damage, projectiles, and low-health bosses](art-sources/enemy-batch-14/encounters-and-effects.png).

Scene previews use controlled dungeon and Dragonspine room fixtures for artwork review rather than boss-fight playthroughs. The Goblin King has a jeweled crown, pointed ears, plate armor, and a trimmed crimson mantle; the Lich has a defined skull, horned crown, layered violet robes, and a connected staff grip; the Demon Lord has curved horns, folded wing membranes, defined muscles, claws, and a curling tail; the Red Dragon has a scaled snout, wing ribs, belly plates, talons, and a curved tail; and the Tarrasque has overlapping shell plates, ivory spikes, a toothed jaw, and heavy claws. The five public drawing functions now use dedicated static templates in `src/renderer/dungeon_boss_art.h`. Combat, collision footprints, saved enemy types, and animation rules are unchanged.

### Batch 15: Ruined Isle and Temple (10)

- [ ] Relic Scarabs — `ENEMY_RELIC_SCARABS`
- [ ] Temple Stalker — `ENEMY_TEMPLE_STALKER`
- [ ] Blowdart Hunter — `ENEMY_BLOWDART_HUNTER`
- [ ] Vinebound Guardian (displayed as Vine Guardian) — `ENEMY_VINEBOUND_GUARDIAN`
- [ ] Sun Priest — `ENEMY_SUN_PRIEST`
- [ ] Serpent Spirit — `ENEMY_SERPENT_SPIRIT`
- [ ] Treasure Wraith — `ENEMY_TREASURE_WRAITH`
- [ ] Lunar Effigy — `ENEMY_LUNAR_EFFIGY`
- [ ] Moonbound Sentinel — `ENEMY_MOONBOUND_SENTINEL`
- [ ] Fallen Sun Guardian (displayed as Sun Guardian) — `ENEMY_FALLEN_SUN_GUARDIAN`

Include the Sun Guardian’s intact and broken appearances. These temple assets display at 28×28 pixels, with the guardian at 40×40; review the artwork at those sizes.

### Batch 16: Castle of No Return (8)

- [ ] Oathbound Soldier — `ENEMY_OATHBOUND_SOLDIER`
- [ ] Iron Warden — `ENEMY_IRON_WARDEN`
- [ ] Royal Marksman — `ENEMY_ROYAL_MARKSMAN`
- [ ] Court Hexer — `ENEMY_COURT_HEXER`
- [ ] Bell Herald — `ENEMY_BELL_HERALD`
- [ ] Castellan — `ENEMY_CASTELLAN`
- [ ] Royal Arcanist — `ENEMY_ROYAL_ARCANIST`
- [ ] Lord Veyr — `ENEMY_LORD_VEYR`

Include Lord Veyr’s three health-dependent forms. Castle enemies display at 24×26 pixels and bosses at 36×36; inspect the actual `castle_draw_enemy()` path and the missing-texture fallback.

## Extra forms and named encounters

These entries reuse enemy types above. They do not increase the 95-type count. Named encounters currently use their base sprites; inspect their identification markers and labels with the owning batch. Distinct new hunt artwork can be reviewed within that batch without adding persistent actor fields.

| Review | Appearance or encounter | Base sprite / owning batch |
|---|---|---|
| [ ] | Lord Veyr: armored form, above two-thirds health | Lord Veyr, batch 16 |
| [ ] | Lord Veyr: damaged form, above one-third through two-thirds health | Lord Veyr, batch 16 |
| [ ] | Lord Veyr: spectral form, at or below one-third health | Lord Veyr, batch 16 |
| [ ] | Sun Guardian: broken form, at or below half health; deactivated appearance | Fallen Sun Guardian, batch 15 |
| [x] | Rimefang and Rimefang Pack | Ice Wolf and Frost Wraith, batch 8 |
| [x] | Shardwarden and Shardwarden Stalker | Shard Golem and Blind Stalker, batch 12 |
| [x] | Dunehex and Dunehex Mummy | Djinn and Mummy, batch 9 |
| [x] | Expedition Guardians | Ice Wolf, Ice Giant, Frost Archer, batch 8 |
| [x] | Resonator Guardians | Crystal Spider, Blind Stalker, Shard Golem, batch 12 |
| [x] | Emberforge Guardians | Cinder Imp, Ash Hound, Obsidian Guardian, batch 11 |
| [x] | Moonseed Guardians | Fey Trickster and Giant Moth, batch 10 |
| [x] | Moonwater Guardian | Living Flower, batch 10 |
| [x] | Mooncircle Guardians | Thorn Guardian, batch 10 |
| [x] | Soldiers’ Memorial Guards | Ancient Skeleton and Bone Sentinel, batch 13 |
| [x] | Watchers’ Memorial Guards | Grave Archer (batch 13), Wraith (batch 2) |
| [x] | Choir Memorial Guards | Bone Cantor and Ancient Skeleton, batch 13 |
| [x] | Watchfire Defenders: Mountains objective | Goblin Archer and Hobgoblin Guard, batch 4 |
| [x] | Watchfire Defenders: Ashen objective | Obsidian Guardian and Cinder Imp, batch 11 |
| [x] | Watchfire Defenders: Dragonspine objective | Giant (batch 1), Goblin Archer (batch 4), Fire Elemental (batch 7) |
| [x] | Elowen’s burial-seal guardians | Skeleton (batch 1); Crypt Bat, Wraith, Crypt Conjurer (batch 2) |
| [x] | Alder’s Warden rescue guardians | Giant Spider, Blighted Wolf, Dark Elf, Pixie, Forest Troll, batch 3 |
| [x] | Blighted Wolves on Crown Road East and West | Blighted Wolf, batch 3 |
| [x] | Road Archers and Horsemen on both Crown Roads | Road Archer and Horseman, batch 7 |
| [x] | Bandits on Crown Road East | Bandit, batch 6 |
| [x] | Hobgoblin Guards on the Crown Roads | Hobgoblin Guard, batch 4 |
| [x] | Goblin Scouts and Archers in Dragonspine | Batch 4 art; reviewed beside batch 7 regional enemies |
| [x] | Lich Guards | Skeleton, batch 1; reviewed with Lich King in batch 14 |
| [x] | Vampire Captor | Vampire, batch 6 |
| [x] | Tunnel Smugglers | Bandit, batch 6 |
| [x] | Smuggler Bodyguards | Hobgoblin Guard, batch 4 |
| [x] | Smuggler Archers | Road Archer, batch 7 |
| [x] | Escape-tunnel spiders and rats | Tunnel Spider (batch 4), Giant Rat (batch 6) |
| [x] | Dain’s Map Bearers and their guards | Goblin Archer, Goblin Bomber, Goblin Shaman; Scout, Tunnel Spider, Hobgoblin Guard escorts, batch 4 |
| [x] | Mara’s beacon guardians | Giant Crab, Merfolk, Animated Statue, Sea Serpent, Water Elemental, batch 5 |
| [x] | Coast cache guards | Giant Crab, Animated Statue, Sea Serpent, batch 5 |
| [x] | Animated Statues in Rook’s labyrinth | Animated Statue, batch 5; checked beside batch 3 Giant Spider |
| [x] | Catacomb corpses and revived enemies | Batch 13; keep revival cues legible |

## Shared artwork and dependencies

- The five catacomb enemies now use dedicated artwork in `src/renderer/catacombs_enemy_art.h`. They no longer share `draw_skeleton()`; the batch 1 comparison preserves the earlier shared-overlay review, and batch 13 verifies that the dungeon Skeleton remains unchanged.
- Swamp Demon now has dedicated artwork in `src/renderer/swamp_enemy_art.h`. It no longer shares `draw_demon_lord()`; the original Demon Lord remains scheduled for batch 14.
- Castle artwork uses `castle_draw_enemy()` in `src/renderer/castle_renderer.c`; the generic castle cases in `draw_enemy()` are not the live castle rendering path. The castle missing-texture fallback uses the Orc sprite from batch 1.
- Temple artwork uses `assets/images/temple-enemies.bmp`, including the separate broken guardian crop. Castle artwork uses `assets/images/castle-enemies.bmp` and `castle-bosses.bmp`.
- Polar Kraken uses `assets/images/polar-kraken.bmp` and a procedural fallback; preserve both paths during batch 8.
- Minotaur retains the former Maze Warden’s saved numeric ID. Its appearance is batch 6, with no enum reordering.

## Completion checks for every batch

- [ ] Each listed sprite has a before/after preview at native size and a nearest-neighbor enlarged view.
- [ ] Sprites have been inspected through the game renderer on their actual terrain, including relevant quest encounters and boss forms.
- [ ] Silhouettes, faces, limbs, held equipment, shading, and outlines remain readable at the game’s display scale.
- [ ] Frozen overlays, damage flashes, health bars, projectile effects, directional indicators, and attack warnings remain legible where applicable.
- [ ] Shared renderers and missing-texture fallbacks have been checked where applicable.
- [ ] Build and appropriate renderer checks pass; only then update the batch status.

This first pass updates static artwork. Retain current collision footprints, combat rules, enemy enum values, and existing saves. Treat animation as a later change set. If persistent fields become necessary, implement save/load support and migrations in that change set.
