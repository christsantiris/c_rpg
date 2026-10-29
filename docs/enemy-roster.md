# Approved enemy rosters by area

All 24 area rosters are approved. The game defines **94 active enemy types**.
Named quest, hunt and summoned enemies reuse these types rather than adding
separate definitions. Retired numeric IDs are reserved and migrated in save/load;
existing actors retain their wounds, deaths, quest identities and progress.

Dark Elf Archers hold firing lanes. Dark Elf Rangers use a slower bow cadence
and reposition after shooting. Goblin Warchiefs are armored melee attackers.
The castle Vampire replaces the former armory miniboss while retaining its
stair unlock and reserved equipment reward.

Sun Priests appear only during Sun alignment; Lunar Effigies appear only during
Moon alignment. Living actors hidden by alignment retain their health and status,
remain counted by temple seals, and return when their alignment returns. Defeated
actors never return. Moon Sentinels retain their existing rule: Moon awakens them,
and they remain active if alignment changes back to Sun. Other temple enemies
are active during either alignment.

| Approved | Area | Enemy types |
| --- | --- | --- |
| ✅ | Ashen Hollow | Cinder Imp; Ash Hound; Obsidian Guardian; Fire Elemental; Cinder Lord |
| ✅ | Blackwater Swamp | Giant Rat; Zombie; Bandit; Wraith; Vampire; Swamp Demon |
| ✅ | Castle of No Return — interior | Oathbound Soldier; Iron Warden; Royal Marksman; Court Hexer; Royal Guard; Vampire; Royal Arcanist; Lord Veyr |
| ✅ | Crown Road East | Bandit; Blighted Wolf; Royal Guard; Road Archer; Horseman |
| ✅ | Crown Road North | Goblin Scout; Goblin Archer; Goblin Warchief; Goblin Shaman; Hobgoblin Guard; Blighted Wolf |
| ✅ | Crown Road West | Bandit; Blighted Wolf; Royal Guard; Road Archer; Horseman |
| ✅ | Dragonspine | Goblin Scout; Goblin Archer; Drake; Stone Giant; Fire Elemental; Red Dragon |
| ✅ | Dungeon | Skeleton; Zombie; Crypt Bat; Wraith; Crypt Conjurer; Lich King |
| ✅ | Elderwood Forest | Blighted Wolf; Dark Elf Ranger; Dark Elf Archer; Thorn Dryad; Treefolk; Elderthorn |
| ✅ | Escape Tunnel | Giant Rat; Giant Spider; Bandit (Tunnel Smuggler); Hobgoblin Guard (Smuggler Bodyguard); Road Archer (Smuggler Archer) |
| ✅ | Frostfell | Blighted Wolf; Frost Archer; Yeti; Frost Wraith; Ice Golem; Frost Giant; Frost Kraken |
| ✅ | Giant’s Peaks | Ogre; Cave Troll; Hill Giant; Stone Giant; Stone Troll; Frost Giant; The Last Jarl |
| ✅ | Goblin Mountains | Goblin Scout; Goblin Archer; Goblin Warchief; Giant Spider; Cave Troll; Hobgoblin Guard; Goblin Shaman; Goblin King |
| ✅ | Haunted Forest | Pixie; Blighted Wolf; Giant Spider; Dark Elf Archer; Dark Elf Ranger; Forest Troll; Necromancer |
| ✅ | Ironfang Mines | Goblin Scout; Goblin Archer; Goblin Warchief; Giant Spider; Cave Troll; Hobgoblin Guard; Goblin Shaman; Goblin Mine Boss (stage 5) |
| ✅ | Rook’s Labyrinth | Skeleton; Giant Spider; Animated Statue; Minotaur |
| ✅ | Royal Catacombs | Crypt Bat; Wraith; Crypt Conjurer; Vampire; Ancient Skeleton; Bone Sentinel; Grave Marshal |
| ✅ | Royal Dungeon — evidence room | Royal Guard |
| ✅ | Ruined Temple | Relic Scarabs; Temple Stalker; Dart Hunter; Vine Guardian; Sun Priest (Sun only); Serpent Spirit; Wraith; Lunar Effigy (Moon only); Moon Sentinel (awakened by Moon); Sun Guardian |
| ✅ | Saltwind Marsh | Lizardfolk Scout; Reed Spearhunter; Marsh Spitter; Giant Crab; Lizardfolk Scaleguard; Sszareth |
| ✅ | Stonewake Quarry | Ogre; Quarry Construct; Stone Troll; Stone Giant (stages 4–5); Quarry Colossus |
| ✅ | Stormwatch Headland | Harpy Raider (stages 1–5); Cliff Gargoyle (2–5); Harpy Stormcaller (3–5); Storm Elemental (4–5); Tempest Matriarch (5) |
| ✅ | Sunken Coast | Illusion; Merfolk; Siren; Giant Crab; Animated Statue; Water Elemental; Sea Serpent; Drowned Queen |
| ✅ | Sunscar Wastes | Scarab; Viper; Mummy; Djinn; Golem; Desert Pharaoh |

