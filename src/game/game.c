#include "giants_crown.h"
#include "ironfang.h"
#include "crownroad_north.h"
#include "game.h"
#include "mangrove.h"
#include "stormwatch.h"
#include "coastal_travel.h"
#include "catacombs.h"
#include "elderwood.h"
#include "heartseed.h"
#include "spice_skiff.h"
#include "stormbell.h"
#include "war_machine.h"
#include "castle.h"
#include "lich.h"
#include "necromancer.h"
#include "goblin_king.h"
#include "drowned_queen.h"
#include "swamp_demon.h"
#include "minotaur.h"
#include "grave_marshal.h"
#include "desert_pharaoh.h"
#include "cinder_lord.h"
#include "prism_sovereign.h"
#include "sun_guardian.h"
#include "red_dragon.h"
#include "jail.h"
#include "town_life.h"
#include "approach.h"
#include "town_road.h"
#include "regional_terrain.h"
#include "stonewake.h"
#include "landscapes.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "../game/actions.h"

static void spawn_temple_enemies(GameState *g);

static int region_order_tier(const GameState *g) {
    const Location regions[4] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST
    };
    int tier = 0;
    for (int i = 0; i < 4; i++) {
        if (regions[i] != g->location &&
            (g->defeated_bosses & (1 << regions[i]))) {
            tier++;
        }
    }
    return tier > 3 ? 3 : tier;
}

static void scale_spawned_enemy(const GameState *g, Enemy *e) {
    static const int regular_hp[4] = {100, 145, 180, 200};
    static const int regular_attack[4] = {0, 7, 12, 15};
    static const int boss_hp[4] = {100, 130, 155, 170};
    static const int boss_attack[4] = {0, 6, 10, 13};
    // Temple base stats remain a late-game challenge after the harbor opens.
    int tier = g->location == LOCATION_TEMPLE ? 0 : region_order_tier(g);
    int level_progress = g->player.level - 1;
    if (level_progress < 0) {
        level_progress = 0;
    }
    if (level_progress > 16) {
        level_progress = 16;
    }
    // Later victories add smaller increments; leveling never adds a two-point attack jump.
    int hp_percent = (e->is_boss ? boss_hp[tier] : regular_hp[tier]) +
        20 * level_progress / 16;
    if (e->type != ENEMY_ILLUSION) {
        e->max_hp = (e->max_hp * hp_percent + 50) / 100;
        e->hp = e->max_hp;
    }
    e->attack += (e->is_boss ? boss_attack[tier] : regular_attack[tier]) +
        level_progress / 4;
    e->defense += tier;
    e->experience = (e->experience * (100 + 10 * tier) + 50) / 100;
}

static void spawn_enemy(GameState *g, Enemy *e, EnemyType type, int x, int y) {
    e->active  = 1;
    e->type    = type;
    e->x       = x;
    e->y       = y;
    e->is_boss = 0;
    e->dain_fragment = 0;
    e->frozen_turns = 0;
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    e->move_timer = 0;
    e->revived = 0;
    e->revive_timer = 0;
    e->facing_dx = 0;
    e->facing_dy = -1;
    e->attack_phase = 0;
    e->boss_attack = 0;
    e->boss_wave_used = 0;
    e->boss_aux_x = -1;
    e->boss_aux_y = -1;
    memset(e->dragon_warning, 0, sizeof(e->dragon_warning));
    switch (type) {
        // Elderwood entities are constructed by its compact region generator.
        case ENEMY_GOBLIN_MINE_BOSS:
        case ENEMY_HARPY_RAIDER:
        case ENEMY_CLIFF_GARGOYLE:
        case ENEMY_HARPY_STORMCALLER:
        case ENEMY_STORM_ELEMENTAL:
        case ENEMY_TEMPEST_MATRIARCH:
        case ENEMY_BRIAR_WOLF:
        case ENEMY_IRONHIDE_BOAR:
        case ENEMY_THORN_DRYAD:
        case ENEMY_TREEFOLK:
        case ENEMY_ELDERTHORN:
        case ENEMY_LIZARDFOLK_SCOUT:
        case ENEMY_REED_SPEARHUNTER:
        case ENEMY_MARSH_SPITTER:
        case ENEMY_LIZARDFOLK_SCALEGUARD:
        case ENEMY_SSZARETH:
        case ENEMY_ROYAL_GUARD:
        case ENEMY_OATHBOUND_SOLDIER:
        case ENEMY_IRON_WARDEN:
        case ENEMY_ROYAL_MARKSMAN:
        case ENEMY_COURT_HEXER:
        case ENEMY_BELL_HERALD:
        case ENEMY_CASTELLAN:
        case ENEMY_ROYAL_ARCANIST:
        case ENEMY_LORD_VEYR:
            break;
        case ENEMY_ANCIENT_SKELETON:
            snprintf(e->name, sizeof(e->name), "Ancient Skeleton");
            e->max_hp = 48; e->hp = 48;
            e->attack = 13; e->defense = 4; e->experience = 45;
            break;
        case ENEMY_BONE_SENTINEL:
            snprintf(e->name, sizeof(e->name), "Bone Sentinel");
            e->max_hp = 80; e->hp = 80;
            e->attack = 17; e->defense = 8; e->experience = 75;
            break;
        case ENEMY_GRAVE_MARSHAL:
            snprintf(e->name, sizeof(e->name), "Grave Marshal");
            e->max_hp = 280; e->hp = 280;
            e->attack = 26; e->defense = 8; e->experience = 750;
            e->is_boss = 1;
            break;
        case ENEMY_SKELETON:
            strncpy(e->name, "Skeleton", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 10; e->hp = 10;
            e->attack = 3;  e->defense = 0;
            e->experience = 8;
            break;
        case ENEMY_ZOMBIE:
            strncpy(e->name, "Zombie", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 22; e->hp = 22;
            e->attack = 6;  e->defense = 1;
            e->experience = 14;
            e->move_timer = 0;
            break;
        case ENEMY_CRYPT_BAT:
            strncpy(e->name, "Crypt Bat", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 7; e->hp = 7;
            e->attack = 4; e->defense = 0;
            e->experience = 10;
            break;
        case ENEMY_WRAITH:
            strncpy(e->name, "Wraith", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 18; e->hp = 18;
            e->attack = 7; e->defense = 2;
            e->experience = 22;
            break;
        case ENEMY_GIANT_RAT:
            strncpy(e->name, "Giant Rat", sizeof(e->name) - 1);
            e->max_hp = 18; e->hp = 18;
            e->attack = 6; e->defense = 1; e->experience = 16;
            break;
        case ENEMY_BANDIT:
            strncpy(e->name, "Bandit", sizeof(e->name) - 1);
            e->max_hp = 30; e->hp = 30;
            e->attack = 9; e->defense = 3; e->experience = 34;
            break;
        case ENEMY_VAMPIRE:
            strncpy(e->name, "Vampire", sizeof(e->name) - 1);
            e->max_hp = 65; e->hp = 65;
            e->attack = 15; e->defense = 5; e->experience = 80;
            break;
        case ENEMY_DRAKE:
            snprintf(e->name, sizeof(e->name), "Drake");
            e->max_hp = 75; e->hp = 75;
            e->attack = 17; e->defense = 6; e->experience = 90;
            break;
        case ENEMY_FIRE_ELEMENTAL:
            snprintf(e->name, sizeof(e->name), "Fire Elemental");
            e->max_hp = 58; e->hp = 58;
            e->attack = 19; e->defense = 4; e->experience = 100;
            break;
        case ENEMY_ROAD_ARCHER:
            snprintf(e->name, sizeof(e->name), "Road Archer");
            e->max_hp = 24; e->hp = 24;
            e->attack = 10; e->defense = 2; e->experience = 38;
            e->move_timer = 1;
            break;
        case ENEMY_HORSEMAN:
            snprintf(e->name, sizeof(e->name), "Horseman");
            e->max_hp = 52; e->hp = 52;
            e->attack = 13; e->defense = 4; e->experience = 60;
            break;
        case ENEMY_ICE_WOLF:
            snprintf(e->name, sizeof(e->name), "Ice Wolf");
            e->max_hp = 30; e->hp = 30;
            e->attack = 10; e->defense = 2; e->experience = 32;
            break;
        case ENEMY_FROST_ARCHER:
            snprintf(e->name, sizeof(e->name), "Frost Archer");
            e->max_hp = 26; e->hp = 26;
            e->attack = 11; e->defense = 2; e->experience = 40;
            break;
        case ENEMY_YETI:
            snprintf(e->name, sizeof(e->name), "Yeti");
            e->max_hp = 60; e->hp = 60;
            e->attack = 14; e->defense = 4; e->experience = 70;
            break;
        case ENEMY_FROST_WRAITH:
            snprintf(e->name, sizeof(e->name), "Frost Wraith");
            e->max_hp = 40; e->hp = 40;
            e->attack = 13; e->defense = 3; e->experience = 75;
            break;
        case ENEMY_ICE_GOLEM:
            snprintf(e->name, sizeof(e->name), "Ice Golem");
            e->max_hp = 85; e->hp = 85;
            e->attack = 14; e->defense = 8; e->experience = 95;
            break;
        case ENEMY_FROST_GIANT:
            snprintf(e->name, sizeof(e->name), "Frost Giant");
            e->max_hp = 100; e->hp = 100;
            e->attack = 18; e->defense = 6; e->experience = 130;
            break;
        case ENEMY_FROST_KRAKEN:
            snprintf(e->name, sizeof(e->name), "Frost Kraken");
            e->max_hp = 260; e->hp = 260;
            e->attack = 22; e->defense = 8; e->experience = 650;
            e->is_boss = 1;
            break;
        case ENEMY_SCARAB:
            snprintf(e->name, sizeof(e->name), "Scarab");
            e->max_hp = 24; e->hp = 24;
            e->attack = 8; e->defense = 2; e->experience = 26;
            break;
        case ENEMY_VIPER:
            snprintf(e->name, sizeof(e->name), "Viper");
            e->max_hp = 28; e->hp = 28;
            e->attack = 10; e->defense = 1; e->experience = 32;
            break;
        case ENEMY_MUMMY:
            snprintf(e->name, sizeof(e->name), "Mummy");
            e->max_hp = 60; e->hp = 60;
            e->attack = 14; e->defense = 4; e->experience = 70;
            break;
        case ENEMY_DJINN:
            snprintf(e->name, sizeof(e->name), "Djinn");
            e->max_hp = 42; e->hp = 42;
            e->attack = 14; e->defense = 3; e->experience = 80;
            break;
        case ENEMY_GOLEM:
            snprintf(e->name, sizeof(e->name), "Golem");
            e->max_hp = 90; e->hp = 90;
            e->attack = 16; e->defense = 8; e->experience = 110;
            break;
        case ENEMY_DESERT_PHARAOH:
            snprintf(e->name, sizeof(e->name), "Desert Pharaoh");
            e->max_hp = 220; e->hp = 220;
            e->attack = 22; e->defense = 7; e->experience = 600;
            e->is_boss = 1;
            break;
        case ENEMY_CINDER_IMP:
            snprintf(e->name, sizeof(e->name), "Cinder Imp");
            e->max_hp = 30; e->hp = 30;
            e->attack = 11; e->defense = 2; e->experience = 40;
            break;
        case ENEMY_ASH_HOUND:
            snprintf(e->name, sizeof(e->name), "Ash Hound");
            e->max_hp = 42; e->hp = 42;
            e->attack = 12; e->defense = 3; e->experience = 45;
            break;
        case ENEMY_OBSIDIAN_GUARDIAN:
            snprintf(e->name, sizeof(e->name), "Obsidian Guardian");
            e->max_hp = 100; e->hp = 100;
            e->attack = 18; e->defense = 8; e->experience = 120;
            break;
        case ENEMY_CINDER_LORD:
            snprintf(e->name, sizeof(e->name), "Cinder Lord");
            e->max_hp = 260; e->hp = 260;
            e->attack = 24; e->defense = 9; e->experience = 750;
            e->is_boss = 1;
            break;
        case ENEMY_CRYSTAL_SPIDER:
            snprintf(e->name, sizeof(e->name), "Rockworm");
            e->max_hp = 30; e->hp = 30;
            e->attack = 11; e->defense = 2; e->experience = 40;
            break;
        case ENEMY_BLIND_STALKER:
            snprintf(e->name, sizeof(e->name), "Quarry Construct");
            e->max_hp = 38; e->hp = 38;
            e->attack = 16; e->defense = 3; e->experience = 60;
            break;
        case ENEMY_SHARD_GOLEM:
            snprintf(e->name, sizeof(e->name), "Stone Troll");
            e->max_hp = 110; e->hp = 110;
            e->attack = 18; e->defense = 9; e->experience = 130;
            break;
        case ENEMY_IRON_SENTINEL:
            snprintf(e->name, sizeof(e->name), "Iron Sentinel");
            e->max_hp = 95; e->hp = 95;
            e->attack = 20; e->defense = 10; e->experience = 120;
            break;
        case ENEMY_PRISM_SOVEREIGN:
            snprintf(e->name, sizeof(e->name), "Quarry Colossus");
            e->max_hp = 275; e->hp = 275;
            e->attack = 24; e->defense = 8; e->experience = 800;
            e->is_boss = 1;
            break;
        case ENEMY_SWAMP_DEMON:
            strncpy(e->name, "Swamp Demon", sizeof(e->name) - 1);
            e->max_hp = 180; e->hp = 180;
            e->attack = 20; e->defense = 7; e->experience = 550;
            e->is_boss = 1;
            break;
        case ENEMY_CRYPT_CONJURER:
            strncpy(e->name, "Crypt Conjurer", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 24; e->hp = 24;
            e->attack = 7; e->defense = 1;
            e->experience = 35;
            break;
        case ENEMY_PIXIE:
            strncpy(e->name, "Pixie", sizeof(e->name) - 1);
            e->max_hp = 7; e->hp = 7; e->attack = 4; e->defense = 0;
            e->experience = 9;
            break;
        case ENEMY_BLIGHTED_WOLF:
            strncpy(e->name, "Blighted Wolf", sizeof(e->name) - 1);
            e->max_hp = 14; e->hp = 14; e->attack = 6; e->defense = 1;
            e->experience = 14;
            break;
        case ENEMY_GIANT_SPIDER:
            strncpy(e->name, "Giant Spider", sizeof(e->name) - 1);
            e->max_hp = 18; e->hp = 18; e->attack = 7; e->defense = 2;
            e->experience = 20;
            break;
        case ENEMY_DARK_ELF:
            strncpy(e->name, "Dark Elf", sizeof(e->name) - 1);
            e->max_hp = 22; e->hp = 22; e->attack = 9; e->defense = 3;
            e->experience = 28;
            break;
        case ENEMY_GIANT_WURM:
            strncpy(e->name, "Giant Wurm", sizeof(e->name) - 1);
            e->max_hp = 38; e->hp = 38; e->attack = 12; e->defense = 5;
            e->experience = 42;
            break;
        case ENEMY_FOREST_TROLL:
            strncpy(e->name, "Forest Troll", sizeof(e->name) - 1);
            e->max_hp = 48; e->hp = 48; e->attack = 13; e->defense = 5;
            e->experience = 48;
            break;
        case ENEMY_FOREST_NECROMANCER:
            strncpy(e->name, "Necromancer", sizeof(e->name) - 1);
            e->max_hp = 165; e->hp = 165; e->attack = 18; e->defense = 6;
            e->experience = 450;
            e->is_boss = 1;
            break;
        case ENEMY_GOBLIN_SCOUT:
            strncpy(e->name, "Goblin Scout", 15);
            e->max_hp=12; e->hp=12; e->attack=5; e->defense=1; e->experience=12;
            break;
        case ENEMY_GOBLIN_ARCHER:
            strncpy(e->name, "Goblin Archer", 15);
            e->max_hp=16; e->hp=16; e->attack=7; e->defense=1; e->experience=19;
            break;
        case ENEMY_GOBLIN_BOMBER:
            strncpy(e->name, "Goblin Bomber", 15);
            e->max_hp=18; e->hp=18; e->attack=9; e->defense=1; e->experience=24;
            break;
        case ENEMY_TUNNEL_SPIDER:
            strncpy(e->name, "Tunnel Spider", 15);
            e->max_hp=20; e->hp=20; e->attack=8; e->defense=3; e->experience=25;
            break;
        case ENEMY_CAVE_TROLL:
            strncpy(e->name, "Cave Troll", 15);
            e->max_hp=52; e->hp=52; e->attack=14; e->defense=5; e->experience=52;
            break;
        case ENEMY_HOBGOBLIN_GUARD:
            strncpy(e->name, "Hobgoblin Guard", 15);
            e->max_hp=36; e->hp=36; e->attack=11; e->defense=7; e->experience=44;
            break;
        case ENEMY_GOBLIN_SHAMAN:
            strncpy(e->name, "Goblin Shaman", 15);
            e->max_hp=26; e->hp=26; e->attack=10; e->defense=2; e->experience=46;
            break;
        case ENEMY_MOUNTAIN_GOBLIN_KING:
            strncpy(e->name, "Goblin King", 15);
            e->max_hp=180; e->hp=180; e->attack=20; e->defense=8; e->experience=500;
            e->is_boss=1;
            break;
        case ENEMY_ILLUSION:
            strncpy(e->name, "Illusion", 15);
            e->max_hp = 1; e->hp = 1; e->attack = 7; e->defense = 0;
            e->experience = 12;
            break;
        case ENEMY_MERFOLK:
            strncpy(e->name, "Merfolk", 15);
            e->max_hp = 20; e->hp = 20; e->attack = 8; e->defense = 2;
            e->experience = 22;
            break;
        case ENEMY_SIREN:
            strncpy(e->name, "Siren", 15);
            e->max_hp = 18; e->hp = 18; e->attack = 9; e->defense = 1;
            e->experience = 30;
            break;
        case ENEMY_GIANT_CRAB:
            strncpy(e->name, "Giant Crab", 15);
            e->max_hp = 42; e->hp = 42; e->attack = 11; e->defense = 6;
            e->experience = 38;
            break;
        case ENEMY_ANIMATED_STATUE:
            strncpy(e->name, "Animated Statue", 15);
            e->max_hp = 58; e->hp = 58; e->attack = 14; e->defense = 8;
            e->experience = 56;
            break;
        case ENEMY_MINOTAUR:
            snprintf(e->name, sizeof(e->name), "Minotaur");
            e->max_hp = 120; e->hp = 120; e->attack = 18; e->defense = 7;
            e->experience = 250;
            e->is_boss = 1;
            break;
        case ENEMY_WATER_ELEMENTAL:
            strncpy(e->name, "Water Elemental", 15);
            e->max_hp = 34; e->hp = 34; e->attack = 13; e->defense = 3;
            e->experience = 48;
            break;
        case ENEMY_SEA_SERPENT:
            strncpy(e->name, "Sea Serpent", 15);
            e->max_hp = 40; e->hp = 40; e->attack = 15; e->defense = 4;
            e->experience = 62;
            break;
        case ENEMY_DROWNED_QUEEN:
            strncpy(e->name, "Drowned Queen", 15);
            e->max_hp = 220; e->hp = 220; e->attack = 22; e->defense = 8;
            e->experience = 650;
            e->is_boss = 1;
            break;
        case ENEMY_RELIC_SCARABS:
            strncpy(e->name, "Relic Scarabs", 15);
            e->max_hp = 32; e->hp = 32; e->attack = 13; e->defense = 3;
            e->experience = 55;
            break;
        case ENEMY_TEMPLE_STALKER:
            strncpy(e->name, "Temple Stalker", 15);
            e->max_hp = 48; e->hp = 48; e->attack = 17; e->defense = 5;
            e->experience = 85;
            break;
        case ENEMY_BLOWDART_HUNTER:
            strncpy(e->name, "Dart Hunter", 15);
            e->max_hp = 38; e->hp = 38; e->attack = 16; e->defense = 3;
            e->experience = 80;
            break;
        case ENEMY_VINEBOUND_GUARDIAN:
            strncpy(e->name, "Vine Guardian", 15);
            e->max_hp = 82; e->hp = 82; e->attack = 19; e->defense = 9;
            e->experience = 115;
            break;
        case ENEMY_SUN_PRIEST:
            strncpy(e->name, "Sun Priest", 15);
            e->max_hp = 52; e->hp = 52; e->attack = 18; e->defense = 4;
            e->experience = 105;
            break;
        case ENEMY_SERPENT_SPIRIT:
            strncpy(e->name, "Serpent Spirit", 15);
            e->max_hp = 58; e->hp = 58; e->attack = 19; e->defense = 5;
            e->experience = 115;
            break;
        case ENEMY_TREASURE_WRAITH:
            strncpy(e->name, "Treasure Wraith", 15);
            e->max_hp = 62; e->hp = 62; e->attack = 20; e->defense = 6;
            e->experience = 130;
            break;
        case ENEMY_LUNAR_EFFIGY:
            strncpy(e->name, "Lunar Effigy", 15);
            e->max_hp = 70; e->hp = 70; e->attack = 19; e->defense = 8;
            e->experience = 135;
            break;
        case ENEMY_MOONBOUND_SENTINEL:
            strncpy(e->name, "Moon Sentinel", 15);
            e->max_hp = 95; e->hp = 95; e->attack = 22; e->defense = 10;
            e->experience = 180;
            break;
        case ENEMY_FALLEN_SUN_GUARDIAN:
            strncpy(e->name, "Sun Guardian", 15);
            e->max_hp = 340; e->hp = 340; e->attack = 27; e->defense = 12;
            e->experience = 1000;
            e->is_boss = 1;
            break;
        case ENEMY_OGRE:
            snprintf(e->name, sizeof(e->name), "Ogre");
            e->max_hp = 42; e->hp = 42;
            e->attack = 12; e->defense = 3; e->experience = 45;
            break;
        case ENEMY_HILL_GIANT:
            snprintf(e->name, sizeof(e->name), "Hill Giant");
            e->max_hp = 90; e->hp = 90;
            e->attack = 18; e->defense = 5; e->experience = 110;
            break;
        case ENEMY_LAST_JARL:
            snprintf(e->name, sizeof(e->name), "The Last Jarl");
            e->max_hp = 650; e->hp = 650;
            e->attack = 32; e->defense = 12; e->experience = 1600;
            e->is_boss = 1;
            break;
        case ENEMY_STONE_GIANT:
            strncpy(e->name, "Stone Giant", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 60; e->hp = 60;
            e->attack = 14; e->defense = 6;
            e->experience = 50;
            break;
        case ENEMY_LICH_KING:
            strncpy(e->name, "Lich King", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 140; e->hp = 140;
            e->attack = 18;  e->defense = 6;
            e->experience = 400;
            e->is_boss = 1;
            break;
        case ENEMY_RED_DRAGON:
            strncpy(e->name, "Red Dragon", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 360; e->hp = 360;
            e->attack = 29; e->defense = 11;
            e->experience = 1200;
            e->is_boss = 1;
            break;
    }
    e->name[sizeof(e->name) - 1] = '\0';
    if (g->location == LOCATION_CATACOMBS && (type == ENEMY_CRYPT_BAT || type == ENEMY_WRAITH || type == ENEMY_CRYPT_CONJURER)) {
        e->max_hp *= 2;
        e->hp = e->max_hp;
        e->attack += 6;
        e->defense += 2;
        e->experience *= 2;
    }
    scale_spawned_enemy(g, e);
    if (g->location == LOCATION_GIANTS_CROWN && !e->is_boss) {
        e->max_hp += (g->level - 1) * 9;
        e->attack += g->level - 1;
        if (type == ENEMY_STONE_GIANT) {
            e->max_hp += 50;
            e->attack += 6;
            e->defense += 3;
        }
        e->hp = e->max_hp;
    }
}

void game_spawn_enemy(GameState *g, Enemy *enemy, EnemyType type, int x, int y) {
    spawn_enemy(g, enemy, type, x, y);
}

static int boss_for_level(const GameState *g, EnemyType *type) {
    if (g->location == LOCATION_GIANTS_CROWN || g->location == LOCATION_CROWNROAD_NORTH || g->location == LOCATION_IRONFANG || g->location == LOCATION_STORMWATCH || g->location == LOCATION_ELDERWOOD || g->location == LOCATION_MANGROVES || g->location == LOCATION_MANGROVE_PATH) {
        return 0;
    }
    int boss_level = DUNGEON_DEPTH;
    if (g->location == LOCATION_FOREST) {
        boss_level = FOREST_BOSS_LEVEL;
    } else if (g->location == LOCATION_MOUNTAINS) {
        boss_level = MOUNTAIN_BOSS_LEVEL;
    } else if (g->location == LOCATION_COAST) {
        boss_level = COAST_DEPTH;
    } else if (g->location == LOCATION_SWAMP) {
        boss_level = SWAMP_BOSS_LEVEL;
    } else if (g->location == LOCATION_FROSTFELL) {
        boss_level = FROSTFELL_DEPTH;
    } else if (g->location == LOCATION_DESERT) {
        boss_level = DESERT_DEPTH;
    } else if (g->location == LOCATION_ASHEN) {
        boss_level = ASHEN_DEPTH;
    } else if (g->location == LOCATION_GLASSDEEP) {
        boss_level = GLASSDEEP_DEPTH;
    } else if (g->location == LOCATION_CATACOMBS) {
        boss_level = CATACOMBS_DEPTH;
    } else if (g->location == LOCATION_DRAGONSPINE) {
        boss_level = DRAGONSPINE_DEPTH;
    }
    if (g->level != boss_level) {
        return 0;
    }
    if (g->defeated_bosses & (1 << g->location)) {
        return 0;
    }
    if (g->location == LOCATION_FOREST) {
        *type = ENEMY_FOREST_NECROMANCER;
    } else if (g->location == LOCATION_MOUNTAINS) {
        *type = ENEMY_MOUNTAIN_GOBLIN_KING;
    } else if (g->location == LOCATION_COAST) {
        *type = ENEMY_DROWNED_QUEEN;
    } else if (g->location == LOCATION_SWAMP) {
        *type = ENEMY_SWAMP_DEMON;
    } else if (g->location == LOCATION_FROSTFELL) {
        *type = ENEMY_FROST_KRAKEN;
    } else if (g->location == LOCATION_DESERT) {
        *type = ENEMY_DESERT_PHARAOH;
    } else if (g->location == LOCATION_ASHEN) {
        *type = ENEMY_CINDER_LORD;
    } else if (g->location == LOCATION_GLASSDEEP) {
        *type = ENEMY_PRISM_SOVEREIGN;
    } else if (g->location == LOCATION_CATACOMBS) {
        *type = ENEMY_GRAVE_MARSHAL;
    } else if (g->location == LOCATION_DRAGONSPINE) {
        *type = ENEMY_RED_DRAGON;
    } else {
        *type = ENEMY_LICH_KING;
    }
    return 1;
}

static int enemy_terrain_open(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y) ||
        (g->map.tiles[y][x] != TILE_FLOOR &&
        g->map.tiles[y][x] != TILE_FOREST_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_BRIDGE &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_CAVE_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_FORTRESS_FLOOR &&
        g->map.tiles[y][x] != TILE_COAST_FLOOR &&
        g->map.tiles[y][x] != TILE_COAST_SAND &&
        g->map.tiles[y][x] != TILE_COAST_SHALLOW_WATER &&
        g->map.tiles[y][x] != TILE_COAST_DRAINED_WATER &&
        g->map.tiles[y][x] != TILE_COAST_CHANNEL_DRY &&
        g->map.tiles[y][x] != TILE_SWAMP_FLOOR &&
        g->map.tiles[y][x] != TILE_SWAMP_REEDS &&
        g->map.tiles[y][x] != TILE_DESERT_FLOOR &&
        g->map.tiles[y][x] != TILE_ASHEN_FLOOR &&
        g->map.tiles[y][x] != TILE_ASHEN_RUIN &&
        g->map.tiles[y][x] != TILE_CATACOMBS_FLOOR &&
        g->map.tiles[y][x] != TILE_GLASSDEEP_FLOOR &&
        g->map.tiles[y][x] != TILE_GLASSDEEP_RUIN &&
        g->map.tiles[y][x] != TILE_STONEWAKE_BRIDGE &&
        g->map.tiles[y][x] != TILE_STONEWAKE_CRANE_UP &&
        g->map.tiles[y][x] != TILE_STONEWAKE_CRANE_DOWN &&
        g->map.tiles[y][x] != TILE_FROST_FLOOR &&
        g->map.tiles[y][x] != TILE_FROST_LAKE &&
        g->map.tiles[y][x] != TILE_DRAGON_FLOOR &&
        g->map.tiles[y][x] != TILE_DRAGON_ASH &&
        g->map.tiles[y][x] != TILE_DRAGON_HOARD)) {
        return 0;
    }
    return 1;
}

void game_repair_forest_enemy_positions(Map *m, Enemy *actors, int count, int px, int py) {
    unsigned char reachable[MAP_H][MAP_W] = {{0}};
    int queue[MAP_W * MAP_H];
    int head = 0;
    int tail = 0;
    int start_x = m->stairs_up_x;
    int start_y = m->stairs_up_y;
    if (!map_is_walkable(m, start_x, start_y)) {
        return;
    }
    reachable[start_y][start_x] = 1;
    queue[tail++] = start_y * MAP_W + start_x;
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % MAP_W;
        int y = cell / MAP_W;
        for (int side = 0; side < 4; side++) {
            int nx = x + dx[side];
            int ny = y + dy[side];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H ||
                reachable[ny][nx] || !map_is_walkable(m, nx, ny)) {
                continue;
            }
            reachable[ny][nx] = 1;
            queue[tail++] = ny * MAP_W + nx;
        }
    }

    for (int i = 0; i < count; i++) {
        Enemy *enemy = &actors[i];
        if (!enemy->active) {
            continue;
        }
        int overlaps_earlier = 0;
        for (int other = 0; other < i; other++) {
            if (actors[other].active && actors[other].x == enemy->x &&
                actors[other].y == enemy->y) {
                overlaps_earlier = 1;
                break;
            }
        }
        if (enemy->x >= 0 && enemy->x < MAP_W && enemy->y >= 0 &&
            enemy->y < MAP_H && reachable[enemy->y][enemy->x] &&
            !(enemy->x == start_x && enemy->y == start_y) &&
            !(enemy->x == px && enemy->y == py) && !overlaps_earlier) {
            continue;
        }
        int best_distance = 2 * (MAP_W + MAP_H) + 1;
        int best_x = -1;
        int best_y = -1;
        for (int y = 1; y < MAP_H - 1; y++) {
            for (int x = 1; x < MAP_W - 1; x++) {
                if (!reachable[y][x] || m->tiles[y][x] != TILE_FOREST_FLOOR ||
                    (x == start_x && y == start_y) ||
                    (x == px && y == py)) {
                    continue;
                }
                int occupied = 0;
                for (int other = 0; other < count; other++) {
                    if (other != i && actors[other].active &&
                        actors[other].x == x && actors[other].y == y) {
                        occupied = 1;
                        break;
                    }
                }
                int distance = abs(x - enemy->x) + abs(y - enemy->y);
                for (int side = 0; side < 4; side++) {
                    TileType neighbor = m->tiles[y + dy[side]][x + dx[side]];
                    if (neighbor == TILE_FOREST_WALL ||
                        neighbor == TILE_FOREST_HIDDEN_TRAIL) {
                        distance += MAP_W + MAP_H;
                        break;
                    }
                }
                if (!occupied && distance < best_distance) {
                    best_distance = distance;
                    best_x = x;
                    best_y = y;
                }
            }
        }
        if (best_x >= 0) {
            enemy->x = best_x;
            enemy->y = best_y;
        }
    }
}

static int enemy_tile_open(const GameState *g, int x, int y) {
    // Keep keys, stairs, traps, and portals visible and unobstructed.
    if (!enemy_terrain_open(g, x, y)) {
        return 0;
    }
    if (x == g->map.stairs_up_x && y == g->map.stairs_up_y) {
        return 0;
    }
    if (g->location == LOCATION_FOREST) {
        static const int dx[4] = {0, 1, 0, -1};
        static const int dy[4] = {-1, 0, 1, 0};
        for (int side = 0; side < 4; side++) {
            int nx = x + dx[side];
            int ny = y + dy[side];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) {
                return 0;
            }
            TileType neighbor = g->map.tiles[ny][nx];
            if (neighbor == TILE_FOREST_WALL ||
                neighbor == TILE_FOREST_HIDDEN_TRAIL) {
                return 0;
            }
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active &&
            g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

static int find_enemy_tile_in_room(GameState *g, int room_index, int *x, int *y) {
    Room *room = &g->map.rooms[room_index];
    for (int ty = room->y + 1; ty < room->y + room->h - 1; ty++) {
        for (int tx = room->x + 1; tx < room->x + room->w - 1; tx++) {
            if (enemy_tile_open(g, tx, ty)) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static int find_enemy_tile(GameState *g, int *x, int *y, int room_limit) {
    for (int attempt = 0; attempt < 100; attempt++) {
        int room_idx = rand() % room_limit;
        Room *room = &g->map.rooms[room_idx];
        if (room->w < 3 || room->h < 3) {
            continue;
        }
        int tx = room->x + 1 + rand() % (room->w - 2);
        int ty = room->y + 1 + rand() % (room->h - 2);
        if (enemy_tile_open(g, tx, ty)) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }

    for (int i = 0; i < room_limit; i++) {
        Room *room = &g->map.rooms[i];
        for (int ty = room->y + 1; ty < room->y + room->h - 1; ty++) {
            for (int tx = room->x + 1; tx < room->x + room->w - 1; tx++) {
                if (enemy_tile_open(g, tx, ty)) {
                    *x = tx;
                    *y = ty;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int spawn_into_open_tile(GameState *g, EnemyType type, int room_limit) {
    int limit = g->location == LOCATION_DESERT ? LEVEL_CACHE_ENEMY_CAPACITY - 3 : AREA_ENEMY_LIMIT;
    if (g->enemy_count >= limit) {
        return 0;
    }
    int x;
    int y;
    if (!find_enemy_tile(g, &x, &y, room_limit)) {
        return 0;
    }
    spawn_enemy(g, &g->enemies[g->enemy_count], type, x, y);
    g->enemy_count++;
    return 1;
}

static Enemy *spawn_quest_enemy_at(GameState *g, EnemyType type, int x, int y) {
    int slot = -1;
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        if (g->enemy_count >= AREA_ENEMY_LIMIT) {
            return NULL;
        }
        slot = g->enemy_count++;
    }
    spawn_enemy(g, &g->enemies[slot], type, x, y);
    return &g->enemies[slot];
}

static Enemy *spawn_quest_enemy_open(GameState *g, EnemyType type, int room_limit) {
    int x;
    int y;
    if (!find_enemy_tile(g, &x, &y, room_limit)) {
        return NULL;
    }
    return spawn_quest_enemy_at(g, type, x, y);
}

static void spawn_quest_enemy_near(GameState *g, EnemyType type, int center_x, int center_y) {
    for (int radius = 1; radius <= 4; radius++) {
        for (int y = center_y - radius; y <= center_y + radius; y++) {
            for (int x = center_x - radius; x <= center_x + radius; x++) {
                if ((x != center_x - radius && x != center_x + radius &&
                    y != center_y - radius && y != center_y + radius) ||
                    !enemy_tile_open(g, x, y)) {
                    continue;
                }
                spawn_quest_enemy_at(g, type, x, y);
                return;
            }
        }
    }
}

static int place_dain_map_bearer(GameState *g) {
    if (g->location != LOCATION_MOUNTAINS || g->dain_quest_state != 1) {
        return 0;
    }
    EnemyType target_type;
    int target_bit;
    if (g->level == 2) {
        target_type = ENEMY_GOBLIN_ARCHER;
        target_bit = DAIN_FRAGMENT_ARCHER;
    } else if (g->level == 3) {
        target_type = ENEMY_GOBLIN_BOMBER;
        target_bit = DAIN_FRAGMENT_BOMBER;
    } else if (g->level == 4) {
        target_type = ENEMY_GOBLIN_SHAMAN;
        target_bit = DAIN_FRAGMENT_SHAMAN;
    } else {
        return 0;
    }
    if (g->dain_map_fragments & target_bit) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active &&
            g->enemies[i].dain_fragment == target_bit) {
            return 0;
        }
    }
    int room_limit = g->level == MOUNTAIN_BOSS_LEVEL
        ? g->map.room_count - 1 : g->map.room_count;
    Enemy *target = spawn_quest_enemy_open(g, target_type, room_limit);
    if (!target) {
        return 0;
    }
    target->dain_fragment = target_bit;
    target->max_hp = target->max_hp * 3 / 2;
    target->hp = target->max_hp;
    target->attack += 2;
    strncpy(target->name, "Map Bearer", sizeof(target->name) - 1);
    target->name[sizeof(target->name) - 1] = '\0';
    EnemyType guard = g->level == 2 ? ENEMY_GOBLIN_SCOUT :
        (g->level == 3 ? ENEMY_TUNNEL_SPIDER : ENEMY_HOBGOBLIN_GUARD);
    spawn_quest_enemy_near(g, guard, target->x, target->y);
    spawn_quest_enemy_near(g, guard, target->x, target->y);
    return 1;
}

static int quest_group_pending(const GameState *g) {
    if (g->location == LOCATION_DUNGEON && g->elowen_quest_state == 1) {
        int bit = g->level == 2 ? 1 : (g->level == 3 ? 2 :
            (g->level == 4 ? 4 : 0));
        if (!bit) {
            return 0;
        }
        return !(g->elowen_seals_restored & bit);
    }
    if (g->location == LOCATION_FOREST && g->alder_quest_state == 1) {
        int bit = g->level == 2 ? ALDER_WARDEN_STAGE_2 :
            (g->level == 3 ? ALDER_WARDEN_STAGE_3 :
            (g->level == 4 ? ALDER_WARDEN_STAGE_4 : 0));
        if (!bit) {
            return 0;
        }
        return !(g->alder_wardens_rescued & bit);
    }
    if (g->location == LOCATION_COAST && g->mara_quest_state == 1) {
        int bit = g->level == 2 ? MARA_BEACON_STAGE_2 :
            (g->level == 3 ? MARA_BEACON_STAGE_3 :
            (g->level == 4 ? MARA_BEACON_STAGE_4 : 0));
        if (!bit) {
            return 0;
        }
        return !(g->mara_beacons_lit & bit);
    }
    return 0;
}

Enemy *game_raise_grove_servant(GameState *g, int x, int y, int boss_index) {
    if (!map_is_walkable(&g->map, x, y) || (g->player.x == x && g->player.y == y)) {
        return NULL;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return NULL;
        }
    }
    Enemy *servant = NULL;
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *dead = &g->enemies[i];
        if (!dead->active && !dead->is_boss && !dead->revived &&
            dead->type >= ENEMY_PIXIE && dead->type <= ENEMY_FOREST_TROLL) {
            servant = dead;
            break;
        }
    }
    if (!servant) {
        // Buried woodland servants still exist when no recent corpse is available.
        servant = spawn_quest_enemy_at(g, ENEMY_BLIGHTED_WOLF, x, y);
    }
    if (servant) {
        servant->x = x;
        servant->y = y;
        servant->active = 1;
        servant->hp = servant->max_hp / 2;
        if (servant->hp < 1) {
            servant->hp = 1;
        }
        snprintf(servant->name, sizeof(servant->name), "Grove Revenant");
        servant->experience = 0;
        servant->revived = 1;
        servant->dain_fragment = 0;
        servant->move_timer = 0;
        servant->attack_target_x = -1;
        servant->attack_target_y = -1;
        servant->frozen_turns = servant - g->enemies > boss_index ? 1 : 0;
    }
    return servant;
}

Enemy *game_spawn_marshal_guard(GameState *g, int x, int y, int boss_index) {
    if (g->location != LOCATION_CATACOMBS || g->level != CATACOMBS_DEPTH ||
        !map_is_walkable(&g->map, x, y) || (g->player.x == x && g->player.y == y) ||
        g->enemy_count >= NON_ROAD_ENEMY_LIMIT) {
        return NULL;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return NULL;
        }
    }
    // Append without consuming corpses or their existing resurrection allowances.
    Enemy *guard = &g->enemies[g->enemy_count++];
    spawn_enemy(g, guard, ENEMY_BONE_SENTINEL, x, y);
    snprintf(guard->name, sizeof(guard->name), "Royal Honor Guard");
    guard->experience = 0;
    guard->revived = 1;
    guard->frozen_turns = guard - g->enemies > boss_index ? 1 : 0;
    return guard;
}

Enemy *game_spawn_lich_shade(GameState *g, int x, int y, int boss_index) {
    if (x < 0 || y < 0 || (g->player.x == x && g->player.y == y)) {
        return NULL;
    }
    Enemy *shade = spawn_quest_enemy_at(g, ENEMY_SKELETON, x, y);
    if (shade) {
        snprintf(shade->name, sizeof(shade->name), "Lich Shade");
        shade->experience = 0;
        shade->revived = 1;
        // Appended/reused slots later in this enemy phase skip their first turn.
        shade->frozen_turns = shade - g->enemies > boss_index ? 1 : 0;
    }
    return shade;
}

void game_add_lich_minions(GameState *g) {
    if (g->location != LOCATION_DUNGEON || g->level != DUNGEON_DEPTH ||
        (g->defeated_bosses & (1 << LOCATION_DUNGEON)) || g->map.room_count == 0) {
        return;
    }
    Enemy *boss = NULL;
    int guards = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_LICH_KING && g->enemies[i].active) {
            boss = &g->enemies[i];
        }
        // Fallen guards still count so loading never resurrects them.
        if (strcmp(g->enemies[i].name, "Lich Guard") == 0) {
            guards++;
        }
    }
    if (!boss) {
        return;
    }
    Room *room = &g->map.rooms[g->map.room_count - 1];
    for (int guard = guards; guard < 2 && g->enemy_count < NON_ROAD_ENEMY_LIMIT; guard++) {
        int target_x = boss->x + (guard == 0 ? -2 : 2);
        int best_x = -1;
        int best_y = -1;
        int best_distance = MAP_W + MAP_H;
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                if (!enemy_tile_open(g, x, y) ||
                    (x == g->player.x && y == g->player.y)) {
                    continue;
                }
                int distance = abs(x - target_x) + abs(y - boss->y);
                if (distance < best_distance) {
                    best_x = x;
                    best_y = y;
                    best_distance = distance;
                }
            }
        }
        if (best_x < 0) {
            return;
        }
        Enemy *minion = &g->enemies[g->enemy_count++];
        spawn_enemy(g, minion, ENEMY_SKELETON, best_x, best_y);
        snprintf(minion->name, sizeof(minion->name), "Lich Guard");
    }
}

static int sunscar_population(int level) {
    return 18 + 4 * level;
}

static EnemyType sunscar_enemy(int level, int roll) {
    if (level == 1) {
        return roll < 65 ? ENEMY_SCARAB : ENEMY_VIPER;
    }
    if (level == 2) {
        return roll < 45 ? ENEMY_SCARAB : roll < 75 ? ENEMY_VIPER : ENEMY_MUMMY;
    }
    if (level == 3) {
        return roll < 30 ? ENEMY_SCARAB : roll < 55 ? ENEMY_VIPER : roll < 85 ? ENEMY_MUMMY : ENEMY_DJINN;
    }
    if (level == 4) {
        return roll < 20 ? ENEMY_SCARAB : roll < 40 ? ENEMY_VIPER : roll < 65 ? ENEMY_MUMMY : roll < 85 ? ENEMY_DJINN : ENEMY_GOLEM;
    }
    return roll < 10 ? ENEMY_SCARAB : roll < 25 ? ENEMY_VIPER : roll < 50 ? ENEMY_MUMMY : roll < 75 ? ENEMY_DJINN : ENEMY_GOLEM;
}

void enemies_spawn(GameState *g) {
    if (g->location == LOCATION_CROWNROAD_NORTH) {
        return;
    }
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        castle_spawn(g);
        return;
    }
    g->enemy_count = 0;
    if (g->map.room_count == 0) {
        return;
    }

    int order_tier = region_order_tier(g);
    int difficulty = g->location == LOCATION_FOREST ? map_forest_difficulty(g->level) : g->level;
    if (g->location == LOCATION_SWAMP) {
        difficulty = map_swamp_difficulty(g->level);
    } else if (g->location == LOCATION_MOUNTAINS) {
        difficulty = map_mountain_difficulty(g->level);
    } else if (g->location == LOCATION_COAST) {
        // Keep the early-to-deep enemy progression within five stages.
        difficulty = g->level <= 2 ? g->level : 2 * (g->level - 1);
    }
    int num_enemies = g->location == LOCATION_DESERT ? sunscar_population(g->level) : 10 + difficulty;
    if (g->location == LOCATION_SWAMP && g->level == SWAMP_RESCUE_LEVEL &&
        g->innkeeper_quest_state == 1) {
        num_enemies--;
    }
    if (quest_group_pending(g)) {
        num_enemies -= 3;
    }
    int area_limit = g->location == LOCATION_DESERT ? LEVEL_CACHE_ENEMY_CAPACITY - 10 : AREA_ENEMY_LIMIT;
    if (num_enemies > area_limit) {
        num_enemies = area_limit;
    }

    EnemyType boss_type;
    if (boss_for_level(g, &boss_type)) {
        int boss_x = g->map.stairs_down_x + 1;
        int boss_y = g->map.stairs_down_y;
        if (g->location == LOCATION_FOREST ||
            g->location == LOCATION_MOUNTAINS ||
            g->location == LOCATION_COAST ||
            g->location == LOCATION_SWAMP ||
            g->location == LOCATION_FROSTFELL ||
            g->location == LOCATION_DESERT ||
            g->location == LOCATION_ASHEN ||
            g->location == LOCATION_GLASSDEEP ||
            g->location == LOCATION_CATACOMBS ||
            g->location == LOCATION_DRAGONSPINE) {
            map_room_center(&g->map.rooms[g->map.room_count - 1],
                &boss_x, &boss_y);
        }
        if (!enemy_tile_open(g, boss_x, boss_y)) {
            find_enemy_tile_in_room(g, g->map.room_count - 1,
                &boss_x, &boss_y);
        }
        if (enemy_tile_open(g, boss_x, boss_y)) {
            spawn_enemy(g, &g->enemies[g->enemy_count++], boss_type,
                boss_x, boss_y);
        }
    }

    game_add_lich_minions(g);
    if (g->location == LOCATION_DRAGONSPINE && g->level == DRAGONSPINE_DEPTH) {
        red_dragon_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->location == LOCATION_GLASSDEEP && g->level == GLASSDEEP_DEPTH) {
        prism_sovereign_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->location == LOCATION_ASHEN && g->level == ASHEN_DEPTH) {
        cinder_lord_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }

    if (g->location == LOCATION_DESERT && g->level == DESERT_DEPTH) {
        desert_pharaoh_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->location == LOCATION_CATACOMBS && g->level == CATACOMBS_DEPTH) {
        grave_marshal_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->location == LOCATION_SWAMP && g->level == SWAMP_BOSS_LEVEL) {
        swamp_demon_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->location == LOCATION_COAST && g->level == COAST_DEPTH) {
        drowned_queen_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->location == LOCATION_MOUNTAINS && g->level == MOUNTAIN_BOSS_LEVEL) {
        goblin_king_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->location == LOCATION_FOREST && g->level == FOREST_BOSS_LEVEL) {
        necromancer_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
    if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH) {
        lich_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }

    int boss_level = g->location == LOCATION_FOREST ? FOREST_BOSS_LEVEL :
        (g->location == LOCATION_MOUNTAINS ? MOUNTAIN_BOSS_LEVEL :
        (g->location == LOCATION_COAST ? COAST_DEPTH :
        (g->location == LOCATION_SWAMP ? SWAMP_BOSS_LEVEL :
        (g->location == LOCATION_FROSTFELL ? FROSTFELL_DEPTH :
        (g->location == LOCATION_DESERT ? DESERT_DEPTH :
        (g->location == LOCATION_ASHEN ? ASHEN_DEPTH :
        (g->location == LOCATION_GLASSDEEP ? GLASSDEEP_DEPTH :
        (g->location == LOCATION_CATACOMBS ? CATACOMBS_DEPTH :
        (g->location == LOCATION_DRAGONSPINE ? DRAGONSPINE_DEPTH : DUNGEON_DEPTH)))))))));
    int regular_room_limit = g->level == boss_level
        ? g->map.room_count - 1 : g->map.room_count;
    place_dain_map_bearer(g);
    if (g->location == LOCATION_COAST) {
        for (int y = 1; y < MAP_H - 1; y++) {
            for (int x = 2; x < MAP_W - 1; x++) {
                if (g->map.tiles[y][x] == TILE_COAST_CACHE &&
                    g->enemy_count < num_enemies && enemy_tile_open(g, x - 2, y)) {
                    EnemyType guard = difficulty >= 6 ? ENEMY_SEA_SERPENT :
                        (g->level >= 3 ? ENEMY_ANIMATED_STATUE : ENEMY_GIANT_CRAB);
                    spawn_enemy(g, &g->enemies[g->enemy_count++], guard, x - 2, y);
                }
            }
        }
    }
    while (g->enemy_count < num_enemies) {
        EnemyType type;
        int roll = rand() % 100;
        // Completed regions advance enemy roles without skipping map stages.
        int level = difficulty + 3 * order_tier;

        if (g->location == LOCATION_DRAGONSPINE) {
            if (g->level == 1) {
                type = roll < 25 ? ENEMY_GOBLIN_SCOUT :
                    (roll < 50 ? ENEMY_GOBLIN_ARCHER : ENEMY_DRAKE);
            } else if (g->level == 2) {
                type = roll < 20 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 55 ? ENEMY_DRAKE : ENEMY_STONE_GIANT);
            } else {
                type = roll < 12 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 40 ? ENEMY_DRAKE :
                    (roll < 65 ? ENEMY_STONE_GIANT : ENEMY_FIRE_ELEMENTAL));
            }
        } else if (g->location == LOCATION_CATACOMBS) {
            int slot = g->enemy_count;
            if (slot % 2 == 0) {
                type = slot % 4 == 0 ? ENEMY_BONE_SENTINEL : ENEMY_ANCIENT_SKELETON;
            } else if (g->level >= 3 && slot % 6 == 5) {
                type = ENEMY_CRYPT_CONJURER;
            } else if (g->level >= 2 && slot % 6 == 3) {
                type = ENEMY_WRAITH;
            } else if (g->level >= 2 && slot % 4 == 1) {
                type = ENEMY_VAMPIRE;
            } else {
                type = ENEMY_CRYPT_BAT;
            }
        } else if (g->location == LOCATION_GLASSDEEP) {
            if (g->level < 3) {
                type = roll < 65 ? ENEMY_CRYSTAL_SPIDER : ENEMY_BLIND_STALKER;
            } else {
                type = roll < 25 ? ENEMY_CRYSTAL_SPIDER :
                    (roll < 55 ? ENEMY_BLIND_STALKER :
                    (g->level >= 4 && roll >= 80 ? ENEMY_IRON_SENTINEL : ENEMY_SHARD_GOLEM));
            }
        } else if (g->location == LOCATION_ASHEN) {
            if (g->level < 3) {
                type = roll < 55 ? ENEMY_CINDER_IMP : ENEMY_ASH_HOUND;
            } else {
                type = roll < 25 ? ENEMY_CINDER_IMP :
                    (roll < 55 ? ENEMY_ASH_HOUND :
                    (roll < 80 ? ENEMY_OBSIDIAN_GUARDIAN : ENEMY_FIRE_ELEMENTAL));
            }
        } else if (g->location == LOCATION_DESERT) {
            type = sunscar_enemy(g->level, roll);
        } else if (g->location == LOCATION_FROSTFELL) {
            // Each deeper stage adds one heavier kind of frost creature.
            if (g->level == 1) {
                type = roll < 60 ? ENEMY_ICE_WOLF : ENEMY_FROST_ARCHER;
            } else if (g->level == 2) {
                type = roll < 40 ? ENEMY_ICE_WOLF :
                    (roll < 70 ? ENEMY_FROST_ARCHER : ENEMY_YETI);
            } else if (g->level == 3) {
                type = roll < 25 ? ENEMY_ICE_WOLF :
                    (roll < 50 ? ENEMY_FROST_ARCHER :
                    (roll < 75 ? ENEMY_YETI : ENEMY_FROST_WRAITH));
            } else if (g->level == 4) {
                type = roll < 15 ? ENEMY_ICE_WOLF :
                    (roll < 35 ? ENEMY_FROST_ARCHER :
                    (roll < 55 ? ENEMY_YETI :
                    (roll < 80 ? ENEMY_FROST_WRAITH : ENEMY_ICE_GOLEM)));
            } else {
                type = roll < 10 ? ENEMY_ICE_WOLF :
                    (roll < 25 ? ENEMY_FROST_ARCHER :
                    (roll < 40 ? ENEMY_YETI :
                    (roll < 60 ? ENEMY_FROST_WRAITH :
                    (roll < 80 ? ENEMY_ICE_GOLEM : ENEMY_FROST_GIANT))));
            }
        } else if (g->location == LOCATION_SWAMP) {
            if (difficulty <= 2) {
                type = roll < 30 ? ENEMY_GIANT_RAT :
                    (roll < 55 ? ENEMY_ZOMBIE :
                    (roll < 78 ? ENEMY_BANDIT : ENEMY_WRAITH));
            } else if (difficulty <= 4) {
                type = roll < 15 ? ENEMY_GIANT_RAT :
                    (roll < 35 ? ENEMY_ZOMBIE :
                    (roll < 58 ? ENEMY_BANDIT :
                    (roll < 80 ? ENEMY_WRAITH : ENEMY_VAMPIRE)));
            } else {
                type = roll < 10 ? ENEMY_GIANT_RAT :
                    (roll < 25 ? ENEMY_ZOMBIE :
                    (roll < 45 ? ENEMY_BANDIT :
                    (roll < 65 ? ENEMY_WRAITH : ENEMY_VAMPIRE)));
            }
        } else if (g->location == LOCATION_COAST) {
            if (level == 1) {
                type = roll < 55 ? ENEMY_ILLUSION : ENEMY_MERFOLK;
            } else if (level == 2) {
                type = roll < 30 ? ENEMY_ILLUSION :
                    (roll < 70 ? ENEMY_MERFOLK : ENEMY_GIANT_CRAB);
            } else if (level == 3) {
                type = roll < 25 ? ENEMY_MERFOLK :
                    (roll < 55 ? ENEMY_SIREN :
                    (roll < 80 ? ENEMY_GIANT_CRAB : ENEMY_ILLUSION));
            } else if (level == 4) {
                type = roll < 25 ? ENEMY_SIREN :
                    (roll < 50 ? ENEMY_GIANT_CRAB :
                    (roll < 75 ? ENEMY_ANIMATED_STATUE : ENEMY_MERFOLK));
            } else if (level == 5) {
                type = roll < 25 ? ENEMY_GIANT_CRAB :
                    (roll < 50 ? ENEMY_ANIMATED_STATUE :
                    (roll < 75 ? ENEMY_WATER_ELEMENTAL : ENEMY_SIREN));
            } else {
                type = roll < 15 ? ENEMY_MERFOLK :
                    (roll < 30 ? ENEMY_SIREN :
                    (roll < 45 ? ENEMY_GIANT_CRAB :
                    (roll < 62 ? ENEMY_ANIMATED_STATUE :
                    (roll < 82 ? ENEMY_WATER_ELEMENTAL : ENEMY_SEA_SERPENT))));
            }
        } else if (g->location == LOCATION_MOUNTAINS) {
            if (level == 1)
                type = ENEMY_GOBLIN_SCOUT;
            else if (level == 2)
                type = roll < 55 ? ENEMY_GOBLIN_SCOUT : ENEMY_GOBLIN_ARCHER;
            else if (level == 3)
                type = roll < 25 ? ENEMY_GOBLIN_SCOUT :
                    (roll < 50 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 75 ? ENEMY_GOBLIN_BOMBER : ENEMY_TUNNEL_SPIDER));
            else if (level == 4)
                type = roll < 20 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 40 ? ENEMY_GOBLIN_BOMBER :
                    (roll < 60 ? ENEMY_TUNNEL_SPIDER :
                    (roll < 80 ? ENEMY_CAVE_TROLL : ENEMY_HOBGOBLIN_GUARD)));
            else
                type = roll < 15 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 30 ? ENEMY_GOBLIN_BOMBER :
                    (roll < 45 ? ENEMY_TUNNEL_SPIDER :
                    (roll < 62 ? ENEMY_CAVE_TROLL :
                    (roll < 82 ? ENEMY_HOBGOBLIN_GUARD : ENEMY_GOBLIN_SHAMAN))));
        } else if (g->location == LOCATION_FOREST) {
            if (level == 1)
                type = roll < 55 ? ENEMY_PIXIE : ENEMY_BLIGHTED_WOLF;
            else if (level == 2)
                type = roll < 30 ? ENEMY_PIXIE :
                    (roll < 65 ? ENEMY_BLIGHTED_WOLF : ENEMY_GIANT_SPIDER);
            else if (level == 3)
                type = roll < 20 ? ENEMY_PIXIE :
                    (roll < 45 ? ENEMY_BLIGHTED_WOLF :
                    (roll < 70 ? ENEMY_GIANT_SPIDER : ENEMY_DARK_ELF));
            else if (level == 4)
                type = roll < 20 ? ENEMY_GIANT_SPIDER :
                    (roll < 45 ? ENEMY_DARK_ELF :
                    (roll < 70 ? ENEMY_GIANT_WURM : ENEMY_FOREST_TROLL));
            else
                type = roll < 15 ? ENEMY_PIXIE :
                    (roll < 30 ? ENEMY_BLIGHTED_WOLF :
                    (roll < 45 ? ENEMY_GIANT_SPIDER :
                    (roll < 65 ? ENEMY_DARK_ELF :
                    (roll < 82 ? ENEMY_GIANT_WURM : ENEMY_FOREST_TROLL))));
        } else if (level == 1) {
            type = ENEMY_SKELETON;
        } else if (level == 2) {
            if (roll < 50) type = ENEMY_SKELETON;
            else if (roll < 80) type = ENEMY_ZOMBIE;
            else type = ENEMY_CRYPT_BAT;
        } else if (level == 3) {
            if (roll < 30) type = ENEMY_SKELETON;
            else if (roll < 65) type = ENEMY_ZOMBIE;
            else if (roll < 85) type = ENEMY_CRYPT_BAT;
            else type = ENEMY_WRAITH;
        } else if (level == 4) {
            if (roll < 20) type = ENEMY_SKELETON;
            else if (roll < 55) type = ENEMY_ZOMBIE;
            else if (roll < 70) type = ENEMY_CRYPT_BAT;
            else if (roll < 90) type = ENEMY_WRAITH;
            else type = ENEMY_CRYPT_CONJURER;
        } else {
            if (roll < 15) type = ENEMY_SKELETON;
            else if (roll < 45) type = ENEMY_ZOMBIE;
            else if (roll < 60) type = ENEMY_CRYPT_BAT;
            else if (roll < 80) type = ENEMY_WRAITH;
            else type = ENEMY_CRYPT_CONJURER;
        }
        if (g->location == LOCATION_CATACOMBS) {
            int room = 1 + (g->enemy_count - (g->level == CATACOMBS_DEPTH && !(g->defeated_bosses & (1 << LOCATION_CATACOMBS)))) / 2;
            if (room >= regular_room_limit) {
                break;
            }
            int x;
            int y;
            if (!find_enemy_tile_in_room(g, room, &x, &y)) {
                break;
            }
            spawn_enemy(g, &g->enemies[g->enemy_count++], type, x, y);
            continue;
        }
        if (!spawn_into_open_tile(g, type, regular_room_limit)) {
            break;
        }
    }
    if (g->location == LOCATION_FOREST) {
        game_repair_forest_enemy_positions(&g->map, g->enemies,
            g->enemy_count, g->map.stairs_up_x, g->map.stairs_up_y);
    }
}

