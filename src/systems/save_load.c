#include "save_load.h"
#include "../game/game.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char *slot_path(int slot) {
    static char path[64];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", slot);
    return path;
}

int save_exists(int slot) {
    FILE *f = fopen(slot_path(slot), "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

static cJSON *serialize_map(const Map *m) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "room_count",    m->room_count);
    cJSON_AddNumberToObject(obj, "stairs_up_x",   m->stairs_up_x);
    cJSON_AddNumberToObject(obj, "stairs_up_y",   m->stairs_up_y);
    cJSON_AddNumberToObject(obj, "stairs_down_x", m->stairs_down_x);
    cJSON_AddNumberToObject(obj, "stairs_down_y", m->stairs_down_y);

    // Rooms
    cJSON *rooms = cJSON_CreateArray();
    for (int i = 0; i < m->room_count; i++) {
        cJSON *r = cJSON_CreateObject();
        cJSON_AddNumberToObject(r, "x", m->rooms[i].x);
        cJSON_AddNumberToObject(r, "y", m->rooms[i].y);
        cJSON_AddNumberToObject(r, "w", m->rooms[i].w);
        cJSON_AddNumberToObject(r, "h", m->rooms[i].h);
        cJSON_AddItemToArray(rooms, r);
    }
    cJSON_AddItemToObject(obj, "rooms", rooms);

    // Tiles as flat array
    cJSON *tiles = cJSON_CreateArray();
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            cJSON_AddItemToArray(tiles, cJSON_CreateNumber(m->tiles[y][x]));
    cJSON_AddItemToObject(obj, "tiles", tiles);

    return obj;
}

static void deserialize_map(const cJSON *obj, Map *m) {
    m->room_count    = cJSON_GetObjectItem(obj, "room_count")->valueint;
    m->stairs_up_x   = cJSON_GetObjectItem(obj, "stairs_up_x")->valueint;
    m->stairs_up_y   = cJSON_GetObjectItem(obj, "stairs_up_y")->valueint;
    m->stairs_down_x = cJSON_GetObjectItem(obj, "stairs_down_x")->valueint;
    m->stairs_down_y = cJSON_GetObjectItem(obj, "stairs_down_y")->valueint;

    cJSON *rooms = cJSON_GetObjectItem(obj, "rooms");
    for (int i = 0; i < m->room_count; i++) {
        cJSON *r = cJSON_GetArrayItem(rooms, i);
        m->rooms[i].x = cJSON_GetObjectItem(r, "x")->valueint;
        m->rooms[i].y = cJSON_GetObjectItem(r, "y")->valueint;
        m->rooms[i].w = cJSON_GetObjectItem(r, "w")->valueint;
        m->rooms[i].h = cJSON_GetObjectItem(r, "h")->valueint;
    }

    cJSON *tiles = cJSON_GetObjectItem(obj, "tiles");
    int idx = 0;
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            m->tiles[y][x] = cJSON_GetArrayItem(tiles, idx++)->valueint;
}

static cJSON *serialize_enemies(const Enemy *enemies, int count) {
    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
        const Enemy *e = &enemies[i];
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "x",          e->x);
        cJSON_AddNumberToObject(obj, "y",          e->y);
        cJSON_AddNumberToObject(obj, "active",     e->active);
        cJSON_AddNumberToObject(obj, "type",       e->type);
        cJSON_AddStringToObject(obj, "name",       e->name);
        cJSON_AddNumberToObject(obj, "hp",         e->hp);
        cJSON_AddNumberToObject(obj, "max_hp",     e->max_hp);
        cJSON_AddNumberToObject(obj, "attack",     e->attack);
        cJSON_AddNumberToObject(obj, "defense",    e->defense);
        cJSON_AddNumberToObject(obj, "experience", e->experience);
        cJSON_AddNumberToObject(obj, "move_timer", e->move_timer);
        cJSON_AddItemToArray(arr, obj);
    }
    return arr;
}

static void deserialize_enemies(const cJSON *arr, Enemy *enemies, int *count) {
    *count = cJSON_GetArraySize(arr);
    for (int i = 0; i < *count; i++) {
        cJSON *obj = cJSON_GetArrayItem(arr, i);
        Enemy *e = &enemies[i];
        e->x          = cJSON_GetObjectItem(obj, "x")->valueint;
        e->y          = cJSON_GetObjectItem(obj, "y")->valueint;
        e->active     = cJSON_GetObjectItem(obj, "active")->valueint;
        e->type       = cJSON_GetObjectItem(obj, "type")->valueint;
        strncpy(e->name, cJSON_GetObjectItem(obj, "name")->valuestring, 15);
        e->hp         = cJSON_GetObjectItem(obj, "hp")->valueint;
        e->max_hp     = cJSON_GetObjectItem(obj, "max_hp")->valueint;
        e->attack     = cJSON_GetObjectItem(obj, "attack")->valueint;
        e->defense    = cJSON_GetObjectItem(obj, "defense")->valueint;
        e->experience = cJSON_GetObjectItem(obj, "experience")->valueint;
        e->move_timer = cJSON_GetObjectItem(obj, "move_timer")->valueint;
    }
}