## Named quest, hunt and summoned enemies

These labels can appear instead of a type's ordinary name. They are not extra
`EnemyType` definitions.

| Enemy | Area(s) |
| --- | --- |
| Lich Guard | Dungeon 5 |
| Lich Shade | Dungeon 5, summoned by the Lich King |
| Grove Revenant | Haunted Forest 5, raised by the Necromancer |
| Map Bearer | Goblin Mountains 2, 3 and 4 |
| Vampire Captor | Blackwater Swamp 3 |
| Tunnel Smuggler | Escape Tunnel |
| Smuggler Bodyguard | Escape Tunnel, deeper section |
| Smuggler Archer | Escape Tunnel, deeper section |
| Watchfire Defender | Goblin Mountains 5; Ashen Hollow 3; Dragonspine 3 |
| Emberforge Guardian | Ashen Hollow 2 and 4 |
| Expedition Guardian | Frostfell 2 and 4 |
| Rimefang | Frostfell 3 |
| Rimefang Pack | Frostfell 3 |
| Shardwarden | Stonewake Quarry 4 |
| Shardwarden Stalker | Stonewake Quarry 4 |
| Dunehex | Sunscar Wastes 3 |
| Dunehex Mummy | Sunscar Wastes 3 |
| Quarry Guardian | Stonewake Quarry 2, 3 and 4 |
| Soldiers' Memorial Guard | Royal Catacombs 2 |
| Watchers' Memorial Guard | Royal Catacombs 3 |
| Choir Memorial Guard | Royal Catacombs 4 |
| Royal Honor Guard | Royal Catacombs 5, summoned by the Grave Marshal |

## Source audit

- [`src/game/enemy.h`](../src/game/enemy.h): complete type definitions.
- [`src/game/game.c`](../src/game/game.c): `spawn_enemy`, `boss_for_level`,
  `enemies_spawn`, `populate_crownroad`, `spawn_labyrinth_enemies`,
  `spawn_temple_enemies`, quest encounters, hunt groups and summoned undead.
- [`src/game/elderwood.c`](../src/game/elderwood.c): `generate` and `add_enemy`.
- [`src/game/mangrove_combat.c`](../src/game/mangrove_combat.c): `mangrove_spawn`
  and `add_enemy`.
- [`src/game/castle.c`](../src/game/castle.c): `castle_spawn`, `spawn_detail` and
  boss reinforcements.
- [`src/game/stormwatch.c`](../src/game/stormwatch.c): `load_stage` and `add_enemy`;
  [`stormwatch_combat.c`](../src/game/stormwatch_combat.c): finite Matriarch reinforcements.
- [`src/game/jail.c`](../src/game/jail.c): `enter_armory` and tunnel encounters.