void game_init(GameState *g) {
    memset(g->giants_crown, 0, sizeof(g->giants_crown));
    g->giants_crown_defeated = 0;
    memset(&g->northroad, 0, sizeof(g->northroad));
    memset(g->ironfang, 0, sizeof(g->ironfang));
    g->ironfang_defeated = 0;
    g->war_machine_quest_state = 0;
    g->war_machine_progress = 0;
    memset(g->stormwatch, 0, sizeof(g->stormwatch));
    g->stormwatch_defeated = 0;
    g->stormbell_quest_state = 0;
    g->stormbell_progress = 0;
    g->stormbell_phase = 0;
    memset(g->stormbell_guards, 0, sizeof(g->stormbell_guards));
    g->mangroves = (MangroveState){0};
    memset(g->mangrove_stages, 0, sizeof(g->mangrove_stages));
    g->mangrove_defeated = 0;
    g->road_town = LOCATION_TOWN;
    g->approach_route = -1;
    g->jail_quest_state = 0;
    memset(g->jail_gear, 0, sizeof(g->jail_gear));
    g->jail_gear_count = 0;
    g->jail_gear_claimed = 0;
    g->jail_gear_state = 0;
    g->jail_main_hand = -1;
    g->jail_off_hand = -1;
    g->jail_armor = -1;
    g->jail_supplies_taken = 0;
    g->jail_armory_initialized = 0;
    memset(g->jail_guards, 0, sizeof(g->jail_guards));
    memset(g->jail_armory_loot, 0, sizeof(g->jail_armory_loot));
    g->jail_armory_loot_count = 0;
    g->prisoner_x = 0;
    g->prisoner_y = 0;
    memset(g->castle_cache, 0, sizeof(g->castle_cache));
    memset(g->castle_loot, 0, sizeof(g->castle_loot));
    memset(g->castle_loot_count, 0, sizeof(g->castle_loot_count));
    memset(g->castle_fire_phase, 0, sizeof(g->castle_fire_phase));
    g->castle_minibosses = 0;
    g->castellan_reward_state = 0;
    g->arcanist_reward_state = 0;
    g->castle_prompt = 0;
    g->game_won = 0;
    g->crown_of_unbound = 0;
    srand((unsigned)time(NULL));
    g->level = 1;
    g->crownroad_cache.valid = 0;
    g->kingroad_west_cache = (CrownroadCache){0};
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        g->level_cache[i].valid = 0;
        g->forest_cache[i].valid = 0;
        g->mountain_cache[i].valid = 0;
        g->coast_cache[i].valid = 0;
        if (i < SWAMP_CACHE_DEPTH) {
            g->swamp_cache[i].valid = 0;
        }
        if (i < DRAGONSPINE_DEPTH) {
            g->dragonspine_cache[i].valid = 0;
        }
        if (i < FROSTFELL_DEPTH) {
            g->frostfell_cache[i].valid = 0;
        }

        if (i < ASHEN_DEPTH) {
            g->ashen_cache[i] = (LevelCache){0};
        }
        if (i < CATACOMBS_DEPTH) {
            g->catacombs_cache[i] = (LevelCache){0};
        }
        if (i < GLASSDEEP_DEPTH) {
            g->glassdeep_cache[i] = (LevelCache){0};
        }
        if (i < DESERT_DEPTH) {
            g->desert_cache[i] = (LevelCache){0};
        }
    }
    g->message_count = 0;
    g->level_cleared = 0;
    controls_reset(g->key_bindings);
    g->max_level_reached = 1;
    g->dungeon_entry_town = LOCATION_TOWN;
    g->dungeon_portal_town = LOCATION_TOWN;
    g->max_forest_level_reached = 1;
    g->forest_entry_town = LOCATION_TOWN;
    g->forest_portal_town = LOCATION_TOWN;
    g->max_mountain_level_reached = 1;
    g->mountain_entry_town = LOCATION_DAWNMERE;
    g->mountain_portal_town = LOCATION_DAWNMERE;
    g->max_coast_level_reached = 1;
    g->max_swamp_level_reached = 1;
    g->swamp_entry_town = LOCATION_TOWN2;
    g->swamp_portal_town = LOCATION_TOWN2;
    g->max_dragonspine_level_reached = 1;
    g->max_frostfell_level_reached = 1;
    g->max_desert_level_reached = 1;
    g->max_ashen_level_reached = 1;
    g->max_glassdeep_level_reached = 1;
    g->max_catacombs_level_reached = 1;
    g->catacombs_mantle_unclaimed = 0;
    g->max_temple_level_reached = 1;
    g->location = LOCATION_TOWN;
    int spawn_x, spawn_y;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
    g->player.name[0] = '\0';
    g->player.level = 1;
    g->player.experience = 0;
    g->player.experience_next = 100;
    g->inventory_count = 0;
    g->equipped_main_hand = -1;
    g->equipped_off_hand = -1;
    g->equipped_armor = -1;
    g->gold = 0;
    g->labyrinth_entry_town = LOCATION_TOWN2;
    g->rook_quest_state = 0;
    g->innkeeper_quest_state = 0;
    g->rook_labyrinth_switches = 0;
    g->rook_quest_completions = 0;
    g->score = 0;
    g->dungeon_key_found = 0;
    g->dungeon_crypt_keys = 0;
    g->lich_reward_state = 0;
    g->necromancer_reward_state = 0;
    g->goblin_king_reward_state = 0;
    g->grave_marshal_reward_state = 0;
    g->minotaur_reward_state = 0;
    g->swamp_demon_reward_state = 0;
    g->drowned_queen_reward_state = 0;
    g->portal_active = 0;
    g->portal_level = 0;
    g->portal_location = LOCATION_DUNGEON;
    g->portal_x = 0;
    g->portal_y = 0;
    g->portal_origin_tile = TILE_FLOOR;
    g->defeated_bosses = 0;
    g->kraken_bow_unclaimed = 0;
    g->sandstorm_staff_unclaimed = 0;
    g->desert_pharaoh_reward_state = 0;
    g->cinder_lord_reward_pending = 0;
    g->prism_sovereign_reward_pending = 0;
    g->sun_guardian_reward_state = 0;
    g->red_dragon_reward_state = 0;
    g->elowen_quest_state = 0;
    g->elowen_seals_restored = 0;
    g->dain_quest_state = 0;
    g->dain_map_fragments = 0;
    g->alder_quest_state = 0;
    g->alder_wardens_rescued = 0;
    g->mara_quest_state = 0;
    g->mara_beacons_lit = 0;
    g->cain_scroll_given = 0;
    g->island_travel_unlocked = 0;
    g->debug_passages_unlocked = 0;
    memset(g->elderwood_cache, 0, sizeof(g->elderwood_cache));
    g->max_elderwood_level_reached = 1;
    g->elderwood_defeated = 0;
    g->heartseed_quest_state = 0;
    g->heartseed_progress = 0;
    g->heartseed_bindings = 0;
    g->heartseed_sites = 0;
    g->heartseed_guardian = HEARTSEED_UNPLACED;
    g->spice_skiff_quest_state = 0;
    g->spice_skiff_progress = 0;
    g->retired_coast_item_count = 0;
    g->elderwood_entry_town = LOCATION_TOWN;
    g->catacombs_entry_town = LOCATION_CASTLE;
    g->northern_portal_town = LOCATION_CASTLE;
    g->last_town_visited = LOCATION_TOWN;
    g->return_portal_town = -1;
    g->dragon_treasure_quest_state = 0;
    g->sunscar_lamp_quest_state = 0;
    g->emberforge_quest_state = 0;
    g->emberforge_progress = 0;
    g->emberforge_encounters = 0;
    g->frostfell_quest_state = 0;
    g->frostfell_quest_progress = 0;
    g->frostfell_quest_encounters = 0;
    g->glassdeep_quest_state = 0;
    g->stonewake_warhammer_state = 0;
    g->glassdeep_quest_progress = 0;
    g->glassdeep_quest_encounters = 0;
    g->catacombs_quest_state = 0;
    g->catacombs_quest_progress = 0;
    g->catacombs_quest_encounters = 0;
    g->watchfire_quest_state = 0;
    g->watchfire_quest_progress = 0;
    g->watchfire_quest_encounters = 0;
    g->hunt_quest_state = 0;
    g->hunt_quest_progress = 0;
    g->hunt_quest_encounters = 0;
    for (int i = 0; i < TEMPLE_DEPTH; i++) {
        g->temple_cache[i].valid = 0;
    }
    for (int i = 0; i < LABYRINTH_DEPTH; i++) {
        g->labyrinth_cache[i].valid = 0;
    }
    g->temple_alignment = 0;
    g->temple_sentinels_awakened = 0;
    g->temple_treasure_state = 0;
    g->dialogue_active = 0;
    g->dialogue_speaker[0] = '\0';
    g->dialogue_text[0] = '\0';
    g->dialogue_x = 0;
    g->dialogue_y = 0;
    g->floor_item_count = 0;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[i].active = 0;
    }
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        g->floor_items[i].active = 0;
    }

    g->player.known_spell_count = 0;
    g->player.equipped_spell = -1;
    g->player.last_dx = 0;
    g->player.last_dy = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    g->player.arrows = g->player.player_class == CLASS_ROGUE ? MAX_ARROWS : 0;
    g->trail_count = 0;
    g->trail_frames = 0;
    g->trail_effect = TRAIL_EFFECT_GENERIC;
    g->trail_started_at = 0;

    switch (g->player.player_class) {
        case CLASS_WARRIOR:
            g->player.max_hp = 150;
            g->player.max_mp = 20;
            g->player.attack = 14;
            g->player.defense = 6;
            g->inventory[g->inventory_count++] = item_make_rusty_sword();
            break;
        case CLASS_MAGE:
            g->player.max_hp = 70;
            g->player.max_mp = 100;
            g->player.attack = 4;
            g->player.defense = 2;
            g->inventory[g->inventory_count++] = item_make_staff();
            g->inventory[g->inventory_count++] = item_make_scroll_magic_arrow();
            break;
        case CLASS_ROGUE:
            g->player.max_hp = 100;
            g->player.max_mp = 40;
            g->player.attack = 10;
            g->player.defense = 4;
            g->inventory[g->inventory_count++] = item_make_bow();
            g->inventory[g->inventory_count++] = item_make_rusty_dagger();
            break;
    }
    g->player.hp = g->player.max_hp;
    g->player.mp = g->player.max_mp;

    // Spawn enemies in random rooms
    enemies_spawn(g);
}

