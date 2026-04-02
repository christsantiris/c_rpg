#ifndef GAME_HEADER_H
#define GAME_HEADER_H

#include "map.h"
#include "enemy.h"
#include "item.h"
#include "actions.h"
#include <stdio.h>
#include "spell.h"

#define MAX_MESSAGES 3
#define MAX_MESSAGE_LEN 40

typedef struct {
    int   x, y;
    char  name[21];
    int   hp,     max_hp;
    int   mp,     max_mp;
    int   attack, defense;
    int   level,  experience, experience_next;
    Spell known_spells[MAX_SPELLS];
    int   known_spell_count;
    int   equipped_spell;
    int   last_dx, last_dy;
} Player;

typedef struct {
    Map   map;
    Enemy enemies[MAX_ENEMIES];
    int   enemy_count;
    int   valid;
} LevelCache;

typedef enum {
    LOCATION_TOWN,
    LOCATION_DUNGEON
} Location;

typedef struct {
    Player     player;
    Map        map;
    int        level;
    Enemy      enemies[MAX_ENEMIES];
    int        enemy_count;
    LevelCache level_cache[MAX_DEPTH];
    char       messages[MAX_MESSAGES][MAX_MESSAGE_LEN];
    int        message_count;
    int        level_cleared;
    Location   location;
    int max_level_reached;
    Item      inventory[MAX_INVENTORY];
    int       inventory_count;
    int       equipped_weapon;
    int       equipped_armor;
    int       gold;
    FloorItem floor_items[MAX_FLOOR_ITEMS];
    int       floor_item_count;
} GameState;

void game_init(GameState *g);
void game_move_player(GameState *g, int dx, int dy);
void game_descend(GameState *g);
void game_ascend(GameState *g);
void enemies_spawn(GameState *g);
void game_enter_dungeon(GameState *g);

void action_resolve_player(GameState *g, Action a);
void action_resolve_enemies(GameState *g);

void player_gain_xp(GameState *g, int xp);
void push_message(GameState *g, const char *msg);

#endif