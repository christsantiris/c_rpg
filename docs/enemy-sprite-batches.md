# Enemy sprite update checklist

The current roster contains **95 enemy types in 16 batches**. Each `EnemyType` from `src/game/enemy.h` appears exactly once below. Additional boss forms, named hunts, quest enemies, and shared-renderer dependencies are tracked separately.

Progress: **9/95 enemy types completed (batches 1–2)**. Completed boxes have passed artwork and renderer review. Complete one batch at a time, then wait for the next batch request. Mark a sprite complete after its artwork and native-size renderer review pass.

## Batch overview

| Batch | Group | Enemy types | Complete |
|---|---|---:|---|
| 1 | Core dungeon enemies | 6 | Yes |
| 2 | Crypt enemies | 3 | Yes |
| 3 | Haunted Forest | 7 | No |
| 4 | Goblin Mountains | 8 | No |
| 5 | Sunken Coast | 8 | No |
| 6 | Blackwater Swamp and Labyrinth | 5 | No |
| 7 | Dragonspine and Crown Roads | 4 | No |
| 8 | Frostfell | 7 | No |
| 9 | Sunscar Wastes | 6 | No |
| 10 | Moonveil Gardens | 5 | No |
| 11 | Ashen Hollow | 4 | No |
| 12 | Glassdeep Caverns | 4 | No |
| 13 | Royal Catacombs | 5 | No |
| 14 | Original dungeon bosses | 5 | No |
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

- [ ] Pixie — `ENEMY_PIXIE`
- [ ] Blighted Wolf — `ENEMY_BLIGHTED_WOLF`
- [ ] Giant Spider — `ENEMY_GIANT_SPIDER`
- [ ] Dark Elf — `ENEMY_DARK_ELF`
- [ ] Giant Wurm — `ENEMY_GIANT_WURM`
- [ ] Forest Troll — `ENEMY_FOREST_TROLL`
- [ ] Forest Necromancer — `ENEMY_FOREST_NECROMANCER`

### Batch 4: Goblin Mountains (8)

- [ ] Goblin Scout — `ENEMY_GOBLIN_SCOUT`
- [ ] Goblin Archer — `ENEMY_GOBLIN_ARCHER`
- [ ] Goblin Bomber — `ENEMY_GOBLIN_BOMBER`
- [ ] Tunnel Spider — `ENEMY_TUNNEL_SPIDER`
- [ ] Cave Troll — `ENEMY_CAVE_TROLL`
- [ ] Hobgoblin Guard — `ENEMY_HOBGOBLIN_GUARD`
- [ ] Goblin Shaman — `ENEMY_GOBLIN_SHAMAN`
- [ ] Mountain Goblin King — `ENEMY_MOUNTAIN_GOBLIN_KING`

### Batch 5: Sunken Coast (8)

- [ ] Illusion — `ENEMY_ILLUSION`
- [ ] Merfolk — `ENEMY_MERFOLK`
- [ ] Siren — `ENEMY_SIREN`
- [ ] Giant Crab — `ENEMY_GIANT_CRAB`
- [ ] Animated Statue — `ENEMY_ANIMATED_STATUE`
- [ ] Water Elemental — `ENEMY_WATER_ELEMENTAL`
- [ ] Sea Serpent — `ENEMY_SEA_SERPENT`
- [ ] Drowned Queen — `ENEMY_DROWNED_QUEEN`

### Batch 6: Blackwater Swamp and Labyrinth (5)

- [ ] Giant Rat — `ENEMY_GIANT_RAT`
- [ ] Bandit — `ENEMY_BANDIT`
- [ ] Vampire — `ENEMY_VAMPIRE`
- [ ] Swamp Demon — `ENEMY_SWAMP_DEMON`
- [ ] Minotaur — `ENEMY_MINOTAUR`

### Batch 7: Dragonspine and Crown Roads (4)

- [ ] Drake — `ENEMY_DRAKE`
- [ ] Fire Elemental — `ENEMY_FIRE_ELEMENTAL`
- [ ] Road Archer — `ENEMY_ROAD_ARCHER`
- [ ] Horseman — `ENEMY_HORSEMAN`

### Batch 8: Frostfell (7)

- [ ] Ice Wolf — `ENEMY_ICE_WOLF`
- [ ] Frost Archer — `ENEMY_FROST_ARCHER`
- [ ] Yeti — `ENEMY_YETI`
- [ ] Frost Wraith — `ENEMY_FROST_WRAITH`
- [ ] Ice Golem — `ENEMY_ICE_GOLEM`
- [ ] Ice Giant — `ENEMY_ICE_GIANT`
- [ ] Polar Kraken — `ENEMY_POLAR_KRAKEN`

Include the Kraken texture and its fallback drawing. Review its 96×96 appearance with targeted warnings, sweeps, recovery, and the half-health phase.

### Batch 9: Sunscar Wastes (6)

- [ ] Scarab — `ENEMY_SCARAB`
- [ ] Viper — `ENEMY_VIPER`
- [ ] Mummy — `ENEMY_MUMMY`
- [ ] Djinn — `ENEMY_DJINN`
- [ ] Golem — `ENEMY_GOLEM`
- [ ] Desert Pharaoh — `ENEMY_DESERT_PHARAOH`