void game_move_player(GameState *g, int dx, int dy) {
    int nx = g->player.x + dx;
    int ny = g->player.y + dy;
    if (!map_is_walkable(&g->map, nx, ny)) {
        return;
    }
    g->player.x = nx;
    g->player.y = ny;
    if (!game_shortcut_prompt_active(g)) {
        g->dialogue_active = 0;
    }
}

static int repaired_equipment_index(const GameState *g, int index, ItemType type) {
    if (index < 0) {
        return -1;
    }
    int start = index < g->inventory_count ? index : g->inventory_count - 1;
    for (int i = start; i >= 0; i--) {
        if (g->inventory[i].type == type) {
            return i;
        }
    }
    for (int i = start + 1; i < g->inventory_count; i++) {
        if (g->inventory[i].type == type) {
            return i;
        }
    }
    return -1;
}

static int valid_off_hand_item(const Item *item) {
    return item->type == ITEM_SHIELD ||
        (item->type == ITEM_WEAPON &&
        item->weapon_hands == WEAPON_HANDS_ONE);
}

void game_repair_equipment_indices(GameState *g) {
    if (g->equipped_main_hand >= g->inventory_count ||
        (g->equipped_main_hand >= 0 &&
        g->inventory[g->equipped_main_hand].type != ITEM_WEAPON)) {
        g->equipped_main_hand = repaired_equipment_index(g,
            g->equipped_main_hand, ITEM_WEAPON);
    }
    if (g->equipped_off_hand >= g->inventory_count ||
        (g->equipped_off_hand >= 0 &&
        !valid_off_hand_item(&g->inventory[g->equipped_off_hand]))) {
        g->equipped_off_hand = -1;
    }
    if (g->equipped_off_hand == g->equipped_main_hand) {
        g->equipped_off_hand = -1;
    }
    if (g->equipped_off_hand >= 0 &&
        (g->equipped_main_hand < 0 ||
        g->inventory[g->equipped_main_hand].weapon_hands == WEAPON_HANDS_TWO ||
        !valid_off_hand_item(&g->inventory[g->equipped_off_hand]))) {
        g->equipped_off_hand = -1;
    }
    if (g->equipped_armor >= g->inventory_count ||
        (g->equipped_armor >= 0 &&
        g->inventory[g->equipped_armor].type != ITEM_ARMOR)) {
        g->equipped_armor = repaired_equipment_index(g,
            g->equipped_armor, ITEM_ARMOR);
    }
}

int game_off_hand_attack_bonus(const Item *weapon) {
    return (weapon->attack_bonus + 1) / 2;
}

void game_unequip_main_hand(GameState *g) {
    if (g->equipped_main_hand < 0 ||
        g->equipped_main_hand >= g->inventory_count) {
        g->equipped_main_hand = -1;
        return;
    }
    Item *weapon = &g->inventory[g->equipped_main_hand];
    g->player.attack -= weapon->attack_bonus;
    g->player.max_hp -= weapon->max_hp_bonus;
    if (g->player.hp > g->player.max_hp) {
        g->player.hp = g->player.max_hp;
    }
    g->player.max_mp -= weapon->max_mp_bonus;
    if (g->player.mp > g->player.max_mp) {
        g->player.mp = g->player.max_mp;
    }
    g->equipped_main_hand = -1;
}

void game_unequip_off_hand(GameState *g) {
    if (g->equipped_off_hand < 0 ||
        g->equipped_off_hand >= g->inventory_count) {
        g->equipped_off_hand = -1;
        return;
    }
    Item *item = &g->inventory[g->equipped_off_hand];
    if (item->type == ITEM_WEAPON) {
        g->player.attack -= game_off_hand_attack_bonus(item);
    } else if (item->type == ITEM_SHIELD) {
        g->player.defense -= item->defense_bonus;
    }
    g->equipped_off_hand = -1;
}

int game_equip_off_hand(GameState *g, int index) {
    if (index < 0 || index >= g->inventory_count ||
        g->equipped_main_hand < 0 ||
        g->equipped_main_hand >= g->inventory_count ||
        index == g->equipped_main_hand) {
        return 0;
    }
    Item *main_hand = &g->inventory[g->equipped_main_hand];
    Item *weapon = &g->inventory[index];
    if (main_hand->weapon_hands != WEAPON_HANDS_ONE ||
        weapon->type != ITEM_WEAPON ||
        weapon->weapon_hands != WEAPON_HANDS_ONE ||
        !item_class_allowed(weapon, g->player.player_class)) {
        return 0;
    }
    game_unequip_off_hand(g);
    g->equipped_off_hand = index;
    g->player.attack += game_off_hand_attack_bonus(weapon);
    return 1;
}

int game_equip_shield(GameState *g, int index) {
    if (index < 0 || index >= g->inventory_count ||
        g->equipped_main_hand < 0 ||
        g->equipped_main_hand >= g->inventory_count) {
        return 0;
    }
    Item *main_hand = &g->inventory[g->equipped_main_hand];
    Item *shield = &g->inventory[index];
    if (main_hand->weapon_hands != WEAPON_HANDS_ONE ||
        shield->type != ITEM_SHIELD ||
        !item_class_allowed(shield, g->player.player_class)) {
        return 0;
    }
    game_unequip_off_hand(g);
    g->equipped_off_hand = index;
    g->player.defense += shield->defense_bonus;
    return 1;
}

int game_equip_main_hand(GameState *g, int index) {
    if (index < 0 || index >= g->inventory_count) {
        return 0;
    }
    Item *weapon = &g->inventory[index];
    if (weapon->type != ITEM_WEAPON ||
        !item_class_allowed(weapon, g->player.player_class)) {
        return 0;
    }
    if (g->equipped_off_hand == index ||
        weapon->weapon_hands == WEAPON_HANDS_TWO) {
        game_unequip_off_hand(g);
    }
    int hp_before = g->player.hp;
    game_unequip_main_hand(g);
    g->equipped_main_hand = index;
    g->player.attack += weapon->attack_bonus;
    g->player.max_hp += weapon->max_hp_bonus;
    g->player.hp = hp_before < g->player.max_hp ? hp_before : g->player.max_hp;
    g->player.max_mp += weapon->max_mp_bonus;
    return 1;
}

void game_recover_retired_items(GameState *g) {
    if (town_road_index(g->location) < 0) {
        return;
    }
    int recovered = 0;
    while (g->retired_coast_item_count > 0) {
        Item *item = &g->retired_coast_items[g->retired_coast_item_count - 1];
        if (item->type == ITEM_GOLD) {
            g->gold += item->value;
            g->score += item->value;
        } else if (g->inventory_count < MAX_INVENTORY) {
            g->inventory[g->inventory_count++] = *item;
        } else {
            break;
        }
        g->retired_coast_item_count--;
        recovered++;
    }
    if (recovered) {
        push_message(g, "Items from a retired area have been returned to you.");
    }
}

void game_remove_inventory_item(GameState *g, int index) {
    if (index < 0 || index >= g->inventory_count) {
        return;
    }
    Item *item = &g->inventory[index];
    int removing_main_hand = g->equipped_main_hand == index;
    int promote_off_hand = removing_main_hand &&
        g->equipped_off_hand >= 0 &&
        g->inventory[g->equipped_off_hand].type == ITEM_WEAPON;
    if (g->equipped_main_hand == index) {
        game_unequip_main_hand(g);
    }
    if (promote_off_hand) {
        int promoted = g->equipped_off_hand;
        game_unequip_off_hand(g);
        g->equipped_main_hand = promoted;
        g->player.attack += g->inventory[promoted].attack_bonus;
    } else if (removing_main_hand && g->equipped_off_hand >= 0) {
        game_unequip_off_hand(g);
    } else if (g->equipped_off_hand == index) {
        game_unequip_off_hand(g);
    }
    if (g->equipped_armor == index) {
        game_remove_armor_bonuses(g, item);
        g->equipped_armor = -1;
    }
    for (int i = index; i < g->inventory_count - 1; i++) {
        g->inventory[i] = g->inventory[i + 1];
    }
    g->inventory_count--;
    if (g->equipped_main_hand > index) {
        g->equipped_main_hand--;
    }
    if (g->equipped_off_hand > index) {
        g->equipped_off_hand--;
    }
    if (g->equipped_armor > index) {
        g->equipped_armor--;
    }
    game_recover_retired_items(g);
}

void game_apply_armor_bonuses(GameState *g, const Item *armor) {
    g->player.defense += armor->defense_bonus;
    g->player.max_hp += armor->max_hp_bonus;
    g->player.hp += armor->max_hp_bonus;
    g->player.max_mp += armor->max_mp_bonus;
    g->player.mp += armor->max_mp_bonus;
}

void game_remove_armor_bonuses(GameState *g, const Item *armor) {
    g->player.defense -= armor->defense_bonus;
    g->player.max_hp -= armor->max_hp_bonus;
    if (g->player.hp > g->player.max_hp) {
        g->player.hp = g->player.max_hp;
    }
    g->player.max_mp -= armor->max_mp_bonus;
    if (g->player.mp > g->player.max_mp) {
        g->player.mp = g->player.max_mp;
    }
}

static LevelCache *active_cache(GameState *g) {
    if (g->location == LOCATION_FOREST) {
        return g->forest_cache;
    }
    if (g->location == LOCATION_MOUNTAINS) {
        return g->mountain_cache;
    }
    if (g->location == LOCATION_COAST) {
        return g->coast_cache;
    }
    if (g->location == LOCATION_SWAMP) {
        return g->swamp_cache;
    }
    if (g->location == LOCATION_DRAGONSPINE) {
        return g->dragonspine_cache;
    }
    if (g->location == LOCATION_FROSTFELL) {
        return g->frostfell_cache;
    }
    if (g->location == LOCATION_DESERT) {
        return g->desert_cache;
    }

    if (g->location == LOCATION_ASHEN) {
        return g->ashen_cache;
    }
    if (g->location == LOCATION_CATACOMBS) {
        return g->catacombs_cache;
    }
    if (g->location == LOCATION_GLASSDEEP) {
        return g->glassdeep_cache;
    }
    if (g->location == LOCATION_TEMPLE) {
        return g->temple_cache;
    }
    return g->level_cache;
}

static int *active_max_level(GameState *g) {
    if (g->location == LOCATION_FOREST) return &g->max_forest_level_reached;
    if (g->location == LOCATION_MOUNTAINS)
        return &g->max_mountain_level_reached;
    if (g->location == LOCATION_COAST) {
        return &g->max_coast_level_reached;
    }
    if (g->location == LOCATION_SWAMP) {
        return &g->max_swamp_level_reached;
    }
    if (g->location == LOCATION_DRAGONSPINE) {
        return &g->max_dragonspine_level_reached;
    }
    if (g->location == LOCATION_FROSTFELL) {
        return &g->max_frostfell_level_reached;
    }
    if (g->location == LOCATION_DESERT) {
        return &g->max_desert_level_reached;
    }

    if (g->location == LOCATION_ASHEN) {
        return &g->max_ashen_level_reached;
    }
    if (g->location == LOCATION_CATACOMBS) {
        return &g->max_catacombs_level_reached;
    }
    if (g->location == LOCATION_GLASSDEEP) {
        return &g->max_glassdeep_level_reached;
    }
    if (g->location == LOCATION_TEMPLE) {
        return &g->max_temple_level_reached;
    }
    return &g->max_level_reached;
}

static int active_depth(const GameState *g) {
    if (g->location == LOCATION_FOREST) {
        return FOREST_DEPTH;
    }
    if (g->location == LOCATION_MOUNTAINS) {
        return MOUNTAIN_DEPTH;
    }
    if (g->location == LOCATION_COAST) {
        return COAST_DEPTH;
    }
    if (g->location == LOCATION_SWAMP) {
        return SWAMP_DEPTH;
    }
    if (g->location == LOCATION_DRAGONSPINE) {
        return DRAGONSPINE_DEPTH;
    }
    if (g->location == LOCATION_FROSTFELL) {
        return FROSTFELL_DEPTH;
    }
    if (g->location == LOCATION_DESERT) {
        return DESERT_DEPTH;
    }

    if (g->location == LOCATION_ASHEN) {
        return ASHEN_DEPTH;
    }
    if (g->location == LOCATION_CATACOMBS) {
        return CATACOMBS_DEPTH;
    }
    if (g->location == LOCATION_GLASSDEEP) {
        return GLASSDEEP_DEPTH;
    }
    if (g->location == LOCATION_TEMPLE) {
        return TEMPLE_DEPTH;
    }
    return DUNGEON_DEPTH;
}

static TileType quest_tile(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                return item->underlying_tile;
            }
        }
    }
    return tile;
}

static void set_temple_tile(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] != TILE_ITEM) {
        g->map.tiles[y][x] = tile;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
        }
    }
}

static int place_elowen_seal(GameState *g) {
    if (g->location != LOCATION_DUNGEON || g->elowen_quest_state != 1) {
        return 0;
    }
    int seal_index = -1;
    if (g->level == 2) {
        seal_index = 0;
    } else if (g->level == 3) {
        seal_index = 1;
    } else if (g->level == 4) {
        seal_index = 2;
    }
    if (seal_index < 0 || g->map.room_count < 2) {
        return 0;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = quest_tile(g, x, y);
            if (tile == TILE_BROKEN_BURIAL_SEAL || tile == TILE_RESTORED_BURIAL_SEAL) {
                return 0;
            }
        }
    }
    Room *room = &g->map.rooms[g->map.room_count / 2];
    int x;
    int y;
    map_room_center(room, &x, &y);
    TileType seal = (g->elowen_seals_restored & (1 << seal_index))
        ? TILE_RESTORED_BURIAL_SEAL : TILE_BROKEN_BURIAL_SEAL;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = seal;
            return !(g->elowen_seals_restored & (1 << seal_index));
        }
    }
    g->map.tiles[y][x] = seal;
    return !(g->elowen_seals_restored & (1 << seal_index));
}

static void spawn_elowen_guardians(GameState *g) {
    EnemyType primary = g->level == 2 ? ENEMY_SKELETON :
        (g->level == 3 ? ENEMY_WRAITH : ENEMY_CRYPT_CONJURER);
    EnemyType support = g->level == 2 ? ENEMY_CRYPT_BAT :
        (g->level == 3 ? ENEMY_SKELETON : ENEMY_WRAITH);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (quest_tile(g, x, y) != TILE_BROKEN_BURIAL_SEAL) {
                continue;
            }
            spawn_quest_enemy_near(g, primary, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            return;
        }
    }
}

static int alder_warden_bit(int level) {
    if (level == 2) {
        return ALDER_WARDEN_STAGE_2;
    } else if (level == 3) {
        return ALDER_WARDEN_STAGE_3;
    } else if (level == 4) {
        return ALDER_WARDEN_STAGE_4;
    }
    return 0;
}

static int place_alder_warden(GameState *g) {
    if (g->location != LOCATION_FOREST || g->alder_quest_state != 1) {
        return 0;
    }
    int bit = alder_warden_bit(g->level);
    if (!bit || (g->alder_wardens_rescued & bit)) {
        return 0;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (quest_tile(g, x, y) == TILE_FOREST_WARDEN) {
                return 0;
            }
        }
    }
    int room_index = g->map.room_count / 2;
    if (room_index >= g->map.room_count) {
        return 0;
    }
    int x;
    int y;
    Room *room = &g->map.rooms[room_index];
    map_room_center(room, &x, &y);
    if (!enemy_tile_open(g, x, y)) {
        int found = 0;
        for (int candidate_y = room->y + 1;
            candidate_y < room->y + room->h - 1 && !found; candidate_y++) {
            for (int candidate_x = room->x + 1;
                candidate_x < room->x + room->w - 1; candidate_x++) {
                if (enemy_tile_open(g, candidate_x, candidate_y)) {
                    x = candidate_x;
                    y = candidate_y;
                    found = 1;
                    break;
                }
            }
        }
        if (!found) {
            return 0;
        }
    }
    g->map.tiles[y][x] = TILE_FOREST_WARDEN;
    return 1;
}

static void spawn_alder_guardian(GameState *g) {
    if (g->location != LOCATION_FOREST) {
        return;
    }
    EnemyType primary;
    EnemyType support;
    if (g->level == 2) {
        primary = ENEMY_GIANT_SPIDER;
        support = ENEMY_BLIGHTED_WOLF;
    } else if (g->level == 3) {
        primary = ENEMY_DARK_ELF;
        support = ENEMY_PIXIE;
    } else if (g->level == 4) {
        primary = ENEMY_FOREST_TROLL;
        support = ENEMY_BLIGHTED_WOLF;
    } else {
        return;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] != TILE_FOREST_WARDEN) {
                continue;
            }
            spawn_quest_enemy_near(g, primary, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            return;
        }
    }
}

static int mara_beacon_bit(int level) {
    if (level == 2) {
        return MARA_BEACON_STAGE_2;
    } else if (level == 3) {
        return MARA_BEACON_STAGE_3;
    } else if (level == 4) {
        return MARA_BEACON_STAGE_4;
    }
    return 0;
}

static int place_mara_beacon(GameState *g) {
    if (g->location != LOCATION_COAST || g->mara_quest_state == 0) {
        return 0;
    }
    int bit = mara_beacon_bit(g->level);
    if (!bit) {
        return 0;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = quest_tile(g, x, y);
            if (tile == TILE_COAST_BEACON_UNLIT || tile == TILE_COAST_BEACON_LIT) {
                return 0;
            }
        }
    }
    int room_index = g->level == 2 ? 1 : (g->level == 3 ? 3 : 8);
    if (room_index >= g->map.room_count) {
        return 0;
    }
    int x;
    int y;
    map_room_center(&g->map.rooms[room_index], &x, &y);
    if (g->level == 4) {
        int tide_is_high = 0;
        for (int map_y = 0; map_y < MAP_H && !tide_is_high; map_y++) {
            for (int map_x = 0; map_x < MAP_W; map_x++) {
                if (g->map.tiles[map_y][map_x] == TILE_COAST_DEEP_WATER) {
                    tide_is_high = 1;
                    break;
                }
            }
        }
        for (int offset_y = -1; offset_y <= 1; offset_y++) {
            for (int offset_x = -1; offset_x <= 1; offset_x++) {
                int water_x = x + offset_x;
                int water_y = y + offset_y;
                if ((offset_x != 0 || offset_y != 0) &&
                    (g->map.tiles[water_y][water_x] == TILE_COAST_FLOOR ||
                    g->map.tiles[water_y][water_x] == TILE_COAST_SAND ||
                    g->map.tiles[water_y][water_x] ==
                        TILE_COAST_SHALLOW_WATER)) {
                    g->map.tiles[water_y][water_x] = tide_is_high
                        ? TILE_COAST_DEEP_WATER : TILE_COAST_DRAINED_WATER;
                }
            }
        }
    }
    TileType beacon = (g->mara_beacons_lit & bit)
        ? TILE_COAST_BEACON_LIT : TILE_COAST_BEACON_UNLIT;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = beacon;
            return !(g->mara_beacons_lit & bit);
        }
    }
    g->map.tiles[y][x] = beacon;
    return !(g->mara_beacons_lit & bit);
}

static void spawn_mara_guardian(GameState *g) {
    if (g->location != LOCATION_COAST) {
        return;
    }
    EnemyType primary;
    EnemyType support;
    if (g->level == 2) {
        primary = ENEMY_GIANT_CRAB;
        support = ENEMY_MERFOLK;
    } else if (g->level == 3) {
        primary = ENEMY_ANIMATED_STATUE;
        support = ENEMY_GIANT_CRAB;
    } else if (g->level == 4) {
        primary = ENEMY_SEA_SERPENT;
        support = ENEMY_WATER_ELEMENTAL;
    } else {
        return;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (quest_tile(g, x, y) != TILE_COAST_BEACON_UNLIT) {
                continue;
            }
            spawn_quest_enemy_near(g, primary, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            return;
        }
    }
}

static void place_dragon_treasure(GameState *g) {
    if (g->location != LOCATION_DRAGONSPINE ||
        g->level != DRAGONSPINE_DEPTH || g->dragon_treasure_quest_state != 1) {
        return;
    }
    Room *lair = &g->map.rooms[g->map.room_count - 1];
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active &&
            g->floor_items[i].underlying_tile == TILE_DRAGON_TREASURE) {
            return;
        }
    }
    for (int y = lair->y; y < lair->y + lair->h; y++) {
        for (int x = lair->x; x < lair->x + lair->w; x++) {
            if (g->map.tiles[y][x] == TILE_DRAGON_TREASURE) {
                return;
            }
        }
    }
    int center_x;
    int center_y;
    map_room_center(lair, &center_x, &center_y);
    int best_x = -1;
    int best_y = -1;
    int best_distance = MAP_W + MAP_H;
    for (int y = lair->y + 1; y < lair->y + lair->h - 1; y++) {
        for (int x = lair->x + 1; x < lair->x + lair->w - 1; x++) {
            TileType tile = g->map.tiles[y][x];
            if (tile != TILE_DRAGON_FLOOR && tile != TILE_DRAGON_ASH &&
                tile != TILE_DRAGON_HOARD) {
                continue;
            }
            int distance = abs(x - center_x - 2) + abs(y - center_y - 1);
            if (distance < best_distance) {
                best_x = x;
                best_y = y;
                best_distance = distance;
            }
        }
    }
    if (best_x >= 0) {
        g->map.tiles[best_y][best_x] = TILE_DRAGON_TREASURE;
    }
}

// Keep the unique reward available even when a fresh visit regenerates the map.
static void restore_frostfell_reward(GameState *g) {
    if (g->location != LOCATION_FROSTFELL || g->level != FROSTFELL_DEPTH ||
        !g->kraken_bow_unclaimed || g->map.room_count == 0) {
        return;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active &&
            strcmp(g->floor_items[i].item.name, "Krakenbone Bow") == 0) {
            return;
        }
    }
    if (g->floor_item_count >= MAX_FLOOR_ITEMS) {
        return;
    }
    int x;
    int y;
    map_room_center(&g->map.rooms[g->map.room_count - 1], &x, &y);
    FloorItem *reward = &g->floor_items[g->floor_item_count++];
    *reward = (FloorItem){
        .active = 1, .x = x, .y = y, .underlying_tile = TILE_FROST_LAKE,
        .item = item_make_krakenbone_bow()
    };
    g->map.tiles[y][x] = TILE_ITEM;
}

static void place_desert_lamp(GameState *g) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_LAMP_LEVEL ||
        g->sunscar_lamp_quest_state != 1 || g->map.room_count == 0) {
        return;
    }
    int x;
    int y;
    map_room_center(&g->map.rooms[g->map.room_count - 1], &x, &y);
    if (g->map.tiles[y][x] == TILE_PORTAL && g->portal_origin_tile == TILE_DESERT_LAMP) {
        return;
    }
    int occupied = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = TILE_DESERT_LAMP;
            occupied = 1;
        }
    }
    if (!occupied) {
        g->map.tiles[y][x] = TILE_DESERT_LAMP;
    }
}

static void restore_desert_reward(GameState *g) {
    desert_pharaoh_restore_reward(g);
}

static int emberforge_stage_bit(const GameState *g) {
    if (g->location != LOCATION_ASHEN) {
        return 0;
    }
    if (g->level == EMBERFORGE_MECHANISM_LEVEL) {
        return EMBERFORGE_MECHANISM_RECOVERED;
    }
    return g->level == EMBERFORGE_FURNACE_LEVEL ? EMBERFORGE_RESTORED : 0;
}

static void set_emberforge_tile(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] == TILE_PORTAL) {
        g->portal_origin_tile = tile;
        return;
    }
    int covered = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
            covered = 1;
        }
    }
    if (!covered) {
        g->map.tiles[y][x] = tile;
    }
}

static int spawn_emberforge_guard(GameState *g, EnemyType type, int cx, int cy) {
    int slot = g->enemy_count;
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].active && strcmp(g->enemies[i].name, "Emberforge Guardian") != 0) {
            slot = i;
            break;
        }
    }
    if (slot >= NON_ROAD_ENEMY_LIMIT) {
        return 0;
    }
    for (int radius = 1; radius <= 4; radius++) {
        for (int y = cy - radius; y <= cy + radius; y++) {
            for (int x = cx - radius; x <= cx + radius; x++) {
                if (!enemy_tile_open(g, x, y)) {
                    continue;
                }
                if (slot == g->enemy_count) {
                    g->enemy_count++;
                }
                Enemy *enemy = &g->enemies[slot];
                spawn_enemy(g, enemy, type, x, y);
                snprintf(enemy->name, sizeof(enemy->name), "Emberforge Guardian");
                g->level_cleared = 0;
                return 1;
            }
        }
    }
    return 0;
}

static void place_emberforge_encounter(GameState *g) {
    int bit = emberforge_stage_bit(g);
    if (!bit || !g->emberforge_quest_state || !g->map.room_count) {
        return;
    }
    int x;
    int y;
    map_room_center(&g->map.rooms[g->map.room_count - 1], &x, &y);
    if (bit == EMBERFORGE_MECHANISM_RECOVERED) {
        set_emberforge_tile(g, x, y, g->emberforge_progress & bit ? TILE_ASHEN_RUIN : TILE_EMBERFORGE_MECHANISM);
    } else {
        set_emberforge_tile(g, x, y, g->emberforge_progress & bit ? TILE_EMBERFORGE_LIT : TILE_EMBERFORGE_COLD);
    }
    if ((g->emberforge_progress & bit) || (g->emberforge_encounters & bit)) {
        return;
    }
    int guards = bit == EMBERFORGE_RESTORED ? 4 : 3;
    int existing = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (strcmp(g->enemies[i].name, "Emberforge Guardian") == 0) {
            existing++;
        }
    }
    for (int i = existing; i < guards; i++) {
        EnemyType type = i < 2 ? (bit == EMBERFORGE_RESTORED ? ENEMY_OBSIDIAN_GUARDIAN : ENEMY_ASH_HOUND) : ENEMY_CINDER_IMP;
        if (!spawn_emberforge_guard(g, type, x, y)) {
            return;
        }
    }
    g->emberforge_encounters |= bit;
}

static int frostfell_quest_stage_bit(const GameState *g) {
    if (g->location != LOCATION_FROSTFELL) {
        return 0;
    }
    if (g->level == FROSTFELL_JOURNAL_LEVEL) {
        return FROSTFELL_JOURNAL_RECOVERED;
    }
    return g->level == FROSTFELL_SURVIVOR_LEVEL ? FROSTFELL_SURVIVOR_RESCUED : 0;
}

static TileType quest_object_tile(const GameState *g, int x, int y) {
    TileType tile = quest_tile(g, x, y);
    if (tile == TILE_PORTAL && g->portal_active && g->portal_location == g->location &&
        g->portal_level == g->level && g->portal_x == x && g->portal_y == y) {
        return g->portal_origin_tile;
    }
    return tile;
}

static void set_quest_object_tile(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] == TILE_PORTAL) {
        g->portal_origin_tile = tile;
        return;
    }
    int covered = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
            covered = 1;
        }
    }
    if (!covered) {
        g->map.tiles[y][x] = tile;
    }
}

static int spawn_quest_guard(GameState *g, EnemyType type, int cx, int cy, const char *name) {
    int slot = g->enemy_count;
    // Catacomb corpses may still revive; adding guards must preserve their slots.
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->location != LOCATION_CATACOMBS && !g->enemies[i].active && strcmp(g->enemies[i].name, name) != 0) {
            slot = i;
            break;
        }
    }
    int limit = g->location == LOCATION_DESERT ? LEVEL_CACHE_ENEMY_CAPACITY : NON_ROAD_ENEMY_LIMIT;
    if (slot >= limit) {
        return 0;
    }
    for (int radius = 1; radius <= 4; radius++) {
        for (int y = cy - radius; y <= cy + radius; y++) {
            for (int x = cx - radius; x <= cx + radius; x++) {
                if (!enemy_tile_open(g, x, y) || (x == cx && y == cy) || (g->player.x == x && g->player.y == y) ||
                    (g->portal_active && g->portal_location == g->location && g->portal_level == g->level &&
                    g->portal_x == x && g->portal_y == y)) {
                    continue;
                }
                if (slot == g->enemy_count) {
                    g->enemy_count++;
                }
                spawn_enemy(g, &g->enemies[slot], type, x, y);
                snprintf(g->enemies[slot].name, sizeof(g->enemies[slot].name), "%s", name);
                g->level_cleared = 0;
                return 1;
            }
        }
    }
    return 0;
}

static void place_frostfell_quest_encounter(GameState *g) {
    int bit = frostfell_quest_stage_bit(g);
    if (!bit || g->frostfell_quest_state != 1 || (g->frostfell_quest_progress & bit) || !g->map.room_count) {
        return;
    }
    TileType objective = bit == FROSTFELL_JOURNAL_RECOVERED ? TILE_FROST_JOURNAL : TILE_NPC_FROST_SURVIVOR;
    int x = -1;
    int y = -1;
    for (int ty = 0; ty < MAP_H && x < 0; ty++) {
        for (int tx = 0; tx < MAP_W; tx++) {
            if (quest_object_tile(g, tx, ty) == objective) {
                x = tx;
                y = ty;
                break;
            }
        }
    }
    if (x < 0) {
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        for (int ty = room->y + 1; ty < room->y + room->h - 1 && x < 0; ty++) {
            for (int tx = room->x + 1; tx < room->x + room->w - 1; tx++) {
                if (g->map.tiles[ty][tx] == TILE_FROST_FLOOR && enemy_tile_open(g, tx, ty) &&
                    (g->player.x != tx || g->player.y != ty) &&
                    !(bit == FROSTFELL_SURVIVOR_RESCUED && g->portal_active &&
                    g->portal_location == g->location && g->portal_level == g->level &&
                    g->portal_x == tx && g->portal_y == ty)) {
                    x = tx;
                    y = ty;
                    break;
                }
            }
        }
    }
    if (x < 0) {
        return;
    }
    set_quest_object_tile(g, x, y, objective);
    if (g->frostfell_quest_encounters & bit) {
        return;
    }
    int existing = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (strcmp(g->enemies[i].name, "Expedition Guardian") == 0) {
            existing++;
        }
    }
    for (int i = existing; i < 2; i++) {
        EnemyType type = bit == FROSTFELL_JOURNAL_RECOVERED ? ENEMY_ICE_WOLF :
            (i == 0 ? ENEMY_FROST_GIANT : ENEMY_FROST_ARCHER);
        if (!spawn_quest_guard(g, type, x, y, "Expedition Guardian")) {
            return;
        }
    }
    g->frostfell_quest_encounters |= bit;
}

