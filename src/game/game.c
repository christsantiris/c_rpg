#include "game.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "../game/actions.h"

static void spawn_enemy(Enemy *e, EnemyType type, int x, int y) {
    e->active  = 1;
    e->type    = type;
    e->x       = x;
    e->y       = y;
    e->is_boss = 0;
    e->dain_fragment = 0;
    e->move_timer = 0;
    switch (type) {
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
            e->max_hp = 500; e->hp = 500;
            e->attack = 55;  e->defense = 22;
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
}

static int boss_for_level(const GameState *g, EnemyType *type) {
    int boss_level = DUNGEON_DEPTH;
    if (g->location == LOCATION_FOREST) {
        boss_level = FOREST_DEPTH;
    } else if (g->location == LOCATION_MOUNTAINS) {
        boss_level = MOUNTAIN_DEPTH;
    } else if (g->location == LOCATION_COAST) {
        boss_level = COAST_DEPTH;
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
    } else {
        *type = ENEMY_LICH_KING;
    }
    return 1;
}

static int enemy_tile_open(const GameState *g, int x, int y) {
    // Keep keys, stairs, traps, and portals visible and unobstructed.
    if (!map_is_walkable(&g->map, x, y) ||
        (g->map.tiles[y][x] != TILE_FLOOR &&
        g->map.tiles[y][x] != TILE_FOREST_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_BRIDGE &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_CAVE_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_FORTRESS_FLOOR &&
        g->map.tiles[y][x] != TILE_COAST_FLOOR &&
        g->map.tiles[y][x] != TILE_COAST_SHALLOW_WATER &&
        g->map.tiles[y][x] != TILE_COAST_DRAINED_WATER)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active &&
            g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
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
    if (g->enemy_count >= MAX_ENEMIES) {
        return 0;
    }
    int x;
    int y;
    if (!find_enemy_tile(g, &x, &y, room_limit)) {
        return 0;
    }
    spawn_enemy(&g->enemies[g->enemy_count], type, x, y);
    g->enemy_count++;
    return 1;
}

void enemies_spawn(GameState *g) {
    g->enemy_count = 0;
    if (g->map.room_count == 0) {
        return;
    }

    int num_enemies = 10 + g->level;
    if (num_enemies > MAX_ENEMIES) {
        num_enemies = MAX_ENEMIES;
    }

    EnemyType boss_type;
    if (boss_for_level(g, &boss_type)) {
        int boss_x = g->map.stairs_down_x + 1;
        int boss_y = g->map.stairs_down_y;
        if (g->location == LOCATION_FOREST ||
            g->location == LOCATION_MOUNTAINS ||
            g->location == LOCATION_COAST) {
            map_room_center(&g->map.rooms[g->map.room_count - 1],
                &boss_x, &boss_y);
        }
        spawn_enemy(&g->enemies[g->enemy_count++], boss_type,
            boss_x, boss_y);
    }

    int boss_level = g->location == LOCATION_FOREST ? FOREST_DEPTH :
        (g->location == LOCATION_MOUNTAINS ? MOUNTAIN_DEPTH :
        (g->location == LOCATION_COAST ? COAST_DEPTH : DUNGEON_DEPTH));
    int regular_room_limit = g->level == boss_level
        ? g->map.room_count - 1 : g->map.room_count;
    if (g->location == LOCATION_MOUNTAINS && g->dain_quest_state == 1) {
        EnemyType quest_target = ENEMY_GOBLIN_SCOUT;
        int target_bit = 0;
        if (g->level == 2) {
            quest_target = ENEMY_GOBLIN_ARCHER;
            target_bit = DAIN_FRAGMENT_ARCHER;
        } else if (g->level == 3) {
            quest_target = ENEMY_GOBLIN_BOMBER;
            target_bit = DAIN_FRAGMENT_BOMBER;
        } else if (g->level == 5) {
            quest_target = ENEMY_GOBLIN_SHAMAN;
            target_bit = DAIN_FRAGMENT_SHAMAN;
        }
        if (target_bit && !(g->dain_map_fragments & target_bit)) {
            int target_index = g->enemy_count;
            if (spawn_into_open_tile(g, quest_target, regular_room_limit)) {
                Enemy *target = &g->enemies[target_index];
                target->dain_fragment = target_bit;
                target->max_hp = target->max_hp * 3 / 2;
                target->hp = target->max_hp;
                target->attack += 2;
                strncpy(target->name, "Map Bearer",
                    sizeof(target->name) - 1);
                target->name[sizeof(target->name) - 1] = '\0';
            }
        }
    }
    while (g->enemy_count < num_enemies) {
        EnemyType type;
        int roll = rand() % 100;
        int level = g->level;

        if (g->location == LOCATION_COAST) {
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
        if (!spawn_into_open_tile(g, type, regular_room_limit)) {
            break;
        }
    }
}

void game_init(GameState *g) {
    srand((unsigned)time(NULL));
    g->level = 1;
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        g->level_cache[i].valid = 0;
        g->forest_cache[i].valid = 0;
        g->mountain_cache[i].valid = 0;
        g->coast_cache[i].valid = 0;
    }
    g->message_count = 0;
    g->level_cleared = 0;
    g->max_level_reached = 1;
    g->max_forest_level_reached = 1;
    g->max_mountain_level_reached = 1;
    g->max_coast_level_reached = 1;
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
    g->score = 0;
    g->dungeon_key_found = 0;
    g->portal_active = 0;
    g->portal_level = 0;
    g->portal_location = LOCATION_DUNGEON;
    g->portal_x = 0;
    g->portal_y = 0;
    g->portal_origin_tile = TILE_FLOOR;
    g->defeated_bosses = 0;
    g->elowen_quest_state = 0;
    g->elowen_seals_restored = 0;
    g->dain_quest_state = 0;
    g->dain_map_fragments = 0;
    g->alder_quest_state = 0;
    g->alder_wardens_rescued = 0;
    g->mara_quest_state = 0;
    g->mara_beacons_lit = 0;
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
    g->inventory[g->inventory_count++] = item_make_scroll_return_to_town();
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
    g->dialogue_active = 0;
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
    g->player.attack -=
        g->inventory[g->equipped_main_hand].attack_bonus;
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
    return g->level_cache;
}

static int *active_max_level(GameState *g) {
    if (g->location == LOCATION_FOREST) return &g->max_forest_level_reached;
    if (g->location == LOCATION_MOUNTAINS)
        return &g->max_mountain_level_reached;
    if (g->location == LOCATION_COAST) {
        return &g->max_coast_level_reached;
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
    return DUNGEON_DEPTH;
}

static void place_elowen_seal(GameState *g) {
    if (g->location != LOCATION_DUNGEON || g->elowen_quest_state != 1) {
        return;
    }
    int seal_index = -1;
    if (g->level == 2) {
        seal_index = 0;
    } else if (g->level == 4) {
        seal_index = 1;
    } else if (g->level == 6) {
        seal_index = 2;
    }
    if (seal_index < 0 || g->map.room_count < 2) {
        return;
    }
    Room *room = &g->map.rooms[g->map.room_count / 2];
    int x;
    int y;
    map_room_center(room, &x, &y);
    g->map.tiles[y][x] =
        (g->elowen_seals_restored & (1 << seal_index))
        ? TILE_RESTORED_BURIAL_SEAL : TILE_BROKEN_BURIAL_SEAL;
}

static int alder_warden_bit(int level) {
    if (level == 2) {
        return ALDER_WARDEN_STAGE_2;
    }
    if (level == 5) {
        return ALDER_WARDEN_STAGE_5;
    }
    if (level == 7) {
        return ALDER_WARDEN_STAGE_7;
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
    int room_index = g->level == 2 ? 7 : (g->level == 5 ? 8 : 5);
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
    if (g->location != LOCATION_FOREST || g->enemy_count >= MAX_ENEMIES) {
        return;
    }
    EnemyType type;
    if (g->level == 2) {
        type = ENEMY_GIANT_SPIDER;
    } else if (g->level == 5) {
        type = ENEMY_DARK_ELF;
    } else if (g->level == 7) {
        type = ENEMY_FOREST_TROLL;
    } else {
        return;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] != TILE_FOREST_WARDEN) {
                continue;
            }
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    int guardian_x = x + dx;
                    int guardian_y = y + dy;
                    if ((dx == 0 && dy == 0) ||
                        !enemy_tile_open(g, guardian_x, guardian_y)) {
                        continue;
                    }
                    spawn_enemy(&g->enemies[g->enemy_count++], type,
                        guardian_x, guardian_y);
                    return;
                }
            }
        }
    }
}

