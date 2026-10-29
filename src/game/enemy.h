#ifndef ENEMY_HEADER_H
#define ENEMY_HEADER_H

#include "map.h"

// Sunscar needs larger caches; other regions retain their spawning limits.
#define NON_ROAD_ENEMY_LIMIT 30
#define LEVEL_CACHE_ENEMY_CAPACITY 48
#define CROWNROAD_BASE_ENEMIES 30
#define CROWNROAD_ENEMIES (CROWNROAD_BASE_ENEMIES * CROWNROAD_LENGTH_SCALE)
#define MAX_ENEMIES CROWNROAD_ENEMIES
#define AREA_ENEMY_LIMIT 15

typedef enum {
    // Explicit serialized IDs keep retired slots reserved for save migrations.
    ENEMY_SKELETON = 0,
    ENEMY_ZOMBIE = 2,
    ENEMY_STONE_GIANT = 5,
    ENEMY_LICH_KING = 7,
    ENEMY_RED_DRAGON = 9,
    ENEMY_CRYPT_BAT = 11,
    ENEMY_WRAITH = 12,
    ENEMY_CRYPT_CONJURER = 13,
    ENEMY_PIXIE = 14,
    ENEMY_BLIGHTED_WOLF = 15,
    ENEMY_GIANT_SPIDER = 16,
    ENEMY_DARK_ELF_ARCHER = 17,
    ENEMY_FOREST_TROLL = 19,
    ENEMY_FOREST_NECROMANCER = 20,
    ENEMY_GOBLIN_SCOUT = 21,
    ENEMY_GOBLIN_ARCHER = 22,
    ENEMY_CAVE_TROLL = 25,
    ENEMY_HOBGOBLIN_GUARD = 26,
    ENEMY_GOBLIN_SHAMAN = 27,
    ENEMY_MOUNTAIN_GOBLIN_KING = 28,
    ENEMY_ILLUSION = 29,
    ENEMY_MERFOLK = 30,
    ENEMY_SIREN = 31,
    ENEMY_GIANT_CRAB = 32,
    ENEMY_ANIMATED_STATUE = 33,
    ENEMY_WATER_ELEMENTAL = 34,
    ENEMY_SEA_SERPENT = 35,
    ENEMY_DROWNED_QUEEN = 36,
    ENEMY_RELIC_SCARABS = 37,
    ENEMY_TEMPLE_STALKER = 38,
    ENEMY_BLOWDART_HUNTER = 39,
    ENEMY_VINEBOUND_GUARDIAN = 40,
    ENEMY_SUN_PRIEST = 41,
    ENEMY_SERPENT_SPIRIT = 42,
    ENEMY_LUNAR_EFFIGY = 44,
    ENEMY_MOONBOUND_SENTINEL = 45,
    ENEMY_FALLEN_SUN_GUARDIAN = 46,
    ENEMY_MINOTAUR = 47,
    ENEMY_GIANT_RAT = 48,
    ENEMY_BANDIT = 49,
    ENEMY_VAMPIRE = 50,
    ENEMY_SWAMP_DEMON = 51,
    ENEMY_DRAKE = 52,
    ENEMY_FIRE_ELEMENTAL = 53,
    ENEMY_ROAD_ARCHER = 54,
    ENEMY_HORSEMAN = 55,
    ENEMY_FROST_ARCHER = 57,
    ENEMY_YETI = 58,
    ENEMY_FROST_WRAITH = 59,
    ENEMY_ICE_GOLEM = 60,
    ENEMY_FROST_GIANT = 61,
    ENEMY_FROST_KRAKEN = 62,
    ENEMY_SCARAB = 63,
    ENEMY_VIPER = 64,
    ENEMY_MUMMY = 65,
    ENEMY_DJINN = 66,
    ENEMY_GOLEM = 67,
    ENEMY_DESERT_PHARAOH = 68,
    ENEMY_CINDER_IMP = 74,
    ENEMY_ASH_HOUND = 75,
    ENEMY_OBSIDIAN_GUARDIAN = 76,
    ENEMY_CINDER_LORD = 77,
    ENEMY_BLIND_STALKER = 79,
    ENEMY_SHARD_GOLEM = 80,
    ENEMY_PRISM_SOVEREIGN = 81,
    ENEMY_ANCIENT_SKELETON = 82,
    ENEMY_BONE_SENTINEL = 83,
    ENEMY_GRAVE_MARSHAL = 86,
    ENEMY_OATHBOUND_SOLDIER = 87,
    ENEMY_IRON_WARDEN = 88,
    ENEMY_ROYAL_MARKSMAN = 89,
    ENEMY_COURT_HEXER = 90,
    ENEMY_ROYAL_ARCANIST = 93,
    ENEMY_LORD_VEYR = 94,
    ENEMY_THORN_DRYAD = 97,
    ENEMY_TREEFOLK = 98,
    ENEMY_ELDERTHORN = 99,
    ENEMY_ROYAL_GUARD = 100,
    ENEMY_LIZARDFOLK_SCOUT = 107,
    ENEMY_REED_SPEARHUNTER = 108,
    ENEMY_MARSH_SPITTER = 109,
    ENEMY_LIZARDFOLK_SCALEGUARD = 111,
    ENEMY_SSZARETH = 112,
    ENEMY_HARPY_RAIDER = 114,
    ENEMY_CLIFF_GARGOYLE = 115,
    ENEMY_HARPY_STORMCALLER = 116,
    ENEMY_STORM_ELEMENTAL = 117,
    ENEMY_TEMPEST_MATRIARCH = 118,
    ENEMY_GOBLIN_MINE_BOSS = 119,
    ENEMY_OGRE = 120,
    ENEMY_HILL_GIANT = 121,
    ENEMY_LAST_JARL = 122,
    ENEMY_DARK_ELF_RANGER = 123,
    ENEMY_GOBLIN_WARCHIEF = 124
} EnemyType;

#define DRAGON_WARNING_RADIUS 12
#define DRAGON_WARNING_SIZE 2
#define DRAGON_WARNING_WORD_BITS 21

typedef struct {
    int       x, y;
    int       active;
    EnemyType type;
    char name[32];
    int       hp, max_hp;
    int       attack, defense;
    int       experience;
    int       move_timer;
    int       is_boss;
    int       dain_fragment;
    int       frozen_turns;
    int poison_turns;
    int alignment_dormant;
    int attack_target_x, attack_target_y;
    int revived;
    int revive_timer;
    int facing_dx, facing_dy;
    int attack_phase;
    // Regional boss signature, finite-wave flags and second summon position.
    int boss_attack;
    int boss_wave_used;
    int boss_aux_x, boss_aux_y;
    unsigned int dragon_warning[DRAGON_WARNING_SIZE]; // 42 fixed cells in the dragon's facing coordinates.
} Enemy;

#endif