int save_game(const GameState *g, int slot) {
    mkdir("saves", 0755);
    cJSON *root = cJSON_CreateObject();

    // Player
    cJSON *player = cJSON_CreateObject();
    cJSON_AddStringToObject(player, "name",       g->player.name);
    cJSON_AddNumberToObject(player, "x",          g->player.x);
    cJSON_AddNumberToObject(player, "y",          g->player.y);
    cJSON_AddNumberToObject(player, "hp",         g->player.hp);
    cJSON_AddNumberToObject(player, "max_hp",     g->player.max_hp);
    cJSON_AddNumberToObject(player, "mp",         g->player.mp);
    cJSON_AddNumberToObject(player, "max_mp",     g->player.max_mp);
    cJSON_AddNumberToObject(player, "attack",     g->player.attack);
    cJSON_AddNumberToObject(player, "defense",    g->player.defense);
    cJSON_AddNumberToObject(player, "level",      g->player.level);
    cJSON_AddNumberToObject(player, "experience", g->player.experience);
    cJSON_AddItemToObject(root, "player", player);

    // Game state
    cJSON_AddNumberToObject(root, "level",         g->level);
    cJSON_AddNumberToObject(root, "level_cleared", g->level_cleared);
    cJSON_AddNumberToObject(root, "max_level_reached", g->max_level_reached);
    cJSON_AddNumberToObject(root, "message_count", g->message_count);

    // Messages
    cJSON *messages = cJSON_CreateArray();
    for (int i = 0; i < g->message_count; i++)
        cJSON_AddItemToArray(messages, cJSON_CreateString(g->messages[i]));
    cJSON_AddItemToObject(root, "messages", messages);

    // Current map
    cJSON_AddItemToObject(root, "map", serialize_map(&g->map));

    // Current enemies
    cJSON_AddItemToObject(root, "enemies",
        serialize_enemies(g->enemies, g->enemy_count));
    cJSON_AddNumberToObject(root, "enemy_count", g->enemy_count);

    // Level cache
    cJSON *cache = cJSON_CreateArray();
    for (int i = 0; i < MAX_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->level_cache[i].valid);
        if (g->level_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->level_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->level_cache[i].enemies,
                                  g->level_cache[i].enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count",
                g->level_cache[i].enemy_count);
        }
        cJSON_AddItemToArray(cache, entry);
    }
    cJSON_AddItemToObject(root, "level_cache", cache);

    char *json = cJSON_Print(root);
    cJSON_Delete(root);

    FILE *f = fopen(slot_path(slot), "w");
    if (!f) { free(json); return 0; }
    fputs(json, f);
    fclose(f);
    free(json);
    return 1;
}

int load_game(GameState *g, int slot) {
    FILE *f = fopen(slot_path(slot), "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) return 0;

    // Player
    cJSON *player = cJSON_GetObjectItem(root, "player");
    strncpy(g->player.name, cJSON_GetObjectItem(player, "name")->valuestring, 20);
    g->player.x          = cJSON_GetObjectItem(player, "x")->valueint;
    g->player.y          = cJSON_GetObjectItem(player, "y")->valueint;
    g->player.hp         = cJSON_GetObjectItem(player, "hp")->valueint;
    g->player.max_hp     = cJSON_GetObjectItem(player, "max_hp")->valueint;
    g->player.mp         = cJSON_GetObjectItem(player, "mp")->valueint;
    g->player.max_mp     = cJSON_GetObjectItem(player, "max_mp")->valueint;
    g->player.attack     = cJSON_GetObjectItem(player, "attack")->valueint;
    g->player.defense    = cJSON_GetObjectItem(player, "defense")->valueint;
    g->player.level      = cJSON_GetObjectItem(player, "level")->valueint;
    g->player.experience = cJSON_GetObjectItem(player, "experience")->valueint;

    // Game state
    g->level         = cJSON_GetObjectItem(root, "level")->valueint;
    g->level_cleared = cJSON_GetObjectItem(root, "level_cleared")->valueint;
    g->max_level_reached = cJSON_GetObjectItem(root, "max_level_reached")->valueint;
    g->message_count = cJSON_GetObjectItem(root, "message_count")->valueint;

    // Messages
    cJSON *messages = cJSON_GetObjectItem(root, "messages");
    for (int i = 0; i < g->message_count && i < MAX_MESSAGES; i++)
        strncpy(g->messages[i],
            cJSON_GetArrayItem(messages, i)->valuestring,
            MAX_MESSAGE_LEN - 1);

    // Current map
    deserialize_map(cJSON_GetObjectItem(root, "map"), &g->map);

    // Current enemies
    deserialize_enemies(cJSON_GetObjectItem(root, "enemies"),
                        g->enemies, &g->enemy_count);

    // Level cache
    cJSON *cache = cJSON_GetObjectItem(root, "level_cache");
    for (int i = 0; i < MAX_DEPTH; i++) {
        cJSON *entry = cJSON_GetArrayItem(cache, i);
        g->level_cache[i].valid = cJSON_GetObjectItem(entry, "valid")->valueint;
        if (g->level_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                            &g->level_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                                g->level_cache[i].enemies,
                                &g->level_cache[i].enemy_count);
        }
    }

    cJSON_Delete(root);
    return 1;
}

int get_save_preview(int slot, char *name_out, int *level_out) {
    FILE *f = fopen(slot_path(slot), "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) return 0;

    cJSON *player = cJSON_GetObjectItem(root, "player");
    strncpy(name_out, cJSON_GetObjectItem(player, "name")->valuestring, 20);
    *level_out = cJSON_GetObjectItem(root, "level")->valueint;

    cJSON_Delete(root);
    return 1;
}