static int mara_beacon_bit(int level) {
    if (level == 1) {
        return MARA_BEACON_STAGE_1;
    }
    if (level == 3) {
        return MARA_BEACON_STAGE_3;
    }
    if (level == 6) {
        return MARA_BEACON_STAGE_6;
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
            if (g->map.tiles[y][x] == TILE_COAST_BEACON_UNLIT ||
                g->map.tiles[y][x] == TILE_COAST_BEACON_LIT) {
                return 0;
            }
        }
    }
    int room_index = g->level == 1 ? 1 : (g->level == 3 ? 3 : 8);
    if (room_index >= g->map.room_count) {
        return 0;
    }
    int x;
    int y;
    map_room_center(&g->map.rooms[room_index], &x, &y);
    if (g->level == 6) {
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
    g->map.tiles[y][x] = (g->mara_beacons_lit & bit)
        ? TILE_COAST_BEACON_LIT : TILE_COAST_BEACON_UNLIT;
    return !(g->mara_beacons_lit & bit);
}

static void spawn_mara_guardian(GameState *g) {
    if (g->location != LOCATION_COAST || g->enemy_count >= MAX_ENEMIES) {
        return;
    }
    EnemyType type;
    if (g->level == 3) {
        type = ENEMY_ANIMATED_STATUE;
    } else if (g->level == 6) {
        type = ENEMY_SEA_SERPENT;
    } else {
        return;
    }
    int room_index = g->level == 3 ? 3 : 8;
    if (room_index >= g->map.room_count) {
        return;
    }
    Room *room = &g->map.rooms[room_index];
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (enemy_tile_open(g, x, y)) {
                spawn_enemy(&g->enemies[g->enemy_count++], type, x, y);
                return;
            }
        }
    }
}

