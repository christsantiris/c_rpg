#ifndef ENEMY_HEADER_H
#define ENEMY_HEADER_H

#define MAX_ENEMIES 15

typedef enum {
    ENEMY_SKELETON = 0,
    ENEMY_GOBLIN,
    ENEMY_ZOMBIE,
    ENEMY_ORC,
    ENEMY_TROLL,
    ENEMY_GIANT,
    ENEMY_GOBLIN_KING,
    ENEMY_LICH_KING,
    ENEMY_DEMON_LORD,
    ENEMY_RED_DRAGON,
    ENEMY_TARRASQUE,
    ENEMY_CRYPT_BAT,
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
    ENEMY_DROWNED_QUEEN
} EnemyType;

typedef struct {
    int       x, y;
    int       active;
    EnemyType type;
    char      name[16];
    int       hp, max_hp;
    int       attack, defense;
    int       experience;
    int       move_timer;
    int       is_boss;
} Enemy;

#endif