static int glassdeep_quest_bit(const GameState *g) {
    if (g->location != LOCATION_GLASSDEEP || g->level < 2 || g->level > 4) {
        return 0;
    }
    return 1 << (g->level - 2);
}

static const char *glassdeep_resonator_name(const GameState *g) {
    static const char *names[3] = {"West Winch", "Workshop Hoist", "Main Gantry"};
    return names[g->level - 2];
}

static const char *glassdeep_inscription(const GameState *g) {
    static const char *clues[3] = {
        "The inscription reads: Let the roots answer with a low rumble.",
        "Reflected carvings read: Between the deep and the sky, let the middle voice flow.",
        "The inscription reads: Let the crown ring high above the stone."
    };
    return clues[g->level - 2];
}

static void place_glassdeep_quest_encounter(GameState *g) {
    int bit = glassdeep_quest_bit(g);
    if (!bit || !g->glassdeep_quest_state || !g->map.room_count) {
        return;
    }
    int x = -1;
    int y = -1;
    int best = -100000;
    const Room *room = &g->map.rooms[stonewake_map(&g->map) ? g->level + 2 : g->map.room_count - 1];
    int cx;
    int cy;
    map_room_center(room, &cx, &cy);
    for (int ty = 1; ty < MAP_H - 1; ty++) {
        for (int tx = 1; tx < MAP_W - 1; tx++) {
            TileType tile = quest_object_tile(g, tx, ty);
            if (tile == TILE_GLASSDEEP_RESONATOR || tile == TILE_GLASSDEEP_RESONATOR_LIT) {
                x = tx;
                y = ty;
                goto found;
            }
            if ((g->map.tiles[ty][tx] != TILE_GLASSDEEP_FLOOR && g->map.tiles[ty][tx] != TILE_GLASSDEEP_RUIN) ||
                !enemy_tile_open(g, tx, ty) || (g->player.x == tx && g->player.y == ty) ||
                abs(tx - g->map.stairs_up_x) + abs(ty - g->map.stairs_up_y) < 8 ||
                (g->portal_active && g->portal_location == g->location && g->portal_level == g->level &&
                g->portal_x == tx && g->portal_y == ty)) {
                continue;
            }
            int score = -abs(tx - cx) - abs(ty - cy);
            if (g->level == 3) {
                const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
                int pool = 0;
                int rock = 0;
                for (int i = 0; i < 4; i++) {
                    TileType neighbor = g->map.tiles[ty + offsets[i][1]][tx + offsets[i][0]];
                    pool |= neighbor == TILE_GLASSDEEP_POOL;
                    rock |= neighbor == TILE_GLASSDEEP_WALL;
                }
                score += (pool ? 2 : rock) * (MAP_W + MAP_H);
            }
            if (g->level == 4 && tile == TILE_GLASSDEEP_RUIN) {
                score += MAP_W + MAP_H;
            }
            if (score > best) {
                best = score;
                x = tx;
                y = ty;
            }
        }
    }
found:
    if (x < 0) {
        return;
    }
    int restored = g->glassdeep_quest_progress & bit;
    if (g->level == 3) {
        // Pools carved into solid rock keep the connected walking route intact.
        for (int ty = y - 1; ty <= y + 1; ty++) {
            for (int tx = x - 1; tx <= x + 1; tx++) {
                if (tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H && g->map.tiles[ty][tx] == TILE_GLASSDEEP_WALL) {
                    g->map.tiles[ty][tx] = TILE_GLASSDEEP_POOL;
                }
            }
        }
    } else {
        for (int ty = y - 1; ty <= y + 1; ty++) {
            for (int tx = x - 1; tx <= x + 1; tx++) {
                if (g->map.tiles[ty][tx] == TILE_GLASSDEEP_FLOOR) {
                    g->map.tiles[ty][tx] = TILE_GLASSDEEP_RUIN;
                }
            }
        }
    }
    set_quest_object_tile(g, x, y, restored ? TILE_GLASSDEEP_RESONATOR_LIT : TILE_GLASSDEEP_RESONATOR);
    if (restored || (g->glassdeep_quest_encounters & bit)) {
        return;
    }
    int guards = g->level == 4 ? 1 : 2;
    int existing = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (strcmp(g->enemies[i].name, "Quarry Guardian") == 0) {
            existing++;
        }
    }
    EnemyType type = g->level == 2 ? ENEMY_CRYSTAL_SPIDER : g->level == 3 ? ENEMY_BLIND_STALKER : ENEMY_SHARD_GOLEM;
    for (int i = existing; i < guards; i++) {
        if (!spawn_quest_guard(g, type, x, y, "Quarry Guardian")) {
            return;
        }
    }
    g->glassdeep_quest_encounters |= bit;
}

static void place_catacombs_quest_encounter(GameState *g) {
    catacombs_refresh_quest(g);
    if (g->location != LOCATION_CATACOMBS || g->catacombs_quest_state != 1 ||
        g->level < 2 || g->level > 4 || g->map.room_count <= CATACOMBS_MEMORIAL_ROOM) {
        return;
    }
    int bit = 1 << (g->level - 2);
    if ((g->catacombs_quest_progress | g->catacombs_quest_encounters) & bit) {
        return;
    }
    static const EnemyType types[3][2] = {
        {ENEMY_ANCIENT_SKELETON, ENEMY_BONE_SENTINEL},
        {ENEMY_VAMPIRE, ENEMY_WRAITH},
        {ENEMY_CRYPT_CONJURER, ENEMY_ANCIENT_SKELETON}
    };
    static const char *names[3] = {"Soldiers' Memorial Guard", "Watchers' Memorial Guard", "Choir Memorial Guard"};
    const char *name = names[g->level - 2];
    int existing = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        existing += strcmp(g->enemies[i].name, name) == 0;
    }
    int x;
    int y;
    map_room_center(&g->map.rooms[CATACOMBS_MEMORIAL_ROOM], &x, &y);
    for (int i = existing; i < 2; i++) {
        if (!spawn_quest_guard(g, types[g->level - 2][i], x, y, name)) {
            return;
        }
    }
    g->catacombs_quest_encounters |= bit;
}

static int watchfire_stage_bit(const GameState *g) {
    if (g->location == LOCATION_MOUNTAINS && g->level == WATCHFIRE_MOUNTAINS_LEVEL) {
        return 1;
    }
    if (g->location == LOCATION_ASHEN && g->level == WATCHFIRE_ASHEN_LEVEL) {
        return 2;
    }
    return g->location == LOCATION_DRAGONSPINE && g->level == WATCHFIRE_DRAGONSPINE_LEVEL ? 4 : 0;
}

static int watchfire_position(const GameState *g, int *x, int *y) {
    for (int ty = 0; ty < MAP_H; ty++) {
        for (int tx = 0; tx < MAP_W; tx++) {
            TileType tile = quest_object_tile(g, tx, ty);
            if (tile == TILE_WATCHFIRE_COLD || tile == TILE_WATCHFIRE_LIT) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static void place_watchfire_encounter(GameState *g) {
    int bit = watchfire_stage_bit(g);
    if (!bit || !g->watchfire_quest_state || !g->map.room_count) {
        return;
    }
    int x;
    int y;
    if (!watchfire_position(g, &x, &y)) {
        // Use an ordinary floor in the middle clearing, preserving landmarks and paths.
        const Room *room = &g->map.rooms[g->map.room_count / 2];
        int cx;
        int cy;
        map_room_center(room, &cx, &cy);
        int best = MAP_W + MAP_H;
        x = -1;
        y = -1;
        for (int ty = room->y; ty < room->y + room->h; ty++) {
            for (int tx = room->x; tx < room->x + room->w; tx++) {
                int distance = abs(tx - cx) + abs(ty - cy);
                if (distance < best && enemy_tile_open(g, tx, ty) &&
                    (g->player.x != tx || g->player.y != ty) &&
                    (tx != g->map.stairs_down_x || ty != g->map.stairs_down_y)) {
                    x = tx;
                    y = ty;
                    best = distance;
                }
            }
        }
        if (x < 0) {
            return;
        }
    }
    set_quest_object_tile(g, x, y, g->watchfire_quest_progress & bit ? TILE_WATCHFIRE_LIT : TILE_WATCHFIRE_COLD);
    if ((g->watchfire_quest_progress | g->watchfire_quest_encounters) & bit) {
        return;
    }
    static const EnemyType mountain_guards[4] = {ENEMY_GOBLIN_ARCHER, ENEMY_GOBLIN_ARCHER, ENEMY_HOBGOBLIN_GUARD, ENEMY_HOBGOBLIN_GUARD};
    static const EnemyType ashen_guards[3] = {ENEMY_OBSIDIAN_GUARDIAN, ENEMY_CINDER_IMP, ENEMY_CINDER_IMP};
    static const EnemyType dragon_guards[3] = {ENEMY_STONE_GIANT, ENEMY_GOBLIN_ARCHER, ENEMY_FIRE_ELEMENTAL};
    const EnemyType *types = bit == 1 ? mountain_guards : bit == 2 ? ashen_guards : dragon_guards;
    int guards = bit == 1 ? 4 : 3;
    int existing = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        existing += strcmp(g->enemies[i].name, "Watchfire Defender") == 0;
    }
    for (int i = existing; i < guards; i++) {
        if (!spawn_quest_guard(g, types[i], x, y, "Watchfire Defender")) {
            return;
        }
    }
    g->watchfire_quest_encounters |= bit;
}

int game_has_watchfire_interaction(const GameState *g) {
    int x;
    int y;
    return watchfire_stage_bit(g) && watchfire_position(g, &x, &y) &&
        abs(g->player.x - x) + abs(g->player.y - y) <= 1;
}

int game_interact_watchfire(GameState *g) {
    if (!game_has_watchfire_interaction(g)) {
        return 0;
    }
    int x;
    int y;
    watchfire_position(g, &x, &y);
    int bit = watchfire_stage_bit(g);
    if (g->watchfire_quest_progress & bit) {
        push_message(g, "The restored watchfire burns brightly.");
        return 1;
    }
    if (g->watchfire_quest_state != 1) {
        return 1;
    }
    place_watchfire_encounter(g);
    if (!(g->watchfire_quest_encounters & bit)) {
        push_message(g, "The watchfire's defenders must be cleared before repairs can begin.");
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        if (enemy->active && (strcmp(enemy->name, "Watchfire Defender") == 0 || abs(enemy->x - x) + abs(enemy->y - y) <= 4)) {
            push_message(g, "Defeat the watchfire's defenders and nearby threats before lighting it.");
            return 1;
        }
    }
    g->watchfire_quest_progress |= bit;
    set_quest_object_tile(g, x, y, TILE_WATCHFIRE_LIT);
    if (g->watchfire_quest_progress == 7) {
        g->watchfire_quest_state = 2;
        push_message(g, "All three watchfires burn! Return to Marshal Veyra in Ridgeshire Town Hall.");
    } else {
        push_message(g, "Watchfire restored. Check your journal for the remaining regions.");
    }
    return 1;
}

typedef struct {
    Location location;
    int level;
    const char *leader;
    const char *support;
    EnemyType types[4];
    int count;
    int offset;
} GuildHunt;

static const GuildHunt guild_hunts[3] = {
    {LOCATION_FROSTFELL, HUNT_FROSTFELL_LEVEL, "Rimefang", "Rimefang Pack",
        {ENEMY_ICE_WOLF, ENEMY_ICE_WOLF, ENEMY_ICE_WOLF, ENEMY_FROST_WRAITH}, 4, 0},
    {LOCATION_GLASSDEEP, HUNT_GLASSDEEP_LEVEL, "Shardwarden", "Shardwarden Stalker",
        {ENEMY_SHARD_GOLEM, ENEMY_BLIND_STALKER, ENEMY_BLIND_STALKER}, 3, 4},
    {LOCATION_DESERT, HUNT_SUNSCAR_LEVEL, "Dunehex", "Dunehex Mummy",
        {ENEMY_DJINN, ENEMY_MUMMY, ENEMY_MUMMY}, 3, 7}
};

static int hunt_stage_index(const GameState *g) {
    for (int i = 0; i < 3; i++) {
        if (g->location == guild_hunts[i].location && g->level == guild_hunts[i].level) {
            return i;
        }
    }
    return -1;
}

int game_hunt_enemy_index(const Enemy *enemy) {
    for (int i = 0; i < 3; i++) {
        if (strcmp(enemy->name, guild_hunts[i].leader) == 0 || strcmp(enemy->name, guild_hunts[i].support) == 0) {
            return i;
        }
    }
    return -1;
}

int game_is_hunt_leader(const Enemy *enemy) {
    int index = game_hunt_enemy_index(enemy);
    return index >= 0 && strcmp(enemy->name, guild_hunts[index].leader) == 0;
}

static int hunt_encounter_mask(int index) {
    return ((1 << guild_hunts[index].count) - 1) << guild_hunts[index].offset;
}

static void refresh_hunt_progress(GameState *g) {
    int index = hunt_stage_index(g);
    if (index < 0 || g->hunt_quest_state != 1 || (g->hunt_quest_progress & (1 << index)) ||
        (g->hunt_quest_encounters & hunt_encounter_mask(index)) != hunt_encounter_mask(index)) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && game_hunt_enemy_index(&g->enemies[i]) == index) {
            return;
        }
    }
    g->hunt_quest_progress |= 1 << index;
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Hunt complete: %s and its entire group are defeated.", guild_hunts[index].leader);
    push_message(g, message);
    if (g->hunt_quest_progress == 7) {
        g->hunt_quest_state = 2;
        push_message(g, "All three hunts are complete! Speak to Huntmaster Selene in Rosemoor's Adventurer's Guild.");
    }
}

static void place_hunt_encounter(GameState *g) {
    int index = hunt_stage_index(g);
    if (index < 0 || g->hunt_quest_state != 1 || (g->hunt_quest_progress & (1 << index)) || !g->map.room_count) {
        return;
    }
    const GuildHunt *hunt = &guild_hunts[index];
    if ((g->hunt_quest_encounters & hunt_encounter_mask(index)) == hunt_encounter_mask(index)) {
        return;
    }
    const Room *room = &g->map.rooms[g->map.room_count / 2];
    int cx;
    int cy;
    map_room_center(room, &cx, &cy);
    int x = -1;
    int y = -1;
    int best = MAP_W + MAP_H;
    TileType floor = index == 0 ? TILE_FROST_FLOOR : index == 1 ? TILE_GLASSDEEP_FLOOR : TILE_DESERT_FLOOR;
    for (int ty = room->y; ty < room->y + room->h; ty++) {
        for (int tx = room->x; tx < room->x + room->w; tx++) {
            int distance = abs(tx - cx) + abs(ty - cy);
            if (distance < best && g->map.tiles[ty][tx] == floor && enemy_tile_open(g, tx, ty) &&
                (g->player.x != tx || g->player.y != ty)) {
                x = tx;
                y = ty;
                best = distance;
            }
        }
    }
    if (x < 0) {
        return;
    }
    for (int i = 0; i < hunt->count; i++) {
        int bit = 1 << (hunt->offset + i);
        if (g->hunt_quest_encounters & bit) {
            continue;
        }
        const char *name = i == 0 ? hunt->leader : hunt->support;
        if (!spawn_quest_guard(g, hunt->types[i], x, y, name)) {
            return;
        }
        g->hunt_quest_encounters |= bit;
        if (i == 0) {
            for (int e = 0; e < g->enemy_count; e++) {
                Enemy *leader = &g->enemies[e];
                if (leader->active && strcmp(leader->name, name) == 0) {
                    leader->max_hp += (leader->max_hp + 3) / 4;
                    leader->hp = leader->max_hp;
                    break;
                }
            }
        }
    }
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Hunt sighted: %s. Defeat the leader and its entire group.", hunt->leader);
    push_message(g, message);
}

void game_refresh_quest_encounters(GameState *g) {
    if (g->location == LOCATION_DUNGEON) {
        map_ensure_dungeon_connectivity(&g->map, g->level);
    }
    game_reveal_forest_shortcut(g);
    game_reveal_swamp_shortcut(g);
    game_reveal_mountain_shortcut(g);
    lich_restore_reward(g);
    necromancer_restore_reward(g);
    goblin_king_restore_reward(g);
    drowned_queen_restore_reward(g);
    swamp_demon_restore_reward(g);
    minotaur_restore_reward(g);
    restore_frostfell_reward(g);
    restore_desert_reward(g);

    cinder_lord_restore_reward(g);
    prism_sovereign_restore_reward(g);
    sun_guardian_restore_reward(g);
    red_dragon_restore_reward(g);
    catacombs_restore_reward(g);
    int seal_placed = place_elowen_seal(g);
    place_desert_lamp(g);
    int warden_placed = place_alder_warden(g);
    int beacon_placed = place_mara_beacon(g);
    place_dain_map_bearer(g);
    place_dragon_treasure(g);
    place_emberforge_encounter(g);
    place_frostfell_quest_encounter(g);
    place_glassdeep_quest_encounter(g);

    place_catacombs_quest_encounter(g);
    place_watchfire_encounter(g);
    place_hunt_encounter(g);
    refresh_hunt_progress(g);
    if (seal_placed) {
        spawn_elowen_guardians(g);
    }
    if (warden_placed) {
        spawn_alder_guardian(g);
    }
    if (beacon_placed) {
        spawn_mara_guardian(g);
    }
}

static void generate_active_level(GameState *g) {
    if (g->location == LOCATION_FOREST) {
        map_generate_forest(&g->map, g->level);
    } else if (g->location == LOCATION_MOUNTAINS) {
        map_generate_mountains(&g->map, g->level);
    } else if (g->location == LOCATION_COAST) {
        map_generate_coast(&g->map, g->level);
    } else if (g->location == LOCATION_SWAMP) {
        map_generate_swamp(&g->map, g->level);
    } else if (g->location == LOCATION_FROSTFELL) {
        map_generate_frostfell(&g->map, g->level);
    } else if (g->location == LOCATION_DESERT) {
        map_generate_desert(&g->map, g->level);
    } else if (g->location == LOCATION_ASHEN) {
        map_generate_ashen(&g->map, g->level);
    } else if (g->location == LOCATION_GLASSDEEP) {
        map_generate_glassdeep(&g->map, g->level);
    } else if (g->location == LOCATION_CATACOMBS) {
        map_generate_catacombs(&g->map, g->level);
    } else if (g->location == LOCATION_DRAGONSPINE) {
        map_generate_dragonspine(&g->map, g->level);
    } else if (g->location == LOCATION_TEMPLE) {
        int spawn_x;
        int spawn_y;
        map_generate_temple(&g->map, g->level, &spawn_x, &spawn_y);
    } else {
        map_generate(&g->map, g->level);
    }
    g->enemy_count = 0;
    if (g->location == LOCATION_TEMPLE) {
        g->temple_alignment = 0;
        g->temple_sentinels_awakened = 0;
        spawn_temple_enemies(g);
        sun_guardian_restore_reward(g);
        game_update_level_progress(g);
        return;
    }
    int seal_placed = place_elowen_seal(g);
    int warden_placed = place_alder_warden(g);
    int beacon_placed = place_mara_beacon(g);
    place_dragon_treasure(g);
    int daughter_x = 0;
    place_desert_lamp(g);
    int daughter_y = 0;
    int daughter_placed = g->location == LOCATION_SWAMP && g->level == SWAMP_RESCUE_LEVEL &&
        g->innkeeper_quest_state == 1;
    if (daughter_placed) {
        map_room_center(&g->map.rooms[7], &daughter_x, &daughter_y);
        g->map.tiles[daughter_y][daughter_x] = TILE_SWAMP_DAUGHTER;
    }
    enemies_spawn(g);
    if (daughter_placed) {
        for (int radius = 1; radius <= 4; radius++) {
            int found = 0;
            for (int y = daughter_y - radius; y <= daughter_y + radius && !found; y++) {
                for (int x = daughter_x - radius; x <= daughter_x + radius; x++) {
                    if (!enemy_tile_open(g, x, y)) {
                        continue;
                    }
                    Enemy *captor = spawn_quest_enemy_at(g, ENEMY_VAMPIRE, x, y);
                    if (captor) {
                        snprintf(captor->name, sizeof(captor->name), "Vampire Captor");
                        captor->max_hp += 20;
                        captor->hp = captor->max_hp;
                    }
                    found = 1;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
    if (seal_placed) {
        spawn_elowen_guardians(g);
    }
    if (warden_placed) {
        spawn_alder_guardian(g);
    }
    if (beacon_placed) {
        spawn_mara_guardian(g);
    }
    lich_restore_reward(g);
    necromancer_restore_reward(g);
    goblin_king_restore_reward(g);
    drowned_queen_restore_reward(g);
    swamp_demon_restore_reward(g);
    minotaur_restore_reward(g);
    restore_frostfell_reward(g);
    restore_desert_reward(g);

    cinder_lord_restore_reward(g);
    prism_sovereign_restore_reward(g);
    catacombs_restore_reward(g);
    red_dragon_restore_reward(g);
    place_emberforge_encounter(g);
    // A regenerated stage needs new guards for any unfinished objective.
    g->frostfell_quest_encounters &= ~frostfell_quest_stage_bit(g);
    place_frostfell_quest_encounter(g);
    g->glassdeep_quest_encounters &= ~glassdeep_quest_bit(g);
    place_glassdeep_quest_encounter(g);

    if (g->location == LOCATION_CATACOMBS && g->level >= 2 && g->level <= 4) {
        g->catacombs_quest_encounters &= ~(1 << (g->level - 2));
    }
    place_catacombs_quest_encounter(g);
    g->watchfire_quest_encounters &= ~watchfire_stage_bit(g);
    place_watchfire_encounter(g);
    int hunt = hunt_stage_index(g);
    if (hunt >= 0 && !(g->hunt_quest_progress & (1 << hunt))) {
        g->hunt_quest_encounters &= ~hunt_encounter_mask(hunt);
    }
    place_hunt_encounter(g);
    game_update_level_progress(g);
}

static int dungeon_map_is_foreign(const Map *map) {
    if (map->room_count > 0) {
        return 0;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if (tile == TILE_TAVERN_EXIT || tile == TILE_TOWN_EXIT ||
                tile == TILE_FOREST_FLOOR || tile == TILE_SWAMP_FLOOR ||
                tile == TILE_HIGH_PASS_ENTRANCE) {
                return 1;
            }
        }
    }
    return 0;
}

void game_repair_dungeon_maps(GameState *g) {
    for (int i = 0; i < DUNGEON_DEPTH; i++) {
        LevelCache *cache = &g->level_cache[i];
        if (cache->valid && dungeon_map_is_foreign(&cache->map)) {
            cache->valid = 0;
            cache->level_cleared = 0;
        }
    }
    if (g->location != LOCATION_DUNGEON || g->level < 1 ||
        g->level > DUNGEON_DEPTH || !dungeon_map_is_foreign(&g->map)) {
        return;
    }
    LevelCache *cache = &g->level_cache[g->level - 1];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        g->level_cleared = cache->level_cleared;
        memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    // Recover dropped items beside the entrance of the restored floor.
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        item->x = g->player.x + 1;
        item->y = g->player.y;
        item->underlying_tile = TILE_FLOOR;
        g->map.tiles[item->y][item->x] = TILE_ITEM;
    }
    if (g->portal_active && g->portal_location == LOCATION_DUNGEON && g->portal_level == g->level) {
        g->portal_x = g->player.x;
        g->portal_y = g->player.y;
        g->portal_origin_tile = TILE_STAIRS_UP;
    }
}

static void sync_temple_floor_state(GameState *g) {
    if (g->location != LOCATION_TEMPLE) {
        return;
    }
    int guardian_defeated =
        g->defeated_bosses & (1 << LOCATION_TEMPLE);
    g->temple_alignment = 0;
    g->temple_sentinels_awakened = 1;
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            if (quest_tile(g, x, y) == TILE_TEMPLE_MOON_DOOR_OPEN) {
                g->temple_alignment = 1;
            }
            if (g->map.tiles[y][x] == TILE_TEMPLE_DORMANT_SENTINEL) {
                g->temple_sentinels_awakened = 0;
            }
            if (guardian_defeated &&
                g->map.tiles[y][x] == TILE_TEMPLE_VAULT_DOOR) {
                g->map.tiles[y][x] = TILE_TEMPLE_FLOOR;
            }
            if (g->temple_treasure_state >= 2 &&
                g->map.tiles[y][x] == TILE_TEMPLE_TREASURE) {
                g->map.tiles[y][x] = TILE_TEMPLE_RUBBLE;
            }
        }
    }
    if (g->level == TEMPLE_DEPTH) {
        sun_guardian_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
        sun_guardian_restore_reward(g);
    }
}

static void clear_floor_loot(GameState *g) {
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x >= 0 && item->x < MAP_W &&
            item->y >= 0 && item->y < MAP_H &&
            g->map.tiles[item->y][item->x] == TILE_ITEM &&
            item->underlying_tile != TILE_ITEM) {
            g->map.tiles[item->y][item->x] =
                (TileType)item->underlying_tile;
        }
    }
    g->floor_item_count = 0;
}

static void prepare_forest_arrival(GameState *g, int from_high) {
    if (g->location != LOCATION_FOREST) {
        return;
    }
    map_reveal_forest_entrance(&g->map);
    map_reveal_forest_exit(&g->map);
    if (g->level == FOREST_BOSS_LEVEL && (g->defeated_bosses & (1 << LOCATION_FOREST))) {
        game_reveal_forest_shortcut(g);
        return;
    }
    int landmark = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_FOREST_LANDMARK) {
                landmark = 1;
            }
        }
    }
    if (!landmark) {
        return;
    }
    int x = from_high ? g->map.stairs_up_x : g->map.stairs_down_x;
    int y = from_high ? g->map.stairs_up_y : g->map.stairs_down_y;
    if (x == 1) {
        x = 0;
    } else if (x == MAP_W - 2) {
        x = MAP_W - 1;
    } else if (y == 1) {
        y = 0;
    } else {
        y = MAP_H - 1;
    }
    g->map.tiles[y][x] = TILE_FOREST_WALL;
}

void game_descend(GameState *g) {
    if (g->location == LOCATION_GIANTS_CROWN) {
        giants_change_level(g, 1);
        return;
    }
    if (g->location == LOCATION_CROWNROAD_NORTH) {
        return;
    }
    if (g->location == LOCATION_IRONFANG) {
        ironfang_change_level(g, 1);
        return;
    }
    if (g->location == LOCATION_STORMWATCH) {
        stormwatch_change_level(g, 1);
        return;
    }
    if (g->location == LOCATION_MANGROVES) {
        mangrove_change_level(g, 1);
        return;
    }
    if (g->location == LOCATION_ELDERWOOD) {
        elderwood_change_level(g, 1);
        return;
    }
    if (castle_travel(g, 1)) {
        return;
    }
    int depth = active_depth(g);
    if (g->level >= depth) return;

    clear_floor_loot(g);
    LevelCache *cache = active_cache(g);
    int *max_level = active_max_level(g);

    if (g->level >= 1 && g->level <= depth) {
        cache[g->level - 1].map         = g->map;
        cache[g->level - 1].enemy_count = g->enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            cache[g->level - 1].enemies[i] = g->enemies[i];
        cache[g->level - 1].valid = 1;
        cache[g->level - 1].level_cleared = g->level_cleared;
    }

    g->level++;
    if (g->level > *max_level)
        *max_level = g->level;
    g->level_cleared = 0;
    if (g->level <= depth && cache[g->level - 1].valid) {
        g->map         = cache[g->level - 1].map;
        g->enemy_count = cache[g->level - 1].enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = cache[g->level - 1].enemies[i];
        g->level_cleared = cache[g->level - 1].level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    prepare_forest_arrival(g, 0);
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    sync_temple_floor_state(g);
}

void game_ascend(GameState *g) {
    if (g->location == LOCATION_GIANTS_CROWN) {
        giants_change_level(g, 0);
        return;
    }
    if (g->location == LOCATION_CROWNROAD_NORTH) {
        return;
    }
    if (g->location == LOCATION_IRONFANG) {
        ironfang_change_level(g, 0);
        return;
    }
    if (g->location == LOCATION_STORMWATCH) {
        stormwatch_change_level(g, 0);
        return;
    }
    if (g->location == LOCATION_MANGROVES) {
        mangrove_change_level(g, 0);
        return;
    }
    if (g->location == LOCATION_ELDERWOOD) {
        elderwood_change_level(g, 0);
        return;
    }
    if (castle_travel(g, 0)) {
        return;
    }
    if (g->level <= 1) return;

    clear_floor_loot(g);
    LevelCache *cache = active_cache(g);

    if (g->level <= active_depth(g)) {
        cache[g->level - 1].map           = g->map;
        cache[g->level - 1].enemy_count   = g->enemy_count;
        cache[g->level - 1].level_cleared = g->level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            cache[g->level - 1].enemies[i] = g->enemies[i];
        cache[g->level - 1].valid = 1;
    }

    g->level--;

    if (cache[g->level - 1].valid) {
        g->map         = cache[g->level - 1].map;
        g->enemy_count = cache[g->level - 1].enemy_count;
        g->level_cleared = cache[g->level - 1].level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = cache[g->level - 1].enemies[i];
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        if (g->location == LOCATION_FOREST || g->location == LOCATION_SWAMP || g->location == LOCATION_MOUNTAINS) {
            generate_active_level(g);
        }
    }

    prepare_forest_arrival(g, 1);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
    sync_temple_floor_state(g);
}

static int expedition_spawn_open(const GameState *g, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    if (!enemy_terrain_open(g, x, y) && tile != TILE_TEMPLE_FLOOR && tile != TILE_LABYRINTH_FLOOR) {
        return 0;
    }
    if ((x == g->map.stairs_up_x && y == g->map.stairs_up_y) ||
        (x == g->map.stairs_down_x && y == g->map.stairs_down_y) ||
        (g->portal_active && g->portal_location == g->location && g->portal_level == g->level &&
        x == g->portal_x && y == g->portal_y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

static void respawn_expedition_enemies(GameState *g) {
    unsigned char regular[MAX_ENEMIES] = {0};
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (enemy->is_boss || enemy->dain_fragment || enemy->experience <= 0) {
            continue;
        }
        Enemy fresh = {0};
        spawn_enemy(g, &fresh, enemy->type, enemy->x, enemy->y);
        // Named quest guards and boss reinforcements retain their exact state.
        if (strcmp(enemy->name, fresh.name) != 0) {
            continue;
        }
        regular[i] = 1;
        enemy->active = 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (!regular[i]) {
            continue;
        }
        Enemy *enemy = &g->enemies[i];
        int x = enemy->x;
        int y = enemy->y;
        if (!expedition_spawn_open(g, x, y)) {
            int found = 0;
            for (int ty = 1; ty < MAP_H - 1 && !found; ty++) {
                for (int tx = 1; tx < MAP_W - 1 && !found; tx++) {
                    if (expedition_spawn_open(g, tx, ty)) {
                        x = tx;
                        y = ty;
                        found = 1;
                    }
                }
            }
            if (!found) {
                continue;
            }
        }
        spawn_enemy(g, enemy, enemy->type, x, y);
        g->level_cleared = 0;
    }
}

static void sunscar_add_patrols(GameState *g) {
    if (g->location != LOCATION_DESERT || !g->map.room_count) {
        return;
    }
    int target = sunscar_population(g->level) - (quest_group_pending(g) ? 3 : 0);
    int regular = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        if (enemy->active && enemy->is_boss) {
            target--;
        } else if (enemy->active && !enemy->dain_fragment && enemy->experience > 0) {
            Enemy fresh = {0};
            spawn_enemy(g, &fresh, enemy->type, enemy->x, enemy->y);
            regular += strcmp(enemy->name, fresh.name) == 0;
        }
    }
    int rooms = g->map.room_count - (g->level == DESERT_DEPTH ? 1 : 0);
    while (regular < target && g->enemy_count < LEVEL_CACHE_ENEMY_CAPACITY - 3) {
        if (!spawn_into_open_tile(g, sunscar_enemy(g->level, rand() % 100), rooms)) {
            break;
        }
        regular++;
        g->level_cleared = 0;
    }
}

static void start_expedition(GameState *g, LevelCache *cache, int depth) {
    int level = g->level;
    for (int i = 0; i < depth; i++) {
        if (!cache[i].valid) {
            continue;
        }
        g->level = i + 1;
        landscape_new_expedition(&cache[i].map);
        g->map = cache[i].map;
        g->enemy_count = cache[i].enemy_count;
        g->level_cleared = cache[i].level_cleared;
        memcpy(g->enemies, cache[i].enemies, sizeof(cache[i].enemies));
        respawn_expedition_enemies(g);
        sunscar_add_patrols(g);
        game_update_level_progress(g);
        cache[i].map = g->map;
        cache[i].enemy_count = g->enemy_count;
        memcpy(cache[i].enemies, g->enemies, sizeof(cache[i].enemies));
        cache[i].level_cleared = g->level_cleared;
    }
    g->level = level;
}

static void enter_adventure(GameState *g, Location location) {
    clear_floor_loot(g);
    g->location = location;
    LevelCache *cache = active_cache(g);
    int *max_level = active_max_level(g);

    start_expedition(g, cache, active_depth(g));
    g->level = 1;
    g->level_cleared = 0;
    if (*max_level < 1) {
        *max_level = 1;
    }
    if (cache[0].valid) {
        g->map = cache[0].map;
        g->enemy_count = cache[0].enemy_count;
        memcpy(g->enemies, cache[0].enemies, sizeof(cache[0].enemies));
        g->level_cleared = cache[0].level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_enter_dungeon(GameState *g) {
    g->dungeon_entry_town = g->location == LOCATION_DAWNMERE ? LOCATION_DAWNMERE : LOCATION_TOWN;
    game_repair_dungeon_maps(g);
    enter_adventure(g, LOCATION_DUNGEON);
}

void game_enter_forest(GameState *g) {
    int from_stillbury = g->location == LOCATION_TOWN2;
    clear_floor_loot(g);
    g->forest_entry_town = from_stillbury ? LOCATION_TOWN2 : LOCATION_TOWN;
    g->location = LOCATION_FOREST;
    start_expedition(g, g->forest_cache, FOREST_DEPTH);
    g->level = 1;
    LevelCache *cache = &g->forest_cache[g->level - 1];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
        g->level_cleared = cache->level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    if (g->level > g->max_forest_level_reached) {
        g->max_forest_level_reached = g->level;
    }
    prepare_forest_arrival(g, 0);
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
}

void game_enter_mountains(GameState *g) {
    int from_ridgeshire = g->location == LOCATION_TOWN4;
    clear_floor_loot(g);
    g->mountain_entry_town = from_ridgeshire ? LOCATION_TOWN4 : LOCATION_DAWNMERE;
    g->location = LOCATION_MOUNTAINS;
    start_expedition(g, g->mountain_cache, MOUNTAIN_DEPTH);
    g->level = 1;
    LevelCache *cache = &g->mountain_cache[g->level - 1];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
        g->level_cleared = cache->level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    if (g->level > g->max_mountain_level_reached) {
        g->max_mountain_level_reached = g->level;
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "Press the interact key (A by default) beside rockfalls or broken bridges to clear or repair them.");
}

void game_enter_coast(GameState *g) {
    enter_adventure(g, LOCATION_COAST);
    push_message(g, "Cross sandbanks and pale shallows; dark open sea is impassable.");
    push_message(g, "Stand on tide controls and press the interact key (A by default) to expose flats and shift waterways.");
}

void game_enter_swamp(GameState *g) {
    int from_rosemoor = g->location == LOCATION_TOWN3;
    clear_floor_loot(g);
    g->swamp_entry_town = from_rosemoor ? LOCATION_TOWN3 : LOCATION_TOWN2;
    g->location = LOCATION_SWAMP;
    start_expedition(g, g->swamp_cache, SWAMP_DEPTH);
    g->level = 1;
    LevelCache *cache = &g->swamp_cache[g->level - 1];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
        g->level_cleared = cache->level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    if (g->level > g->max_swamp_level_reached) {
        g->max_swamp_level_reached = g->level;
    }
    g->player.x = g->map.stairs_up_x + 1;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "Braided mudbanks cross blackwater channels. Reeds hide you from distant, unhurt foes.");
}

void game_enter_frostfell(GameState *g) {
    enter_adventure(g, LOCATION_FROSTFELL);
    push_message(g, "Snow ridges wind around crevasses. Slick ice slides; brittle shortcuts collapse behind you.");
    push_message(g, "Press the interact key (A by default) on or beside the expedition journal or watchfires to use them.");
}

void game_enter_desert(GameState *g) {
    enter_adventure(g, LOCATION_DESERT);
    push_message(g, "Dense patrols roam between sandstone mesas. Use the rocks to break enemy lines of fire.");
    push_message(g, "Press the interact key (A by default) beside an oasis well for water, once per expedition.");
}

void game_enter_ashen(GameState *g) {
    clear_floor_loot(g);
    g->location = LOCATION_ASHEN;
    start_expedition(g, g->ashen_cache, ASHEN_DEPTH);
    g->level = 1;
    LevelCache *cache = &g->ashen_cache[0];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
        g->level_cleared = cache->level_cleared;
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "Lava heats the crossings. Diversion valves turn them into lasting basalt paths.");
    push_message(g, "Press the interact key (A by default) on or beside diversion valves, forge mechanisms, furnaces or sluices to use them.");
}

void game_enter_catacombs(GameState *g) {
    if (!northern_approach(g)) {
        g->catacombs_entry_town = LOCATION_CASTLE;
    }
    clear_floor_loot(g);
    g->location = LOCATION_CATACOMBS;
    start_expedition(g, g->catacombs_cache, CATACOMBS_DEPTH);
    g->level = 1;
    LevelCache *cache = &g->catacombs_cache[0];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
        g->level_cleared = cache->level_cleared;
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "The royal dead stir beneath the castle. Blue braziers bind their bones.");
    push_message(g, "Press the interact key (A by default) beside braziers to extinguish them and prevent resurrection.");
}

void game_enter_glassdeep(GameState *g) {
    clear_floor_loot(g);
    stonewake_begin_expedition(g);
    g->location = LOCATION_GLASSDEEP;
    start_expedition(g, g->glassdeep_cache, GLASSDEEP_DEPTH);
    g->level = 1;
    LevelCache *cache = &g->glassdeep_cache[0];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
        g->level_cleared = cache->level_cleared;
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "Quarry ramps loop around flooded cuts. Cranes choose a through crossing or a spur to isolated salvage supplies.");
    push_message(g, "Press the interact key (A by default) beside cranes, guarded machinery or quarry supplies.");
}