static void generate_active_level(GameState *g) {
    if (g->location == LOCATION_FOREST) {
        map_generate_forest(&g->map, g->level);
    } else if (g->location == LOCATION_MOUNTAINS) {
        map_generate_mountains(&g->map, g->level);
    } else if (g->location == LOCATION_COAST) {
        map_generate_coast(&g->map, g->level);
    } else {
        map_generate(&g->map, g->level);
    }
    g->enemy_count = 0;
    place_elowen_seal(g);
    int warden_placed = place_alder_warden(g);
    int beacon_placed = place_mara_beacon(g);
    enemies_spawn(g);
    if (warden_placed) {
        spawn_alder_guardian(g);
    }
    if (beacon_placed) {
        spawn_mara_guardian(g);
    }
    if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH &&
        (g->defeated_bosses & (1 << LOCATION_DUNGEON))) {
        g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] =
            TILE_RETURN_EXIT;
        g->level_cleared = 1;
    }
}

void game_descend(GameState *g) {
    int depth = active_depth(g);
    if (g->level >= depth) return;

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
        if (place_alder_warden(g)) {
            spawn_alder_guardian(g);
        }
        if (place_mara_beacon(g)) {
            spawn_mara_guardian(g);
        }
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_ascend(GameState *g) {
    if (g->level <= 1) return;

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
        if (place_alder_warden(g)) {
            spawn_alder_guardian(g);
        }
        if (place_mara_beacon(g)) {
            spawn_mara_guardian(g);
        }
    } else {
        g->level_cleared = 0;
    }

    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
}

static void enter_adventure(GameState *g, Location location) {
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
    enter_adventure(g, LOCATION_FOREST);
}

void game_enter_mountains(GameState *g) {
    enter_adventure(g, LOCATION_MOUNTAINS);
}

void game_enter_coast(GameState *g) {
    enter_adventure(g, LOCATION_COAST);
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
    g->player.x = 8;
    g->player.y = 21;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    if (g->portal_active) {
        g->map.tiles[2][20] = TILE_PORTAL;
    }
    push_message(g, "You step back into town.");
}

void game_return_to_town(GameState *g) {
    LevelCache *cache = active_cache(g);
    Location returning_from = g->location;
    // Cache current level before leaving
    if (g->level >= 1 && g->level <= active_depth(g)) {
        cache[g->level - 1].map = g->map;
        cache[g->level - 1].enemy_count   = g->enemy_count;
        cache[g->level - 1].level_cleared = g->level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            cache[g->level - 1].enemies[i] = g->enemies[i];
        cache[g->level - 1].valid = 1;
    }

    g->location = LOCATION_TOWN;
    int spawn_x, spawn_y;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    if (returning_from == LOCATION_FOREST) {
        g->player.x = 1; g->player.y = 12;
    } else if (returning_from == LOCATION_MOUNTAINS) {
        g->player.x = TOWN_W - 2; g->player.y = 12;
    } else if (returning_from == LOCATION_COAST) {
        g->player.x = 20; g->player.y = TOWN_H - 2;
    } else {
        g->player.x = 20; g->player.y = 1;
    }
    g->floor_item_count = 0;
    g->enemy_count = 0;
    if (g->portal_active) {
        g->map.tiles[2][20] = TILE_PORTAL;
    }
}

