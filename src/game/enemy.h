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
    // Keep retired serialized IDs reserved so existing actors retain their meanings.
    ENEMY_SKELETON = 0,
    ENEMY_ZOMBIE = 2,
    ENEMY_STONE_GIANT = 5,
    ENEMY_LICH_KING = 7,
    ENEMY_RED_DRAGON = 9,
    ENEMY_CRYPT_BAT = 11,
    ENEMY_WRAITH,
    ENEMY_CRYPT_CONJURER,
    ENEMY_PIXIE,
    ENEMY_BLIGHTED_WOLF,
    ENEMY_GIANT_SPIDER,
    ENEMY_DARK_ELF,
    ENEMY_GIANT_WURM,
    ENEMY_FOREST_TROLL,
    ENEMY_FOREST_NECROMANCER,
    ENEMY_GOBLIN_SCOUT,
    ENEMY_GOBLIN_ARCHER,
    ENEMY_GOBLIN_BOMBER,
    ENEMY_TUNNEL_SPIDER,
    ENEMY_CAVE_TROLL,
    ENEMY_HOBGOBLIN_GUARD,
    ENEMY_GOBLIN_SHAMAN,
    ENEMY_MOUNTAIN_GOBLIN_KING,
    ENEMY_ILLUSION,
    ENEMY_MERFOLK,
    ENEMY_SIREN,
    ENEMY_GIANT_CRAB,
    ENEMY_ANIMATED_STATUE,
    ENEMY_WATER_ELEMENTAL,
    ENEMY_SEA_SERPENT,
    ENEMY_DROWNED_QUEEN,
    ENEMY_RELIC_SCARABS,
    ENEMY_TEMPLE_STALKER,
    ENEMY_BLOWDART_HUNTER,
    ENEMY_VINEBOUND_GUARDIAN,
    ENEMY_SUN_PRIEST,
    ENEMY_SERPENT_SPIRIT,
    ENEMY_TREASURE_WRAITH,
    ENEMY_LUNAR_EFFIGY,
    ENEMY_MOONBOUND_SENTINEL,
    ENEMY_FALLEN_SUN_GUARDIAN,
    ENEMY_MINOTAUR, // Keep the former Maze Warden's saved numeric ID.
    ENEMY_GIANT_RAT,
    ENEMY_BANDIT,
    ENEMY_VAMPIRE,
    ENEMY_SWAMP_DEMON,
    ENEMY_DRAKE,
    ENEMY_FIRE_ELEMENTAL,
    ENEMY_ROAD_ARCHER,
    ENEMY_HORSEMAN,
    ENEMY_ICE_WOLF,
    ENEMY_FROST_ARCHER,
    ENEMY_YETI,
    ENEMY_FROST_WRAITH,
    ENEMY_ICE_GOLEM,
    ENEMY_FROST_GIANT,
    ENEMY_FROST_KRAKEN,
    ENEMY_SCARAB,
    ENEMY_VIPER,
    ENEMY_MUMMY,
    ENEMY_DJINN,
    ENEMY_GOLEM,
    ENEMY_DESERT_PHARAOH,
    ENEMY_CINDER_IMP = 74,
    ENEMY_ASH_HOUND,
    ENEMY_OBSIDIAN_GUARDIAN,
    ENEMY_CINDER_LORD,
    ENEMY_CRYSTAL_SPIDER,
    ENEMY_BLIND_STALKER,
    ENEMY_SHARD_GOLEM,
    ENEMY_PRISM_SOVEREIGN,
    ENEMY_ANCIENT_SKELETON,
    ENEMY_BONE_SENTINEL,
    // Reserve two retired serialized IDs.
    ENEMY_GRAVE_MARSHAL = ENEMY_BONE_SENTINEL + 3,
    ENEMY_OATHBOUND_SOLDIER,
    ENEMY_IRON_WARDEN,
    ENEMY_ROYAL_MARKSMAN,
    ENEMY_COURT_HEXER,
    ENEMY_BELL_HERALD,
    ENEMY_CASTELLAN,
    ENEMY_ROYAL_ARCANIST,
    ENEMY_LORD_VEYR,
    ENEMY_BRIAR_WOLF,
    ENEMY_IRONHIDE_BOAR,
    ENEMY_THORN_DRYAD,
    ENEMY_TREEFOLK,
    ENEMY_ELDERTHORN,
    ENEMY_ROYAL_GUARD,
    // Skip the six retired pirate enemy IDs.
    ENEMY_LIZARDFOLK_SCOUT = ENEMY_ROYAL_GUARD + 7,
    ENEMY_REED_SPEARHUNTER,
    ENEMY_MARSH_SPITTER,
    // Reserve the retired shore creature ID.
    ENEMY_LIZARDFOLK_SCALEGUARD = ENEMY_MARSH_SPITTER + 2,
    ENEMY_SSZARETH,
    ENEMY_IRON_SENTINEL,
    ENEMY_HARPY_RAIDER,
    ENEMY_CLIFF_GARGOYLE,
    ENEMY_HARPY_STORMCALLER,
    ENEMY_STORM_ELEMENTAL,
    ENEMY_TEMPEST_MATRIARCH,
    ENEMY_GOBLIN_MINE_BOSS,
    ENEMY_OGRE,
    ENEMY_HILL_GIANT,
    ENEMY_LAST_JARL
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