void game_enter_high_pass(GameState *g, int from_town) {
    int from_boss = g->location == LOCATION_MOUNTAINS;
    if (g->location == LOCATION_TOWN || g->location == LOCATION_TOWN4) {
        g->mountain_entry_town = from_town ? LOCATION_TOWN : LOCATION_TOWN4;
    }
    g->location = LOCATION_HIGH_PASS;
    map_generate_high_pass(&g->map);
    g->player.x = HIGH_PASS_X;
    g->player.y = from_town ? HIGH_PASS_H - 2 : 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    push_message(g, from_boss ? "The boss trail joins the High Pass between Ridgeshire and OakHaven." : from_town ? "The High Pass leads north to Ridgeshire." :
        "The High Pass leads south to OakHaven.");
}

void game_enter_dragonspine(GameState *g) {
    enter_adventure(g, LOCATION_DRAGONSPINE);
    push_message(g, "Broad cliff ledges fork around deep ravines. Boulders provide movable cover.");
    push_message(g, "Press the interact key (A by default) beside a loose boulder to push it away from you.");
}

int game_harbor_unlocked(const GameState *g) {
    return g->debug_passages_unlocked || (g->defeated_bosses & (1 << LOCATION_COAST)) != 0;
}

static void offer_rook_quest(GameState *g) {
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "Take the west gate toward Stillbury. The labyrinth is beside the Haunted Forest entrance. "
        "Recover my ivory rook; beware false stairs and the Minotaur. Return here to Oakhaven's Tavern.");
}

void game_refresh_debug_passages(GameState *g) {
    jail_refresh_hatch(g);
    coastal_refresh_docks(g);
    if (!g->debug_passages_unlocked) {
        return;
    }
    if (northern_approach(g)) {
        for (int x = 1; x < ELDERWOOD_W - 1; x++) {
            if (quest_object_tile(g, x, NORTH_FOREST_BARRIER_Y) == TILE_ELDERWOOD_WALL) {
                set_quest_object_tile(g, x, NORTH_FOREST_BARRIER_Y, x == NORTH_ROAD_X ? TILE_TOWN_PATH : TILE_TOWN_FLOOR);
            }
        }
        set_quest_object_tile(g, NORTH_ROAD_X, 0, TILE_NORTH_CASTLE);
        set_quest_object_tile(g, NORTH_ROAD_X, ELDERWOOD_H - 1, TILE_NORTH_OAKHAVEN);
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType old = quest_object_tile(g, x, y);
            TileType open = old;
            switch (old) {
                case TILE_ELDERWOOD_BRAMBLE: open = TILE_ELDERWOOD_FLOOR; break;
                case TILE_LOCKED_DOOR:
                case TILE_CRYPT_DOOR:
                case TILE_DUNGEON_GATE: open = TILE_FLOOR; break;
                case TILE_MOUNTAIN_GATE:
                case TILE_MOUNTAIN_ROCKFALL: open = TILE_MOUNTAIN_FLOOR; break;
                case TILE_TEMPLE_MOON_DOOR_CLOSED: open = TILE_TEMPLE_MOON_DOOR_OPEN; break;
                case TILE_TEMPLE_VAULT_DOOR: open = TILE_TEMPLE_FLOOR; break;
                case TILE_LABYRINTH_GATE: open = TILE_LABYRINTH_FLOOR; break;
                case TILE_CASTLE_GATE: open = TILE_CASTLE_GATE_OPEN; break;
                case TILE_GLASS_SEAL_CLOSED: open = TILE_GLASS_SEAL_OPEN; break;
                case TILE_STONEWAKE_CRANE_NEW:
                    open = TILE_STONEWAKE_CRANE_DOWN;
                    for (int span = 1; span <= 9 && x + span < MAP_W; span++) {
                        if (quest_object_tile(g, x + span, y) == TILE_STONEWAKE_GAP) {
                            set_quest_object_tile(g, x + span, y, TILE_STONEWAKE_BRIDGE);
                        }
                    }
                    break;
                case TILE_IRON_BROKEN: open = TILE_IRON_RAIL; break;
                case TILE_IRON_CRACK: open = TILE_IRON_FLOOR; break;
                case TILE_GIANTS_CRACK: open = TILE_GIANTS_PAVING; break;
                case TILE_DUNGEON_STAIRS_SEALED: open = TILE_DUNGEON_STAIRS_RETURN; break;
                default: break;
            }
            if (open != old) {
                set_quest_object_tile(g, x, y, open);
            }
        }
    }
    if (g->location == LOCATION_JAIL) {
        set_quest_object_tile(g, 4, 8, TILE_TUNNEL_EXIT);
    }
    if (g->location == LOCATION_COAST &&
        quest_object_tile(g, g->map.stairs_down_x, g->map.stairs_down_y) == TILE_COAST_DEEP_WATER) {
        set_quest_object_tile(g, g->map.stairs_down_x, g->map.stairs_down_y, TILE_COAST_EXIT);
    }
}

static void place_town_portal(GameState *g) {
    if (!g->portal_active) {
        return;
    }
    if (g->return_portal_town >= 0) {
        if (g->location != g->return_portal_town) {
            return;
        }
        Location anchor = LOCATION_TOWN;
        switch (g->portal_location) {
            case LOCATION_DUNGEON: anchor = g->dungeon_portal_town; break;
            case LOCATION_FOREST: anchor = g->forest_portal_town; break;
            case LOCATION_MOUNTAINS: anchor = g->mountain_portal_town; break;
            case LOCATION_SWAMP: anchor = g->swamp_portal_town; break;
            case LOCATION_ELDERWOOD:
            case LOCATION_CATACOMBS: anchor = g->northern_portal_town; break;
            case LOCATION_DESERT: anchor = LOCATION_TOWN2; break;
            case LOCATION_GLASSDEEP: anchor = LOCATION_TOWN2; break;
            case LOCATION_STORMWATCH:
            case LOCATION_MANGROVES: anchor = LOCATION_DAWNMERE; break;
            case LOCATION_FROSTFELL:
            case LOCATION_GIANTS_CROWN: anchor = LOCATION_TOWN3; break;
            case LOCATION_ASHEN: anchor = LOCATION_CASTLE; break;
            case LOCATION_IRONFANG:
            case LOCATION_DRAGONSPINE: anchor = LOCATION_TOWN4; break;
            default: break;
        }
        if (anchor != g->location) {
            g->map.tiles[12][21] = TILE_PORTAL;
            return;
        }
    }
    if ((g->portal_location == LOCATION_ELDERWOOD || g->portal_location == LOCATION_CATACOMBS) &&
        g->location == g->northern_portal_town && (g->location == LOCATION_TOWN || g->location == LOCATION_CASTLE)) {
        g->map.tiles[g->location == LOCATION_TOWN ? 2 : TOWN_H - 3][21] = TILE_PORTAL;
        return;
    }
    if (g->location == LOCATION_CASTLE && g->portal_location == LOCATION_ASHEN) {
        g->map.tiles[2][CASTLE_ASHEN_GATE_X + 1] = TILE_PORTAL;
        return;
    }
    if (g->location == LOCATION_DAWNMERE) {
        if (g->portal_location == LOCATION_DUNGEON && g->dungeon_portal_town == LOCATION_DAWNMERE) {
            g->map.tiles[13][2] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_STORMWATCH) {
            g->map.tiles[12][TOWN_W - 3] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_MANGROVES) {
            g->map.tiles[TOWN_H - 3][21] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_MOUNTAINS && g->mountain_portal_town == LOCATION_DAWNMERE) {
            g->map.tiles[2][21] = TILE_PORTAL;
        }
        return;
    }
    if (g->location == LOCATION_TOWN4) {
        if (g->portal_location == LOCATION_IRONFANG) {
            g->map.tiles[12][TOWN_W - 3] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_DRAGONSPINE) {
            g->map.tiles[RIDGESHIRE_DRAGON_PORTAL_Y][RIDGESHIRE_DRAGON_PORTAL_X] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_MOUNTAINS && g->mountain_portal_town == LOCATION_TOWN4) {
            g->map.tiles[TOWN_H - 3][21] = TILE_PORTAL;
        }
        return;
    }
    if (g->location == LOCATION_TOWN2) {
        if (g->portal_location == LOCATION_GLASSDEEP) {
            g->map.tiles[TOWN_H - 3][21] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_SWAMP && g->swamp_portal_town == LOCATION_TOWN2) {
            g->map.tiles[2][21] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_DESERT) {
            g->map.tiles[12][2] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_FOREST && g->forest_portal_town == LOCATION_TOWN2) {
            g->map.tiles[13][TOWN_W - 3] = TILE_PORTAL;
        }
        return;
    }
    if (g->location == LOCATION_TOWN3) {
        if (g->portal_location == LOCATION_GIANTS_CROWN || g->portal_location == LOCATION_ASHEN) {
            g->map.tiles[ROSEMOOR_ASHEN_PORTAL_Y][ROSEMOOR_ASHEN_PORTAL_X] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_CATACOMBS) {
            g->map.tiles[TOWN3_KING_GATE_Y + 1][TOWN_W - 3] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_FROSTFELL) {
            g->map.tiles[2][20] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_SWAMP && g->swamp_portal_town == LOCATION_TOWN3) {
            g->map.tiles[TOWN_H - 3][21] = TILE_PORTAL;
        }
        return;
    }
    if (g->location != LOCATION_TOWN ||
        (g->portal_location == LOCATION_DUNGEON && g->dungeon_portal_town != LOCATION_TOWN) ||
        (g->portal_location == LOCATION_FOREST && g->forest_portal_town != LOCATION_TOWN) ||
        (g->portal_location == LOCATION_MOUNTAINS && g->mountain_portal_town != LOCATION_TOWN) ||
        g->portal_location == LOCATION_DRAGONSPINE ||
        g->portal_location == LOCATION_SWAMP ||
        g->portal_location == LOCATION_DESERT ||
        g->portal_location == LOCATION_ASHEN ||
        g->portal_location == LOCATION_GLASSDEEP ||
        g->portal_location == LOCATION_CATACOMBS ||
        g->portal_location == LOCATION_FROSTFELL) {
        return;
    }
    int x = 21;
    int y = 2;
    if (g->portal_location == LOCATION_FOREST) {
        x = 2;
        y = 13;
    } else if (g->portal_location == LOCATION_DUNGEON) {
        x = TOWN_W - 3;
        y = 13;
    } else if (g->portal_location == LOCATION_COAST) {
        x = 21;
        y = TOWN_H - 3;
    } else if (g->portal_location == LOCATION_TEMPLE) {
        x = 21;
        y = TOWN_H - 3;
    }
    g->map.tiles[y][x] = TILE_PORTAL;
}

void game_enter_tavern(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_TAVERN;
    map_generate_tavern(&g->map, &spawn_x, &spawn_y);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "You enter the Lantern & Cask.");
}

void game_leave_tavern(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_TOWN;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    map_set_town2_road(&g->map,
        g->defeated_bosses & (1 << LOCATION_FOREST));
    map_set_town4_road(&g->map,
        g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
    g->player.x = TOWN_TAVERN_DOOR_X;
    g->player.y = TOWN_TAVERN_DOOR_Y + 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    place_town_portal(g);
    push_message(g, "You step back into town.");
}

void game_enter_town2(GameState *g) {
    g->last_town_visited = LOCATION_TOWN2;
    int spawn_x;
    int spawn_y;
    if (g->location == LOCATION_FOREST && g->level >= 1 && g->level <= FOREST_DEPTH) {
        LevelCache *cache = &g->forest_cache[g->level - 1];
        cache->map = g->map;
        cache->enemy_count = g->enemy_count;
        cache->level_cleared = g->level_cleared;
        for (int i = 0; i < g->enemy_count; i++) {
            cache->enemies[i] = g->enemies[i];
        }
        cache->valid = 1;
    }
    g->location = LOCATION_TOWN2;
    map_generate_town2(&g->map, &spawn_x, &spawn_y);
    map_set_town3_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
    map_set_stillbury_forest_road(&g->map, g->defeated_bosses & (1 << LOCATION_FOREST));
    place_town_portal(g);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, "You arrive in Stillbury.");
}

void game_enter_forest_road(GameState *g) {
    int from_town2 = g->location == LOCATION_TOWN2;
    int from_boss = g->location == LOCATION_FOREST;
    if (g->location == LOCATION_TOWN || from_town2) {
        g->forest_entry_town = g->location;
    }
    g->location = LOCATION_FOREST_ROAD;
    map_generate_forest_road(&g->map);
    g->player.x = from_town2 ? 1 : FOREST_ROAD_W - 2;
    g->player.y = FOREST_ROAD_Y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, from_boss ? "The boss trail joins the forest road between Stillbury and OakHaven." : from_town2 ? "The forest road leads east to OakHaven." :
        "The forest road leads west to Stillbury.");
}

void game_leave_forest_road(GameState *g, Location destination) {
    int spawn_x;
    int spawn_y;
    g->location = destination;
    if (destination == LOCATION_TOWN2) {
        map_generate_town2(&g->map, &spawn_x, &spawn_y);
        map_set_town3_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
        map_set_stillbury_forest_road(&g->map, g->defeated_bosses & (1 << LOCATION_FOREST));
        place_town_portal(g);
        g->player.x = TOWN_W - 2;
        g->player.y = 12;
    } else {
        map_generate_town(&g->map, &spawn_x, &spawn_y);
        map_set_town2_road(&g->map,
            g->defeated_bosses & (1 << LOCATION_FOREST));
        map_set_town4_road(&g->map,
            g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
        place_town_portal(g);
        g->player.x = 1;
        g->player.y = 12;
    }
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, destination == LOCATION_TOWN2 ?
        "You arrive in Stillbury." : "You return to OakHaven.");
    approach_return_from_shortcut(g, LOCATION_FOREST);
}

int game_is_king_road(const GameState *g) {
    return g->location == LOCATION_CROWNROAD || g->location == LOCATION_KING_ROAD_WEST;
}

static int crownroad_spawn_open(const GameState *g, const Map *map, const Enemy *enemies, int count, int x, int y) {
    if (x < 1 || x >= CROWNROAD_W - 1 || y < 1 || y >= CROWNROAD_H - 1 ||
        !map_is_walkable(map, x, y) || map->tiles[y][x] == TILE_ITEM || map->tiles[y][x] == TILE_PORTAL ||
        (enemies == g->enemies && g->player.x == x && g->player.y == y)) {
        return 0;
    }
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].x == x && enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

static void populate_crownroad(GameState *g, const Map *map, Enemy *roster, int *count, int *cleared) {
    static const EnemyType enemies[CROWNROAD_BASE_ENEMIES] = {
        ENEMY_BANDIT, ENEMY_BLIGHTED_WOLF, ENEMY_BANDIT,
        ENEMY_HOBGOBLIN_GUARD, ENEMY_BANDIT, ENEMY_BLIGHTED_WOLF,
        ENEMY_HOBGOBLIN_GUARD, ENEMY_BANDIT,
        ENEMY_ROAD_ARCHER, ENEMY_HORSEMAN, ENEMY_ROAD_ARCHER,
        ENEMY_HORSEMAN, ENEMY_ROAD_ARCHER, ENEMY_HORSEMAN,
        ENEMY_ROAD_ARCHER,
        // Second wave, added after the first 15 so older saved roads keep
        // their enemies and gain these on the next visit.
        ENEMY_BANDIT, ENEMY_BLIGHTED_WOLF, ENEMY_BANDIT,
        ENEMY_HOBGOBLIN_GUARD, ENEMY_BANDIT, ENEMY_BLIGHTED_WOLF,
        ENEMY_HOBGOBLIN_GUARD, ENEMY_BANDIT,
        ENEMY_HORSEMAN, ENEMY_ROAD_ARCHER, ENEMY_HORSEMAN,
        ENEMY_ROAD_ARCHER, ENEMY_ROAD_ARCHER, ENEMY_HORSEMAN,
        ENEMY_ROAD_ARCHER
    };
    static const int x[CROWNROAD_BASE_ENEMIES] = {
        17, 23, 14, 24, 17, 25, 16, 23, 23, 20, 17, 20, 23, 20, 17,
        24, 15, 25, 16, 24, 14, 24, 15, 20, 17, 20, 23, 16, 20, 24
    };
    static const int y[CROWNROAD_BASE_ENEMIES] = {
        6, 11, 17, 21, 27, 32, 38, 43, 8, 14, 20, 26, 34, 40, 44,
        5, 10, 15, 23, 29, 33, 37, 41, 9, 12, 20, 24, 30, 33, 39
    };
    int old_count = *count;
    int was_cleared = *cleared;
    Location location = g->location;
    // Road enemies use road difficulty even when migrating a cache from town.
    g->location = LOCATION_CROWNROAD;
    for (int i = old_count; i < CROWNROAD_ENEMIES; i++) {
        int base = i % CROWNROAD_BASE_ENEMIES;
        int sx = (CROWNROAD_LEGACY_W - 1 - y[base]) * CROWNROAD_LENGTH_SCALE + i / CROWNROAD_BASE_ENEMIES;
        int sy = x[base];
        int active = !was_cleared && !(base < old_count && !roster[base].active);
        // Existing patrols may have moved onto a new spawn. Choose the
        // nearest free road tile without moving saved enemies, loot, or player.
        if (active && !crownroad_spawn_open(g, map, roster, *count, sx, sy)) {
            int found = 0;
            for (int radius = 1; radius < CROWNROAD_W && !found; radius++) {
                for (int dy = -radius; dy <= radius && !found; dy++) {
                    int dx = radius - abs(dy);
                    for (int side = -1; side <= 1 && !found; side += 2) {
                        int nx = sx + side * dx;
                        int ny = sy + dy;
                        if (crownroad_spawn_open(g, map, roster, *count, nx, ny)) {
                            sx = nx;
                            sy = ny;
                            found = 1;
                        }
                    }
                }
            }
            if (!found) {
                break;
            }
        }
        spawn_enemy(g, &roster[i], enemies[base], sx, sy);
        roster[i].active = active;
        (*count)++;
    }
    g->location = location;
}

void game_migrate_crownroad_density(GameState *g) {
    if (game_is_king_road(g)) {
        populate_crownroad(g, &g->map, g->enemies, &g->enemy_count, &g->level_cleared);
    }
    Map road;
    map_generate_crownroad(&road);
    CrownroadCache *caches[2] = {&g->crownroad_cache, &g->kingroad_west_cache};
    for (int i = 0; i < 2; i++) {
        CrownroadCache *cache = caches[i];
        if (cache->valid) {
            populate_crownroad(g, &road, cache->enemies, &cache->enemy_count, &cache->level_cleared);
        }
    }
}

void game_enter_king_road(GameState *g, Location road, int from_castle) {
    g->location = road;
    g->level = 1;
    map_generate_crownroad(&g->map);
    g->enemy_count = 0;
    g->level_cleared = 0;
    int from_west = (road == LOCATION_CROWNROAD) != from_castle;
    g->player.x = from_west ? 1 : CROWNROAD_W - 2;
    g->player.y = CROWNROAD_Y;
    populate_crownroad(g, &g->map, g->enemies, &g->enemy_count, &g->level_cleared);
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, road == LOCATION_CROWNROAD ?
        "Crown Road East links Rosemoor to the Castle of No Return." :
        "Crown Road West links Ridgeshire to the Castle of No Return.");
}

static void save_crownroad_cache(GameState *g) {
    CrownroadCache *cache = g->location == LOCATION_KING_ROAD_WEST ?
        &g->kingroad_west_cache : &g->crownroad_cache;
    clear_floor_loot(g);
    cache->enemy_count = g->enemy_count;
    cache->level_cleared = g->level_cleared;
    for (int i = 0; i < g->enemy_count; i++) {
        cache->enemies[i] = g->enemies[i];
    }
    cache->valid = 1;
}

void game_leave_crownroad(GameState *g, Location destination) {
    Location road = g->location;
    save_crownroad_cache(g);
    int spawn_x;
    int spawn_y;
    g->location = destination;
    if (destination == LOCATION_CASTLE) {
        map_generate_castle(&g->map, &spawn_x, &spawn_y);
        g->player.x = road == LOCATION_CROWNROAD ? 1 : TOWN_W - 2;
        g->player.y = CASTLE_ROAD_Y;
    } else if (destination == LOCATION_TOWN4) {
        map_generate_town4(&g->map, &spawn_x, &spawn_y);
        map_set_ridgeshire_mountain_road(&g->map, g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
        g->player.x = 1;
        g->player.y = 12;
    } else {
        map_generate_town3(&g->map, &spawn_x, &spawn_y);
        map_set_rosemoor_swamp_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
        g->player.x = TOWN_W - 2;
        g->player.y = TOWN3_KING_GATE_Y;
    }
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    place_town_portal(g);
    push_message(g, destination == LOCATION_CASTLE ?
        "You reach the grounds of the Castle of No Return." :
        (destination == LOCATION_TOWN4 ? "You return to Ridgeshire." : "You return to Rosemoor."));
    if (destination == LOCATION_TOWN3 || destination == LOCATION_TOWN4) {
        town_road_enter(g, destination, destination == LOCATION_TOWN3 ? TOWN_SIDE_EAST : TOWN_SIDE_WEST, 0);
    }
}

void game_enter_inn(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_INN;
    map_generate_inn(&g->map, &spawn_x, &spawn_y);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "You enter the inn.");
}

void game_enter_guild(GameState *g) {
    int sx;
    int sy;
    g->location = LOCATION_GUILD;
    map_generate_guild(&g->map, &sx, &sy);
    g->player.x = sx;
    g->player.y = sy;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "You enter the Adventurer's Guild.");
}

void town_life_enter(GameState *g) {
    const TownBuilding *b = town_life_building(g->location);
    if (!b || b->town != g->location) {
        return;
    }
    g->location = b->interior;
    town_life_generate(&g->map, g->location, &g->player.x, &g->player.y);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    g->dialogue_active = 0;
}

void town_life_leave(GameState *g) {
    const TownBuilding *b = town_life_building(g->location);
    if (!b || b->interior != g->location) {
        return;
    }
    if (b->town == LOCATION_TOWN) {
        game_leave_tavern(g);
    } else if (b->town == LOCATION_DAWNMERE) {
        g->location = LOCATION_DAWNMERE;
        map_generate_dawnmere(&g->map, &g->player.x, &g->player.y);
        place_town_portal(g);
    } else if (b->town == LOCATION_TOWN2) {
        game_leave_inn(g);
    } else if (b->town == LOCATION_TOWN3) {
        g->location = LOCATION_TOWN3;
        map_generate_town3(&g->map, &g->player.x, &g->player.y);
        map_set_rosemoor_swamp_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
        place_town_portal(g);
    } else {
        g->location = LOCATION_TOWN4;
        map_generate_town4(&g->map, &g->player.x, &g->player.y);
        map_set_ridgeshire_mountain_road(&g->map, g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
        place_town_portal(g);
    }
    g->player.x = b->x + b->w / 2;
    g->player.y = b->y + b->h;
    g->dialogue_active = 0;
    g->enemy_count = 0;
    g->floor_item_count = 0;
}

void game_enter_workshop(GameState *g) {
    g->location = LOCATION_WORKSHOP;
    map_generate_workshop(&g->map, &g->player.x, &g->player.y);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "Garrick sharpens swords, axes and daggers: +1 attack for 50 gold, once per weapon.");
    push_message(g, "Tessa sells arrows by the benches. Press T beside either craftsperson.");
}

void game_leave_workshop(GameState *g) {
    g->location = LOCATION_TOWN4;
    map_generate_town4(&g->map, &g->player.x, &g->player.y);
    map_set_ridgeshire_mountain_road(&g->map, g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
    g->player.x = TOWN4_WORKSHOP_DOOR_X;
    g->player.y = TOWN4_WORKSHOP_DOOR_Y + 1;
    place_town_portal(g);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "You step out of the workshop.");
}

void game_enter_town_hall(GameState *g) {
    g->location = LOCATION_TOWN_HALL;
    map_generate_town_hall(&g->map, &g->player.x, &g->player.y);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "Ridgeshire Town Hall. Press T beside an NPC to talk.");
}

void game_leave_town_hall(GameState *g) {
    g->location = LOCATION_TOWN4;
    map_generate_town4(&g->map, &g->player.x, &g->player.y);
    map_set_ridgeshire_mountain_road(&g->map, g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
    g->player.x = TOWN4_HALL_DOOR_X;
    g->player.y = TOWN4_HALL_DOOR_Y + 1;
    place_town_portal(g);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "You step out of the Town Hall.");
}

void game_talk_to_veyra(GameState *g) {
    if (g->location != LOCATION_TOWN_HALL || abs(g->player.x - HALL_VEYRA_X) > 1 || abs(g->player.y - HALL_VEYRA_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    g->dialogue_x = HALL_VEYRA_X;
    g->dialogue_y = HALL_VEYRA_Y;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Marshal Veyra");
    if (!g->watchfire_quest_state) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Our warning fires are dark. Restore the watchfires in Goblin Mountains 5, Ashen Hollow 3, and Dragonspine 3, in any order. Defeat their guards, then press A beside each. Return for 180 gold.");
    } else if (g->watchfire_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Watchfires: Mountains 5 %s; Ashen Hollow 3 %s; Dragonspine 3 %s. Ashen lies north of Castle Grounds. Defeat the guards, then press A beside each fire. Bosses are optional.",
            g->watchfire_quest_progress & 1 ? "lit" : "dark", g->watchfire_quest_progress & 2 ? "lit" : "dark", g->watchfire_quest_progress & 4 ? "lit" : "dark");
    } else if (g->watchfire_quest_state == 2) {
        g->watchfire_quest_state = 3;
        g->gold += WATCHFIRE_REWARD_GOLD;
        g->score += WATCHFIRE_REWARD_SCORE;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Three watchfires burn again. Warnings can cross the realm once more. Take 180 gold with the town's thanks.");
        push_message(g, "Completed: Watchfires of Ridgeshire. 180 gold and 1400 score awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "Ridgeshire's watchfires still burn. Our scouts remember who restored them.");
    }
}

void game_talk_to_steward(GameState *g) {
    if (g->location != LOCATION_TOWN_HALL ||
        abs(g->player.x - HALL_STEWARD_X) > 1 || abs(g->player.y - HALL_STEWARD_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Steward Hadrin");
    g->dialogue_x = HALL_STEWARD_X;
    g->dialogue_y = HALL_STEWARD_Y;
    if (g->emberforge_quest_state == 0) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Ashen Hollow lies north of Castle Grounds. Recover the forge mechanism on stage 2, then repair the furnace on stage 4. Defeat the guards and press A beside each. Return for 100 gold.");
    } else if (g->emberforge_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, g->emberforge_progress & EMBERFORGE_MECHANISM_RECOVERED ?
            "You have the mechanism. Defeat the Obsidian Guardians at the Emberforge on Ashen Hollow stage 4, "
            "then press A beside the furnace to repair it." :
            "The stolen mechanism lies in Ashen Hollow stage 2's last clearing. Defeat its guards and press A beside it. "
            "The abandoned furnace is on stage 4.");
    } else if (g->emberforge_quest_state == 2) {
        g->emberforge_quest_state = 3;
        g->gold += EMBERFORGE_REWARD_GOLD;
        g->score += EMBERFORGE_REWARD_SCORE;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The Emberforge burns again! Ridgeshire's miners can reclaim their livelihood. "
            "Here are your 100 gold, with the town's thanks.");
        push_message(g, "Completed: Reclaim the Emberforge. 100 gold and 800 score awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "The Emberforge is restored. Ridgeshire will remember your help.");
    }
}

static int emberforge_target(const GameState *g, int *x, int *y) {
    if (!emberforge_stage_bit(g)) {
        return 0;
    }
    const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 5; i++) {
        int tx = g->player.x + offsets[i][0];
        int ty = g->player.y + offsets[i][1];
        if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H) {
            continue;
        }
        TileType tile = quest_tile(g, tx, ty);
        if (tile == TILE_PORTAL && g->portal_active && g->portal_location == g->location && g->portal_level == g->level) {
            tile = g->portal_origin_tile;
        }
        if (tile == TILE_EMBERFORGE_MECHANISM || tile == TILE_EMBERFORGE_COLD || tile == TILE_EMBERFORGE_LIT) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }
    return 0;
}

