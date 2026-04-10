#include "save_load.h"
#include "../game/game.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char b64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char *tiles_to_base64(const TileType tiles[MAP_H][MAP_W]) {
    int src_len = MAP_H * MAP_W * sizeof(TileType);
    int dst_len = ((src_len + 2) / 3) * 4 + 1;
    char *out = malloc(dst_len);
    if (!out) return NULL;
    const unsigned char *src = (const unsigned char *)tiles;
    int i = 0, j = 0;
    while (i < src_len) {
        unsigned int a = i < src_len ? src[i++] : 0;
        unsigned int b = i < src_len ? src[i++] : 0;
        unsigned int c = i < src_len ? src[i++] : 0;
        unsigned int t = (a << 16) | (b << 8) | c;
        out[j++] = b64[(t >> 18) & 0x3f];
        out[j++] = b64[(t >> 12) & 0x3f];
        out[j++] = b64[(t >>  6) & 0x3f];
        out[j++] = b64[(t >>  0) & 0x3f];
    }
    int pad = src_len % 3;
    if (pad == 1) { out[j-1] = '='; out[j-2] = '='; }
    if (pad == 2) { out[j-1] = '='; }
    out[j] = '\0';
    return out;
}

static void base64_to_tiles(const char *src, TileType tiles[MAP_H][MAP_W]) {
    static const unsigned char dec[256] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,63,
        52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,
        0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,
        0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0
    };
    unsigned char *dst = (unsigned char *)tiles;
    int len = strlen(src);
    int j = 0;
    for (int i = 0; i < len; i += 4) {
        unsigned int a = dec[(unsigned char)src[i]];
        unsigned int b = dec[(unsigned char)src[i+1]];
        unsigned int c = src[i+2] == '=' ? 0 : dec[(unsigned char)src[i+2]];
        unsigned int d = src[i+3] == '=' ? 0 : dec[(unsigned char)src[i+3]];
        unsigned int t = (a << 18) | (b << 12) | (c << 6) | d;
        dst[j++] = (t >> 16) & 0xff;
        if (src[i+2] != '=') dst[j++] = (t >> 8) & 0xff;
        if (src[i+3] != '=') dst[j++] = t & 0xff;
    }
}

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

    // Tiles as base64 string
    char *b64tiles = tiles_to_base64(m->tiles);
    cJSON_AddStringToObject(obj, "tiles_b64", b64tiles);
    free(b64tiles);

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

    base64_to_tiles(cJSON_GetObjectItem(obj, "tiles_b64")->valuestring, m->tiles);
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
    cJSON_AddStringToObject(player, "name",              g->player.name);
    cJSON_AddNumberToObject(player, "x",                 g->player.x);
    cJSON_AddNumberToObject(player, "y",                 g->player.y);
    cJSON_AddNumberToObject(player, "hp",                g->player.hp);
    cJSON_AddNumberToObject(player, "max_hp",            g->player.max_hp);
    cJSON_AddNumberToObject(player, "mp",                g->player.mp);
    cJSON_AddNumberToObject(player, "max_mp",            g->player.max_mp);
    cJSON_AddNumberToObject(player, "attack",            g->player.attack);
    cJSON_AddNumberToObject(player, "defense",           g->player.defense);
    cJSON_AddNumberToObject(player, "level",             g->player.level);
    cJSON_AddNumberToObject(player, "experience",        g->player.experience);
    cJSON_AddNumberToObject(player, "experience_next",   g->player.experience_next);
    cJSON_AddNumberToObject(player, "last_dx",           g->player.last_dx);
    cJSON_AddNumberToObject(player, "last_dy",           g->player.last_dy);
    cJSON_AddNumberToObject(player, "equipped_spell",    g->player.equipped_spell);
    cJSON_AddNumberToObject(player, "known_spell_count", g->player.known_spell_count);
    cJSON_AddNumberToObject(player, "player_class",      g->player.player_class);

    // Known spells
    cJSON *spells = cJSON_CreateArray();
    for (int i = 0; i < g->player.known_spell_count; i++) {
        const Spell *sp = &g->player.known_spells[i];
        cJSON *s = cJSON_CreateObject();
        cJSON_AddStringToObject(s, "name",     sp->name);
        cJSON_AddNumberToObject(s, "id",       sp->id);
        cJSON_AddNumberToObject(s, "type",     sp->type);
        cJSON_AddNumberToObject(s, "mp_cost",  sp->mp_cost);
        cJSON_AddNumberToObject(s, "damage",   sp->damage);
        cJSON_AddNumberToObject(s, "heal_hp",  sp->heal_hp);
        cJSON_AddNumberToObject(s, "range",    sp->range);
        cJSON_AddNumberToObject(s, "radius",   sp->radius);
        cJSON_AddItemToArray(spells, s);
    }
    cJSON_AddItemToObject(player, "spells", spells);
    cJSON_AddItemToObject(root, "player", player);

    // Game state
    cJSON_AddNumberToObject(root, "level",             g->level);
    cJSON_AddNumberToObject(root, "level_cleared",     g->level_cleared);
    cJSON_AddNumberToObject(root, "max_level_reached", g->max_level_reached);
    cJSON_AddNumberToObject(root, "message_count",     g->message_count);
    cJSON_AddNumberToObject(root, "gold",              g->gold);
    cJSON_AddNumberToObject(root, "score",             g->score);
    cJSON_AddNumberToObject(root, "equipped_weapon",   g->equipped_weapon);
    cJSON_AddNumberToObject(root, "equipped_armor",    g->equipped_armor);
    cJSON_AddNumberToObject(root, "location",          g->location);

    // Messages
    cJSON *messages = cJSON_CreateArray();
    for (int i = 0; i < g->message_count; i++)
        cJSON_AddItemToArray(messages, cJSON_CreateString(g->messages[i]));
    cJSON_AddItemToObject(root, "messages", messages);

    // Inventory
    cJSON *inventory = cJSON_CreateArray();
    for (int i = 0; i < g->inventory_count; i++) {
        const Item *item = &g->inventory[i];
        cJSON *it = cJSON_CreateObject();
        cJSON_AddNumberToObject(it, "active",        item->active);
        cJSON_AddNumberToObject(it, "type",          item->type);
        cJSON_AddStringToObject(it, "name",          item->name);
        cJSON_AddNumberToObject(it, "heal_hp",       item->heal_hp);
        cJSON_AddNumberToObject(it, "heal_mp",       item->heal_mp);
        cJSON_AddNumberToObject(it, "attack_bonus",  item->attack_bonus);
        cJSON_AddNumberToObject(it, "defense_bonus", item->defense_bonus);
        cJSON_AddNumberToObject(it, "value",         item->value);
        cJSON_AddNumberToObject(it, "spell_id",      item->spell_id);
        cJSON_AddNumberToObject(it, "is_ranged",     item->is_ranged);
        cJSON_AddNumberToObject(it, "range",         item->range);
        cJSON_AddNumberToObject(it, "is_two_handed", item->is_two_handed);
        cJSON_AddItemToArray(inventory, it);
    }
    cJSON_AddItemToObject(root, "inventory", inventory);
    cJSON_AddNumberToObject(root, "inventory_count", g->inventory_count);

    // Floor items
    cJSON *floor_items = cJSON_CreateArray();
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *fi = &g->floor_items[i];
        cJSON *f = cJSON_CreateObject();
        cJSON_AddNumberToObject(f, "active", fi->active);
        cJSON_AddNumberToObject(f, "x",      fi->x);
        cJSON_AddNumberToObject(f, "y",      fi->y);
        cJSON *it = cJSON_CreateObject();
        cJSON_AddNumberToObject(it, "active",        fi->item.active);
        cJSON_AddNumberToObject(it, "type",          fi->item.type);
        cJSON_AddStringToObject(it, "name",          fi->item.name);
        cJSON_AddNumberToObject(it, "heal_hp",       fi->item.heal_hp);
        cJSON_AddNumberToObject(it, "heal_mp",       fi->item.heal_mp);
        cJSON_AddNumberToObject(it, "attack_bonus",  fi->item.attack_bonus);
        cJSON_AddNumberToObject(it, "defense_bonus", fi->item.defense_bonus);
        cJSON_AddNumberToObject(it, "value",         fi->item.value);
        cJSON_AddNumberToObject(it, "spell_id",      fi->item.spell_id);
        cJSON_AddNumberToObject(it, "is_ranged",     fi->item.is_ranged);
        cJSON_AddNumberToObject(it, "range",         fi->item.range);
        cJSON_AddNumberToObject(it, "is_two_handed", fi->item.is_two_handed);
        cJSON_AddItemToObject(f, "item", it);
        cJSON_AddItemToArray(floor_items, f);
    }
    cJSON_AddItemToObject(root, "floor_items", floor_items);
    cJSON_AddNumberToObject(root, "floor_item_count", g->floor_item_count);

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
        cJSON_AddNumberToObject(entry, "valid",         g->level_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared", g->level_cache[i].level_cleared);
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
    g->player.x                = cJSON_GetObjectItem(player, "x")->valueint;
    g->player.y                = cJSON_GetObjectItem(player, "y")->valueint;
    g->player.hp               = cJSON_GetObjectItem(player, "hp")->valueint;
    g->player.max_hp           = cJSON_GetObjectItem(player, "max_hp")->valueint;
    g->player.mp               = cJSON_GetObjectItem(player, "mp")->valueint;
    g->player.max_mp           = cJSON_GetObjectItem(player, "max_mp")->valueint;
    g->player.attack           = cJSON_GetObjectItem(player, "attack")->valueint;
    g->player.defense          = cJSON_GetObjectItem(player, "defense")->valueint;
    g->player.level            = cJSON_GetObjectItem(player, "level")->valueint;
    g->player.experience       = cJSON_GetObjectItem(player, "experience")->valueint;
    g->player.experience_next  = cJSON_GetObjectItem(player, "experience_next")->valueint;
    g->player.last_dx          = cJSON_GetObjectItem(player, "last_dx")->valueint;
    g->player.last_dy          = cJSON_GetObjectItem(player, "last_dy")->valueint;
    g->player.equipped_spell   = cJSON_GetObjectItem(player, "equipped_spell")->valueint;
    g->player.known_spell_count = cJSON_GetObjectItem(player, "known_spell_count")->valueint;
    g->player.player_class      = cJSON_GetObjectItem(player, "player_class")->valueint;

    // Known spells
    cJSON *spells = cJSON_GetObjectItem(player, "spells");
    for (int i = 0; i < g->player.known_spell_count; i++) {
        cJSON *s = cJSON_GetArrayItem(spells, i);
        Spell *sp = &g->player.known_spells[i];
        strncpy(sp->name, cJSON_GetObjectItem(s, "name")->valuestring,
            sizeof(sp->name) - 1);
        sp->id      = cJSON_GetObjectItem(s, "id")->valueint;
        sp->type    = cJSON_GetObjectItem(s, "type")->valueint;
        sp->mp_cost = cJSON_GetObjectItem(s, "mp_cost")->valueint;
        sp->damage  = cJSON_GetObjectItem(s, "damage")->valueint;
        sp->heal_hp = cJSON_GetObjectItem(s, "heal_hp")->valueint;
        sp->range   = cJSON_GetObjectItem(s, "range")->valueint;
        sp->radius  = cJSON_GetObjectItem(s, "radius")->valueint;
    }

    // Game state
    g->level             = cJSON_GetObjectItem(root, "level")->valueint;
    g->level_cleared     = cJSON_GetObjectItem(root, "level_cleared")->valueint;
    g->max_level_reached = cJSON_GetObjectItem(root, "max_level_reached")->valueint;
    g->message_count     = cJSON_GetObjectItem(root, "message_count")->valueint;
    g->gold              = cJSON_GetObjectItem(root, "gold")->valueint;
    g->score             = cJSON_GetObjectItem(root, "score")->valueint;
    g->equipped_weapon   = cJSON_GetObjectItem(root, "equipped_weapon")->valueint;
    g->equipped_armor    = cJSON_GetObjectItem(root, "equipped_armor")->valueint;
    g->location          = cJSON_GetObjectItem(root, "location")->valueint;

    // Messages
    cJSON *messages = cJSON_GetObjectItem(root, "messages");
    for (int i = 0; i < g->message_count && i < MAX_MESSAGES; i++)
        strncpy(g->messages[i],
            cJSON_GetArrayItem(messages, i)->valuestring,
            MAX_MESSAGE_LEN - 1);

    // Inventory
    g->inventory_count = cJSON_GetObjectItem(root, "inventory_count")->valueint;
    cJSON *inventory = cJSON_GetObjectItem(root, "inventory");
    for (int i = 0; i < g->inventory_count; i++) {
        cJSON *it = cJSON_GetArrayItem(inventory, i);
        Item *item = &g->inventory[i];
        item->active        = cJSON_GetObjectItem(it, "active")->valueint;
        item->type          = cJSON_GetObjectItem(it, "type")->valueint;
        strncpy(item->name, cJSON_GetObjectItem(it, "name")->valuestring,
            sizeof(item->name) - 1);
        item->heal_hp       = cJSON_GetObjectItem(it, "heal_hp")->valueint;
        item->heal_mp       = cJSON_GetObjectItem(it, "heal_mp")->valueint;
        item->attack_bonus  = cJSON_GetObjectItem(it, "attack_bonus")->valueint;
        item->defense_bonus = cJSON_GetObjectItem(it, "defense_bonus")->valueint;
        item->value         = cJSON_GetObjectItem(it, "value")->valueint;
        item->spell_id      = cJSON_GetObjectItem(it, "spell_id")->valueint;
        item->is_ranged     = cJSON_GetObjectItem(it, "is_ranged")->valueint;
        item->range         = cJSON_GetObjectItem(it, "range")->valueint;
        item->is_two_handed = cJSON_GetObjectItem(it, "is_two_handed")->valueint;
    }

    // Floor items
    g->floor_item_count = cJSON_GetObjectItem(root, "floor_item_count")->valueint;
    cJSON *floor_items = cJSON_GetObjectItem(root, "floor_items");
    for (int i = 0; i < g->floor_item_count; i++) {
        cJSON *f = cJSON_GetArrayItem(floor_items, i);
        FloorItem *fi = &g->floor_items[i];
        fi->active = cJSON_GetObjectItem(f, "active")->valueint;
        fi->x      = cJSON_GetObjectItem(f, "x")->valueint;
        fi->y      = cJSON_GetObjectItem(f, "y")->valueint;
        cJSON *it = cJSON_GetObjectItem(f, "item");
        fi->item.active        = cJSON_GetObjectItem(it, "active")->valueint;
        fi->item.type          = cJSON_GetObjectItem(it, "type")->valueint;
        strncpy(fi->item.name, cJSON_GetObjectItem(it, "name")->valuestring,
            sizeof(fi->item.name) - 1);
        fi->item.heal_hp       = cJSON_GetObjectItem(it, "heal_hp")->valueint;
        fi->item.heal_mp       = cJSON_GetObjectItem(it, "heal_mp")->valueint;
        fi->item.attack_bonus  = cJSON_GetObjectItem(it, "attack_bonus")->valueint;
        fi->item.defense_bonus = cJSON_GetObjectItem(it, "defense_bonus")->valueint;
        fi->item.value         = cJSON_GetObjectItem(it, "value")->valueint;
        fi->item.spell_id      = cJSON_GetObjectItem(it, "spell_id")->valueint;
        fi->item.is_ranged     = cJSON_GetObjectItem(it, "is_ranged")->valueint;
        fi->item.range         = cJSON_GetObjectItem(it, "range")->valueint;
        fi->item.is_two_handed = cJSON_GetObjectItem(it, "is_two_handed")->valueint;
    }

    // Current map
    deserialize_map(cJSON_GetObjectItem(root, "map"), &g->map);

    // Current enemies
    deserialize_enemies(cJSON_GetObjectItem(root, "enemies"),
                        g->enemies, &g->enemy_count);

    // Level cache
    cJSON *cache = cJSON_GetObjectItem(root, "level_cache");
    for (int i = 0; i < MAX_DEPTH; i++) {
        cJSON *entry = cJSON_GetArrayItem(cache, i);
        g->level_cache[i].valid         = cJSON_GetObjectItem(entry, "valid")->valueint;
        g->level_cache[i].level_cleared = cJSON_GetObjectItem(entry, "level_cleared")->valueint;
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