void game_open_town_portal(GameState *g) {
    if (g->location != LOCATION_DUNGEON &&
        g->location != LOCATION_FOREST &&
        g->location != LOCATION_MOUNTAINS &&
        g->location != LOCATION_COAST) {
        return;
    }
    g->portal_active = 1;
    g->portal_level = g->level;
    g->portal_location = g->location;
    g->portal_x = g->player.x;
    g->portal_y = g->player.y;
    g->portal_origin_tile = g->map.tiles[g->player.y][g->player.x];
    g->map.tiles[g->player.y][g->player.x] = TILE_PORTAL;
    game_return_to_town(g);
    g->map.tiles[2][20] = TILE_PORTAL;
    push_message(g, "A return portal remains open.");
}

void game_use_town_portal(GameState *g) {
    if (!g->portal_active || g->portal_level < 1 ||
        g->portal_level > MAX_REGION_DEPTH) return;
    int level = g->portal_level;
    LevelCache *cache = g->level_cache;
    if (g->portal_location == LOCATION_FOREST) {
        cache = g->forest_cache;
    } else if (g->portal_location == LOCATION_MOUNTAINS) {
        cache = g->mountain_cache;
    } else if (g->portal_location == LOCATION_COAST) {
        cache = g->coast_cache;
    }
    if (!cache[level - 1].valid) return;

    g->location = g->portal_location;
    g->level = level;
    g->map = cache[level - 1].map;
    place_elowen_seal(g);
    g->enemy_count = cache[level - 1].enemy_count;
    g->level_cleared = cache[level - 1].level_cleared;
    for (int i = 0; i < g->enemy_count; i++)
        g->enemies[i] = cache[level - 1].enemies[i];
    if (place_alder_warden(g)) {
        spawn_alder_guardian(g);
    }
    if (place_mara_beacon(g)) {
        spawn_mara_guardian(g);
    }
    g->player.x = g->portal_x;
    g->player.y = g->portal_y;
    g->map.tiles[g->portal_y][g->portal_x] = g->portal_origin_tile;
    g->portal_active = 0;
    push_message(g, "Returned through the portal.");
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
        strncpy(g->dialogue_text,
            "Three shattered burial seals let the dead rise. Restore the seals on floors 2, 4, and 6.",
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
        g->gold += 100;
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
        strncpy(g->dialogue_text,
            "Goblins tore an old dwarven treasure map into three pieces. An Archer, a Bomber, and a Shaman carry the fragments. Recover them.",
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
        g->gold += 150;
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
        strncpy(g->dialogue_text,
            "Three of my wardens followed the dead paths beneath the trees. Find them on stages 2, 5, and 7 before the forest claims them.",
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
            "You have rescued %d of my 3 wardens. Search the secluded groves on forest stages 2, 5, and 7.",
            rescued);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status), "Quest progress: %d/3 wardens.",
            rescued);
        push_message(g, status);
        return;
    }
    if (g->alder_quest_state == 2) {
        g->alder_quest_state = 3;
        g->gold += 175;
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
            "You cut through the spider silk binding me. I can follow your trail home from here.",
            MAX_DIALOGUE_LEN - 1);
    } else if (g->level == 5) {
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
        strncpy(g->dialogue_text,
            "Take this sheltered ember to the drowned beacons on coast stages 1, 3, and 6. Lower the tide, then relight each flame.",
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
            "You have relit %d of 3 beacons. The remaining lights wait on coast stages 1, 3, and 6.",
            lit);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status), "Quest progress: %d/3 beacons.", lit);
        push_message(g, status);
        return;
    }
    if (g->mara_quest_state == 2) {
        g->mara_quest_state = 3;
        g->gold += 200;
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
    if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH) {
        g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] =
            TILE_RETURN_EXIT;
        push_message(g, "A passage to town opens!");
    }
}

void game_update_level_progress(GameState *g) {
    int active_enemies = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].active) continue;
        active_enemies++;
    }

    if (active_enemies == 0) game_mark_level_cleared(g);
}

void player_gain_xp(GameState *g, int xp) {
    g->player.experience += xp;

    while (g->player.experience >= g->player.experience_next &&
           g->player.level < 50) {
        g->player.experience    -= g->player.experience_next;
        g->player.level++;
        g->player.max_hp        += 10;
        g->player.hp             = g->player.max_hp;
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
