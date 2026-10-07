#include "game.h"
#include "catacombs.h"
#include "castle.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
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
    switch (type) {
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
        case ENEMY_GRAVE_ARCHER:
            snprintf(e->name, sizeof(e->name), "Grave Archer");
            e->max_hp = 40; e->hp = 40;
            e->attack = 15; e->defense = 3; e->experience = 55;
            break;
        case ENEMY_BONE_CANTOR:
            snprintf(e->name, sizeof(e->name), "Bone Cantor");
            e->max_hp = 58; e->hp = 58;
            e->attack = 16; e->defense = 4; e->experience = 90;
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
        case ENEMY_GOBLIN:
            strncpy(e->name, "Goblin", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 15; e->hp = 15;
            e->attack = 4;  e->defense = 1;
            e->experience = 10;
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
        case ENEMY_ICE_GIANT:
            snprintf(e->name, sizeof(e->name), "Ice Giant");
            e->max_hp = 100; e->hp = 100;
            e->attack = 18; e->defense = 6; e->experience = 130;
            break;
        case ENEMY_POLAR_KRAKEN:
            snprintf(e->name, sizeof(e->name), "Polar Kraken");
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
        case ENEMY_FEY_TRICKSTER:
            snprintf(e->name, sizeof(e->name), "Fey Trickster");
            e->max_hp = 28; e->hp = 28;
            e->attack = 10; e->defense = 2; e->experience = 35;
            break;
        case ENEMY_GIANT_MOTH:
            snprintf(e->name, sizeof(e->name), "Giant Moth");
            e->max_hp = 24; e->hp = 24;
            e->attack = 11; e->defense = 1; e->experience = 32;
            break;
        case ENEMY_LIVING_FLOWER:
            snprintf(e->name, sizeof(e->name), "Carnivorous Flower");
            e->max_hp = 50; e->hp = 50;
            e->attack = 14; e->defense = 3; e->experience = 70;
            break;
        case ENEMY_THORN_GUARDIAN:
            snprintf(e->name, sizeof(e->name), "Thorn Guardian");
            e->max_hp = 95; e->hp = 95;
            e->attack = 17; e->defense = 7; e->experience = 110;
            break;
        case ENEMY_THORN_REGENT:
            snprintf(e->name, sizeof(e->name), "Thorn Regent");
            e->max_hp = 240; e->hp = 240;
            e->attack = 23; e->defense = 8; e->experience = 700;
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
            snprintf(e->name, sizeof(e->name), "Crystal Spider");
            e->max_hp = 30; e->hp = 30;
            e->attack = 11; e->defense = 2; e->experience = 40;
            break;
        case ENEMY_BLIND_STALKER:
            snprintf(e->name, sizeof(e->name), "Blind Stalker");
            e->max_hp = 38; e->hp = 38;
            e->attack = 16; e->defense = 3; e->experience = 60;
            break;
        case ENEMY_SHARD_GOLEM:
            snprintf(e->name, sizeof(e->name), "Shard Golem");
            e->max_hp = 110; e->hp = 110;
            e->attack = 18; e->defense = 9; e->experience = 130;
            break;
        case ENEMY_PRISM_SOVEREIGN:
            snprintf(e->name, sizeof(e->name), "Prism Sovereign");
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
        case ENEMY_ORC:
            strncpy(e->name, "Orc", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 25; e->hp = 25;
            e->attack = 7;  e->defense = 2;
            e->experience = 20;
            break;
        case ENEMY_TROLL:
            strncpy(e->name, "Troll", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 40; e->hp = 40;
            e->attack = 10; e->defense = 4;
            e->experience = 30;
            break;
        case ENEMY_GIANT:
            strncpy(e->name, "Giant", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 60; e->hp = 60;
            e->attack = 14; e->defense = 6;
            e->experience = 50;
            break;
        case ENEMY_GOBLIN_KING:
            strncpy(e->name, "Goblin King", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 100; e->hp = 100;
            e->attack = 15;  e->defense = 5;
            e->experience = 200;
            e->is_boss = 1;
            break;
        case ENEMY_LICH_KING:
            strncpy(e->name, "Lich King", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 140; e->hp = 140;
            e->attack = 18;  e->defense = 6;
            e->experience = 400;
            e->is_boss = 1;
            break;
        case ENEMY_DEMON_LORD:
            strncpy(e->name, "Demon Lord", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 350; e->hp = 350;
            e->attack = 38;  e->defense = 15;
            e->experience = 700;
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
        case ENEMY_TARRASQUE:
            strncpy(e->name, "Tarrasque", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 800; e->hp = 800;
            e->attack = 80;  e->defense = 35;
            e->experience = 2000;
            e->is_boss = 1;
            break;
    }
    e->name[sizeof(e->name) - 1] = '\0';
    if (g->location == LOCATION_CATACOMBS && (type == ENEMY_CRYPT_BAT || type == ENEMY_WRAITH)) {
        e->max_hp *= 2;
        e->hp = e->max_hp;
        e->attack += 6;
        e->defense += 2;
        e->experience *= 2;
    }
    scale_spawned_enemy(g, e);
}

static int boss_for_level(const GameState *g, EnemyType *type) {
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
    } else if (g->location == LOCATION_MOONVEIL) {
        boss_level = MOONVEIL_DEPTH;
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
        *type = ENEMY_POLAR_KRAKEN;
    } else if (g->location == LOCATION_DESERT) {
        *type = ENEMY_DESERT_PHARAOH;
    } else if (g->location == LOCATION_MOONVEIL) {
        *type = ENEMY_THORN_REGENT;
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
        g->map.tiles[y][x] != TILE_COAST_SHALLOW_WATER &&
        g->map.tiles[y][x] != TILE_COAST_DRAINED_WATER &&
        g->map.tiles[y][x] != TILE_COAST_CHANNEL_DRY &&
        g->map.tiles[y][x] != TILE_SWAMP_FLOOR &&
        g->map.tiles[y][x] != TILE_DESERT_FLOOR &&
        g->map.tiles[y][x] != TILE_MOONVEIL_FLOOR &&
        g->map.tiles[y][x] != TILE_MOONVEIL_CIRCLE &&
        g->map.tiles[y][x] != TILE_ASHEN_FLOOR &&
        g->map.tiles[y][x] != TILE_ASHEN_RUIN &&
        g->map.tiles[y][x] != TILE_CATACOMBS_FLOOR &&
        g->map.tiles[y][x] != TILE_GLASSDEEP_FLOOR &&
        g->map.tiles[y][x] != TILE_GLASSDEEP_RUIN &&
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
    if (g->enemy_count >= AREA_ENEMY_LIMIT) {
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
    if (g->level == 1) {
        target_type = ENEMY_GOBLIN_ARCHER;
        target_bit = DAIN_FRAGMENT_ARCHER;
    } else if (g->level == 2) {
        target_type = ENEMY_GOBLIN_BOMBER;
        target_bit = DAIN_FRAGMENT_BOMBER;
    } else if (g->level == 3) {
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
    EnemyType guard = g->level == 1 ? ENEMY_GOBLIN_SCOUT :
        (g->level == 2 ? ENEMY_TUNNEL_SPIDER : ENEMY_HOBGOBLIN_GUARD);
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
        int bit = g->level == 1 ? ALDER_WARDEN_STAGE_1 :
            (g->level == 2 ? ALDER_WARDEN_STAGE_2 :
            (g->level == 3 ? ALDER_WARDEN_STAGE_3 : 0));
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

void enemies_spawn(GameState *g) {
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
    int num_enemies = 10 + difficulty;
    if (g->location == LOCATION_SWAMP && g->level == SWAMP_RESCUE_LEVEL &&
        g->innkeeper_quest_state == 1) {
        num_enemies--;
    }
    if (quest_group_pending(g)) {
        num_enemies -= 3;
    }
    if (num_enemies > AREA_ENEMY_LIMIT) {
        num_enemies = AREA_ENEMY_LIMIT;
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
            g->location == LOCATION_MOONVEIL ||
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

    int boss_level = g->location == LOCATION_FOREST ? FOREST_BOSS_LEVEL :
        (g->location == LOCATION_MOUNTAINS ? MOUNTAIN_BOSS_LEVEL :
        (g->location == LOCATION_COAST ? COAST_DEPTH :
        (g->location == LOCATION_SWAMP ? SWAMP_BOSS_LEVEL :
        (g->location == LOCATION_FROSTFELL ? FROSTFELL_DEPTH :
        (g->location == LOCATION_DESERT ? DESERT_DEPTH :
        (g->location == LOCATION_MOONVEIL ? MOONVEIL_DEPTH :
        (g->location == LOCATION_ASHEN ? ASHEN_DEPTH :
        (g->location == LOCATION_GLASSDEEP ? GLASSDEEP_DEPTH :
        (g->location == LOCATION_CATACOMBS ? CATACOMBS_DEPTH :
        (g->location == LOCATION_DRAGONSPINE ? DRAGONSPINE_DEPTH : DUNGEON_DEPTH))))))))));
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
                    (roll < 55 ? ENEMY_DRAKE : ENEMY_GIANT);
            } else {
                type = roll < 12 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 40 ? ENEMY_DRAKE :
                    (roll < 65 ? ENEMY_GIANT : ENEMY_FIRE_ELEMENTAL));
            }
        } else if (g->location == LOCATION_CATACOMBS) {
            int slot = g->enemy_count;
            if (slot % 2 == 0) {
                type = slot % 4 == 0 ? ENEMY_BONE_SENTINEL : ENEMY_ANCIENT_SKELETON;
            } else if (g->level >= 3 && slot % 6 == 5) {
                type = ENEMY_BONE_CANTOR;
            } else if (g->level >= 2 && slot % 6 == 3) {
                type = ENEMY_WRAITH;
            } else if (g->level >= 2 && slot % 4 == 1) {
                type = ENEMY_GRAVE_ARCHER;
            } else {
                type = ENEMY_CRYPT_BAT;
            }
        } else if (g->location == LOCATION_GLASSDEEP) {
            if (g->level < 3) {
                type = roll < 65 ? ENEMY_CRYSTAL_SPIDER : ENEMY_BLIND_STALKER;
            } else {
                type = roll < 30 ? ENEMY_CRYSTAL_SPIDER :
                    (roll < 65 ? ENEMY_BLIND_STALKER : ENEMY_SHARD_GOLEM);
            }
        } else if (g->location == LOCATION_ASHEN) {
            if (g->level < 3) {
                type = roll < 55 ? ENEMY_CINDER_IMP : ENEMY_ASH_HOUND;
            } else {
                type = roll < 30 ? ENEMY_CINDER_IMP :
                    (roll < 65 ? ENEMY_ASH_HOUND : ENEMY_OBSIDIAN_GUARDIAN);
            }
        } else if (g->location == LOCATION_MOONVEIL) {
            if (g->level == 1) {
                type = roll < 55 ? ENEMY_GIANT_MOTH : ENEMY_FEY_TRICKSTER;
            } else if (g->level == 2) {
                type = roll < 35 ? ENEMY_GIANT_MOTH :
                    (roll < 75 ? ENEMY_FEY_TRICKSTER : ENEMY_LIVING_FLOWER);
            } else {
                type = roll < 20 ? ENEMY_GIANT_MOTH :
                    (roll < 45 ? ENEMY_FEY_TRICKSTER :
                    (roll < 75 ? ENEMY_LIVING_FLOWER : ENEMY_THORN_GUARDIAN));
            }
        } else if (g->location == LOCATION_DESERT) {
            if (g->level == 1) {
                type = roll < 65 ? ENEMY_SCARAB : ENEMY_VIPER;
            } else if (g->level == 2) {
                type = roll < 45 ? ENEMY_SCARAB :
                    (roll < 80 ? ENEMY_VIPER : ENEMY_MUMMY);
            } else if (g->level == 3) {
                type = roll < 30 ? ENEMY_SCARAB :
                    (roll < 55 ? ENEMY_VIPER :
                    (roll < 80 ? ENEMY_MUMMY : ENEMY_DJINN));
            } else if (g->level == 4) {
                type = roll < 20 ? ENEMY_SCARAB :
                    (roll < 40 ? ENEMY_VIPER :
                    (roll < 65 ? ENEMY_MUMMY :
                    (roll < 85 ? ENEMY_DJINN : ENEMY_GOLEM)));
            } else {
                type = roll < 10 ? ENEMY_SCARAB :
                    (roll < 25 ? ENEMY_VIPER :
                    (roll < 50 ? ENEMY_MUMMY :
                    (roll < 75 ? ENEMY_DJINN : ENEMY_GOLEM)));
            }
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
                    (roll < 80 ? ENEMY_ICE_GOLEM : ENEMY_ICE_GIANT))));
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
    memset(g->castle_cache, 0, sizeof(g->castle_cache));
    memset(g->castle_loot, 0, sizeof(g->castle_loot));
    memset(g->castle_loot_count, 0, sizeof(g->castle_loot_count));
    g->castle_minibosses = 0;
    g->castle_prompt = 0;
    g->game_won = 0;
    srand((unsigned)time(NULL));
    g->level = 1;
    g->crownroad_cache.valid = 0;
    g->kingroad_west_cache = (CrownroadCache){0};
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        g->level_cache[i].valid = 0;
        g->forest_cache[i].valid = 0;
        g->mountain_cache[i].valid = 0;
        g->coast_cache[i].valid = 0;
        if (i < SWAMP_DEPTH) {
            g->swamp_cache[i].valid = 0;
        }
        if (i < DRAGONSPINE_DEPTH) {
            g->dragonspine_cache[i].valid = 0;
        }
        if (i < FROSTFELL_DEPTH) {
            g->frostfell_cache[i].valid = 0;
        }
        if (i < MOONVEIL_DEPTH) {
            g->moonveil_cache[i] = (LevelCache){0};
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
    g->max_forest_level_reached = 1;
    g->forest_entry_town = LOCATION_TOWN;
    g->forest_portal_town = LOCATION_TOWN;
    g->max_mountain_level_reached = 1;
    g->mountain_entry_town = LOCATION_TOWN;
    g->mountain_portal_town = LOCATION_TOWN;
    g->max_coast_level_reached = 1;
    g->max_swamp_level_reached = 1;
    g->swamp_entry_town = LOCATION_TOWN2;
    g->swamp_portal_town = LOCATION_TOWN2;
    g->max_dragonspine_level_reached = 1;
    g->max_frostfell_level_reached = 1;
    g->max_desert_level_reached = 1;
    g->max_moonveil_level_reached = 1;
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
    g->rook_quest_state = 0;
    g->innkeeper_quest_state = 0;
    g->rook_labyrinth_switches = 0;
    g->rook_quest_completions = 0;
    g->score = 0;
    g->dungeon_key_found = 0;
    g->dungeon_crypt_keys = 0;
    g->portal_active = 0;
    g->portal_level = 0;
    g->portal_location = LOCATION_DUNGEON;
    g->portal_x = 0;
    g->portal_y = 0;
    g->portal_origin_tile = TILE_FLOOR;
    g->defeated_bosses = 0;
    g->kraken_bow_unclaimed = 0;
    g->sandstorm_staff_unclaimed = 0;
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
    g->dragon_treasure_quest_state = 0;
    g->sunscar_lamp_quest_state = 0;
    g->emberforge_quest_state = 0;
    g->emberforge_progress = 0;
    g->emberforge_encounters = 0;
    g->frostfell_quest_state = 0;
    g->frostfell_quest_progress = 0;
    g->frostfell_quest_encounters = 0;
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
    game_unequip_main_hand(g);
    g->equipped_main_hand = index;
    g->player.attack += weapon->attack_bonus;
    g->player.max_mp += weapon->max_mp_bonus;
    return 1;
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
    if (g->location == LOCATION_MOONVEIL) {
        return g->moonveil_cache;
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
    if (g->location == LOCATION_MOONVEIL) {
        return &g->max_moonveil_level_reached;
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
    if (g->location == LOCATION_MOONVEIL) {
        return MOONVEIL_DEPTH;
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
    if (level == 1) {
        return ALDER_WARDEN_STAGE_1;
    } else if (level == 2) {
        return ALDER_WARDEN_STAGE_2;
    } else if (level == 3) {
        return ALDER_WARDEN_STAGE_3;
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
            if (g->map.tiles[y][x] == TILE_FOREST_WARDEN) {
                return 0;
            }
        }
    }
    int room_index = g->level == 1 ? 4 : (g->level == 2 ? 7 : 6);
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
    if (g->level == 1) {
        primary = ENEMY_GIANT_SPIDER;
        support = ENEMY_BLIGHTED_WOLF;
    } else if (g->level == 2) {
        primary = ENEMY_DARK_ELF;
        support = ENEMY_PIXIE;
    } else if (g->level == 3) {
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
    if (g->location != LOCATION_DESERT || g->level != DESERT_DEPTH ||
        !g->sandstorm_staff_unclaimed || g->map.room_count == 0) {
        return;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active &&
            strcmp(g->floor_items[i].item.name, "Sandstorm Staff") == 0) {
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
        .active = 1, .x = x, .y = y, .underlying_tile = TILE_DESERT_FLOOR,
        .item = item_make_sandstorm_staff()
    };
    g->map.tiles[y][x] = TILE_ITEM;
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
    if (slot >= MAX_ENEMIES) {
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

static TileType frostfell_quest_tile(const GameState *g, int x, int y) {
    TileType tile = quest_tile(g, x, y);
    if (tile == TILE_PORTAL && g->portal_active && g->portal_location == g->location &&
        g->portal_level == g->level && g->portal_x == x && g->portal_y == y) {
        return g->portal_origin_tile;
    }
    return tile;
}

static void set_frostfell_quest_tile(GameState *g, int x, int y, TileType tile) {
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

static int spawn_frostfell_quest_guard(GameState *g, EnemyType type, int cx, int cy) {
    int slot = g->enemy_count;
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].active && strcmp(g->enemies[i].name, "Expedition Guardian") != 0) {
            slot = i;
            break;
        }
    }
    if (slot >= MAX_ENEMIES) {
        return 0;
    }
    for (int radius = 1; radius <= 4; radius++) {
        for (int y = cy - radius; y <= cy + radius; y++) {
            for (int x = cx - radius; x <= cx + radius; x++) {
                if (!enemy_tile_open(g, x, y) || (g->player.x == x && g->player.y == y)) {
                    continue;
                }
                if (slot == g->enemy_count) {
                    g->enemy_count++;
                }
                spawn_enemy(g, &g->enemies[slot], type, x, y);
                snprintf(g->enemies[slot].name, sizeof(g->enemies[slot].name), "Expedition Guardian");
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
            if (frostfell_quest_tile(g, tx, ty) == objective) {
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
    set_frostfell_quest_tile(g, x, y, objective);
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
            (i == 0 ? ENEMY_ICE_GIANT : ENEMY_FROST_ARCHER);
        if (!spawn_frostfell_quest_guard(g, type, x, y)) {
            return;
        }
    }
    g->frostfell_quest_encounters |= bit;
}

void game_refresh_quest_encounters(GameState *g) {
    game_reveal_forest_shortcut(g);
    game_reveal_swamp_shortcut(g);
    game_reveal_mountain_shortcut(g);
    restore_frostfell_reward(g);
    restore_desert_reward(g);
    catacombs_restore_reward(g);
    int seal_placed = place_elowen_seal(g);
    place_desert_lamp(g);
    int warden_placed = place_alder_warden(g);
    int beacon_placed = place_mara_beacon(g);
    place_dain_map_bearer(g);
    place_dragon_treasure(g);
    place_emberforge_encounter(g);
    place_frostfell_quest_encounter(g);
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
    } else if (g->location == LOCATION_MOONVEIL) {
        map_generate_moonveil(&g->map, g->level);
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
    restore_frostfell_reward(g);
    restore_desert_reward(g);
    catacombs_restore_reward(g);
    place_emberforge_encounter(g);
    // A regenerated stage needs new guards for any unfinished objective.
    g->frostfell_quest_encounters &= ~frostfell_quest_stage_bit(g);
    place_frostfell_quest_encounter(g);
    game_update_level_progress(g);
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
            if (g->map.tiles[y][x] == TILE_TEMPLE_MOON_DOOR_OPEN) {
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

static void enter_adventure(GameState *g, Location location) {
    clear_floor_loot(g);
    g->location = location;
    LevelCache *cache = active_cache(g);
    int *max_level = active_max_level(g);

    g->level = 1;
    g->level_cleared = 0;
    if (!g->portal_active || g->portal_location != location) {
        *max_level = 1;
        for (int i = 0; i < active_depth(g); i++) {
            cache[i].valid = 0;
        }
    }
    generate_active_level(g);
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_enter_dungeon(GameState *g) {
    enter_adventure(g, LOCATION_DUNGEON);
}

void game_enter_forest(GameState *g) {
    int from_stillbury = g->location == LOCATION_TOWN2;
    clear_floor_loot(g);
    g->forest_entry_town = from_stillbury ? LOCATION_TOWN2 : LOCATION_TOWN;
    g->location = LOCATION_FOREST;
    g->level = from_stillbury ? FOREST_DEPTH : 1;
    LevelCache *cache = &g->forest_cache[g->level - 1];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(g->enemies));
        g->level_cleared = cache->level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    if (g->level > g->max_forest_level_reached) {
        g->max_forest_level_reached = g->level;
    }
    prepare_forest_arrival(g, from_stillbury);
    g->player.x = from_stillbury ? g->map.stairs_down_x : g->map.stairs_up_x;
    g->player.y = from_stillbury ? g->map.stairs_down_y : g->map.stairs_up_y;
    g->dialogue_active = 0;
}

void game_enter_mountains(GameState *g) {
    int from_ridgeshire = g->location == LOCATION_TOWN4;
    clear_floor_loot(g);
    g->mountain_entry_town = from_ridgeshire ? LOCATION_TOWN4 : LOCATION_TOWN;
    g->location = LOCATION_MOUNTAINS;
    g->level = from_ridgeshire ? MOUNTAIN_DEPTH : 1;
    LevelCache *cache = &g->mountain_cache[g->level - 1];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(g->enemies));
        g->level_cleared = cache->level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    if (g->level > g->max_mountain_level_reached) {
        g->max_mountain_level_reached = g->level;
    }
    g->player.x = from_ridgeshire ? g->map.stairs_down_x : g->map.stairs_up_x;
    g->player.y = from_ridgeshire ? g->map.stairs_down_y : g->map.stairs_up_y;
    g->dialogue_active = 0;
}

void game_enter_coast(GameState *g) {
    enter_adventure(g, LOCATION_COAST);
}

void game_enter_swamp(GameState *g) {
    int from_rosemoor = g->location == LOCATION_TOWN3;
    clear_floor_loot(g);
    g->swamp_entry_town = from_rosemoor ? LOCATION_TOWN3 : LOCATION_TOWN2;
    g->location = LOCATION_SWAMP;
    g->level = from_rosemoor ? SWAMP_DEPTH : 1;
    LevelCache *cache = &g->swamp_cache[g->level - 1];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(g->enemies));
        g->level_cleared = cache->level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    if (g->level > g->max_swamp_level_reached) {
        g->max_swamp_level_reached = g->level;
    }
    g->player.x = from_rosemoor ? g->map.stairs_down_x - 1 : g->map.stairs_up_x + 1;
    g->player.y = from_rosemoor ? g->map.stairs_down_y : g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "The black water closes around the swamp trail.");
}

void game_enter_frostfell(GameState *g) {
    enter_adventure(g, LOCATION_FROSTFELL);
    push_message(g, "Bitter wind howls across the Frostfell Wastes.");
}

void game_enter_desert(GameState *g) {
    enter_adventure(g, LOCATION_DESERT);
    push_message(g, "Scorching sands stretch across the Sunscar Wastes.");
}

void game_enter_moonveil(GameState *g) {
    clear_floor_loot(g);
    g->location = LOCATION_MOONVEIL;
    g->level = 1;
    LevelCache *cache = &g->moonveil_cache[0];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(g->enemies));
        g->level_cleared = cache->level_cleared;
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "Moonlit blossoms glow along the paths of Moonveil Gardens.");
}

void game_enter_ashen(GameState *g) {
    clear_floor_loot(g);
    g->location = LOCATION_ASHEN;
    g->level = 1;
    LevelCache *cache = &g->ashen_cache[0];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(g->enemies));
        g->level_cleared = cache->level_cleared;
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "Ash drifts across the basalt paths of Ashen Hollow.");
}

void game_enter_catacombs(GameState *g) {
    clear_floor_loot(g);
    g->location = LOCATION_CATACOMBS;
    g->level = 1;
    LevelCache *cache = &g->catacombs_cache[0];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(g->enemies));
        g->level_cleared = cache->level_cleared;
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "The royal dead stir beneath the castle. Blue braziers bind their bones.");
}

void game_enter_glassdeep(GameState *g) {
    clear_floor_loot(g);
    g->location = LOCATION_GLASSDEEP;
    g->level = 1;
    LevelCache *cache = &g->glassdeep_cache[0];
    if (cache->valid) {
        g->map = cache->map;
        g->enemy_count = cache->enemy_count;
        memcpy(g->enemies, cache->enemies, sizeof(g->enemies));
        g->level_cleared = cache->level_cleared;
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    push_message(g, "Crystals illuminate the abandoned quarry of Glassdeep Caverns.");
}

void game_enter_high_pass(GameState *g, int from_town) {
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
    push_message(g, from_town ? "The High Pass leads north to Ridgeshire." :
        "The High Pass leads south to OakHaven.");
}

void game_enter_dragonspine(GameState *g) {
    enter_adventure(g, LOCATION_DRAGONSPINE);
    push_message(g, "Dragonspine rises above the clouds.");
}

int game_harbor_unlocked(const GameState *g) {
    return (g->defeated_bosses & (1 << LOCATION_COAST)) != 0;
}

static void assign_rook_quest(GameState *g) {
    g->rook_quest_state = 1;
    g->rook_labyrinth_switches = 0;
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "Recover my stolen ivory rook from the labyrinth across from the witch's "
        "hut. Beware the false stairs and the Minotaur below.");
    push_message(g, "Assigned: The Ivory Rook.");
    push_message(g, "The labyrinth across from the witch's hut is now open.");
}

static void place_harbor_road(GameState *g) {
    if (!game_harbor_unlocked(g)) {
        return;
    }
    // Branch below Rowan, then bend through a landing onto the dock.
    for (int x = 21; x < TOWN_HARBOR_X; x++) {
        g->map.tiles[TOWN_HARBOR_Y + 1][x] = TILE_TOWN_PATH;
    }
    for (int y = TOWN_HARBOR_Y + 1; y <= TOWN_HARBOR_ENTRANCE_Y; y++) {
        g->map.tiles[y][TOWN_HARBOR_X - 1] = TILE_TOWN_PATH;
    }
    for (int x = TOWN_HARBOR_X; x <= TOWN_HARBOR_ENTRANCE_X; x++) {
        g->map.tiles[TOWN_HARBOR_ENTRANCE_Y][x] = TILE_TOWN_PATH;
    }
}

static void place_town_portal(GameState *g) {
    if (!g->portal_active) {
        return;
    }
    if (g->location == LOCATION_TOWN4) {
        if (g->portal_location == LOCATION_ASHEN) {
            g->map.tiles[2][RIDGESHIRE_ASHEN_GATE_X + 1] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_DRAGONSPINE) {
            g->map.tiles[TOWN4_PORTAL_Y][TOWN4_PORTAL_X] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_MOUNTAINS && g->mountain_portal_town == LOCATION_TOWN4) {
            g->map.tiles[TOWN_H - 3][21] = TILE_PORTAL;
        }
        return;
    }
    if (g->location == LOCATION_TOWN2) {
        if (g->portal_location == LOCATION_GLASSDEEP) {
            g->map.tiles[TOWN_H - 3][STILLBURY_GLASSDEEP_GATE_X + 1] = TILE_PORTAL;
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
        if (g->portal_location == LOCATION_CATACOMBS) {
            g->map.tiles[TOWN3_KING_GATE_Y + 1][TOWN_W - 3] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_MOONVEIL) {
            g->map.tiles[ROSEMOOR_MOONVEIL_GATE_Y + 1][2] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_FROSTFELL) {
            g->map.tiles[2][20] = TILE_PORTAL;
        } else if (g->portal_location == LOCATION_SWAMP && g->swamp_portal_town == LOCATION_TOWN3) {
            g->map.tiles[TOWN_H - 3][21] = TILE_PORTAL;
        }
        return;
    }
    if (g->location != LOCATION_TOWN ||
        (g->portal_location == LOCATION_FOREST && g->forest_portal_town != LOCATION_TOWN) ||
        (g->portal_location == LOCATION_MOUNTAINS && g->mountain_portal_town != LOCATION_TOWN) ||
        g->portal_location == LOCATION_DRAGONSPINE ||
        g->portal_location == LOCATION_SWAMP ||
        g->portal_location == LOCATION_DESERT ||
        g->portal_location == LOCATION_MOONVEIL ||
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
        x = TOWN_HARBOR_X - 1;
        y = TOWN_HARBOR_ENTRANCE_Y;
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
    place_harbor_road(g);
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
    g->location = LOCATION_FOREST_ROAD;
    map_generate_forest_road(&g->map);
    g->player.x = from_town2 ? 1 : FOREST_ROAD_W - 2;
    g->player.y = FOREST_ROAD_Y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, from_town2 ? "The forest road leads east to OakHaven." :
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
        g->player.y = TOWN_ROAD_EXIT_Y;
    } else {
        map_generate_town(&g->map, &spawn_x, &spawn_y);
        map_set_town2_road(&g->map,
            g->defeated_bosses & (1 << LOCATION_FOREST));
        map_set_town4_road(&g->map,
            g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
        place_harbor_road(g);
        place_town_portal(g);
        g->player.x = 1;
        g->player.y = TOWN_ROAD_EXIT_Y;
    }
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, destination == LOCATION_TOWN2 ?
        "You arrive in Stillbury." : "You return to OakHaven.");
}

int game_is_king_road(const GameState *g) {
    return g->location == LOCATION_CROWNROAD || g->location == LOCATION_KING_ROAD_WEST;
}

void game_enter_king_road(GameState *g, Location road, int from_castle) {
    static const EnemyType enemies[MAX_ENEMIES] = {
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
    static const int x[MAX_ENEMIES] = {
        17, 23, 14, 24, 17, 25, 16, 23, 23, 20, 17, 20, 23, 20, 17,
        24, 15, 25, 16, 24, 14, 24, 15, 20, 17, 20, 23, 16, 20, 24
    };
    static const int y[MAX_ENEMIES] = {
        6, 11, 17, 21, 27, 32, 38, 43, 8, 14, 20, 26, 34, 40, 44,
        5, 10, 15, 23, 29, 33, 37, 41, 9, 12, 20, 24, 30, 33, 39
    };
    CrownroadCache *cache = road == LOCATION_KING_ROAD_WEST ?
        &g->kingroad_west_cache : &g->crownroad_cache;
    g->location = road;
    g->level = 1;
    map_generate_crownroad(&g->map);
    if (cache->valid) {
        g->enemy_count = cache->enemy_count;
        g->level_cleared = cache->level_cleared;
        for (int i = 0; i < g->enemy_count; i++) {
            g->enemies[i] = cache->enemies[i];
        }
    } else {
        g->enemy_count = 0;
        g->level_cleared = 0;
    }
    if (g->enemy_count < MAX_ENEMIES) {
        g->level_cleared = 0;
    }
    for (int i = g->enemy_count; i < MAX_ENEMIES; i++) {
        spawn_enemy(g, &g->enemies[g->enemy_count++], enemies[i],
            CROWNROAD_W - 1 - y[i], x[i]);
    }
    int from_west = (road == LOCATION_CROWNROAD) != from_castle;
    g->player.x = from_west ? 1 : CROWNROAD_W - 2;
    g->player.y = CROWNROAD_Y;
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

void game_enter_workshop(GameState *g) {
    g->location = LOCATION_WORKSHOP;
    map_generate_workshop(&g->map, &g->player.x, &g->player.y);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "Garrick sharpens swords, axes and daggers: +1 attack for 50 gold, once per weapon.");
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
    push_message(g, "Ridgeshire Town Hall. Approach Steward Hadrin and press T to talk.");
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
        g->emberforge_quest_state = 1;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Reclaim the Emberforge! Recover its mechanism on Ashen Hollow stage 2, "
            "then repair the furnace on stage 4. Defeat the guards and press A beside each. Return for 100 gold.");
        push_message(g, "Assigned: Reclaim the Emberforge. See your quest journal.");
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

void game_talk_to_brenna(GameState *g) {
    if (g->location != LOCATION_TAVERN || abs(g->player.x - BRENNA_X) > 1 || abs(g->player.y - BRENNA_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Quartermaster Brenna");
    g->dialogue_x = BRENNA_X;
    g->dialogue_y = BRENNA_Y;
    if (g->frostfell_quest_state == 0) {
        g->frostfell_quest_state = 1;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Frostfell lies two towns away, in the far north. Follow the woods to Stillbury, then cross the swamp to Rosemoor. "
            "My expedition vanished beyond Rosemoor's north gate.");
        push_message(g, "Assigned: The Silent Expedition. Recover the journal on Frostfell stage 2 and rescue the surveyor on stage 4.");
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
        frostfell_quest_tile(g, x, y) != TILE_NPC_FROST_SURVIVOR) {
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
        set_frostfell_quest_tile(g, x, y, TILE_FROST_FLOOR);
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The journal shows the route! I can make my own way home now. Tell Brenna in Oakhaven's Tavern that the expedition's work is safe.");
        push_message(g, "Surveyor Fen heads home. Return to Quartermaster Brenna in Oakhaven's Tavern.");
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
        if (tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H && frostfell_quest_tile(g, tx, ty) == TILE_FROST_JOURNAL) {
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
    set_frostfell_quest_tile(g, x, y, TILE_FROST_FLOOR);
    push_message(g, "Expedition journal recovered. Find Surveyor Fen on Frostfell stage 4 and speak to him.");
    return 1;
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
}

int game_labyrinth_is_open(const GameState *g) {
    return g->rook_quest_state != 0;
}

void game_enter_labyrinth(GameState *g) {
    g->location = LOCATION_LABYRINTH;
    g->level = 1;
    for (int i = 0; i < LABYRINTH_DEPTH; i++) {
        g->labyrinth_cache[i].valid = 0;
    }
    load_labyrinth_floor(g);
    g->player.x = 2;
    g->player.y = g->map.stairs_up_y;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, "You enter Rook's labyrinth.");
    push_message(g, "Five runes open the deepest vault.");
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

void game_leave_labyrinth(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_TOWN2;
    map_generate_town2(&g->map, &spawn_x, &spawn_y);
    map_set_town3_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
    map_set_stillbury_forest_road(&g->map, g->defeated_bosses & (1 << LOCATION_FOREST));
    place_town_portal(g);
    g->player.x = TOWN_LABYRINTH_X;
    g->player.y = TOWN_LABYRINTH_Y + 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    push_message(g, "You emerge from Rook's labyrinth.");
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
            push_message(g, "Return it to Rook at the inn.");
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
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_TOWN;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    map_set_town2_road(&g->map,
        g->defeated_bosses & (1 << LOCATION_FOREST));
    map_set_town4_road(&g->map,
        g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
    place_harbor_road(g);
    g->player.x = TOWN_HARBOR_ENTRANCE_X;
    g->player.y = TOWN_HARBOR_ENTRANCE_Y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    place_town_portal(g);
    push_message(g, "The ship returns you to town.");
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
}

void game_enter_temple(GameState *g) {
    g->location = LOCATION_TEMPLE;
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
            temple_interaction_tile(g->map.tiles[y][x])) {
            return 1;
        }
    }
    return 0;
}

static void toggle_temple_alignment(GameState *g) {
    g->temple_alignment = !g->temple_alignment;
    if (g->temple_alignment) {
        g->temple_sentinels_awakened = 1;
    }
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            TileType tile = g->map.tiles[y][x];
            if (g->temple_alignment && tile == TILE_TEMPLE_MOON_DOOR_CLOSED) {
                g->map.tiles[y][x] = TILE_TEMPLE_MOON_DOOR_OPEN;
            } else if (!g->temple_alignment &&
                tile == TILE_TEMPLE_MOON_DOOR_OPEN) {
                g->map.tiles[y][x] = TILE_TEMPLE_MOON_DOOR_CLOSED;
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
        TileType tile = g->map.tiles[y][x];
        if (tile == TILE_TEMPLE_ALTAR) {
            toggle_temple_alignment(g);
            return 1;
        }
        if (tile == TILE_TEMPLE_TREASURE) {
            if (!(g->defeated_bosses & (1 << LOCATION_TEMPLE))) {
                push_message(g, "The Fallen Sun Guardian seals the treasure vault.");
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
    if (returning_from == LOCATION_FROSTFELL || returning_from == LOCATION_DESERT ||
        returning_from == LOCATION_MOONVEIL || returning_from == LOCATION_GLASSDEEP ||
        returning_from == LOCATION_CATACOMBS ||
        destination == LOCATION_TOWN4 ||
        destination == LOCATION_TOWN3) {
        clear_floor_loot(g);
    }
    LevelCache *cache = active_cache(g);
    // Cache current level before leaving
    if (returning_from == LOCATION_CROWNROAD || returning_from == LOCATION_KING_ROAD_WEST) {
        save_crownroad_cache(g);
    } else if (returning_from != LOCATION_HIGH_PASS && returning_from != LOCATION_SWAMP_ROAD &&
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
        place_harbor_road(g);
    }
    if (destination == LOCATION_CASTLE) {
        g->player.x = CROWNROAD_X;
        g->player.y = TOWN_H - 2;
    } else if (destination == LOCATION_TOWN3) {
        g->player.x = returning_from == LOCATION_FROSTFELL ? 20 : spawn_x;
        g->player.y = returning_from == LOCATION_FROSTFELL ? 1 : spawn_y;
        if (returning_from == LOCATION_CATACOMBS) {
            g->player.x = TOWN_W - 2;
            g->player.y = TOWN3_KING_GATE_Y;
        }
        if (returning_from == LOCATION_MOONVEIL) {
            g->player.x = 1;
            g->player.y = ROSEMOOR_MOONVEIL_GATE_Y;
        }
    } else if (destination == LOCATION_TOWN4) {
        g->player.x = returning_from == LOCATION_MOUNTAINS ? spawn_x : TOWN_W - 2;
        g->player.y = returning_from == LOCATION_MOUNTAINS ? spawn_y : TOWN4_DRAGON_GATE_Y;
        if (returning_from == LOCATION_ASHEN) {
            g->player.x = RIDGESHIRE_ASHEN_GATE_X;
            g->player.y = 1;
        }
    } else if (returning_from == LOCATION_CROWNROAD) {
        g->player.x = CROWNROAD_X;
        g->player.y = 1;
    } else if (returning_from == LOCATION_SWAMP) {
        g->player.x = 20;
        g->player.y = 1;
    } else if (returning_from == LOCATION_GLASSDEEP) {
        g->player.x = STILLBURY_GLASSDEEP_GATE_X;
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
}

void game_leave_catacombs(GameState *g) {
    return_to_town(g, LOCATION_CASTLE);
    push_message(g, "You emerge at the castle's south gate.");
}

void game_return_to_town(GameState *g) {
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        castle_leave(g, 1);
        return;
    }
    if (g->location == LOCATION_CROWNROAD || g->location == LOCATION_KING_ROAD_WEST) {
        game_leave_crownroad(g, g->location == LOCATION_CROWNROAD ? LOCATION_TOWN3 : LOCATION_TOWN4);
        return;
    }
    Location destination = LOCATION_TOWN;
    if (g->location == LOCATION_FOREST) {
        destination = g->forest_entry_town;
    } else if (g->location == LOCATION_SWAMP) {
        destination = g->swamp_entry_town;
    } else if (g->location == LOCATION_MOUNTAINS) {
        destination = g->mountain_entry_town;
    } else if (g->location == LOCATION_DESERT || g->location == LOCATION_GLASSDEEP) {
        destination = LOCATION_TOWN2;
    } else if (g->location == LOCATION_FROSTFELL || g->location == LOCATION_MOONVEIL ||
        g->location == LOCATION_CATACOMBS) {
        destination = LOCATION_TOWN3;
    } else if (g->location == LOCATION_DRAGONSPINE || g->location == LOCATION_HIGH_PASS ||
        g->location == LOCATION_ASHEN) {
        destination = LOCATION_TOWN4;
    }
    return_to_town(g, destination);
}

void game_leave_forest(GameState *g, Location town, int shortcut) {
    return_to_town(g, town);
    g->player.x = town == LOCATION_TOWN2 ? TOWN_W - 2 : 1;
    g->player.y = shortcut ? TOWN_ROAD_EXIT_Y : 12;
    push_message(g, town == LOCATION_TOWN2 ? "You arrive in Stillbury." : "You arrive in OakHaven.");
}

void game_leave_mountains(GameState *g, Location town, int shortcut) {
    return_to_town(g, town);
    g->player.x = shortcut ? (town == LOCATION_TOWN4 ? RIDGESHIRE_MOUNTAIN_ROAD_X : TOWN4_ROAD_X) : 20;
    g->player.y = town == LOCATION_TOWN4 ? TOWN_H - 2 : 1;
    push_message(g, town == LOCATION_TOWN4 ? "You arrive in Ridgeshire." : "You arrive in OakHaven.");
}

void game_enter_town3(GameState *g) {
    return_to_town(g, LOCATION_TOWN3);
}

void game_leave_swamp(GameState *g, Location town, int shortcut) {
    return_to_town(g, town);
    g->player.x = shortcut ? (town == LOCATION_TOWN3 ? ROSEMOOR_SWAMP_ROAD_X : TOWN3_ROAD_X) : 20;
    g->player.y = town == LOCATION_TOWN3 ? TOWN_H - 2 : 1;
    push_message(g, town == LOCATION_TOWN3 ? "You arrive in Rosemoor." : "You arrive in Stillbury.");
}

void game_enter_swamp_road(GameState *g) {
    int from_town2 = g->location == LOCATION_TOWN2;
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
    push_message(g, from_town2 ? "The safe swamp trail leads north to Rosemoor." :
        "The safe swamp trail leads south to Stillbury.");
}

void game_leave_swamp_road(GameState *g, Location destination) {
    return_to_town(g, destination);
    g->player.x = destination == LOCATION_TOWN3 ? ROSEMOOR_SWAMP_ROAD_X : TOWN3_ROAD_X;
    g->player.y = destination == LOCATION_TOWN3 ? TOWN_H - 2 : 1;
    push_message(g, destination == LOCATION_TOWN3 ?
        "You arrive in Rosemoor." : "You return to Stillbury.");
}

void game_enter_town4(GameState *g) {
    return_to_town(g, LOCATION_TOWN4);
}

void game_leave_high_pass(GameState *g, Location destination) {
    return_to_town(g, destination);
    g->player.x = destination == LOCATION_TOWN4 ? RIDGESHIRE_MOUNTAIN_ROAD_X : TOWN4_ROAD_X;
    g->player.y = destination == LOCATION_TOWN4 ? TOWN_H - 2 : 1;
    push_message(g, destination == LOCATION_TOWN4 ?
        "You arrive in Ridgeshire." : "You return to OakHaven.");
}

void game_open_town_portal(GameState *g) {
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
        g->location != LOCATION_MOONVEIL &&
        g->location != LOCATION_ASHEN &&
        g->location != LOCATION_GLASSDEEP &&
        g->location != LOCATION_CATACOMBS &&
        g->location != LOCATION_DRAGONSPINE) {
        return;
    }
    game_hide_portal_destination(g);
    if (g->location == LOCATION_DESERT || g->location == LOCATION_MOONVEIL ||
        g->location == LOCATION_ASHEN || g->location == LOCATION_GLASSDEEP ||
        g->location == LOCATION_CATACOMBS) {
        clear_floor_loot(g);
    }
    g->portal_active = 1;
    g->portal_level = g->level;
    g->portal_location = g->location;
    if (g->location == LOCATION_FOREST) {
        g->forest_portal_town = g->forest_entry_town;
    } else if (g->location == LOCATION_SWAMP) {
        g->swamp_portal_town = g->swamp_entry_town;
    } else if (g->location == LOCATION_MOUNTAINS) {
        g->mountain_portal_town = g->mountain_entry_town;
    }
    g->portal_x = g->player.x;
    g->portal_y = g->player.y;
    g->portal_origin_tile = g->map.tiles[g->player.y][g->player.x];
    game_return_to_town(g);
    push_message(g, "A return portal remains open.");
}

void game_hide_portal_destination(GameState *g) {
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
    } else if (g->portal_location == LOCATION_MOONVEIL) {
        if (g->portal_level > MOONVEIL_DEPTH) {
            return;
        }
        cache = g->moonveil_cache;
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
    if (g->portal_location == LOCATION_CASTLE_INTERIOR) {
        g->portal_active = 0;
        push_message(g, "The castle has severed this portal.");
        return;
    }
    if (!g->portal_active || g->portal_level < 1 ||
        g->portal_level > MAX_REGION_DEPTH) return;
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
    } else if (g->portal_location == LOCATION_MOONVEIL) {
        if (g->portal_level > MOONVEIL_DEPTH) {
            return;
        }
        cache = g->moonveil_cache;
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
        g->portal_location == LOCATION_MOONVEIL || g->portal_location == LOCATION_ASHEN ||
        g->portal_location == LOCATION_GLASSDEEP || g->portal_location == LOCATION_CATACOMBS) {
        clear_floor_loot(g);
    }
    g->location = g->portal_location;
    if (g->location == LOCATION_FOREST) {
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
            if (e->type == ENEMY_GRAVE_MARSHAL && catacombs_sweep_marks(&g->map, e, landing_x, landing_y)) {
                e->attack_target_x = -1;
                e->attack_target_y = -1;
                e->move_timer = 1;
            }
        }
    }
    cache[level - 1].map = g->map;
    g->portal_active = 0;
}

void game_talk_to_cain(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Cain");
    g->dialogue_x = TOWN_CAIN_X;
    g->dialogue_y = TOWN_CAIN_Y;
    const char *warning = "Goblins lurk north, beasts west, undead east, and sea horrors south.";
    if (g->cain_scroll_given) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s Read your scroll to learn Return to Town. Use it when danger grows!", warning);
    } else if (g->inventory_count >= MAX_INVENTORY) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s Make room in your pack, then speak to me for a Return to Town scroll.", warning);
    } else {
        g->inventory[g->inventory_count++] = item_make_scroll_return_to_town();
        g->cain_scroll_given = 1;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s Take this Return to Town scroll for your adventure. Read it to learn a way home!", warning);
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
    return g->island_travel_unlocked || game_has_treasure_map(g);
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
            push_message(g, "The broken guardian faces the ruined temple gate.");
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
        g->temple_treasure_state = 1;
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
        push_message(g, "Quest assigned: The Buried Sun.");
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
    g->dialogue_x = TOWN_ROWAN_X;
    g->dialogue_y = TOWN_ROWAN_Y;
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
        "Defeat her and I can take you to the island beyond these shores.");
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
    if (g->location != LOCATION_GUILD) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Zara");
    g->dialogue_x = GUILD_ZARA_X;
    g->dialogue_y = GUILD_ZARA_Y;
    if (g->sunscar_lamp_quest_state == 0) {
        g->sunscar_lamp_quest_state = 1;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Recover a magic lamp from Sunscar Wastes level 4. Enter through "
            "Stillbury's west gate. Stand on the lamp and press A, then return "
            "to me here for 80 gold.");
        push_message(g, "Assigned: The Lost Magic Lamp.");
    } else if (g->sunscar_lamp_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The magic lamp lies in the last clearing of Sunscar Wastes level 4. "
            "Return it to me at the Guild in Rosemoor.");
    } else if (g->sunscar_lamp_quest_state == 2) {
        g->sunscar_lamp_quest_state = 3;
        g->gold += 80;
        g->score += 600;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You found the lamp! The Guild will keep it safe. Here are your 80 gold.");
        push_message(g, "Completed: The Lost Magic Lamp. 80 gold awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The magic lamp is safe with the Guild. Thank you for recovering it.");
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
    push_message(g, "Magic lamp recovered. Return to Zara at the Guild in Rosemoor.");
}

void game_talk_to_dragon_seeker(GameState *g) {
    if (g->location != LOCATION_TOWN4) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Ilya");
    g->dialogue_x = TOWN4_ILYA_X;
    g->dialogue_y = TOWN4_ILYA_Y;
    if (g->dragon_treasure_quest_state == 0) {
        g->dragon_treasure_quest_state = 1;
        if (g->portal_active && g->portal_location == LOCATION_DRAGONSPINE &&
            g->map.tiles[TOWN4_PORTAL_Y][TOWN4_PORTAL_X] == TILE_PORTAL) {
            g->map.tiles[TOWN4_PORTAL_Y][TOWN4_PORTAL_X] = TILE_TOWN_FLOOR;
        }
        prepare_quest_expedition(g, LOCATION_DRAGONSPINE);
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "A golden goblet lies in the hoard atop Dragonspine. "
            "Bring it back and I will give you a Potion of Strength. "
            "Dragonspine lies beyond this town's east gate.");
        push_message(g, "Assigned: The Dragon's Hoard.");
    } else if (g->dragon_treasure_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Find the golden goblet in the dragon's hoard on Dragonspine's "
            "fifth stage. Stand on it and press A, then return to me.");
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
            "Dragonspine's treasure is safe. The east gate is open to you.");
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
    push_message(g, "Golden goblet recovered. Return it to Ilya in Ridgeshire.");
}

void game_talk_to_elowen(GameState *g) {
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Elowen", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = 10;
    g->dialogue_y = 7;
    if (g->elowen_quest_state == 0) {
        g->elowen_quest_state = 1;
        g->elowen_seals_restored = 0;
        prepare_quest_expedition(g, LOCATION_DUNGEON);
        strncpy(g->dialogue_text,
            "The dead have gathered around shattered burial seals on dungeon floors 2, 3, and 4. Break through them and restore each seal.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Quest assigned: The Broken Seals.");
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
            "You have restored %d of 3 burial seals. Return when all three are whole.",
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
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Dain", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = 18;
    g->dialogue_y = 7;
    if (g->dain_quest_state == 0) {
        g->dain_quest_state = 1;
        g->dain_map_fragments = 0;
        prepare_quest_expedition(g, LOCATION_MOUNTAINS);
        strncpy(g->dialogue_text,
            "Three goblin warbands carry pieces of an old dwarven map. Hunt their leaders on mountain stages 1, 2, and 3, between OakHaven and the peak.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Assigned: Recover the Treasure Map.");
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
            "You have recovered %d of 3 map fragments. The remaining pieces are still carried through the mountains.",
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
        push_message(g, "Treasure map complete. Return to Dain.");
    }
}

void game_talk_to_alder(GameState *g) {
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Alder", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = 28;
    g->dialogue_y = 7;
    if (g->alder_quest_state == 0) {
        g->alder_quest_state = 1;
        g->alder_wardens_rescued = 0;
        prepare_quest_expedition(g, LOCATION_FOREST);
        strncpy(g->dialogue_text,
            "Three of my wardens are trapped on forest stages 1, 2, and 3, between OakHaven and the Necromancer. Defeat their captors and bring them home.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Assigned: The Lost Wardens.");
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
            "You have rescued %d of my 3 wardens. Search forest stages 1, 2, and 3 on the OakHaven side of the Necromancer.",
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
    if (g->level == 1) {
        strncpy(g->dialogue_text,
            "You cut through the spider web binding me. I can follow your trail home from here.",
            MAX_DIALOGUE_LEN - 1);
    } else if (g->level == 2) {
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
        push_message(g, "All wardens rescued. Return to Alder.");
    }
}

void game_talk_to_mara(GameState *g) {
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Mara", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = 31;
    g->dialogue_y = 18;
    if (g->mara_quest_state == 0) {
        g->mara_quest_state = 1;
        g->mara_beacons_lit = 0;
        prepare_quest_expedition(g, LOCATION_COAST);
        strncpy(g->dialogue_text,
            "Drowned guardians surround beacons on coast stages 2, 3, and 4. Lower the tide, defeat them, and relight each flame.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Assigned: Relight the Drowned Beacons.");
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
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Rook");
    g->dialogue_x = 10;
    g->dialogue_y = 18;
    if (g->rook_quest_state == 0) {
        assign_rook_quest(g);
    } else if (g->rook_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "One rune per floor opens the Minotaur's vault. Defeat the Minotaur and "
            "bring my ivory rook home.");
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
        g->innkeeper_quest_state = 1;
        prepare_quest_expedition(g, LOCATION_SWAMP);
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "A vampire holds Mira on swamp level 3, between Stillbury and the Demon. "
            "Defeat her captor, then speak to her and bring her home. You can retreat and return if needed.");
        push_message(g, "Assigned: Bring Mira Home.");
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
        push_message(g, "All beacons lit. Return to Mara.");
    }
}

void game_mark_level_cleared(GameState *g) {
    g->level_cleared = 1;
}

int game_shortcut_prompt_active(const GameState *g) {
    if (g->castle_prompt) {
        return 1;
    }
    return g->dialogue_active && strcmp(g->dialogue_speaker, "Shortcut found") == 0 &&
        (g->location == LOCATION_FOREST || g->location == LOCATION_SWAMP || g->location == LOCATION_MOUNTAINS);
}

int game_handle_shortcut_prompt_key(GameState *g, int key, int repeat) {
    if (castle_prompt_key(g, key, repeat)) {
        return 1;
    }
    if (!game_shortcut_prompt_active(g)) {
        return 0;
    }
    if (!repeat && (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER)) {
        g->dialogue_active = 0;
    }
    return 1;
}

static void reveal_boss_shortcut(GameState *g, Map *map, Enemy *enemies, int count, Location region, EnemyType boss, TileType shortcut, TileType floor) {
    if (!(g->defeated_bosses & (1 << region)) || map->room_count == 0) {
        return;
    }
    int x;
    int y;
    map_room_center(&map->rooms[map->room_count - 1], &x, &y);
    for (int i = 0; i < count; i++) {
        if (enemies[i].type == boss && !enemies[i].active && enemies[i].x >= 0 && enemies[i].x < MAP_W && enemies[i].y >= 0 && enemies[i].y < MAP_H) {
            x = enemies[i].x;
            y = enemies[i].y;
            break;
        }
    }
    int active = map == &g->map;
    int items = active ? g->floor_item_count : 0;
    int boss_level = region == LOCATION_FOREST ? FOREST_BOSS_LEVEL :
        (region == LOCATION_SWAMP ? SWAMP_BOSS_LEVEL : MOUNTAIN_BOSS_LEVEL);
    int portal = g->portal_active && g->portal_location == region && g->portal_level == boss_level;
    int sx = x;
    int sy = y;
    int found = 0;
    // Reuse a nearby entrance, including one covered by loot or a return portal.
    for (int row = y - 1; row <= y + 1 && !found; row++) {
        for (int column = x - 1; column <= x + 1; column++) {
            if (column < 0 || column >= MAP_W || row < 0 || row >= MAP_H || abs(column - x) + abs(row - y) > 1) {
                continue;
            }
            int marked = map->tiles[row][column] == shortcut ||
                (portal && column == g->portal_x && row == g->portal_y && g->portal_origin_tile == shortcut);
            for (int i = 0; i < items; i++) {
                FloorItem *item = &g->floor_items[i];
                marked |= item->active && item->x == column && item->y == row && item->underlying_tile == shortcut;
            }
            if (marked) {
                sx = column;
                sy = row;
                found = 1;
                break;
            }
        }
    }
    static const int offsets[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    for (int direction = 0; direction < 4 && !found; direction++) {
        int nx = x + offsets[direction][0];
        int ny = y + offsets[direction][1];
        if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) {
            continue;
        }
        TileType tile = map->tiles[ny][nx];
        if (tile != floor && tile != TILE_MOUNTAIN_FLOOR && tile != TILE_MOUNTAIN_CAVE_FLOOR && tile != TILE_MOUNTAIN_BRIDGE) {
            continue;
        }
        int occupied = 0;
        for (int i = 0; i < count; i++) {
            occupied |= enemies[i].active && enemies[i].x == nx && enemies[i].y == ny;
        }
        if (!occupied) {
            sx = nx;
            sy = ny;
            found = 1;
        }
    }
    // Repair the former fixed entrance when loading or revisiting saved maps.
    for (int row = 0; row < MAP_H; row++) {
        for (int column = 0; column < MAP_W; column++) {
            if (map->tiles[row][column] == shortcut && (column != sx || row != sy)) {
                map->tiles[row][column] = floor;
            }
        }
    }
    for (int i = 0; i < items; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->underlying_tile == shortcut && (item->x != sx || item->y != sy)) {
            item->underlying_tile = floor;
        }
    }
    if (portal && g->portal_origin_tile == shortcut && (g->portal_x != sx || g->portal_y != sy)) {
        g->portal_origin_tile = floor;
    }
    int covered = portal && g->portal_x == sx && g->portal_y == sy && map->tiles[sy][sx] == TILE_PORTAL;
    if (covered) {
        g->portal_origin_tile = shortcut;
    }
    for (int i = 0; i < items; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == sx && item->y == sy) {
            item->underlying_tile = shortcut;
            covered = 1;
        }
    }
    if (!covered) {
        map->tiles[sy][sx] = shortcut;
    }
    map_mark_explored(map, sx, sy);
}

void game_reveal_forest_shortcut(GameState *g) {
    if (g->location != LOCATION_FOREST || g->level != FOREST_BOSS_LEVEL) {
        return;
    }
    if (g->defeated_bosses & (1 << LOCATION_FOREST)) {
        map_reveal_forest_entrance(&g->map);
        map_reveal_forest_exit(&g->map);
    }
    reveal_boss_shortcut(g, &g->map, g->enemies, g->enemy_count, LOCATION_FOREST, ENEMY_FOREST_NECROMANCER, TILE_FOREST_SHORTCUT, TILE_FOREST_FLOOR);
}

void game_reveal_swamp_shortcut(GameState *g) {
    if (g->location == LOCATION_SWAMP && g->level == SWAMP_BOSS_LEVEL) {
        reveal_boss_shortcut(g, &g->map, g->enemies, g->enemy_count, LOCATION_SWAMP, ENEMY_SWAMP_DEMON, TILE_SWAMP_SHORTCUT, TILE_SWAMP_FLOOR);
    }
}

void game_reveal_mountain_shortcut(GameState *g) {
    if (g->location == LOCATION_MOUNTAINS && g->level == MOUNTAIN_BOSS_LEVEL) {
        reveal_boss_shortcut(g, &g->map, g->enemies, g->enemy_count, LOCATION_MOUNTAINS, ENEMY_MOUNTAIN_GOBLIN_KING, TILE_MOUNTAIN_SHORTCUT, TILE_MOUNTAIN_FORTRESS_FLOOR);
    }
}

void game_migrate_boss_shortcuts(GameState *g) {
    LevelCache *caches[3] = {&g->forest_cache[FOREST_BOSS_LEVEL - 1], &g->swamp_cache[SWAMP_BOSS_LEVEL - 1], &g->mountain_cache[MOUNTAIN_BOSS_LEVEL - 1]};
    const Location regions[3] = {LOCATION_FOREST, LOCATION_SWAMP, LOCATION_MOUNTAINS};
    const EnemyType bosses[3] = {ENEMY_FOREST_NECROMANCER, ENEMY_SWAMP_DEMON, ENEMY_MOUNTAIN_GOBLIN_KING};
    const TileType shortcuts[3] = {TILE_FOREST_SHORTCUT, TILE_SWAMP_SHORTCUT, TILE_MOUNTAIN_SHORTCUT};
    const TileType floors[3] = {TILE_FOREST_FLOOR, TILE_SWAMP_FLOOR, TILE_MOUNTAIN_FORTRESS_FLOOR};
    for (int i = 0; i < 3; i++) {
        LevelCache *cache = caches[i];
        if (cache->valid) {
            reveal_boss_shortcut(g, &cache->map, cache->enemies, cache->enemy_count, regions[i], bosses[i], shortcuts[i], floors[i]);
        }
    }
}

void game_update_level_progress(GameState *g) {
    if (g->defeated_bosses & (1 << g->location)) {
        if (g->location == LOCATION_FOREST && g->level == FOREST_BOSS_LEVEL) {
            game_reveal_forest_shortcut(g);
        } else if (g->location == LOCATION_SWAMP && g->level == SWAMP_BOSS_LEVEL) {
            game_reveal_swamp_shortcut(g);
        } else if (g->location == LOCATION_MOUNTAINS && g->level == MOUNTAIN_BOSS_LEVEL) {
            game_reveal_mountain_shortcut(g);
        } else if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH &&
            g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] != TILE_RETURN_EXIT) {
            g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_RETURN_EXIT;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == g->map.stairs_down_x &&
                    item->y == g->map.stairs_down_y) {
                    item->underlying_tile = TILE_RETURN_EXIT;
                }
            }
            push_message(g, "A passage to town opens!");
        }
    }

    int active_enemies = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active || (g->location == LOCATION_CATACOMBS && g->enemies[i].revive_timer > 0)) {
            active_enemies++;
        }
    }

    if (active_enemies == 0) {
        game_mark_level_cleared(g);
    } else if (g->location == LOCATION_CATACOMBS) {
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