### Batch 10: Moonveil Gardens (5)

- [ ] Fey Trickster — `ENEMY_FEY_TRICKSTER`
- [ ] Giant Moth — `ENEMY_GIANT_MOTH`
- [ ] Living Flower (displayed as Carnivorous Flower) — `ENEMY_LIVING_FLOWER`
- [ ] Thorn Guardian — `ENEMY_THORN_GUARDIAN`
- [ ] Thorn Regent — `ENEMY_THORN_REGENT`

### Batch 11: Ashen Hollow (4)

- [ ] Cinder Imp — `ENEMY_CINDER_IMP`
- [ ] Ash Hound — `ENEMY_ASH_HOUND`
- [ ] Obsidian Guardian — `ENEMY_OBSIDIAN_GUARDIAN`
- [ ] Cinder Lord — `ENEMY_CINDER_LORD`

### Batch 12: Glassdeep Caverns (4)

- [ ] Crystal Spider — `ENEMY_CRYSTAL_SPIDER`
- [ ] Blind Stalker — `ENEMY_BLIND_STALKER`
- [ ] Shard Golem — `ENEMY_SHARD_GOLEM`
- [ ] Prism Sovereign — `ENEMY_PRISM_SOVEREIGN`

### Batch 13: Royal Catacombs (5)

- [ ] Ancient Skeleton — `ENEMY_ANCIENT_SKELETON`
- [ ] Bone Sentinel — `ENEMY_BONE_SENTINEL`
- [ ] Grave Archer — `ENEMY_GRAVE_ARCHER`
- [ ] Bone Cantor — `ENEMY_BONE_CANTOR`
- [ ] Grave Marshal — `ENEMY_GRAVE_MARSHAL`

### Batch 14: Original dungeon bosses (5)

- [ ] Goblin King — `ENEMY_GOBLIN_KING`
- [ ] Lich King — `ENEMY_LICH_KING`
- [ ] Demon Lord — `ENEMY_DEMON_LORD`
- [ ] Red Dragon — `ENEMY_RED_DRAGON`
- [ ] Tarrasque — `ENEMY_TARRASQUE`

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
| [ ] | Rimefang and Rimefang Pack | Ice Wolf and Frost Wraith, batch 8 |
| [ ] | Shardwarden and Shardwarden Stalker | Shard Golem and Blind Stalker, batch 12 |
| [ ] | Dunehex and Dunehex Mummy | Djinn and Mummy, batch 9 |
| [ ] | Expedition Guardians | Ice Wolf, Ice Giant, Frost Archer, batch 8 |
| [ ] | Resonator Guardians | Crystal Spider, Blind Stalker, Shard Golem, batch 12 |
| [ ] | Emberforge Guardians | Cinder Imp, Ash Hound, Obsidian Guardian, batch 11 |
| [ ] | Moonseed Guardians | Fey Trickster and Giant Moth, batch 10 |
| [ ] | Moonwater Guardian | Living Flower, batch 10 |
| [ ] | Mooncircle Guardians | Thorn Guardian, batch 10 |
| [ ] | Soldiers’ Memorial Guards | Ancient Skeleton and Bone Sentinel, batch 13 |
| [ ] | Watchers’ Memorial Guards | Grave Archer (batch 13), Wraith (batch 2) |
| [ ] | Choir Memorial Guards | Bone Cantor and Ancient Skeleton, batch 13 |
| [ ] | Watchfire Defenders: Mountains objective | Goblin Archer and Hobgoblin Guard, batch 4 |
| [ ] | Watchfire Defenders: Ashen objective | Obsidian Guardian and Cinder Imp, batch 11 |
| [ ] | Watchfire Defenders: Dragonspine objective | Giant (batch 1), Goblin Archer (batch 4), Fire Elemental (batch 7) |
| [x] | Elowen’s burial-seal guardians | Skeleton (batch 1); Crypt Bat, Wraith, Crypt Conjurer (batch 2) |
| [ ] | Lich Guards | Skeleton, batch 1; recheck with Lich King in batch 14 |
| [ ] | Vampire Captor | Vampire, batch 6 |
| [ ] | Tunnel Smugglers | Bandit, batch 6 |
| [ ] | Smuggler Bodyguards | Hobgoblin Guard, batch 4 |
| [ ] | Smuggler Archers | Road Archer, batch 7 |
| [ ] | Escape-tunnel spiders and rats | Tunnel Spider (batch 4), Giant Rat (batch 6) |
| [ ] | Dain fragment carriers | Existing gold border; inspect alongside affected base sprites |
| [ ] | Catacomb corpses and revived enemies | Batch 13; keep revival cues legible |

## Shared artwork and dependencies

- The five catacomb enemies currently call `draw_skeleton()`. Batch 1 must also check the existing catacomb overlays for visual regressions; their dedicated art remains scheduled for batch 13.
- Swamp Demon currently calls `draw_demon_lord()`. Give it its own swamp identity in batch 6 and recheck it when the Demon Lord changes in batch 14.
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
