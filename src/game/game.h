#ifndef GAME_HEADER_H
#define GAME_HEADER_H

#include "map.h"
#include "enemy.h"
#include "item.h"
#include "actions.h"
#include <stdio.h>
#include "spell.h"
#include <SDL2/SDL.h>

#define MAX_MESSAGES 3
#define MAX_MESSAGE_LEN 40
#define MAX_DIALOGUE_LEN 192
#define MAX_SPEAKER_LEN 24

#define MAX_TRAIL 16

typedef struct {
    int active;
    int x, y;
    Uint8 r, g, b;
    int is_impact;
} TrailTile;

typedef enum {
    TRAIL_EFFECT_GENERIC = 0,
    TRAIL_EFFECT_WEAPON_ARROW,
    TRAIL_EFFECT_MAGIC_ARROW,
    TRAIL_EFFECT_FIREBALL
} TrailEffect;

typedef enum {
    CLASS_WARRIOR = 0,
    CLASS_MAGE,
    CLASS_ROGUE
} PlayerClass;

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
    int poison_turns;
    PlayerClass player_class;
} Player;

typedef struct {
    Map   map;
    Enemy enemies[MAX_ENEMIES];
    int   enemy_count;
    int   valid;
    int   level_cleared;
} LevelCache;

typedef enum {
    LOCATION_TOWN,
    LOCATION_DUNGEON,
    LOCATION_FOREST,
    LOCATION_MOUNTAINS,
    LOCATION_COAST,
    LOCATION_TAVERN
} Location;

typedef struct {
    Player     player;
    Map        map;
    int        level;
    Enemy      enemies[MAX_ENEMIES];
    int        enemy_count;
    LevelCache level_cache[MAX_REGION_DEPTH];
    LevelCache forest_cache[MAX_REGION_DEPTH];
    LevelCache mountain_cache[MAX_REGION_DEPTH];
    LevelCache coast_cache[MAX_REGION_DEPTH];
    char       messages[MAX_MESSAGES][MAX_MESSAGE_LEN];
    int        message_count;
    int        level_cleared;
    Location   location;
    int max_level_reached;
    int max_forest_level_reached;
    int max_mountain_level_reached;
    int max_coast_level_reached;
    Item      inventory[MAX_INVENTORY];
    int       inventory_count;
    int       equipped_weapon;
    int       equipped_armor;
    int       gold;
    FloorItem floor_items[MAX_FLOOR_ITEMS];
    int       floor_item_count;
    TrailTile trail[MAX_TRAIL];
    int       trail_count;
    int       trail_frames;
    TrailEffect trail_effect;
    Uint32    trail_started_at;
    int score;
    int dungeon_key_found;
    int portal_active;
    int portal_level;
    Location portal_location;
    int portal_x, portal_y;
    TileType portal_origin_tile;
    int defeated_bosses;
    int elowen_quest_state;
    int elowen_seals_restored;
    int dialogue_active;
    char dialogue_speaker[MAX_SPEAKER_LEN];
    char dialogue_text[MAX_DIALOGUE_LEN];
} GameState;

void game_init(GameState *g);
void game_move_player(GameState *g, int dx, int dy);
void game_descend(GameState *g);
void game_ascend(GameState *g);
void enemies_spawn(GameState *g);
void game_enter_dungeon(GameState *g);
void game_enter_forest(GameState *g);
void game_enter_mountains(GameState *g);
void game_enter_coast(GameState *g);
void game_enter_tavern(GameState *g);
void game_leave_tavern(GameState *g);

void action_resolve_player(GameState *g, Action a);
void action_resolve_enemies(GameState *g);

void player_gain_xp(GameState *g, int xp);
void push_message(GameState *g, const char *msg);
void game_mark_level_cleared(GameState *g);
void game_update_level_progress(GameState *g);

void game_return_to_town(GameState *g);
void game_open_town_portal(GameState *g);
void game_use_town_portal(GameState *g);
void game_talk_to_elowen(GameState *g);
void game_repair_equipment_indices(GameState *g);

#endif