int game_has_emberforge_interaction(const GameState *g) {
    int x;
    int y;
    return emberforge_target(g, &x, &y);
}

int game_interact_emberforge(GameState *g) {
    int x;
    int y;
    if (!emberforge_target(g, &x, &y)) {
        return 0;
    }
    int bit = emberforge_stage_bit(g);
    if (g->emberforge_progress & bit) {
        push_message(g, "The Emberforge already burns brightly.");
        return 1;
    }
    if (g->emberforge_quest_state != 1) {
        return 1;
    }
    if (bit == EMBERFORGE_RESTORED && !(g->emberforge_progress & EMBERFORGE_MECHANISM_RECOVERED)) {
        push_message(g, "The furnace needs its stolen mechanism from Ashen Hollow stage 2.");
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        if (enemy->active && (strcmp(enemy->name, "Emberforge Guardian") == 0 ||
            abs(enemy->x - x) + abs(enemy->y - y) <= 4)) {
            push_message(g, "Defeat the Emberforge's defenders before working here.");
            return 1;
        }
    }
    g->emberforge_progress |= bit;
    if (bit == EMBERFORGE_MECHANISM_RECOVERED) {
        set_emberforge_tile(g, x, y, TILE_ASHEN_RUIN);
        push_message(g, "Forge mechanism recovered. Repair the Emberforge on Ashen Hollow stage 4.");
    } else {
        set_emberforge_tile(g, x, y, TILE_EMBERFORGE_LIT);
        g->emberforge_quest_state = 2;
        push_message(g, "The Emberforge burns again! Return to Steward Hadrin in Ridgeshire Town Hall.");
    }
    return 1;
}

void game_talk_to_selene(GameState *g) {
    if (g->location != LOCATION_GUILD || abs(g->player.x - GUILD_SELENE_X) > 1 || abs(g->player.y - GUILD_SELENE_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    g->dialogue_x = GUILD_SELENE_X;
    g->dialogue_y = GUILD_SELENE_Y;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Huntmaster Selene");
    if (!g->hunt_quest_state) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Hunt Rimefang in Frostfell 3, Shardwarden in Stonewake 4, and Dunehex in Sunscar 3. Defeat each entire group, in any order. Return to me here in Rosemoor's Guild for 200 gold.");
    } else if (g->hunt_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Rimefang, Frostfell 3: %s. Shardwarden, Stonewake 4: %s. Dunehex, Sunscar 3: %s. Defeat each entire group. Bosses are optional. Return to me here in Rosemoor's Guild.",
            g->hunt_quest_progress & 1 ? "done" : "hunting", g->hunt_quest_progress & 2 ? "done" : "hunting", g->hunt_quest_progress & 4 ? "done" : "hunting");
    } else if (g->hunt_quest_state == 2) {
        g->hunt_quest_state = 3;
        g->gold += HUNT_REWARD_GOLD;
        g->score += HUNT_REWARD_SCORE;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Three great hunts completed. Our expedition routes are safer thanks to you. Take 200 gold, and the Guild's respect.");
        push_message(g, "Completed: The Three Great Hunts. 200 gold and 1500 score awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "The Guild remembers your three great hunts. Our expeditions can travel those routes again.");
    }
}

void game_talk_to_orin(GameState *g) {
    if (g->location != LOCATION_GUILD || abs(g->player.x - GUILD_ORIN_X) > 1 || abs(g->player.y - GUILD_ORIN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    g->dialogue_x = GUILD_ORIN_X;
    g->dialogue_y = GUILD_ORIN_Y;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Surveyor Orin");
    if (g->glassdeep_quest_state == 0) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Stonewake Quarry lies south of Stillbury. Reclaim the West Winch on stage 2, Workshop Hoist on 3, and Main Gantry on 4. Return here for 120 gold and a Magic Stone Warhammer.");
    } else if (g->glassdeep_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Clear the machinery's guards on Stonewake stages 2, 3 and 4, then press A beside each mechanism to repair it. Cranes lower optional walkways; the permanent ramps always remain open. Return to me here in Rosemoor's Guild.");
    } else {
        if (g->glassdeep_quest_state == 2) {
            g->glassdeep_quest_state = 3;
            g->gold += GLASSDEEP_REWARD_GOLD;
            g->score += GLASSDEEP_REWARD_SCORE;
            g->stonewake_warhammer_state = 1;
            push_message(g, "Completed: Reclaim the Quarry. 120 gold and 1,000 score awarded.");
        }
        stonewake_claim_warhammer(g);
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, g->stonewake_warhammer_state == 1 ?
            "The quarry can work again. Your Magic Stone Warhammer is safe with me. Make room in your pack and speak to me again to collect it." :
            "The quarry can work again! The runed stone warhammer strengthens its wielder: 30 maximum HP while equipped. The Guild thanks you.");
    }
}

static int glassdeep_resonator_target(const GameState *g, int *x, int *y) {
    if (!glassdeep_quest_bit(g) || !g->glassdeep_quest_state) {
        return 0;
    }
    const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 5; i++) {
        int tx = g->player.x + offsets[i][0];
        int ty = g->player.y + offsets[i][1];
        if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H) {
            continue;
        }
        TileType tile = quest_object_tile(g, tx, ty);
        if (tile == TILE_GLASSDEEP_RESONATOR || tile == TILE_GLASSDEEP_RESONATOR_LIT) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }
    return 0;
}

static int glassdeep_resonator_guarded(const GameState *g, int x, int y) {
    if (!(g->glassdeep_quest_encounters & glassdeep_quest_bit(g))) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        if (enemy->active && (strcmp(enemy->name, "Quarry Guardian") == 0 ||
            abs(enemy->x - x) + abs(enemy->y - y) <= 4)) {
            return 1;
        }
    }
    return 0;
}

int game_has_glassdeep_interaction(const GameState *g) {
    int x;
    int y;
    return glassdeep_resonator_target(g, &x, &y);
}

int game_interact_glassdeep(GameState *g) {
    int x;
    int y;
    if (!glassdeep_resonator_target(g, &x, &y)) {
        return 0;
    }
    if (g->glassdeep_quest_progress & glassdeep_quest_bit(g)) {
        push_message(g, "The repaired quarry machinery is working steadily.");
        return 1;
    }
    if (glassdeep_resonator_guarded(g, x, y)) {
        push_message(g, "Defeat the machinery's defenders before repairing it.");
        return 1;
    }
    if (stonewake_map(&g->map)) {
        g->glassdeep_quest_progress |= glassdeep_quest_bit(g);
        set_quest_object_tile(g, x, y, TILE_GLASSDEEP_RESONATOR_LIT);
        char message[MAX_MESSAGE_LEN];
        snprintf(message, sizeof(message), "%s repaired!", glassdeep_resonator_name(g));
        push_message(g, message);
        if (g->glassdeep_quest_progress == 7) {
            g->glassdeep_quest_state = 2;
            push_message(g, "All three mechanisms repaired. Return to Surveyor Orin in Rosemoor's Guild.");
        }
        return 1;
    }
    g->dialogue_active = 1;
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "%s", glassdeep_resonator_name(g));
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "%s Choose: 1 Low, 2 Middle, 3 High. Esc cancels.", glassdeep_inscription(g));
    return 1;
}

int game_glassdeep_prompt_active(const GameState *g) {
    int x;
    int y;
    return g->dialogue_active && g->glassdeep_quest_state == 1 &&
        glassdeep_resonator_target(g, &x, &y) && x == g->dialogue_x && y == g->dialogue_y &&
        !(g->glassdeep_quest_progress & glassdeep_quest_bit(g)) &&
        strcmp(g->dialogue_speaker, glassdeep_resonator_name(g)) == 0;
}

int game_handle_glassdeep_prompt_key(GameState *g, int key, int repeat) {
    if (!game_glassdeep_prompt_active(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_RETURN) {
        g->dialogue_active = 0;
        return 1;
    }
    int tone = key == SDL_SCANCODE_1 || key == SDL_SCANCODE_KP_1 ? 1 :
        key == SDL_SCANCODE_2 || key == SDL_SCANCODE_KP_2 ? 2 :
        key == SDL_SCANCODE_3 || key == SDL_SCANCODE_KP_3 ? 3 : 0;
    if (!tone) {
        return 1;
    }
    if (glassdeep_resonator_guarded(g, g->dialogue_x, g->dialogue_y)) {
        g->dialogue_active = 0;
        push_message(g, "Defeat the resonator's defenders before tuning it.");
        return 1;
    }
    if (tone != g->level - 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "The crystal stays dark. %s Choose: 1 Low, 2 Middle, 3 High. Esc cancels.", glassdeep_inscription(g));
        push_message(g, "That tone does not match the inscription. Try another resonance.");
        return 1;
    }
    g->glassdeep_quest_progress |= glassdeep_quest_bit(g);
    set_quest_object_tile(g, g->dialogue_x, g->dialogue_y, TILE_GLASSDEEP_RESONATOR_LIT);
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "The crystal answers and shines. Its resonance has been restored.");
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "%s restored!", glassdeep_resonator_name(g));
    push_message(g, message);
    if (g->glassdeep_quest_progress == 7) {
        g->glassdeep_quest_state = 2;
        push_message(g, "All three resonators shine. Return to Surveyor Orin in Rosemoor's Adventurer's Guild.");
    }
    return 1;
}

void game_talk_to_brenna(GameState *g) {
    if (g->location != LOCATION_INN || abs(g->player.x - BRENNA_INN_X) > 1 || abs(g->player.y - BRENNA_INN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Quartermaster Brenna");
    g->dialogue_x = BRENNA_INN_X;
    g->dialogue_y = BRENNA_INN_Y;
    if (g->frostfell_quest_state == 0) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Frostfell lies in the far north. Cross the swamp north of Stillbury to Rosemoor, then head through its north gate into Frostfell. "
            "My expedition vanished beyond Rosemoor's north gate.");
    } else if (g->frostfell_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, g->frostfell_quest_progress & FROSTFELL_JOURNAL_RECOVERED ?
            "The journal says Surveyor Fen survived. Find him on Frostfell stage 4, defeat his captors, and speak to him. Return here for 100 gold." :
            "Recover the journal guarded by Ice Wolves on Frostfell stage 2. Press A beside it, then find Surveyor Fen on stage 4. Return here for 100 gold.");
    } else if (g->frostfell_quest_state == 2) {
        g->frostfell_quest_state = 3;
        g->gold += FROSTFELL_REWARD_GOLD;
        g->score += FROSTFELL_REWARD_SCORE;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Fen has returned safely, and the journal preserves our expedition's work. Take these 100 gold with my thanks.");
        push_message(g, "Completed: The Silent Expedition. 100 gold and 800 score awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "Fen is recovering. Thanks to you, our expedition will not be forgotten.");
    }
}

static int frostfell_quest_guarded(const GameState *g, int x, int y) {
    int bit = frostfell_quest_stage_bit(g);
    if (!(g->frostfell_quest_encounters & bit)) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        if (enemy->active && (strcmp(enemy->name, "Expedition Guardian") == 0 ||
            abs(enemy->x - x) + abs(enemy->y - y) <= 4)) {
            return 1;
        }
    }
    return 0;
}

void game_talk_to_frost_survivor(GameState *g, int x, int y) {
    if (g->location != LOCATION_FROSTFELL || g->level != FROSTFELL_SURVIVOR_LEVEL ||
        g->frostfell_quest_state != 1 || x < 0 || x >= MAP_W || y < 0 || y >= MAP_H ||
        abs(g->player.x - x) > 1 || abs(g->player.y - y) > 1 ||
        quest_object_tile(g, x, y) != TILE_NPC_FROST_SURVIVOR) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Surveyor Fen");
    g->dialogue_x = x;
    g->dialogue_y = y;
    if (!(g->frostfell_quest_progress & FROSTFELL_JOURNAL_RECOVERED)) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Our journal lies back on Frostfell stage 2. Please recover it first; its markings show the safe route home.");
    } else if (frostfell_quest_guarded(g, x, y)) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "I cannot escape while my captors are alive. Please defeat them first!");
    } else {
        g->frostfell_quest_progress |= FROSTFELL_SURVIVOR_RESCUED;
        g->frostfell_quest_state = 2;
        set_quest_object_tile(g, x, y, TILE_FROST_FLOOR);
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The journal shows the route! I can make my own way home now. Tell Brenna in Stillbury's Inn that the expedition's work is safe.");
        push_message(g, "Surveyor Fen heads home. Return to Quartermaster Brenna in Stillbury's Inn.");
    }
}

static int frostfell_journal_target(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_FROSTFELL || g->level != FROSTFELL_JOURNAL_LEVEL || g->frostfell_quest_state != 1) {
        return 0;
    }
    const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 5; i++) {
        int tx = g->player.x + offsets[i][0];
        int ty = g->player.y + offsets[i][1];
        if (tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H && quest_object_tile(g, tx, ty) == TILE_FROST_JOURNAL) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }
    return 0;
}

int game_has_frostfell_interaction(const GameState *g) {
    int x;
    int y;
    return frostfell_journal_target(g, &x, &y);
}

int game_interact_frostfell(GameState *g) {
    int x;
    int y;
    if (!frostfell_journal_target(g, &x, &y)) {
        return 0;
    }
    if (frostfell_quest_guarded(g, x, y)) {
        push_message(g, "Defeat the expedition journal's defenders before retrieving it.");
        return 1;
    }
    g->frostfell_quest_progress |= FROSTFELL_JOURNAL_RECOVERED;
    set_quest_object_tile(g, x, y, TILE_FROST_FLOOR);
    push_message(g, "Expedition journal recovered. Find Surveyor Fen on Frostfell stage 4 and speak to him.");
    return 1;
}

int game_workshop_near_fletcher(const GameState *g) {
    return g->location == LOCATION_WORKSHOP &&
        abs(g->player.x - WORKSHOP_FLETCHER_X) + abs(g->player.y - WORKSHOP_FLETCHER_Y) == 1;
}

int game_workshop_near_smith(const GameState *g) {
    return g->location == LOCATION_WORKSHOP &&
        abs(g->player.x - WORKSHOP_SMITH_X) + abs(g->player.y - WORKSHOP_SMITH_Y) == 1;
}

int game_sharpen_weapon(GameState *g, int index) {
    if (!game_workshop_near_smith(g) || index < 0 || index >= g->inventory_count) {
        return 0;
    }
    Item *item = &g->inventory[index];
    if (item->sharpened) {
        push_message(g, "Garrick: This weapon has already been sharpened.");
        return 0;
    }
    if (!item_can_sharpen(item)) {
        push_message(g, "Garrick: I sharpen swords, axes and daggers.");
        return 0;
    }
    if (g->gold < WORKSHOP_SHARPEN_PRICE) {
        push_message(g, "Garrick: Sharpening costs 50 gold.");
        return 0;
    }
    int old_off_hand = game_off_hand_attack_bonus(item);
    item->attack_bonus++;
    item->sharpened = 1;
    g->gold -= WORKSHOP_SHARPEN_PRICE;
    if (g->equipped_main_hand == index) {
        g->player.attack++;
    } else if (g->equipped_off_hand == index) {
        g->player.attack += game_off_hand_attack_bonus(item) - old_off_hand;
    }
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Garrick sharpens %s: +1 attack. Paid 50 gold.", item->name);
    push_message(g, message);
    return 1;
}

void game_leave_guild(GameState *g) {
    game_enter_town3(g);
    g->player.x = TOWN_GUILD_DOOR_X;
    g->player.y = TOWN_GUILD_DOOR_Y + 1;
    push_message(g, "You step out of the Adventurer's Guild.");
}

void game_leave_inn(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_TOWN2;
    map_generate_town2(&g->map, &spawn_x, &spawn_y);
    map_set_town3_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
    map_set_stillbury_forest_road(&g->map, g->defeated_bosses & (1 << LOCATION_FOREST));
    place_town_portal(g);
    g->player.x = TOWN_INN_DOOR_X;
    g->player.y = TOWN_INN_DOOR_Y + 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, "You step out of the inn.");
}

static void spawn_labyrinth_enemies(GameState *g) {
    int positions_x[400];
    int positions_y[400];
    int position_count = 0;
    for (int y = 2; y < LABYRINTH_H - 2; y++) {
        for (int x = 2; x < LABYRINTH_W - 2; x++) {
            if (g->map.tiles[y][x] != TILE_LABYRINTH_FLOOR || x >= 33 ||
                abs(x - g->map.stairs_up_x) +
                    abs(y - g->map.stairs_up_y) < 6) {
                continue;
            }
            positions_x[position_count] = x;
            positions_y[position_count] = y;
            position_count++;
        }
    }
    int target = 3 + g->level;
    for (int i = 0; i < target && position_count > 0; i++) {
        int position = (i + 1) * position_count / (target + 1);
        EnemyType type = g->level == 1 ?
            (i % 2 == 0 ? ENEMY_SKELETON : ENEMY_GIANT_SPIDER) :
            (i % 2 == 0 ? ENEMY_ANIMATED_STATUE : ENEMY_GIANT_SPIDER);
        spawn_enemy(g, &g->enemies[g->enemy_count], type,
            positions_x[position], positions_y[position]);
        g->enemy_count++;
    }
    if (g->level == LABYRINTH_DEPTH &&
        !(g->defeated_bosses & (1 << LOCATION_LABYRINTH))) {
        spawn_enemy(g, &g->enemies[g->enemy_count], ENEMY_MINOTAUR,
            39, g->map.stairs_up_y);
        g->enemy_count++;
        minotaur_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
}

static void save_labyrinth_floor(GameState *g) {
    LevelCache *cache = &g->labyrinth_cache[g->level - 1];
    cache->map = g->map;
    cache->enemy_count = g->enemy_count;
    cache->level_cleared = g->level_cleared;
    for (int i = 0; i < g->enemy_count; i++) {
        cache->enemies[i] = g->enemies[i];
    }
    cache->valid = 1;
}

static void load_labyrinth_floor(GameState *g) {
    int spawn_x;
    int spawn_y;
    LevelCache *cache = &g->labyrinth_cache[g->level - 1];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        g->level_cleared = cache->level_cleared;
        for (int i = 0; i < g->enemy_count; i++) {
            g->enemies[i] = cache->enemies[i];
        }
    } else {
        map_generate_labyrinth(&g->map, g->level, g->rook_labyrinth_switches,
            &spawn_x, &spawn_y);
        g->enemy_count = 0;
        g->level_cleared = 0;
        spawn_labyrinth_enemies(g);
    }
    if (g->level == LABYRINTH_DEPTH && g->rook_quest_state >= 2) {
        g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] =
            TILE_LABYRINTH_FLOOR;
    }
    if (g->level == LABYRINTH_DEPTH &&
        g->rook_labyrinth_switches == (1 << LABYRINTH_SWITCH_COUNT) - 1) {
        g->map.tiles[g->map.stairs_up_y][35] = TILE_LABYRINTH_FLOOR;
    }
    g->floor_item_count = 0;
    minotaur_restore_reward(g);
}

int game_labyrinth_is_open(const GameState *g) {
    return g->debug_passages_unlocked || g->rook_quest_state != 0;
}

void game_enter_labyrinth(GameState *g) {
    const TownApproach *route = approach_route(g);
    g->labyrinth_entry_town = route && route->area == LOCATION_FOREST ? route->town : LOCATION_TOWN2;
    g->approach_route = -1;
    g->location = LOCATION_LABYRINTH;
    start_expedition(g, g->labyrinth_cache, LABYRINTH_DEPTH);
    g->level = 1;
    load_labyrinth_floor(g);
    g->player.x = 2;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, "You enter Rook's labyrinth.");
    push_message(g, "Five runes open the deepest vault.");
    push_message(g, "Press the interact key (A by default) on or beside rune switches to activate them.");
}

void game_change_labyrinth_floor(GameState *g, int descending, int false_stair) {
    clear_floor_loot(g);
    save_labyrinth_floor(g);
    g->level += descending ? 1 : -1;
    load_labyrinth_floor(g);
    if (descending) {
        g->player.x = false_stair ? LABYRINTH_FALSE_EXIT_X + 1 : 2;
        g->player.y = false_stair ? LABYRINTH_FALSE_EXIT_Y :
            g->map.stairs_up_y;
    } else {
        g->player.x = g->map.stairs_down_x;
        g->player.y = g->map.stairs_down_y;
        if (false_stair) {
            for (int y = 0; y < LABYRINTH_H; y++) {
                for (int x = 0; x < LABYRINTH_W; x++) {
                    if (g->map.tiles[y][x] == TILE_LABYRINTH_STAIRS &&
                        (x != g->map.stairs_down_x ||
                        y != g->map.stairs_down_y)) {
                        g->player.x = x;
                        g->player.y = y;
                    }
                }
            }
        }
    }
    g->dialogue_active = 0;
    if (false_stair && descending) {
        push_message(g, "The stairs end in a blind corridor.");
    } else if (false_stair) {
        push_message(g, "You return to the maze above.");
    } else {
        push_message(g, "You reach another level of the labyrinth.");
    }
}

static int labyrinth_interaction_tile(TileType tile) {
    return tile == TILE_LABYRINTH_SWITCH_OFF ||
        tile == TILE_LABYRINTH_SWITCH_ON || tile == TILE_LABYRINTH_RELIC;
}

int game_has_labyrinth_interaction(const GameState *g) {
    if (g->location != LOCATION_LABYRINTH) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
            labyrinth_interaction_tile(g->map.tiles[y][x])) {
            return 1;
        }
    }
    return 0;
}

int game_interact_labyrinth(GameState *g) {
    if (g->location != LOCATION_LABYRINTH) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
            continue;
        }
        TileType tile = g->map.tiles[y][x];
        if (tile == TILE_LABYRINTH_SWITCH_ON) {
            push_message(g, "This labyrinth rune is already lit.");
            return 1;
        }
        if (tile == TILE_LABYRINTH_SWITCH_OFF) {
            g->map.tiles[y][x] = TILE_LABYRINTH_SWITCH_ON;
            g->rook_labyrinth_switches |= 1 << (g->level - 1);
            int lit = 0;
            for (int floor = 0; floor < LABYRINTH_DEPTH; floor++) {
                lit += (g->rook_labyrinth_switches >> floor) & 1;
            }
            if (lit == LABYRINTH_SWITCH_COUNT) {
                for (int map_y = 0; map_y < MAP_H; map_y++) {
                    for (int map_x = 0; map_x < MAP_W; map_x++) {
                        if (g->map.tiles[map_y][map_x] ==
                            TILE_LABYRINTH_GATE) {
                            g->map.tiles[map_y][map_x] =
                                TILE_LABYRINTH_FLOOR;
                        }
                    }
                }
                if (g->labyrinth_cache[LABYRINTH_DEPTH - 1].valid) {
                    g->labyrinth_cache[LABYRINTH_DEPTH - 1].map.tiles[
                        g->labyrinth_cache[LABYRINTH_DEPTH - 1].map.stairs_up_y][35] =
                        TILE_LABYRINTH_FLOOR;
                }
                push_message(g, "The fifth rune opens the relic vault!");
            } else {
                char message[MAX_MESSAGE_LEN];
                snprintf(message, sizeof(message),
                    "Labyrinth rune lit: %d of %d.",
                    lit, LABYRINTH_SWITCH_COUNT);
                push_message(g, message);
            }
            return 1;
        }
        if (tile == TILE_LABYRINTH_RELIC) {
            if (g->rook_quest_state != 1) {
                push_message(g, "The empty pedestal holds nothing for you.");
                return 1;
            }
            for (int enemy = 0; enemy < g->enemy_count; enemy++) {
                if (g->enemies[enemy].active && g->enemies[enemy].is_boss) {
                    push_message(g, "The Minotaur guards the ivory rook.");
                    return 1;
                }
            }
            g->rook_quest_state = 2;
            g->map.tiles[y][x] = TILE_LABYRINTH_FLOOR;
            push_message(g, "You recover Rook's stolen ivory rook.");
            push_message(g, "Return it to Rook in Oakhaven's Tavern.");
            return 1;
        }
    }
    return 0;
}

void game_enter_island(GameState *g) {
    for (int i = 0; i < g->inventory_count; i++) {
        if (g->inventory[i].type == ITEM_TREASURE_MAP) {
            game_remove_inventory_item(g, i);
            break;
        }
    }
    g->island_travel_unlocked = 1;
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_ISLAND;
    g->level = 1;
    g->level_cleared = 0;
    map_generate_island(&g->map, &spawn_x, &spawn_y);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "You make landfall on the Ruined Isle.");
    push_message(g, "The island route is now open for return voyages.");
}

void game_leave_island(GameState *g) {
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    coastal_return_from_island(g);
}

static void cache_temple_level(GameState *g) {
    LevelCache *cache = &g->temple_cache[g->level - 1];
    cache->map = g->map;
    cache->enemy_count = g->enemy_count;
    cache->level_cleared = g->level_cleared;
    for (int i = 0; i < g->enemy_count; i++) {
        cache->enemies[i] = g->enemies[i];
    }
    cache->valid = 1;
}

static void spawn_temple_enemies(GameState *g) {
    static const int counts[TEMPLE_DEPTH] = {6, 7, 8, 8, 7};
    static const EnemyType types[TEMPLE_DEPTH][8] = {
        {ENEMY_RELIC_SCARABS, ENEMY_TEMPLE_STALKER,
            ENEMY_BLOWDART_HUNTER, ENEMY_VINEBOUND_GUARDIAN,
            ENEMY_SUN_PRIEST, ENEMY_SERPENT_SPIRIT},
        {ENEMY_RELIC_SCARABS, ENEMY_BLOWDART_HUNTER,
            ENEMY_VINEBOUND_GUARDIAN, ENEMY_SUN_PRIEST,
            ENEMY_SERPENT_SPIRIT, ENEMY_TEMPLE_STALKER,
            ENEMY_LUNAR_EFFIGY},
        {ENEMY_TEMPLE_STALKER, ENEMY_BLOWDART_HUNTER,
            ENEMY_VINEBOUND_GUARDIAN, ENEMY_SUN_PRIEST,
            ENEMY_SERPENT_SPIRIT, ENEMY_TREASURE_WRAITH,
            ENEMY_LUNAR_EFFIGY, ENEMY_RELIC_SCARABS},
        {ENEMY_VINEBOUND_GUARDIAN, ENEMY_SUN_PRIEST,
            ENEMY_SERPENT_SPIRIT, ENEMY_TREASURE_WRAITH,
            ENEMY_LUNAR_EFFIGY, ENEMY_TREASURE_WRAITH,
            ENEMY_TEMPLE_STALKER, ENEMY_RELIC_SCARABS},
        {ENEMY_RELIC_SCARABS, ENEMY_TEMPLE_STALKER,
            ENEMY_BLOWDART_HUNTER, ENEMY_SUN_PRIEST,
            ENEMY_SERPENT_SPIRIT, ENEMY_TREASURE_WRAITH,
            ENEMY_FALLEN_SUN_GUARDIAN}
    };
    static const int positions[TEMPLE_DEPTH][8][2] = {
        {{14, 26}, {10, 29}, {19, 24}, {7, 24},
            {53, 24}, {48, 29}},
        {{10, 25}, {18, 23}, {48, 24}, {55, 27},
            {26, 17}, {39, 17}, {42, 6}},
        {{10, 25}, {18, 24}, {48, 25}, {55, 27},
            {24, 17}, {40, 17}, {12, 6}, {50, 6}},
        {{10, 25}, {18, 24}, {48, 25}, {55, 27},
            {24, 17}, {40, 17}, {12, 6}, {50, 6}},
        {{14, 26}, {10, 29}, {19, 24}, {53, 24},
            {48, 29}, {41, 19}, {32, 9}}
    };
    g->enemy_count = 0;
    int floor = g->level - 1;
    for (int i = 0; i < counts[floor]; i++) {
        EnemyType type = types[floor][i];
        if (type == ENEMY_FALLEN_SUN_GUARDIAN &&
            (g->defeated_bosses & (1 << LOCATION_TEMPLE))) {
            continue;
        }
        spawn_enemy(g, &g->enemies[g->enemy_count++], type,
            positions[floor][i][0], positions[floor][i][1]);
    }
    if (g->level == TEMPLE_DEPTH) {
        sun_guardian_prepare_map(&g->map, g->enemies, g->enemy_count, g->player.x, g->player.y);
    }
}

void game_enter_temple(GameState *g) {
    g->location = LOCATION_TEMPLE;
    start_expedition(g, g->temple_cache, TEMPLE_DEPTH);
    g->level = 1;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    if (g->temple_cache[0].valid) {
        g->map = g->temple_cache[0].map;
        g->enemy_count = g->temple_cache[0].enemy_count;
        g->level_cleared = g->temple_cache[0].level_cleared;
        for (int i = 0; i < g->enemy_count; i++) {
            g->enemies[i] = g->temple_cache[0].enemies[i];
        }
    } else {
        int spawn_x;
        int spawn_y;
        map_generate_temple(&g->map, 1, &spawn_x, &spawn_y);
        g->player.x = spawn_x;
        g->player.y = spawn_y;
        g->level_cleared = 0;
        g->temple_alignment = 0;
        g->temple_sentinels_awakened = 0;
        spawn_temple_enemies(g);
    }
    sync_temple_floor_state(g);
    g->player.x = TEMPLE_ENTRANCE_X;
    g->player.y = TEMPLE_ENTRANCE_Y;
    push_message(g, "You enter the Ruined Temple. The sun seal burns.");
    push_message(g, "Press the interact key (A by default) on or beside solar altars to switch between Sun and Moon.");
}

void game_leave_temple(GameState *g) {
    cache_temple_level(g);
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_ISLAND;
    map_generate_island(&g->map, &spawn_x, &spawn_y);
    g->player.x = ISLAND_GATE_X;
    g->player.y = ISLAND_GATE_Y + 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->player.poison_turns = 0;
    push_message(g, "You step back onto the island.");
}

static int temple_interaction_tile(TileType tile) {
    return tile == TILE_TEMPLE_ALTAR || tile == TILE_TEMPLE_TREASURE;
}

int game_temple_dormant_sentinels(const GameState *g) {
    if (g->location != LOCATION_TEMPLE) {
        return 0;
    }
    int count = 0;
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            if (g->map.tiles[y][x] == TILE_TEMPLE_DORMANT_SENTINEL) {
                count++;
            }
        }
    }
    return count;
}

int game_temple_remaining_enemies(const GameState *g) {
    if (g->location != LOCATION_TEMPLE) {
        return 0;
    }
    int count = game_temple_dormant_sentinels(g);
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active) {
            count++;
        }
    }
    return count;
}

int game_has_temple_interaction(const GameState *g) {
    if (g->location != LOCATION_TEMPLE) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
            temple_interaction_tile(quest_tile(g, x, y))) {
            return 1;
        }
    }
    return 0;
}

static void toggle_temple_alignment(GameState *g) {
    // Refuse the whole switch rather than closing a lunar door on an occupant.
    if (g->temple_alignment) {
        for (int y = 0; y < TEMPLE_H; y++) {
            for (int x = 0; x < TEMPLE_W; x++) {
                if (quest_tile(g, x, y) != TILE_TEMPLE_MOON_DOOR_OPEN) {
                    continue;
                }
                int occupied = (g->player.x == x && g->player.y == y) || g->map.tiles[y][x] == TILE_ITEM;
                for (int i = 0; i < g->enemy_count; i++) {
                    occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
                }
                if (occupied) {
                    push_message(g, "Lunar doorway occupied. Clear it before raising the Sun.");
                    return;
                }
            }
        }
    }
    g->temple_alignment = !g->temple_alignment;
    if (g->temple_alignment) {
        g->temple_sentinels_awakened = 1;
    }
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            TileType tile = quest_tile(g, x, y);
            if (g->temple_alignment && tile == TILE_TEMPLE_MOON_DOOR_CLOSED) {
                set_temple_tile(g, x, y, TILE_TEMPLE_MOON_DOOR_OPEN);
            } else if (!g->temple_alignment &&
                tile == TILE_TEMPLE_MOON_DOOR_OPEN) {
                set_temple_tile(g, x, y, TILE_TEMPLE_MOON_DOOR_CLOSED);
            } else if (g->temple_alignment &&
                tile == TILE_TEMPLE_DORMANT_SENTINEL) {
                g->map.tiles[y][x] = TILE_TEMPLE_FLOOR;
                if (g->enemy_count < AREA_ENEMY_LIMIT) {
                    spawn_enemy(g, &g->enemies[g->enemy_count++],
                        ENEMY_MOONBOUND_SENTINEL, x, y);
                }
            }
        }
    }
    game_update_level_progress(g);
    push_message(g, g->temple_alignment
        ? "Moon rises: lunar doors open and sentinels awaken!"
        : "Sun rises: lunar doors close and solar traps ignite!");
}

int game_interact_temple(GameState *g) {
    if (g->location != LOCATION_TEMPLE) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
            continue;
        }
        TileType tile = quest_tile(g, x, y);
        if (tile == TILE_TEMPLE_ALTAR) {
            toggle_temple_alignment(g);
            return 1;
        }
        if (tile == TILE_TEMPLE_TREASURE) {
            if (!(g->defeated_bosses & (1 << LOCATION_TEMPLE))) {
                push_message(g, "The Fallen Sun Guardian seals the treasure vault.");
                return 1;
            }
            if (g->temple_treasure_state < 2 && game_temple_remaining_enemies(g) > 0) {
                push_message(g, "Treasure sealed: clear all Sun and Moon enemies. Use an altar to awaken dormant sentinels.");
                return 1;
            }
            if (g->temple_treasure_state < 2) {
                g->temple_treasure_state = 2;
                g->map.tiles[y][x] = TILE_TEMPLE_RUBBLE;
                push_message(g, "The Buried Sun is recovered. Return it to Nahla.");
            }
            return 1;
        }
    }
    return 0;
}

void game_record_temple_enemy_defeated(GameState *g, EnemyType type) {
    if (g->location != LOCATION_TEMPLE ||
        type != ENEMY_FALLEN_SUN_GUARDIAN) {
        return;
    }
    g->defeated_bosses |= 1 << LOCATION_TEMPLE;
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            if (g->map.tiles[y][x] == TILE_TEMPLE_VAULT_DOOR) {
                g->map.tiles[y][x] = TILE_TEMPLE_FLOOR;
            }
        }
    }
    push_message(g, "The guardian falls. The buried vault opens!");
}

static void return_to_town(GameState *g, Location destination) {
    Location returning_from = g->location;
    giants_store(g);
    northroad_store(g);
    ironfang_store(g);
    stormwatch_store(g);
    mangrove_store(g);
    elderwood_store(g);
    if (returning_from == LOCATION_CROWNROAD_NORTH || returning_from == LOCATION_FROSTFELL || returning_from == LOCATION_DESERT ||
        returning_from == LOCATION_GLASSDEEP ||
        returning_from == LOCATION_CATACOMBS || returning_from == LOCATION_ELDERWOOD ||
        destination == LOCATION_TOWN4 ||
        destination == LOCATION_TOWN3) {
        clear_floor_loot(g);
    }
    LevelCache *cache = active_cache(g);
    // Cache current level before leaving
    if (returning_from == LOCATION_CROWNROAD || returning_from == LOCATION_KING_ROAD_WEST) {
        save_crownroad_cache(g);
    } else if ((returning_from == LOCATION_DUNGEON || cache != g->level_cache) &&
        g->level >= 1 &&
        g->level <= active_depth(g)) {
        cache[g->level - 1].map = g->map;
        cache[g->level - 1].enemy_count = g->enemy_count;
        cache[g->level - 1].level_cleared = g->level_cleared;
        for (int i = 0; i < g->enemy_count; i++) {
            cache[g->level - 1].enemies[i] = g->enemies[i];
        }
        cache[g->level - 1].valid = 1;
    }

    int spawn_x;
    int spawn_y;
    g->location = destination;
    if (destination == LOCATION_CASTLE) {
        map_generate_castle(&g->map, &spawn_x, &spawn_y);
    } else if (destination == LOCATION_DAWNMERE) {
        map_generate_dawnmere(&g->map, &spawn_x, &spawn_y);
    } else if (destination == LOCATION_TOWN4) {
        map_generate_town4(&g->map, &spawn_x, &spawn_y);
        map_set_ridgeshire_mountain_road(&g->map, g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
    } else if (destination == LOCATION_TOWN3) {
        map_generate_town3(&g->map, &spawn_x, &spawn_y);
        map_set_rosemoor_swamp_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
    } else if (destination == LOCATION_TOWN2) {
        map_generate_town2(&g->map, &spawn_x, &spawn_y);
        map_set_town3_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
        map_set_stillbury_forest_road(&g->map, g->defeated_bosses & (1 << LOCATION_FOREST));
    } else {
        map_generate_town(&g->map, &spawn_x, &spawn_y);
        map_set_town2_road(&g->map,
            g->defeated_bosses & (1 << LOCATION_FOREST));
        map_set_town4_road(&g->map,
            g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
    }
    if (destination == LOCATION_DAWNMERE) {
        g->player.x = returning_from == LOCATION_MOUNTAINS ? 20 : 1;
        g->player.y = returning_from == LOCATION_MOUNTAINS ? 1 : 12;
    } else if (destination == LOCATION_CASTLE) {
        g->player.x = CROWNROAD_X;
        g->player.y = TOWN_H - 2;
    } else if (destination == LOCATION_TOWN3) {
        g->player.x = returning_from == LOCATION_FROSTFELL ? 20 : spawn_x;
        g->player.y = returning_from == LOCATION_FROSTFELL ? 1 : spawn_y;
        if (returning_from == LOCATION_ASHEN) {
            g->player.x = 1;
            g->player.y = ROSEMOOR_ASHEN_GATE_Y;
        }
        if (returning_from == LOCATION_CATACOMBS) {
            g->player.x = TOWN_W - 2;
            g->player.y = TOWN3_KING_GATE_Y;
        }

    } else if (destination == LOCATION_TOWN4) {
        g->player.x = returning_from == LOCATION_MOUNTAINS ? spawn_x : TOWN_W - 2;
        g->player.y = returning_from == LOCATION_MOUNTAINS ? spawn_y : TOWN4_KING_GATE_Y;
        if (returning_from == LOCATION_DRAGONSPINE) {
            g->player.x = RIDGESHIRE_DRAGON_GATE_X;
            g->player.y = 1;
        }
    } else if (returning_from == LOCATION_CROWNROAD) {
        g->player.x = CROWNROAD_X;
        g->player.y = 1;
    } else if (returning_from == LOCATION_SWAMP) {
        g->player.x = 20;
        g->player.y = 1;
    } else if (returning_from == LOCATION_GLASSDEEP) {
        g->player.x = SOUTH_AREA_GATE_X;
        g->player.y = TOWN_H - 2;
    } else if (returning_from == LOCATION_DESERT) {
        g->player.x = 1;
        g->player.y = 12;
    } else if (returning_from == LOCATION_FOREST) {
        g->player.x = destination == LOCATION_TOWN2 ? TOWN_W - 2 : 1;
        g->player.y = 12;
    } else if (returning_from == LOCATION_DUNGEON) {
        g->player.x = TOWN_W - 2;
        g->player.y = 12;
    } else if (returning_from == LOCATION_COAST) {
        g->player.x = 20; g->player.y = TOWN_H - 2;
    } else if (returning_from == LOCATION_TEMPLE) {
        g->player.x = TOWN_HARBOR_X - 1;
        g->player.y = TOWN_HARBOR_ENTRANCE_Y;
    } else {
        g->player.x = 20; g->player.y = 1;
    }
    g->floor_item_count = 0;
    g->enemy_count = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    g->dialogue_active = 0;
    place_town_portal(g);
    game_recover_retired_items(g);
}

static void enter_return_approach(GameState *g, Location area) {
    for (int i = 0; i < town_approach_count; i++) {
        const TownApproach *route = &town_approaches[i];
        if (route->area != area || route->town != g->location) {
            continue;
        }
        if (approach_enter(g, route->gate_x, route->gate_y)) {
            int dx = g->player.last_dx;
            int dy = g->player.last_dy;
            g->player.x = g->map.stairs_down_x - dx;
            g->player.y = g->map.stairs_down_y - dy;
            g->player.last_dx = -dx;
            g->player.last_dy = -dy;
            if (area == LOCATION_DUNGEON || area == LOCATION_MOUNTAINS || area == LOCATION_FOREST || area == LOCATION_SWAMP || approach_entrance_rows(area)) {
                g->player.x = g->map.stairs_down_x;
                g->player.y = g->map.stairs_down_y + 1;
                g->player.last_dx = 0;
                g->player.last_dy = 1;
            }
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message), "You emerge onto the path to %s.", approach_town_name(route->town));
            push_message(g, message);
        }
        return;
    }
}

void game_leave_labyrinth(GameState *g) {
    save_labyrinth_floor(g);
    return_to_town(g, g->labyrinth_entry_town);
    enter_return_approach(g, LOCATION_FOREST);
    g->player.x = APPROACH_LABYRINTH_X;
    g->player.y = APPROACH_LABYRINTH_Y + 1;
    g->player.last_dx = 0;
    g->player.last_dy = 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "You emerge from Rook's labyrinth onto the forest road.");
}

void game_return_to_approach(GameState *g) {
    if (g->location == LOCATION_GIANTS_CROWN) {
        giants_leave(g);
        return;
    }
    if (g->location == LOCATION_CROWNROAD_NORTH) {
        northroad_leave(g, 0);
        return;
    }
    if (g->location == LOCATION_IRONFANG) {
        ironfang_leave(g);
        return;
    }
    if (g->location == LOCATION_STORMWATCH) {
        stormwatch_leave(g);
        return;
    }
    if (g->location == LOCATION_MANGROVES) {
        mangrove_leave(g);
        return;
    }
    if (g->location == LOCATION_ELDERWOOD) {
        elderwood_leave(g);
        return;
    }
    if (g->location == LOCATION_CATACOMBS) {
        game_leave_catacombs(g);
        return;
    }
    Location area = g->location;
    int lich_exit = area == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH &&
        (g->debug_passages_unlocked || (g->defeated_bosses & (1 << LOCATION_DUNGEON)));
    int mountain_exit = area == LOCATION_MOUNTAINS && g->level == MOUNTAIN_DEPTH &&
        (g->debug_passages_unlocked || (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)));
    int forest_exit = area == LOCATION_FOREST && g->level == FOREST_DEPTH &&
        (g->debug_passages_unlocked || (g->defeated_bosses & (1 << LOCATION_FOREST)));
    int swamp_exit = area == LOCATION_SWAMP && g->level == SWAMP_DEPTH &&
        (g->debug_passages_unlocked || (g->defeated_bosses & (1 << LOCATION_SWAMP)));
    game_return_to_town(g);
    enter_return_approach(g, area);
    if (swamp_exit && g->location == LOCATION_APPROACH) {
        g->player.x = g->map.stairs_up_x;
        g->player.y = g->map.stairs_down_y + 1;
        g->player.last_dx = -1;
        g->player.last_dy = 0;
        push_message(g, "The road is open: north to Rosemoor, south to Stillbury.");
    }
    if (forest_exit && g->location == LOCATION_APPROACH) {
        g->player.x = g->map.stairs_down_x;
        g->player.y = g->map.stairs_up_y;
        g->player.last_dx = 0;
        g->player.last_dy = 1;
        push_message(g, "The road is open: west to Stillbury, east to Oakhaven.");
    }
    if (mountain_exit && g->location == LOCATION_APPROACH) {
        g->player.x = g->map.stairs_up_x;
        g->player.y = g->map.stairs_down_y + 1;
        g->player.last_dx = -1;
        g->player.last_dy = 0;
        push_message(g, "The road is open: north to Ridgeshire, south to Dawnmere.");
    }
    if (lich_exit && g->location == LOCATION_APPROACH) {
        g->player.x = g->map.stairs_down_x;
        g->player.y = g->map.stairs_down_y + 2;
        g->player.last_dx = 0;
        g->player.last_dy = 1;
        push_message(g, "The road is open: west to Oakhaven, east to Dawnmere.");
    }
}

void game_leave_approach_to_town(GameState *g, Location town) {
    const TownApproach *route = approach_route(g);
    if (!route) {
        return;
    }
    if (northern_approach(g) && town == LOCATION_TOWN) {
        town_road_enter(g, LOCATION_TOWN, TOWN_SIDE_NORTH, 0);
        return;
    }
    TownApproach destination = *route;
    for (int i = 0; i < town_approach_count; i++) {
        if (town_approaches[i].town == town && town_approaches[i].area == route->area) {
            destination = town_approaches[i];
            break;
        }
    }
    if (town_road_enter(g, destination.town, town_road_side(destination.gate_x, destination.gate_y), 0)) {
        return;
    }
    game_enter_town_gate(g, destination.town, destination.gate_x, destination.gate_y);
}

void game_enter_town_gate(GameState *g, Location town, int x, int y) {
    return_to_town(g, town);
    if (town_road_index(town) >= 0) {
        g->last_town_visited = town;
    }
    g->approach_route = -1;
    g->player.x = x == 0 ? 1 : x == TOWN_W - 1 ? TOWN_W - 2 : x;
    g->player.y = y == 0 ? 1 : y == TOWN_H - 1 ? TOWN_H - 2 : y;
    g->player.last_dx = x == 0 ? 1 : x == TOWN_W - 1 ? -1 : 0;
    g->player.last_dy = y == 0 ? 1 : y == TOWN_H - 1 ? -1 : 0;
    push_message(g, "You enter town beside its exit gate.");
}

void game_leave_approach(GameState *g) {
    const TownApproach *route = approach_route(g);
    if (route) {
        game_leave_approach_to_town(g, route->town);
    }
}

void game_leave_catacombs(GameState *g) {
    Location town = g->catacombs_entry_town;
    return_to_town(g, town);
    northern_return(g, town, 1);
}

void game_return_to_town(GameState *g) {
    if (jail_blocks_portal(g)) {
        return;
    }
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        castle_leave(g, 1);
        return;
    }
    if (g->location == LOCATION_CROWNROAD || g->location == LOCATION_KING_ROAD_WEST) {
        game_leave_crownroad(g, g->location == LOCATION_CROWNROAD ? LOCATION_TOWN3 : LOCATION_TOWN4);
        return;
    }
    Location destination = LOCATION_TOWN;
    if (g->location == LOCATION_MANGROVES) {
        destination = LOCATION_DAWNMERE;
    } else if (g->location == LOCATION_GLASSDEEP) {
        destination = LOCATION_TOWN2;
    } else if (g->location == LOCATION_ELDERWOOD) {
        destination = g->elderwood_entry_town;
    } else if (g->location == LOCATION_CATACOMBS) {
        destination = g->catacombs_entry_town;
    } else if (g->location == LOCATION_DUNGEON) {
        destination = g->dungeon_entry_town;
    } else if (g->location == LOCATION_FOREST) {
        destination = g->forest_entry_town;
    } else if (g->location == LOCATION_SWAMP) {
        destination = g->swamp_entry_town;
    } else if (g->location == LOCATION_MOUNTAINS) {
        destination = g->mountain_entry_town;
    } else if (g->location == LOCATION_DESERT) {
        destination = LOCATION_TOWN2;
    } else if (g->location == LOCATION_ASHEN) {
        destination = LOCATION_CASTLE;
    } else if (g->location == LOCATION_FROSTFELL || g->location == LOCATION_GIANTS_CROWN) {
        destination = LOCATION_TOWN3;
    } else if (g->location == LOCATION_DRAGONSPINE || g->location == LOCATION_HIGH_PASS) {
        destination = LOCATION_TOWN4;
    }
    return_to_town(g, destination);
}

void game_leave_forest(GameState *g, Location town, int shortcut) {
    (void)shortcut;
    int from_forest = g->location == LOCATION_FOREST;
    return_to_town(g, town);
    g->player.x = town == LOCATION_TOWN2 ? TOWN_W - 2 : 1;
    g->player.y = 12;
    push_message(g, town == LOCATION_TOWN2 ? "You arrive in Stillbury." : "You arrive in OakHaven.");
    if (from_forest) {
        enter_return_approach(g, LOCATION_FOREST);
    }
}

void game_leave_mountains(GameState *g, Location town, int shortcut) {
    (void)shortcut;
    int from_mountains = g->location == LOCATION_MOUNTAINS;
    return_to_town(g, town);
    g->player.x = 20;
    g->player.y = town == LOCATION_TOWN4 ? TOWN_H - 2 : 1;
    push_message(g, town == LOCATION_TOWN4 ? "You arrive in Ridgeshire." : "You arrive in OakHaven.");
    if (from_mountains) {
        enter_return_approach(g, LOCATION_MOUNTAINS);
    }
}

void game_enter_town3(GameState *g) {
    g->last_town_visited = LOCATION_TOWN3;
    return_to_town(g, LOCATION_TOWN3);
}

void game_leave_swamp(GameState *g, Location town, int shortcut) {
    (void)shortcut;
    int from_swamp = g->location == LOCATION_SWAMP;
    return_to_town(g, town);
    g->player.x = 20;
    g->player.y = town == LOCATION_TOWN3 ? TOWN_H - 2 : 1;
    push_message(g, town == LOCATION_TOWN3 ? "You arrive in Rosemoor." : "You arrive in Stillbury.");
    if (from_swamp) {
        enter_return_approach(g, LOCATION_SWAMP);
    }
}

void game_enter_swamp_road(GameState *g) {
    int from_town2 = g->location == LOCATION_TOWN2;
    int from_boss = g->location == LOCATION_SWAMP;
    if (from_town2 || g->location == LOCATION_TOWN3) {
        g->swamp_entry_town = g->location;
    }
    g->location = LOCATION_SWAMP_ROAD;
    map_generate_swamp_road(&g->map);
    g->player.x = SWAMP_ROAD_X;
    g->player.y = from_town2 ? SWAMP_ROAD_H - 2 : 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    push_message(g, from_boss ? "The boss trail joins the safe path between Rosemoor and Stillbury." : from_town2 ? "The safe swamp trail leads north to Rosemoor." :
        "The safe swamp trail leads south to Stillbury.");
}

void game_leave_swamp_road(GameState *g, Location destination) {
    return_to_town(g, destination);
    g->player.x = 20;
    g->player.y = destination == LOCATION_TOWN3 ? TOWN_H - 2 : 1;
    push_message(g, destination == LOCATION_TOWN3 ?
        "You arrive in Rosemoor." : "You return to Stillbury.");
    approach_return_from_shortcut(g, LOCATION_SWAMP);
}

void game_enter_town4(GameState *g) {
    g->last_town_visited = LOCATION_TOWN4;
    return_to_town(g, LOCATION_TOWN4);
}

void game_leave_high_pass(GameState *g, Location destination) {
    return_to_town(g, destination);
    g->player.x = 20;
    g->player.y = destination == LOCATION_TOWN4 ? TOWN_H - 2 : 1;
    push_message(g, destination == LOCATION_TOWN4 ?
        "You arrive in Ridgeshire." : "You return to OakHaven.");
    approach_return_from_shortcut(g, LOCATION_MOUNTAINS);
}

void game_open_town_portal(GameState *g) {
    if (jail_blocks_portal(g)) {
        return;
    }
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        castle_request(g, 1);
        return;
    }
    if (g->location != LOCATION_DUNGEON &&
        g->location != LOCATION_FOREST &&
        g->location != LOCATION_MOUNTAINS &&
        g->location != LOCATION_COAST &&
        g->location != LOCATION_TEMPLE &&
        g->location != LOCATION_SWAMP &&
        g->location != LOCATION_FROSTFELL &&
        g->location != LOCATION_DESERT &&
        g->location != LOCATION_ASHEN &&
        g->location != LOCATION_GLASSDEEP &&
        g->location != LOCATION_CATACOMBS &&
        g->location != LOCATION_CROWNROAD_NORTH &&
        g->location != LOCATION_GIANTS_CROWN &&
        g->location != LOCATION_IRONFANG &&
        g->location != LOCATION_STORMWATCH &&
        g->location != LOCATION_MANGROVES &&
        g->location != LOCATION_ELDERWOOD &&
        g->location != LOCATION_DRAGONSPINE) {
        return;
    }
    game_hide_portal_destination(g);
    if (g->location == LOCATION_DESERT || g->location == LOCATION_ASHEN || g->location == LOCATION_GLASSDEEP ||
        g->location == LOCATION_CATACOMBS) {
        clear_floor_loot(g);
    }
    g->portal_active = 1;
    g->portal_level = g->level;
    g->portal_location = g->location;
    if (g->location == LOCATION_DUNGEON) {
        g->dungeon_portal_town = g->dungeon_entry_town;
    } else if (g->location == LOCATION_FOREST) {
        g->forest_portal_town = g->forest_entry_town;
    } else if (g->location == LOCATION_SWAMP) {
        g->swamp_portal_town = g->swamp_entry_town;
    } else if (g->location == LOCATION_MOUNTAINS) {
        g->mountain_portal_town = g->mountain_entry_town;
    }
    if (g->location == LOCATION_ELDERWOOD || g->location == LOCATION_CATACOMBS) {
        g->northern_portal_town = g->location == LOCATION_ELDERWOOD ? g->elderwood_entry_town : g->catacombs_entry_town;
    }
    g->portal_x = g->player.x;
    g->portal_y = g->player.y;
    g->portal_origin_tile = g->map.tiles[g->player.y][g->player.x];
    g->return_portal_town = town_road_index(g->last_town_visited) >= 0 ? g->last_town_visited : LOCATION_TOWN;
    return_to_town(g, g->return_portal_town);
    push_message(g, "A return portal remains open in the last town you visited.");
}

void game_hide_portal_destination(GameState *g) {
    if (g->portal_location == LOCATION_GIANTS_CROWN) {
        return;
    }
    if (g->portal_location == LOCATION_CROWNROAD_NORTH) {
        return;
    }
    if (g->portal_location == LOCATION_IRONFANG) {
        return;
    }
    if (g->portal_location == LOCATION_STORMWATCH) {
        return;
    }
    if (g->portal_location == LOCATION_MANGROVES) {
        return;
    }
    if (g->portal_location == LOCATION_ELDERWOOD) {
        elderwood_hide_portal(g);
        return;
    }
    if (g->portal_level < 1 || g->portal_level > MAX_REGION_DEPTH ||
        g->portal_x < 0 || g->portal_x >= MAP_W ||
        g->portal_y < 0 || g->portal_y >= MAP_H) {
        return;
    }
    LevelCache *cache = g->level_cache;
    if (g->portal_location == LOCATION_FOREST) {
        cache = g->forest_cache;
    } else if (g->portal_location == LOCATION_MOUNTAINS) {
        cache = g->mountain_cache;
    } else if (g->portal_location == LOCATION_COAST) {
        if (g->portal_level > COAST_DEPTH) {
            return;
        }
        cache = g->coast_cache;
    } else if (g->portal_location == LOCATION_DUNGEON) {
        if (g->portal_level > DUNGEON_DEPTH) {
            return;
        }
    } else if (g->portal_location == LOCATION_SWAMP) {
        cache = g->swamp_cache;
    } else if (g->portal_location == LOCATION_DRAGONSPINE) {
        cache = g->dragonspine_cache;
    } else if (g->portal_location == LOCATION_FROSTFELL) {
        cache = g->frostfell_cache;
    } else if (g->portal_location == LOCATION_DESERT) {
        if (g->portal_level > DESERT_DEPTH) {
            return;
        }
        cache = g->desert_cache;
    } else if (g->portal_location == LOCATION_ASHEN) {
        if (g->portal_level > ASHEN_DEPTH) {
            return;
        }
        cache = g->ashen_cache;
    } else if (g->portal_location == LOCATION_CATACOMBS) {
        if (g->portal_level > CATACOMBS_DEPTH) {
            return;
        }
        cache = g->catacombs_cache;
    } else if (g->portal_location == LOCATION_GLASSDEEP) {
        if (g->portal_level > GLASSDEEP_DEPTH) {
            return;
        }
        cache = g->glassdeep_cache;
    } else if (g->portal_location == LOCATION_TEMPLE) {
        if (g->portal_level > TEMPLE_DEPTH) {
            return;
        }
        cache = g->temple_cache;
    }
    if (cache[g->portal_level - 1].valid &&
        cache[g->portal_level - 1].map.tiles[g->portal_y][g->portal_x] ==
            TILE_PORTAL) {
        cache[g->portal_level - 1].map.tiles[g->portal_y][g->portal_x] =
            g->portal_origin_tile;
    }
    if (g->location == g->portal_location &&
        g->level == g->portal_level &&
        g->map.tiles[g->portal_y][g->portal_x] == TILE_PORTAL) {
        g->map.tiles[g->portal_y][g->portal_x] = g->portal_origin_tile;
    }
}

static int portal_landing_open(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x &&
            g->enemies[i].y == y) {
            return 0;
        }
    }
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int i = 0; i < 4; i++) {
        if (map_is_walkable(&g->map, x + dx[i], y + dy[i])) {
            return 1;
        }
    }
    return 0;
}

void game_use_town_portal(GameState *g) {
    if (jail_blocks_portal(g)) {
        return;
    }
    if (g->portal_location == LOCATION_CASTLE_INTERIOR) {
        g->portal_active = 0;
        push_message(g, "The castle has severed this portal.");
        return;
    }
    if (!g->portal_active || g->portal_level < 1 ||
        g->portal_level > MAX_REGION_DEPTH) return;
    if (g->portal_location == LOCATION_CROWNROAD_NORTH && g->portal_level == 1) {
        northroad_enter(g, 0, 1);
        return;
    }
    if (g->portal_location == LOCATION_GIANTS_CROWN && g->portal_level <= GIANTS_DEPTH) {
        giants_enter(g, 1);
        return;
    }
    if (g->portal_location == LOCATION_IRONFANG && g->portal_level <= IRON_DEPTH) {
        ironfang_enter(g, 1);
        return;
    }
    if (g->portal_location == LOCATION_STORMWATCH && g->portal_level <= STORM_DEPTH) {
        stormwatch_enter(g, 1);
        return;
    }
    if (g->portal_location == LOCATION_ELDERWOOD) {
        elderwood_portal(g);
        return;
    }
    if (g->portal_location == LOCATION_MANGROVES && g->portal_level >= 1 && g->portal_level <= MANGROVE_DEPTH) {
        mangrove_enter(g, 1);
        return;
    }
    if (g->portal_location == LOCATION_CATACOMBS) {
        g->catacombs_entry_town = g->northern_portal_town == LOCATION_TOWN ? LOCATION_TOWN : LOCATION_CASTLE;
    }
    int level = g->portal_level;
    LevelCache *cache = g->level_cache;
    if (g->portal_location == LOCATION_FOREST) {
        cache = g->forest_cache;
    } else if (g->portal_location == LOCATION_MOUNTAINS) {
        cache = g->mountain_cache;
    } else if (g->portal_location == LOCATION_COAST) {
        if (g->portal_level > COAST_DEPTH) {
            return;
        }
        cache = g->coast_cache;
    } else if (g->portal_location == LOCATION_DUNGEON) {
        if (g->portal_level > DUNGEON_DEPTH) {
            return;
        }
    } else if (g->portal_location == LOCATION_SWAMP) {
        cache = g->swamp_cache;
    } else if (g->portal_location == LOCATION_DRAGONSPINE) {
        cache = g->dragonspine_cache;
    } else if (g->portal_location == LOCATION_FROSTFELL) {
        cache = g->frostfell_cache;
    } else if (g->portal_location == LOCATION_DESERT) {
        if (level > DESERT_DEPTH) {
            return;
        }
        cache = g->desert_cache;
    } else if (g->portal_location == LOCATION_ASHEN) {
        if (g->portal_level > ASHEN_DEPTH) {
            return;
        }
        cache = g->ashen_cache;
    } else if (g->portal_location == LOCATION_CATACOMBS) {
        if (g->portal_level > CATACOMBS_DEPTH) {
            return;
        }
        cache = g->catacombs_cache;
    } else if (g->portal_location == LOCATION_GLASSDEEP) {
        if (g->portal_level > GLASSDEEP_DEPTH) {
            return;
        }
        cache = g->glassdeep_cache;
    } else if (g->portal_location == LOCATION_TEMPLE) {
        if (g->portal_level > TEMPLE_DEPTH) {
            return;
        }
        cache = g->temple_cache;
    }
    if (!cache[level - 1].valid) return;

    if (g->portal_location == LOCATION_FROSTFELL || g->portal_location == LOCATION_DESERT ||
        g->portal_location == LOCATION_ASHEN ||
        g->portal_location == LOCATION_GLASSDEEP || g->portal_location == LOCATION_CATACOMBS) {
        clear_floor_loot(g);
    }
    g->location = g->portal_location;
    if (g->location == LOCATION_DUNGEON) {
        g->dungeon_entry_town = g->dungeon_portal_town;
    } else if (g->location == LOCATION_FOREST) {
        g->forest_entry_town = g->forest_portal_town;
    } else if (g->location == LOCATION_SWAMP) {
        g->swamp_entry_town = g->swamp_portal_town;
    } else if (g->location == LOCATION_MOUNTAINS) {
        g->mountain_entry_town = g->mountain_portal_town;
    }
    g->level = level;
    g->map = cache[level - 1].map;
    g->enemy_count = cache[level - 1].enemy_count;
    g->level_cleared = cache[level - 1].level_cleared;
    for (int i = 0; i < g->enemy_count; i++)
        g->enemies[i] = cache[level - 1].enemies[i];
    game_refresh_quest_encounters(g);
    sync_temple_floor_state(g);
    int landing_x = g->portal_x;
    int landing_y = g->portal_y;
    if (landing_x >= 0 && landing_x < MAP_W &&
        landing_y >= 0 && landing_y < MAP_H &&
        g->map.tiles[landing_y][landing_x] == TILE_PORTAL) {
        g->map.tiles[landing_y][landing_x] = g->portal_origin_tile;
    }
    if (!portal_landing_open(g, landing_x, landing_y)) {
        landing_x = g->map.stairs_up_x;
        landing_y = g->map.stairs_up_y;
        if (!portal_landing_open(g, landing_x, landing_y)) {
            g->portal_active = 0;
            game_return_to_town(g);
            push_message(g, "No safe ground beyond the portal.");
            return;
        }
        push_message(g, "The portal returns you at the entrance.");
    } else {
        push_message(g, "Returned through the portal.");
    }
    g->player.x = landing_x;
    g->player.y = landing_y;
    desert_pharaoh_cancel_landing(g, landing_x, landing_y);

    cinder_lord_cancel_landing(g, landing_x, landing_y);
    regional_terrain_cancel_landing(g, landing_x, landing_y);
    prism_sovereign_cancel_landing(g, landing_x, landing_y);
    sun_guardian_cancel_landing(g, landing_x, landing_y);
    red_dragon_cancel_landing(g, landing_x, landing_y);
    if (g->location == LOCATION_CATACOMBS) {
        // Portal wards prevent a saved warning from striking immediately on arrival.
        for (int i = 0; i < g->map.burial_trap_count; i++) {
            BurialTrap *trap = &g->map.burial_traps[i];
            if (trap->timer > 0 && catacombs_trap_marks(&g->map, trap, landing_x, landing_y)) {
                trap->timer = 0;
                trap->spent = 1;
            }
        }
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (e->type == ENEMY_GRAVE_MARSHAL) {
                grave_marshal_cancel_landing(g, e, landing_x, landing_y);
            }
        }
    }
    cache[level - 1].map = g->map;
    g->portal_active = 0;
}

void game_talk_to_bram(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Bram");
    g->dialogue_x = TOWN_BRAM_X;
    g->dialogue_y = TOWN_BRAM_Y;
    const char *warning = (g->defeated_bosses & (1 << LOCATION_DUNGEON))
        ? "Oakhaven is safer now that the Lich King is defeated."
        : "The Lich King in the dungeon east of Oakhaven raises undead and endangers our town.";
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "%s The blacksmith sells weapons and armor; the alchemist sells potions and scrolls for your journey.", warning);
}

void game_talk_to_aldric(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Veteran Aldric");
    g->dialogue_x = TOWN_ALDRIC_X;
    g->dialogue_y = TOWN_ALDRIC_Y;
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "Clear the dungeon and Sunken Coast before harder regions. Revisit cleared areas to train. Rowan's docks are outside Oakhaven's south gate beside the Sunken Coast entrance.");
}

void game_talk_to_cain(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Cain");
    g->dialogue_x = TOWN_CAIN_X;
    g->dialogue_y = TOWN_CAIN_Y;
    const char *warning = "Ancient woods lie north, beasts west, undead east, and sea horrors south.";
    const char *recommendation = "Ask Elowen in Oakhaven's Tavern about The Broken Seals.";
    if (g->cain_scroll_given) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s Read your scroll to learn Return to Town. Use it when danger grows!", warning);
    } else if (g->inventory_count >= MAX_INVENTORY) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s %s Make room in your pack, then return for a Return to Town scroll.", warning, recommendation);
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s %s Take this Return to Town scroll; read it to learn a way home!", warning, recommendation);
        g->inventory[g->inventory_count++] = item_make_scroll_return_to_town();
        g->cain_scroll_given = 1;
        push_message(g, "Cain gives you a Scroll of Return to Town.");
    }
}

int game_has_treasure_map(const GameState *g) {
    for (int i = 0; i < g->inventory_count; i++) {
        if (g->inventory[i].type == ITEM_TREASURE_MAP) {
            return 1;
        }
    }
    return 0;
}

int game_can_sail_to_island(const GameState *g) {
    return g->debug_passages_unlocked || g->island_travel_unlocked || game_has_treasure_map(g);
}

static int island_interaction_tile(TileType tile) {
    return tile == TILE_ISLAND_CAMP || tile == TILE_ISLAND_MARKER ||
        tile == TILE_ISLAND_STATUE || tile == TILE_ISLAND_LAGOON ||
        tile == TILE_ISLAND_TEMPLE_GATE;
}

int game_has_island_interaction(const GameState *g) {
    if (g->location != LOCATION_ISLAND) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
            island_interaction_tile(g->map.tiles[y][x])) {
            return 1;
        }
    }
    return 0;
}

int game_interact_island(GameState *g) {
    if (g->location != LOCATION_ISLAND) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
            continue;
        }
        TileType tile = g->map.tiles[y][x];
        if (tile == TILE_ISLAND_CAMP) {
            push_message(g, "A waterlogged journal warns that the temple guardians still stir.");
            return 1;
        }
        if (tile == TILE_ISLAND_MARKER) {
            push_message(g, "The carved sun matches the symbol on your treasure map.");
            return 1;
        }
        if (tile == TILE_ISLAND_STATUE) {
            push_message(g, "The broken guardian faces the ruined temple.");
            return 1;
        }
        if (tile == TILE_ISLAND_LAGOON) {
            push_message(g, "Fresh water spills from beneath the ancient stonework.");
            return 1;
        }
        if (tile == TILE_ISLAND_TEMPLE_GATE) {
            push_message(g, "The treasure trail continues inside the ruined temple.");
            return 1;
        }
    }
    return 0;
}

static int temple_training_recommended(const GameState *g) {
    int training_bosses = (1 << LOCATION_DUNGEON) |
        (1 << LOCATION_MOUNTAINS);
    return g->temple_treasure_state < 2 &&
        (g->defeated_bosses & training_bosses) == 0;
}

void game_talk_to_nahla(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Nahla");
    g->dialogue_x = ISLAND_NAHLA_X;
    g->dialogue_y = ISLAND_NAHLA_Y;
    if (g->temple_treasure_state == 0) {
        if (temple_training_recommended(g)) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "Climb five tiers, defeat the Fallen Sun Guardian, and recover "
                "the Buried Sun. The temple is dangerous; train in the dungeon "
                "or mountains first if needed. You can return.");
        } else {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "The temple is a stepped pyramid. Climb its five tiers, defeat "
                "the Fallen Sun Guardian, and recover the Buried Sun from the summit vault.");
        }
        return;
    }
    if (g->temple_treasure_state == 1) {
        if (temple_training_recommended(g)) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "The Guardian waits at the summit. If the temple proves too "
                "hard, train in the dungeon or mountains and return. The sun "
                "and moon altars change the safe paths.");
        } else {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "The Guardian waits at the pyramid summit. The sun and moon altars "
                "change which passages are safe as you climb.");
        }
        push_message(g, "Quest active: The Buried Sun.");
        return;
    }
    if (g->temple_treasure_state == 2) {
        g->temple_treasure_state = 3;
        g->gold += 150;
        g->score += 2500;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You found it. The Buried Sun belongs to history again. "
            "Take this reward for surviving the pyramid.");
        push_message(g, "Quest complete: The Buried Sun. 150 gold awarded.");
        return;
    }
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "The summit is quiet now. The island will remember what you recovered.");
}

void game_talk_to_rowan(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Captain Rowan");
    g->dialogue_x = COAST_ROWAN_X;
    g->dialogue_y = COAST_ROWAN_Y;
    if (game_harbor_unlocked(g)) {
        const char *warning = temple_training_recommended(g)
            ? " The ruined temple is dangerous. Clear the dungeon or mountains "
              "for experience first, or sail now and turn back if needed."
            : "";
        if (g->island_travel_unlocked) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "The island route is charted. You may sail whenever you wish.%s",
                warning);
        } else if (game_has_treasure_map(g)) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "Keep the island map safe; it marks the ruined temple.%s",
                warning);
        } else if (g->inventory_count >= MAX_INVENTORY) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "I have an island map for you. Make room in your pack.%s",
                warning);
        } else {
            g->inventory[g->inventory_count++] = item_make_treasure_map();
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "Take this island map. It marks the ruined temple.%s",
                warning);
            push_message(g, "Rowan gives you an Island Treasure Map.");
        }
        return;
    }
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "The Drowned Queen keeps the coast too dangerous to sail. "
        "Defeat her in the Sunken Coast, then return to these docks beside its entrance. I can take you to the island beyond these shores.");
}

static void prepare_quest_expedition(GameState *g, Location location) {
    LevelCache *cache;
    int *max_level;
    int depth = MAX_REGION_DEPTH;
    if (location == LOCATION_DUNGEON) {
        cache = g->level_cache;
        max_level = &g->max_level_reached;
    } else if (location == LOCATION_FOREST) {
        cache = g->forest_cache;
        max_level = &g->max_forest_level_reached;
    } else if (location == LOCATION_MOUNTAINS) {
        cache = g->mountain_cache;
        max_level = &g->max_mountain_level_reached;
    } else if (location == LOCATION_SWAMP) {
        cache = g->swamp_cache;
        max_level = &g->max_swamp_level_reached;
        depth = SWAMP_DEPTH;
    } else if (location == LOCATION_DRAGONSPINE) {
        cache = g->dragonspine_cache;
        max_level = &g->max_dragonspine_level_reached;
        depth = DRAGONSPINE_DEPTH;
    } else {
        cache = g->coast_cache;
        max_level = &g->max_coast_level_reached;
    }
    for (int i = 0; i < depth; i++) {
        cache[i].valid = 0;
        cache[i].level_cleared = 0;
    }
    *max_level = 1;
    if (g->portal_active && g->portal_location == location) {
        g->portal_active = 0;
    }
}

typedef struct {
    const char *speaker;
    Location location;
    size_t state_offset;
    int offered_state;
    const char *message;
} QuestOffer;

static const QuestOffer quest_offers[] = {
    {WAR_SPEAKER, LOCATION_TOWN_HALL, offsetof(GameState, war_machine_quest_state), 0,
        "Assigned: Break the War Machine. Read Ironfang 2's orders, sink Ironfang 4's weapons, and bring back the Mine Boss's seal."},
    {"Watchkeeper Seren", LOCATION_DAWNMERE, offsetof(GameState, stormbell_quest_state), 0,
        "Assigned: The Silent Stormbell. Recover the clapper in Stormwatch 3 and restore the bell in Stormwatch 4."},
    {"Spice Merchant Corvin", LOCATION_DAWNMERE_MERCHANT, offsetof(GameState, spice_skiff_quest_state), 0,
        "Assigned: The Missing Spice Skiff. Search the stranded skiff on Saltwind 2 and recover its cargo on Saltwind 3."},
    {"Forester Tamsin", LOCATION_TOWN, offsetof(GameState, heartseed_quest_state), 0,
        "Assigned: The Stolen Heartseed. Find Dark Elf orders in Elderwood 2 and the bound grove in Elderwood 4."},
    {"Huntmaster Selene", LOCATION_GUILD, offsetof(GameState, hunt_quest_state), 0, "Assigned: The Three Great Hunts. See your quest journal."},
    {"Marshal Veyra", LOCATION_TOWN_HALL, offsetof(GameState, watchfire_quest_state), 0, "Assigned: Watchfires of Ridgeshire. See your quest journal."},
    {"Elowen", LOCATION_TAVERN, offsetof(GameState, elowen_quest_state), 0, "Quest assigned: The Broken Seals."},
    {"Dain", LOCATION_TOWN4, offsetof(GameState, dain_quest_state), 0, "Assigned: Recover the Treasure Map."},
    {"Alder", LOCATION_INN, offsetof(GameState, alder_quest_state), 0, "Assigned: The Lost Wardens."},
    {"Mara", LOCATION_TAVERN, offsetof(GameState, mara_quest_state), 0, "Assigned: Relight the Drowned Beacons."},
    {"Rook", LOCATION_TAVERN, offsetof(GameState, rook_quest_state), 0, "Assigned: The Ivory Rook."},
    {"Bram", LOCATION_INN, offsetof(GameState, innkeeper_quest_state), 0, "Assigned: Bring Mira Home."},
    {"Zara", LOCATION_INN, offsetof(GameState, sunscar_lamp_quest_state), 0, "Assigned: The Lost Magic Lamp."},
    {"Ilya", LOCATION_TAVERN, offsetof(GameState, dragon_treasure_quest_state), 0, "Assigned: The Dragon's Hoard."},
    {"Nahla", LOCATION_ISLAND, offsetof(GameState, temple_treasure_state), 0, "Quest assigned: The Buried Sun."},
    {"Steward Hadrin", LOCATION_TOWN_HALL, offsetof(GameState, emberforge_quest_state), 0, "Assigned: Reclaim the Emberforge. See your quest journal."},
    {"Quartermaster Brenna", LOCATION_INN, offsetof(GameState, frostfell_quest_state), 0,
        "Assigned: The Silent Expedition. Recover the journal on Frostfell stage 2 and rescue the surveyor on stage 4."},
    {"Surveyor Orin", LOCATION_GUILD, offsetof(GameState, glassdeep_quest_state), 0,
        "Assigned: Reclaim the Quarry. Repair machinery on Stonewake stages 2, 3 and 4."},
    {"Brother Oswin", LOCATION_TOWN_HALL, offsetof(GameState, catacombs_quest_state), 0,
        "Assigned: Rest for the Forgotten. See your quest journal."},
    {"Tomas", LOCATION_JAIL, offsetof(GameState, jail_quest_state), 1,
        "Assigned: Guide Tomas Home. Tomas reveals a loose flagstone."}
};

// Existing saved dialogue and quest state also preserve an unanswered offer.
static const QuestOffer *current_quest_offer(const GameState *g) {
    if (!g->dialogue_active) {
        return NULL;
    }
    for (size_t i = 0; i < sizeof(quest_offers) / sizeof(quest_offers[0]); i++) {
        const QuestOffer *offer = &quest_offers[i];
        const int *state = (const int *)((const char *)g + offer->state_offset);
        if (g->location == offer->location && *state == offer->offered_state &&
            strcmp(g->dialogue_speaker, offer->speaker) == 0) {
            return offer;
        }
    }
    return NULL;
}

int game_quest_offer_active(const GameState *g) {
    return current_quest_offer(g) != NULL;
}

int game_handle_quest_offer_key(GameState *g, int key, int repeat) {
    const QuestOffer *offer = current_quest_offer(g);
    if (!offer) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (key == SDL_SCANCODE_N || key == SDL_SCANCODE_ESCAPE) {
        g->dialogue_active = 0;
        push_message(g, "Quest declined. Speak to the NPC again if you change your mind.");
        return 1;
    }
    if (key != SDL_SCANCODE_Y) {
        return 1;
    }
    int *state = (int *)((char *)g + offer->state_offset);
    *state = offer->offered_state + 1;
    if (state == &g->war_machine_quest_state) {
        war_machine_accept(g);
    } else if (state == &g->stormbell_quest_state) {
        stormbell_accept(g);
    } else if (state == &g->spice_skiff_quest_state) {
        spice_skiff_accept(g);
    } else if (state == &g->heartseed_quest_state) {
        heartseed_accept(g);
    } else if (state == &g->elowen_quest_state) {
        g->elowen_seals_restored = 0;
        prepare_quest_expedition(g, LOCATION_DUNGEON);
    } else if (state == &g->dain_quest_state) {
        g->dain_map_fragments = 0;
        prepare_quest_expedition(g, LOCATION_MOUNTAINS);
    } else if (state == &g->alder_quest_state) {
        g->alder_wardens_rescued = 0;
        prepare_quest_expedition(g, LOCATION_FOREST);
    } else if (state == &g->mara_quest_state) {
        g->mara_beacons_lit = 0;
        prepare_quest_expedition(g, LOCATION_COAST);
    } else if (state == &g->innkeeper_quest_state) {
        prepare_quest_expedition(g, LOCATION_SWAMP);
    } else if (state == &g->dragon_treasure_quest_state) {
        prepare_quest_expedition(g, LOCATION_DRAGONSPINE);
    } else if (state == &g->rook_quest_state) {
        g->rook_labyrinth_switches = 0;
    } else if (state == &g->catacombs_quest_state) {
        catacombs_accept_quest(g);
    } else if (state == &g->jail_quest_state) {
        g->map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] = TILE_JAIL_HATCH;
    }
    push_message(g, offer->message);
    if (state == &g->rook_quest_state) {
        push_message(g, "The labyrinth across from the witch's hut is now open.");
    }
    return 1;
}

// The Royal Guards warn travelers but never block the Crown Roads.
void game_talk_to_royal_guard(GameState *g, int x, int y) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Royal Guard");
    g->dialogue_x = x;
    g->dialogue_y = y;
    if (g->location == LOCATION_TOWN4) {
        if (y < TOWN4_KING_GATE_Y) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "Traveler, Crown Road West is dangerous. Bandits, archers and "
                "horsemen roam the route. Stay alert, but we will not stop you.");
        } else {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "Bandits, archers and horsemen prowl the road ahead. Keep "
                "your wits about you; we will let you pass, but cannot "
                "protect you out there.");
        }
    } else if (y < TOWN3_KING_GATE_Y) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Halt, traveler. Beyond this gate Crown Road East swarms with "
            "bandits, archers and horsemen. Few who walk it return.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "We will not stop you, but grow stronger first. Brave Blackwater "
            "Swamp, the labyrinth and the lands near OakHaven before "
            "Crown Road East.");
    }
    push_message(g, "The Royal Guards recommend exploring other areas first.");
}

void game_talk_to_guild_seeker(GameState *g) {
    if (g->location != LOCATION_INN || abs(g->player.x - ZARA_INN_X) > 1 || abs(g->player.y - ZARA_INN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Zara");
    g->dialogue_x = ZARA_INN_X;
    g->dialogue_y = ZARA_INN_Y;
    if (g->sunscar_lamp_quest_state == 0) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Recover a magic lamp from Sunscar Wastes level 4. Enter through "
            "Stillbury's west gate. Stand on the lamp and press P, then return "
            "to me here for 80 gold.");
    } else if (g->sunscar_lamp_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The magic lamp lies in the last clearing of Sunscar Wastes level 4. "
            "Stand on it and press P, then return it to me at the Inn in Stillbury.");
    } else if (g->sunscar_lamp_quest_state == 2) {
        g->sunscar_lamp_quest_state = 3;
        g->gold += 80;
        g->score += 600;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You found the lamp! I will keep it safe. Here are your 80 gold.");
        push_message(g, "Completed: The Lost Magic Lamp. 80 gold awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The magic lamp is safe with me. Thank you for recovering it.");
    }
}

void game_collect_desert_lamp(GameState *g) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_LAMP_LEVEL ||
        g->sunscar_lamp_quest_state != 1 ||
        g->map.tiles[g->player.y][g->player.x] != TILE_DESERT_LAMP) {
        return;
    }
    g->map.tiles[g->player.y][g->player.x] = TILE_DESERT_FLOOR;
    g->sunscar_lamp_quest_state = 2;
    push_message(g, "Magic lamp recovered. Return to Zara in Stillbury's Inn.");
}

void game_talk_to_dragon_seeker(GameState *g) {
    if (g->location != LOCATION_TAVERN || abs(g->player.x - ILYA_TAVERN_X) > 1 || abs(g->player.y - ILYA_TAVERN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Ilya");
    g->dialogue_x = ILYA_TAVERN_X;
    g->dialogue_y = ILYA_TAVERN_Y;
    if (g->dragon_treasure_quest_state == 0) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Travel east through the dungeon road to Dawnmere, then north through the mountains to Ridgeshire and Dragonspine. "
            "Bring its fifth-stage golden goblet back here to Oakhaven's Tavern for a Potion of Strength.");
    } else if (g->dragon_treasure_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Find the golden goblet in the dragon's hoard on Dragonspine's "
            "fifth stage. Stand on it and press A, then return to me in Oakhaven's Tavern.");
    } else if (g->dragon_treasure_quest_state == 2) {
        if (g->inventory_count >= MAX_INVENTORY) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "You found the goblet! Make room in your pack for the "
                "Potion of Strength, then speak with me again.");
            return;
        }
        g->inventory[g->inventory_count++] = item_make_strength_potion();
        g->dragon_treasure_quest_state = 3;
        g->score += 600;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The goblet is safe. This Potion of Strength permanently raises "
            "your attack when you drink it. Thank you.");
        push_message(g, "Completed: The Dragon's Hoard. Potion of Strength awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Dragonspine's treasure is safe. Thank you for returning to Oakhaven with the goblet.");
    }
}

void game_collect_dragon_treasure(GameState *g) {
    if (g->location != LOCATION_DRAGONSPINE ||
        g->level != DRAGONSPINE_DEPTH || g->dragon_treasure_quest_state != 1 ||
        g->map.tiles[g->player.y][g->player.x] != TILE_DRAGON_TREASURE) {
        return;
    }
    g->map.tiles[g->player.y][g->player.x] = TILE_DRAGON_HOARD;
    g->dragon_treasure_quest_state = 2;
    push_message(g, "Golden goblet recovered. Return to Ilya in Oakhaven's Tavern.");
}

void game_talk_to_elowen(GameState *g) {
    if (g->location != LOCATION_TAVERN || abs(g->player.x - ELOWEN_TAVERN_X) > 1 || abs(g->player.y - ELOWEN_TAVERN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Elowen", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = ELOWEN_TAVERN_X;
    g->dialogue_y = ELOWEN_TAVERN_Y;
    if (g->elowen_quest_state == 0) {
        strncpy(g->dialogue_text,
            "The dead gather around shattered burial seals on floors 2, 3, and 4 of Oakhaven's dungeon. Restore each seal, then return to me here in Oakhaven's Tavern.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        return;
    }
    if (g->elowen_quest_state == 1) {
        int restored = 0;
        for (int i = 0; i < 3; i++) {
            if (g->elowen_seals_restored & (1 << i)) {
                restored++;
            }
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You have restored %d of 3 burial seals in Oakhaven's dungeon. Return to me in Oakhaven's Tavern when all three are whole.",
            restored);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status), "Quest progress: %d/3 seals.",
            restored);
        push_message(g, status);
        return;
    }
    if (g->elowen_quest_state == 2) {
        g->elowen_quest_state = 3;
        g->gold += 40;
        g->score += 300;
        strncpy(g->dialogue_text,
            "The crypt is bound once more. Its dead may finally sleep. Take this gold with my gratitude.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Quest complete: The Broken Seals.");
        return;
    }
    strncpy(g->dialogue_text, "You have my gratitude, adventurer.",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    push_message(g, "Elowen's quest is already complete.");
}

void game_talk_to_dain(GameState *g) {
    if (g->location != LOCATION_TOWN4 || abs(g->player.x - DAIN_TOWN_X) > 1 || abs(g->player.y - DAIN_TOWN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Dain", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = DAIN_TOWN_X;
    g->dialogue_y = DAIN_TOWN_Y;
    if (g->dain_quest_state == 0) {
        strncpy(g->dialogue_text,
            "Enter Ridgeshire's south mountain gate. Map Bearers hold fragments on stages 2, 3, and 4 before the Goblin King. Return to Ridgeshire's town center.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        return;
    }
    if (g->dain_quest_state == 1) {
        int defeated = 0;
        for (int bit = 0; bit < 3; bit++) {
            if (g->dain_map_fragments & (1 << bit)) {
                defeated++;
            }
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You have recovered %d of 3 fragments. Search mountain stages 2, 3, and 4. After the Goblin King, exit to the road north to Ridgeshire or south to Dawnmere. Return here to Ridgeshire's town center.",
            defeated);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status),
            "Quest progress: %d/3 map fragments.", defeated);
        push_message(g, status);
        return;
    }
    if (g->dain_quest_state == 2) {
        g->dain_quest_state = 3;
        g->gold += 60;
        g->score += 400;
        strncpy(g->dialogue_text,
            "The treasure map is whole again. It reveals a dwarven hoard the goblins never learned how to find.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Completed: Recover the Treasure Map.");
        return;
    }
    strncpy(g->dialogue_text,
        "The restored map still points toward riches hidden beneath the mountains.",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    push_message(g, "Dain's quest is already complete.");
}

void game_record_dain_kill(GameState *g, EnemyType type) {
    if (g->location != LOCATION_MOUNTAINS || g->dain_quest_state != 1) {
        return;
    }
    int target = 0;
    const char *status = NULL;
    if (type == ENEMY_GOBLIN_ARCHER) {
        target = DAIN_FRAGMENT_ARCHER;
        status = "Map fragment recovered from Archer.";
    } else if (type == ENEMY_GOBLIN_BOMBER) {
        target = DAIN_FRAGMENT_BOMBER;
        status = "Map fragment recovered from Bomber.";
    } else if (type == ENEMY_GOBLIN_SHAMAN) {
        target = DAIN_FRAGMENT_SHAMAN;
        status = "Map fragment recovered from Shaman.";
    }
    if (!target || (g->dain_map_fragments & target)) {
        return;
    }
    g->dain_map_fragments |= target;
    push_message(g, status);
    if ((g->dain_map_fragments & 7) == 7) {
        g->dain_quest_state = 2;
        push_message(g, "Treasure map complete. Return to Dain in Ridgeshire's town center.");
    }
}

void game_talk_to_alder(GameState *g) {
    if (g->location != LOCATION_INN || abs(g->player.x - ALDER_INN_X) > 1 || abs(g->player.y - ALDER_INN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Alder", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = ALDER_INN_X;
    g->dialogue_y = ALDER_INN_Y;
    if (g->alder_quest_state == 0) {
        strncpy(g->dialogue_text,
            "Enter the forest through Stillbury's east gate. Three wardens are trapped on levels 2, 3, and 4 before the Necromancer's grove. Rescue them, then return here to Stillbury's Inn.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        return;
    }
    if (g->alder_quest_state == 1) {
        int rescued = 0;
        for (int bit = 0; bit < 3; bit++) {
            if (g->alder_wardens_rescued & (1 << bit)) {
                rescued++;
            }
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You have rescued %d of my 3 wardens. Search forest levels 2, 3, and 4 from Stillbury. After the Necromancer, exit onto the road west to Stillbury or east to Oakhaven.",
            rescued);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status), "Quest progress: %d/3 wardens.",
            rescued);
        push_message(g, status);
        return;
    }
    if (g->alder_quest_state == 2) {
        g->alder_quest_state = 3;
        g->gold += 70;
        g->score += 500;
        strncpy(g->dialogue_text,
            "All three returned safely. The forest has taken enough from us. Accept this reward with an old ranger's gratitude.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Completed: The Lost Wardens.");
        return;
    }
    strncpy(g->dialogue_text,
        "My wardens are home, and the paths feel less lonely for it.",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    push_message(g, "Alder's quest is already complete.");
}

void game_rescue_forest_warden(GameState *g, int x, int y) {
    if (g->location != LOCATION_FOREST || g->alder_quest_state != 1 ||
        x < 0 || x >= MAP_W || y < 0 || y >= MAP_H ||
        g->map.tiles[y][x] != TILE_FOREST_WARDEN) {
        return;
    }
    int bit = alder_warden_bit(g->level);
    if (!bit || (g->alder_wardens_rescued & bit)) {
        return;
    }
    g->alder_wardens_rescued |= bit;
    g->map.tiles[y][x] = TILE_FOREST_FLOOR;
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Forest Warden", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = x;
    g->dialogue_y = y;
    if (g->level == 2) {
        strncpy(g->dialogue_text,
            "You cut through the spider web binding me. I can follow your trail home from here.",
            MAX_DIALOGUE_LEN - 1);
    } else if (g->level == 3) {
        strncpy(g->dialogue_text,
            "The Dark Elves thought this grove would be my prison. I will make my way back to Alder.",
            MAX_DIALOGUE_LEN - 1);
    } else {
        strncpy(g->dialogue_text,
            "Those cursed roots were pulling me beneath the earth. You reached me just in time.",
            MAX_DIALOGUE_LEN - 1);
    }
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    int rescued = 0;
    for (int index = 0; index < 3; index++) {
        if (g->alder_wardens_rescued & (1 << index)) {
            rescued++;
        }
    }
    char status[MAX_MESSAGE_LEN];
    snprintf(status, sizeof(status), "Warden rescued: %d/3.", rescued);
    push_message(g, status);
    if ((g->alder_wardens_rescued & 7) == 7) {
        g->alder_quest_state = 2;
        push_message(g, "All wardens rescued. Return to Alder in Stillbury's Inn.");
    }
}

void game_talk_to_mara(GameState *g) {
    if (g->location != LOCATION_TAVERN ||
        abs(g->player.x - MARA_TAVERN_X) > 1 || abs(g->player.y - MARA_TAVERN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Mara", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = MARA_TAVERN_X;
    g->dialogue_y = MARA_TAVERN_Y;
    if (g->mara_quest_state == 0) {
        strncpy(g->dialogue_text,
            "Sunken Coast is south of Oakhaven. Relight beacons on stages 2, 3, and 4; lower tides and defeat their guardians. Rowan's docks stand beside the entrance. Return to Oakhaven's Tavern.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        return;
    }
    if (g->mara_quest_state == 1) {
        int lit = 0;
        for (int bit = 0; bit < 3; bit++) {
            if (g->mara_beacons_lit & (1 << bit)) {
                lit++;
            }
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You have relit %d of 3 beacons. The remaining lights wait on coast stages 2, 3, and 4.",
            lit);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status), "Quest progress: %d/3 beacons.", lit);
        push_message(g, status);
        return;
    }
    if (g->mara_quest_state == 2) {
        g->mara_quest_state = 3;
        g->gold += 80;
        g->score += 600;
        strncpy(g->dialogue_text,
            "All three flames shine across the drowned road. Sailors can find safe water again. Take this with my thanks.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Completed: Relight the Drowned Beacons.");
        return;
    }
    strncpy(g->dialogue_text,
        "The beacons still burn. Even this ruined coast can guide travelers home.",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    push_message(g, "Mara's quest is already complete.");
}

void game_talk_to_rook(GameState *g) {
    if (g->location != LOCATION_TAVERN || abs(g->player.x - ROOK_TAVERN_X) > 1 || abs(g->player.y - ROOK_TAVERN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Rook");
    g->dialogue_x = ROOK_TAVERN_X;
    g->dialogue_y = ROOK_TAVERN_Y;
    if (g->rook_quest_state == 0) {
        offer_rook_quest(g);
    } else if (g->rook_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Find the labyrinth beside the forest road. One rune per floor opens the "
            "Minotaur's vault. Defeat it, recover my ivory rook, and return to Oakhaven's Tavern.");
        push_message(g, "Rook is waiting for the ivory rook.");
    } else if (g->rook_quest_state == 2) {
        g->gold += ROOK_QUEST_REWARD;
        g->score += 500;
        g->rook_quest_state = 3;
        g->rook_quest_completions++;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "My ivory rook! Thank you for bringing it back. Please take %d gold "
            "for your trouble.", ROOK_QUEST_REWARD);
        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Completed: The Ivory Rook. %d gold awarded.",
            ROOK_QUEST_REWARD);
        push_message(g, msg);
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Thank you for recovering my ivory rook.");
        push_message(g, "Rook's quest is already complete.");
    }
}

void game_talk_to_innkeeper(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Bram");
    g->dialogue_x = 28;
    g->dialogue_y = 7;
    if (g->innkeeper_quest_state == 0) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "A vampire holds Mira on swamp level 3, between Stillbury and the Demon. "
            "Defeat her captor, then speak to her and bring her home. You can retreat and return if needed.");
    } else if (g->innkeeper_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Mira is on swamp level 3, on the Stillbury side of the Demon. Defeat the vampire holding her, "
            "then speak to her before returning to the inn.");
        push_message(g, "Bram is waiting for Mira.");
    } else if (g->innkeeper_quest_state == 2) {
        g->innkeeper_quest_state = 3;
        g->gold += 80;
        g->score += 600;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Mira made it home. I cannot thank you enough. Please take this "
            "reward for bringing my daughter back to me.");
        push_message(g, "Completed: Bring Mira Home. 80 gold awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Mira is safe upstairs. There is always a room for you here.");
        push_message(g, "Bram thanks you for rescuing Mira.");
    }
}

void game_rescue_innkeeper_daughter(GameState *g, int x, int y) {
    if (g->location != LOCATION_SWAMP || g->level != SWAMP_RESCUE_LEVEL ||
        g->innkeeper_quest_state != 1 || x < 0 || x >= MAP_W ||
        y < 0 || y >= MAP_H || g->map.tiles[y][x] != TILE_SWAMP_DAUGHTER) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active &&
            strcmp(g->enemies[i].name, "Vampire Captor") == 0) {
            push_message(g, "Mira cannot escape while the vampire lives.");
            return;
        }
    }
    g->innkeeper_quest_state = 2;
    g->map.tiles[y][x] = TILE_SWAMP_FLOOR;
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Mira");
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "You broke the vampire's hold. I know the way back to the inn. "
        "Please tell my father I am safe.");
    push_message(g, "Mira rescued. Return to Bram at the inn.");
}

void game_light_coast_beacon(GameState *g, int x, int y) {
    if (g->location != LOCATION_COAST || x < 0 || x >= MAP_W || y < 0 ||
        y >= MAP_H || g->map.tiles[y][x] != TILE_COAST_BEACON_UNLIT) {
        return;
    }
    if (g->mara_quest_state != 1) {
        push_message(g, "An ancient drowned beacon stands here.");
        return;
    }
    for (int map_y = 0; map_y < MAP_H; map_y++) {
        for (int map_x = 0; map_x < MAP_W; map_x++) {
            if (g->map.tiles[map_y][map_x] == TILE_COAST_DEEP_WATER) {
                push_message(g, "Lower the tide before lighting it.");
                return;
            }
        }
    }
    int bit = mara_beacon_bit(g->level);
    if (!bit || (g->mara_beacons_lit & bit)) {
        return;
    }
    g->mara_beacons_lit |= bit;
    g->map.tiles[y][x] = TILE_COAST_BEACON_LIT;
    int lit = 0;
    for (int index = 0; index < 3; index++) {
        if (g->mara_beacons_lit & (1 << index)) {
            lit++;
        }
    }
    char status[MAX_MESSAGE_LEN];
    snprintf(status, sizeof(status), "Beacon lit: %d/3.", lit);
    push_message(g, status);
    if ((g->mara_beacons_lit & 7) == 7) {
        g->mara_quest_state = 2;
        push_message(g, "All beacons lit. Return to Mara in Oakhaven's Tavern.");
    }
}

void game_mark_level_cleared(GameState *g) {
    g->level_cleared = 1;
}

void game_enter_shortcut_boss(GameState *g) {
    (void)g;
}

void game_request_shortcut(GameState *g) {
    (void)g;
}

int game_shortcut_prompt_active(const GameState *g) {
    return g->castle_prompt != 0;
}

int game_handle_shortcut_prompt_key(GameState *g, int key, int repeat) {
    return castle_prompt_key(g, key, repeat);
}

// Keep the legacy API and tile ID for saves; forest victory opens its normal exit.
void game_reveal_forest_shortcut(GameState *g) {
    if (g->location == LOCATION_FOREST && g->level == FOREST_BOSS_LEVEL &&
        (g->defeated_bosses & (1 << LOCATION_FOREST))) {
        map_reveal_forest_entrance(&g->map);
        map_reveal_forest_exit(&g->map);
    }
}

void game_reveal_swamp_shortcut(GameState *g) {
    (void)g;
}

// Retained for callers loading legacy maps; mountains no longer have a shortcut.
void game_reveal_mountain_shortcut(GameState *g) {
    (void)g;
}

void game_migrate_boss_shortcuts(GameState *g) {
    (void)g;
}

void game_update_level_progress(GameState *g) {
    refresh_hunt_progress(g);
    if (g->location < LOCATION_JAIL && (g->defeated_bosses & (1 << g->location))) {
        if (g->location == LOCATION_FOREST && g->level == FOREST_BOSS_LEVEL) {
            game_reveal_forest_shortcut(g);
        } else if (g->location == LOCATION_SWAMP && g->level == SWAMP_BOSS_LEVEL) {
            game_reveal_swamp_shortcut(g);
        } else if (g->location == LOCATION_MOUNTAINS && g->level == MOUNTAIN_BOSS_LEVEL) {
            game_reveal_mountain_shortcut(g);
        } else if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH &&
            quest_object_tile(g, g->map.stairs_down_x, g->map.stairs_down_y) != TILE_DUNGEON_STAIRS_RETURN) {
            set_quest_object_tile(g, g->map.stairs_down_x, g->map.stairs_down_y, TILE_DUNGEON_STAIRS_RETURN);
            if (g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] != TILE_PORTAL) {
                g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_DUNGEON_STAIRS_RETURN;
            }
            push_message(g, "The sealed stairs open! Ascend them to return to Oakhaven.");
        }
    }

    int active_enemies = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active || (g->location == LOCATION_CATACOMBS && g->enemies[i].revive_timer > 0)) {
            active_enemies++;
        }
    }

    if (g->location == LOCATION_TEMPLE) {
        active_enemies = game_temple_remaining_enemies(g);
    }

    if (active_enemies == 0) {
        game_mark_level_cleared(g);
    } else if (g->location == LOCATION_CATACOMBS || g->location == LOCATION_TEMPLE) {
        g->level_cleared = 0;
    }
}

// Mages need MP to fight, and each level takes more kills than the last, so
// their pool grows most while spells and gear are weak: +15 up to level 5,
// +10 up to level 10, then +5. Other classes only use MP for utility.
static int level_up_mp_gain(const Player *p) {
    if (p->player_class != CLASS_MAGE) {
        return 0;
    }
    if (p->level <= 5) {
        return 15;
    }
    if (p->level <= 10) {
        return 10;
    }
    return 5;
}

void player_gain_xp(GameState *g, int xp) {
    g->player.experience += xp;

    while (g->player.experience >= g->player.experience_next &&
           g->player.level < 50) {
        g->player.experience    -= g->player.experience_next;
        g->player.level++;
        g->player.max_hp        += 10;
        g->player.max_mp += level_up_mp_gain(&g->player);
        g->player.hp             = g->player.max_hp;
        g->player.mp             = g->player.max_mp;
        g->player.attack        += 2;

        // Defense grows at half rate, capped at 50% of attack
        int new_defense = g->player.defense + 1;
        int defense_cap = g->player.attack / 2;
        g->player.defense = new_defense > defense_cap ? defense_cap : new_defense;

        g->player.experience_next = g->player.level * 100;

        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Level up! Now level %d", g->player.level);
        push_message(g, msg);
    }
}
