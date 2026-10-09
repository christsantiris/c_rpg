#include "actions.h"
#include "game.h"
#include "catacombs.h"
#include "castle.h"
#include "jail.h"
#include "town_life.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "item.h"
#include "combat_feedback.h"
// sfx.h is excluded from the test runner because it links SDL2_mixer,
// which is not available in the test build. TEST_BUILD is defined in CMakeLists.txt.
#ifndef TEST_BUILD
#include "../audio/sfx.h"
#endif

static int abs_int(int n) { return n < 0 ? -n : n; }

static Action make_move_or_attack(int tx, int ty) {
    Action a;
    a.target_x = tx;
    a.target_y = ty;
    a.type     = ACTION_MOVE;
    return a;
}

void push_message(GameState *g, const char *msg) {
    push_message_kind(g, msg, MESSAGE_NORMAL);
}

void push_message_kind(GameState *g, const char *msg, MessageKind kind) {
    if (g->message_count < MAX_MESSAGES) {
        strncpy(g->messages[g->message_count], msg, MAX_MESSAGE_LEN - 1);
        g->messages[g->message_count][MAX_MESSAGE_LEN - 1] = '\0';
        g->message_kinds[g->message_count] = kind;
        g->message_count++;
    } else {
        for (int i = 0; i < MAX_MESSAGES - 1; i++) {
            strncpy(g->messages[i], g->messages[i + 1], MAX_MESSAGE_LEN);
            g->message_kinds[i] = g->message_kinds[i + 1];
        }
        strncpy(g->messages[MAX_MESSAGES - 1], msg, MAX_MESSAGE_LEN - 1);
        g->messages[MAX_MESSAGES - 1][MAX_MESSAGE_LEN - 1] = '\0';
        g->message_kinds[MAX_MESSAGES - 1] = kind;
    }
}

static void mark_item_tile(GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_DUNGEON_STAIRS_SEALED || tile == TILE_DUNGEON_STAIRS_RETURN) {
        return;
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && (tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN ||
        tile == TILE_CASTLE_LEVER || tile == TILE_CASTLE_GATE_OPEN)) {
        return;
    }
    if (tile != TILE_BURIAL_PLATE && tile != TILE_MOUNTAIN_WEAK_BRIDGE && tile != TILE_MOUNTAIN_CACHE &&
        !map_is_coast_tidal_tile(tile) && !map_is_coast_object(tile)) {
        g->map.tiles[y][x] = TILE_ITEM;
    }
}

static TileType floor_drop_underlay(const GameState *g, int x, int y) {
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *fi = &g->floor_items[i];
        if (fi->active && fi->x == x && fi->y == y) {
            return (TileType)fi->underlying_tile;
        }
    }
    return g->map.tiles[y][x];
}

static void award_gold(GameState *g, int gold) {
    g->gold += gold;
    g->score += gold;
    char msg[MAX_MESSAGE_LEN];
    snprintf(msg, sizeof(msg), "Found %d gold!", gold);
    push_message(g, msg);
}

static void place_gold_drop(GameState *g, int x, int y, int gold) {
    FloorItem fi = {0};
    fi.active = 1;
    fi.x = x;
    fi.y = y;
    fi.underlying_tile = floor_drop_underlay(g, x, y);
    fi.item.active = 1;
    fi.item.type = ITEM_GOLD;
    fi.item.value = gold;
    snprintf(fi.item.name, sizeof(fi.item.name), "%d gold", gold);
    g->floor_items[g->floor_item_count++] = fi;
    mark_item_tile(g, x, y);
    char msg[MAX_MESSAGE_LEN];
    snprintf(msg, sizeof(msg), "%d gold dropped!", gold);
    push_message(g, msg);
}

static void finish_floor_pickup(GameState *g, FloorItem *picked) {
    if (strcmp(picked->item.name, "Krakenbone Bow") == 0) {
        g->kraken_bow_unclaimed = 0;
    }
    if (strcmp(picked->item.name, "Sandstorm Staff") == 0) {
        g->sandstorm_staff_unclaimed = 0;
    }
    if (strcmp(picked->item.name, "Gravekeeper's Mantle") == 0) {
        g->catacombs_mantle_unclaimed = 0;
    }
    picked->active = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *fi = &g->floor_items[i];
        if (fi->active && fi->x == picked->x && fi->y == picked->y) {
            return;
        }
    }
    if (g->map.tiles[picked->y][picked->x] == TILE_ITEM) {
        g->map.tiles[picked->y][picked->x] = (TileType)picked->underlying_tile;
    }
}

Item boss_equipment_reward(EnemyType type) {
    switch (type) {
        case ENEMY_GRAVE_MARSHAL:
            return item_make_gravekeeper_mantle();
        case ENEMY_THORN_REGENT:
        case ENEMY_CINDER_LORD:
        case ENEMY_PRISM_SOVEREIGN:
            return item_make_strength_potion();
        case ENEMY_DESERT_PHARAOH:
            return item_make_sandstorm_staff();
        case ENEMY_LICH_KING:
            return item_make_cryptblade();
        case ENEMY_FOREST_NECROMANCER:
            return item_make_necromancer_cloak();
        case ENEMY_MOUNTAIN_GOBLIN_KING:
        case ENEMY_GOBLIN_KING:
            return item_make_goblin_king_shield();
        case ENEMY_DROWNED_QUEEN:
            return item_make_tidecaller_robes();
        case ENEMY_MINOTAUR:
            return item_make_magic_shield();
        case ENEMY_SWAMP_DEMON:
            return item_make_demonic_sword();
        case ENEMY_RED_DRAGON:
            return item_make_dragon_scale_mantle();
        case ENEMY_POLAR_KRAKEN:
            return item_make_krakenbone_bow();
        default:
            return item_make_cryptblade();
    }
}

static int enemy_score(EnemyType type) {
    switch (type) {
        case ENEMY_ANCIENT_SKELETON: return 80;
        case ENEMY_BONE_SENTINEL: return 150;
        case ENEMY_GRAVE_ARCHER: return 100;
        case ENEMY_BONE_CANTOR: return 170;
        case ENEMY_GRAVE_MARSHAL: return 2400;
        case ENEMY_SKELETON:    return 10;
        case ENEMY_GOBLIN:      return 15;
        case ENEMY_ZOMBIE:      return 20;
        case ENEMY_CRYPT_BAT:   return 18;
        case ENEMY_WRAITH:      return 40;
        case ENEMY_CRYPT_CONJURER: return 65;
        case ENEMY_PIXIE: return 15;
        case ENEMY_BLIGHTED_WOLF: return 22;
        case ENEMY_GIANT_SPIDER: return 32;
        case ENEMY_DARK_ELF: return 48;
        case ENEMY_GIANT_WURM: return 70;
        case ENEMY_FOREST_TROLL: return 80;
        case ENEMY_FOREST_NECROMANCER: return 1100;
        case ENEMY_GOBLIN_SCOUT: return 18;
        case ENEMY_GOBLIN_ARCHER: return 28;
        case ENEMY_GOBLIN_BOMBER: return 36;
        case ENEMY_TUNNEL_SPIDER: return 34;
        case ENEMY_CAVE_TROLL: return 85;
        case ENEMY_HOBGOBLIN_GUARD: return 70;
        case ENEMY_GOBLIN_SHAMAN: return 76;
        case ENEMY_MOUNTAIN_GOBLIN_KING: return 1300;
        case ENEMY_ILLUSION: return 30;
        case ENEMY_MERFOLK: return 55;
        case ENEMY_SIREN: return 80;
        case ENEMY_GIANT_CRAB: return 100;
        case ENEMY_ANIMATED_STATUE: return 145;
        case ENEMY_MINOTAUR: return 900;
        case ENEMY_WATER_ELEMENTAL: return 135;
        case ENEMY_SEA_SERPENT: return 180;
        case ENEMY_DROWNED_QUEEN: return 1600;
        case ENEMY_RELIC_SCARABS: return 85;
        case ENEMY_TEMPLE_STALKER: return 130;
        case ENEMY_BLOWDART_HUNTER: return 125;
        case ENEMY_VINEBOUND_GUARDIAN: return 180;
        case ENEMY_SUN_PRIEST: return 170;
        case ENEMY_SERPENT_SPIRIT: return 185;
        case ENEMY_TREASURE_WRAITH: return 210;
        case ENEMY_LUNAR_EFFIGY: return 220;
        case ENEMY_MOONBOUND_SENTINEL: return 300;
        case ENEMY_FALLEN_SUN_GUARDIAN: return 2200;
        case ENEMY_GIANT_RAT: return 25;
        case ENEMY_BANDIT: return 55;
        case ENEMY_VAMPIRE: return 140;
        case ENEMY_SWAMP_DEMON: return 1500;
        case ENEMY_DRAKE: return 165;
        case ENEMY_FIRE_ELEMENTAL: return 185;
        case ENEMY_ROAD_ARCHER: return 65;
        case ENEMY_HORSEMAN: return 105;
        case ENEMY_ICE_WOLF: return 50;
        case ENEMY_FROST_ARCHER: return 65;
        case ENEMY_YETI: return 110;
        case ENEMY_FROST_WRAITH: return 120;
        case ENEMY_ICE_GOLEM: return 150;
        case ENEMY_ICE_GIANT: return 190;
        case ENEMY_POLAR_KRAKEN: return 1700;
        case ENEMY_SCARAB: return 40;
        case ENEMY_VIPER: return 50;
        case ENEMY_MUMMY: return 100;
        case ENEMY_DJINN: return 130;
        case ENEMY_GOLEM: return 165;
        case ENEMY_DESERT_PHARAOH: return 1600;
        case ENEMY_FEY_TRICKSTER: return 55;
        case ENEMY_GIANT_MOTH: return 45;
        case ENEMY_LIVING_FLOWER: return 110;
        case ENEMY_THORN_GUARDIAN: return 170;
        case ENEMY_THORN_REGENT: return 1800;
        case ENEMY_CINDER_IMP: return 60;
        case ENEMY_ASH_HOUND: return 70;
        case ENEMY_OBSIDIAN_GUARDIAN: return 180;
        case ENEMY_CINDER_LORD: return 1900;
        case ENEMY_CRYSTAL_SPIDER: return 60;
        case ENEMY_BLIND_STALKER: return 85;
        case ENEMY_SHARD_GOLEM: return 190;
        case ENEMY_PRISM_SOVEREIGN: return 2000;
        case ENEMY_ORC:         return 30;
        case ENEMY_TROLL:       return 50;
        case ENEMY_GIANT:       return 80;
        case ENEMY_GOBLIN_KING: return 500;
        case ENEMY_LICH_KING:   return 1000;
        case ENEMY_DEMON_LORD:  return 2000;
        case ENEMY_RED_DRAGON:  return 3500;
        case ENEMY_TARRASQUE:   return 5000;
        default:                return 0;
    }
}

static void drop_loot(GameState *g, Enemy *enemy) {
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        castle_record_death(g, enemy);
        return;
    }
    if (g->location == LOCATION_CATACOMBS && enemy->revived) {
        return;
    }
    catacombs_record_death(g, enemy);
    int x = enemy->x;
    int y = enemy->y;
    EnemyType type = enemy->type;
    int is_boss = enemy->is_boss;
    if (enemy->dain_fragment) {
        game_record_dain_kill(g, type);
    }
    if (is_boss) {
        int first_forest_victory = g->location == LOCATION_FOREST &&
            !(g->defeated_bosses & (1 << LOCATION_FOREST));
        int first_swamp_victory = g->location == LOCATION_SWAMP &&
            !(g->defeated_bosses & (1 << LOCATION_SWAMP));
        int first_mountain_victory = g->location == LOCATION_MOUNTAINS &&
            !(g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
        g->defeated_bosses |= 1 << g->location;
        game_record_temple_enemy_defeated(g, type);
        if (type == ENEMY_THORN_REGENT) {
            push_message(g, "The Thorn Regent falls. The path to Rosemoor opens!");
        }
        if (type == ENEMY_CINDER_LORD) {
            push_message(g, "The Cinder Lord falls. The path to Ridgeshire opens!");
        }
        if (type == ENEMY_GRAVE_MARSHAL) {
            push_message(g, "The Grave Marshal falls. The passage to the castle opens!");
        }
        if (type == ENEMY_PRISM_SOVEREIGN) {
            push_message(g, "The Prism Sovereign shatters. The path to Stillbury opens!");
        }
        if (first_forest_victory) {
            g->dialogue_active = 1;
            g->dialogue_x = g->player.x;
            g->dialogue_y = g->player.y;
            snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Shortcut found");
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "You found a shortcut through the forest to %s! Take the marked shortcut beside the defeated boss, or continue through the forest.",
                g->forest_entry_town == LOCATION_TOWN2 ? "OakHaven" : "Stillbury");
        } else if (first_swamp_victory) {
            g->dialogue_active = 1;
            g->dialogue_x = g->player.x;
            g->dialogue_y = g->player.y;
            snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Shortcut found");
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "You found a shortcut through the swamp to %s! Take the marked shortcut beside the defeated boss, or continue through the swamp.",
                g->swamp_entry_town == LOCATION_TOWN3 ? "Stillbury" : "Rosemoor");
        } else if (first_mountain_victory) {
            g->dialogue_active = 1;
            g->dialogue_x = g->player.x;
            g->dialogue_y = g->player.y;
            snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Shortcut found");
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "You found a shortcut through the mountains to %s! Take the marked shortcut beside the defeated boss, or continue down the mountains.",
                g->mountain_entry_town == LOCATION_TOWN4 ? "OakHaven" : "Ridgeshire");
        }
    }
    int gold = 0;
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
        case ENEMY_ANCIENT_SKELETON: gold = 8 + rand() % 8; break;
        case ENEMY_BONE_SENTINEL: gold = 12 + rand() % 10; break;
        case ENEMY_GRAVE_ARCHER: gold = 10 + rand() % 8; break;
        case ENEMY_BONE_CANTOR: gold = 14 + rand() % 10; break;
        case ENEMY_GRAVE_MARSHAL: gold = 100; break;
        case ENEMY_SKELETON: gold = 2 + rand() % 4;  break;
        case ENEMY_GOBLIN:   gold = 3 + rand() % 5;  break;
        case ENEMY_ZOMBIE:   gold = 4 + rand() % 6;  break;
        case ENEMY_CRYPT_BAT: gold = 2 + rand() % 4; break;
        case ENEMY_WRAITH: gold = 7 + rand() % 7; break;
        case ENEMY_CRYPT_CONJURER: gold = 10 + rand() % 9; break;
        case ENEMY_PIXIE: gold = 2 + rand() % 4; break;
        case ENEMY_BLIGHTED_WOLF: gold = 3 + rand() % 5; break;
        case ENEMY_GIANT_SPIDER: gold = 4 + rand() % 6; break;
        case ENEMY_DARK_ELF: gold = 7 + rand() % 8; break;
        case ENEMY_GIANT_WURM: gold = 10 + rand() % 10; break;
        case ENEMY_FOREST_TROLL: gold = 12 + rand() % 12; break;
        case ENEMY_FOREST_NECROMANCER: gold = 50; break;
        case ENEMY_GOBLIN_SCOUT: gold = 3 + rand() % 5; break;
        case ENEMY_GOBLIN_ARCHER: gold = 5 + rand() % 7; break;
        case ENEMY_GOBLIN_BOMBER: gold = 6 + rand() % 8; break;
        case ENEMY_TUNNEL_SPIDER: gold = 4 + rand() % 6; break;
        case ENEMY_CAVE_TROLL: gold = 12 + rand() % 12; break;
        case ENEMY_HOBGOBLIN_GUARD: gold = 10 + rand() % 10; break;
        case ENEMY_GOBLIN_SHAMAN: gold = 11 + rand() % 11; break;
        case ENEMY_MOUNTAIN_GOBLIN_KING: gold = 60; break;
        case ENEMY_ILLUSION: gold = 2; break;
        case ENEMY_MERFOLK: gold = 5; break;
        case ENEMY_SIREN: gold = 8; break;
        case ENEMY_GIANT_CRAB: gold = 7; break;
        case ENEMY_ANIMATED_STATUE: gold = 12; break;
        case ENEMY_MINOTAUR: gold = 50; break;
        case ENEMY_WATER_ELEMENTAL: gold = 10; break;
        case ENEMY_SEA_SERPENT: gold = 15; break;
        case ENEMY_DROWNED_QUEEN: gold = 75; break;
        case ENEMY_RELIC_SCARABS: gold = 5; break;
        case ENEMY_TEMPLE_STALKER: gold = 7; break;
        case ENEMY_BLOWDART_HUNTER: gold = 8; break;
        case ENEMY_VINEBOUND_GUARDIAN: gold = 10; break;
        case ENEMY_SUN_PRIEST: gold = 12; break;
        case ENEMY_SERPENT_SPIRIT: gold = 9; break;
        case ENEMY_TREASURE_WRAITH: gold = 16; break;
        case ENEMY_LUNAR_EFFIGY: gold = 12; break;
        case ENEMY_MOONBOUND_SENTINEL: gold = 14; break;
        case ENEMY_FALLEN_SUN_GUARDIAN: gold = 0; break;
        case ENEMY_GIANT_RAT: gold = 3 + rand() % 5; break;
        case ENEMY_BANDIT: gold = 8 + rand() % 9; break;
        case ENEMY_VAMPIRE: gold = 14 + rand() % 12; break;
        case ENEMY_SWAMP_DEMON: gold = 70; break;
        case ENEMY_DRAKE: gold = 16 + rand() % 12; break;
        case ENEMY_FIRE_ELEMENTAL: gold = 13 + rand() % 12; break;
        case ENEMY_ROAD_ARCHER: gold = 8 + rand() % 9; break;
        case ENEMY_HORSEMAN: gold = 11 + rand() % 10; break;
        case ENEMY_ICE_WOLF: gold = 4 + rand() % 6; break;
        case ENEMY_FROST_ARCHER: gold = 8 + rand() % 8; break;
        case ENEMY_YETI: gold = 10 + rand() % 10; break;
        case ENEMY_FROST_WRAITH: gold = 9 + rand() % 9; break;
        case ENEMY_ICE_GOLEM: gold = 12 + rand() % 10; break;
        case ENEMY_ICE_GIANT: gold = 18 + rand() % 12; break;
        case ENEMY_POLAR_KRAKEN: gold = 75; break;
        case ENEMY_SCARAB: gold = 4 + rand() % 6; break;
        case ENEMY_VIPER: gold = 5 + rand() % 7; break;
        case ENEMY_MUMMY: gold = 10 + rand() % 10; break;
        case ENEMY_DJINN: gold = 11 + rand() % 10; break;
        case ENEMY_GOLEM: gold = 14 + rand() % 12; break;
        case ENEMY_DESERT_PHARAOH: gold = 75; break;
        case ENEMY_FEY_TRICKSTER: gold = 5 + rand() % 7; break;
        case ENEMY_GIANT_MOTH: gold = 4 + rand() % 6; break;
        case ENEMY_LIVING_FLOWER: gold = 10 + rand() % 10; break;
        case ENEMY_THORN_GUARDIAN: gold = 14 + rand() % 12; break;
        case ENEMY_THORN_REGENT: gold = 80; break;
        case ENEMY_CINDER_IMP: gold = 6 + rand() % 7; break;
        case ENEMY_ASH_HOUND: gold = 7 + rand() % 8; break;
        case ENEMY_OBSIDIAN_GUARDIAN: gold = 15 + rand() % 12; break;
        case ENEMY_CINDER_LORD: gold = 85; break;
        case ENEMY_CRYSTAL_SPIDER: gold = 6 + rand() % 7; break;
        case ENEMY_BLIND_STALKER: gold = 8 + rand() % 8; break;
        case ENEMY_SHARD_GOLEM: gold = 16 + rand() % 12; break;
        case ENEMY_PRISM_SOVEREIGN: gold = 90; break;
        case ENEMY_ORC:      gold = 6 + rand() % 8;  break;
        case ENEMY_TROLL:    gold = 10 + rand() % 10; break;
        case ENEMY_GIANT:    gold = 15 + rand() % 15; break;
        case ENEMY_GOBLIN_KING: break;
        case ENEMY_LICH_KING:  break;
        case ENEMY_DEMON_LORD:  break;
        case ENEMY_RED_DRAGON: gold = 120; break;
        case ENEMY_TARRASQUE:  break;
    }
    
    // Smaller purses and fewer coin drops keep routine combat income modest.
    if (is_boss && gold == 0) {
        gold = 50;
    }
    gold /= 2;
    int has_gold = gold > 0 && (is_boss || rand() % 100 < 10);

    // Each boss leaves a fixed regional reward instead of rolling ordinary
    // equipment, so capstone weapons remain Blacksmith progression.
    if (is_boss) {
        if (type == ENEMY_FALLEN_SUN_GUARDIAN) {
            if (g->floor_item_count < MAX_FLOOR_ITEMS) {
                place_gold_drop(g, x, y, gold);
            } else {
                award_gold(g, gold);
            }
            game_update_level_progress(g);
            return;
        }
        if (g->floor_item_count >= MAX_FLOOR_ITEMS) {
            FloorItem *discarded = &g->floor_items[MAX_FLOOR_ITEMS - 1];
            if (g->map.tiles[discarded->y][discarded->x] == TILE_ITEM) {
                g->map.tiles[discarded->y][discarded->x] =
                    discarded->underlying_tile;
            }
            g->floor_item_count--;
        }
        if (g->floor_item_count < MAX_FLOOR_ITEMS - 1) {
            place_gold_drop(g, x, y, gold);
        } else {
            award_gold(g, gold);
        }
        if (g->floor_item_count < MAX_FLOOR_ITEMS) {
            Item boss_drop = boss_equipment_reward(type);
            FloorItem fi = {0};
            fi.active = 1;
            fi.x = x;
            fi.y = y;
            fi.underlying_tile = floor_drop_underlay(g, x, y);
            fi.item = boss_drop;
            mark_item_tile(g, x, y);
            g->floor_items[g->floor_item_count++] = fi;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "%s dropped!", boss_drop.name);
            if (type == ENEMY_POLAR_KRAKEN) {
                g->kraken_bow_unclaimed = 1;
            }
            if (type == ENEMY_GRAVE_MARSHAL) {
                g->catacombs_mantle_unclaimed = 1;
            }
            if (type == ENEMY_DESERT_PHARAOH) {
                g->sandstorm_staff_unclaimed = 1;
            }
            push_message(g, msg);
        }
        game_update_level_progress(g);
        return;
    }

    if (g->location == LOCATION_TEMPLE) {
        if (has_gold) {
            award_gold(g, gold);
        }
        return;
    }

    TileType drop_tile = g->map.tiles[y][x];
    int plain_floor = drop_tile == TILE_FLOOR ||
        drop_tile == TILE_FOREST_FLOOR ||
        drop_tile == TILE_MOUNTAIN_FLOOR ||
        drop_tile == TILE_MOUNTAIN_CAVE_FLOOR ||
        drop_tile == TILE_MOUNTAIN_FORTRESS_FLOOR ||
        drop_tile == TILE_COAST_FLOOR ||
        drop_tile == TILE_TEMPLE_FLOOR ||
        drop_tile == TILE_LABYRINTH_FLOOR ||
        drop_tile == TILE_SWAMP_FLOOR ||
        drop_tile == TILE_DESERT_FLOOR ||
        drop_tile == TILE_MOONVEIL_FLOOR ||
        drop_tile == TILE_MOONVEIL_CIRCLE ||
        drop_tile == TILE_MOONVEIL_BLOSSOMS ||
        drop_tile == TILE_ASHEN_FLOOR ||
        drop_tile == TILE_ASHEN_RUIN ||
        drop_tile == TILE_CATACOMBS_FLOOR ||
        drop_tile == TILE_GLASSDEEP_FLOOR ||
        drop_tile == TILE_GLASSDEEP_RUIN ||
        drop_tile == TILE_FROST_FLOOR ||
        drop_tile == TILE_FROST_LAKE ||
        drop_tile == TILE_DRAGON_FLOOR ||
        drop_tile == TILE_DRAGON_ASH ||
        drop_tile == TILE_DRAGON_HOARD;
    int occupied = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *fi = &g->floor_items[i];
        if (fi->active && fi->x == x && fi->y == y) {
            occupied = 1;
            break;
        }
    }
    // Credit coins directly when another pickup or a map mechanism occupies
    // the tile, so coin markers cannot hide items, gates, or landmarks.
    if (has_gold && (occupied || !plain_floor ||
        g->floor_item_count >= MAX_FLOOR_ITEMS)) {
        award_gold(g, gold);
        has_gold = 0;
    }
    if (occupied || !plain_floor || !has_gold ||
        g->floor_item_count >= MAX_FLOOR_ITEMS) {
        return;
    }

    place_gold_drop(g, x, y, gold);
}

static int apply_melee_cleave(GameState *g, Enemy *target, int attack, int percent) {
    int hits = 0;
    int defeated = 0;

    if (percent <= 0) {
        return 0;
    }

    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (!enemy->active || enemy == target) {
            continue;
        }
        if (abs(enemy->x - target->x) > 1 ||
            abs(enemy->y - target->y) > 1) {
            continue;
        }

        int damage = attack * percent / 100 - enemy->defense;
        if (damage < 1) {
            damage = 1;
        }
        damage = catacombs_enemy_damage(g, enemy, damage);
        damage = castle_enemy_damage(g, enemy, damage);
        enemy->hp -= damage;
        combat_feedback_add(g, FEEDBACK_ENEMY_DAMAGE, FEEDBACK_NOW, enemy->x, enemy->y, damage);
        hits++;
        if (enemy->hp <= 0) {
            enemy->active = 0;
            drop_loot(g, enemy);
            player_gain_xp(g, enemy->experience);
            defeated = 1;
        }
    }

    if (defeated) {
        game_update_level_progress(g);
    }
    return hits;
}

static int equipped_spell_power(const GameState *g) {
    if (g->equipped_main_hand < 0 ||
        g->equipped_main_hand >= g->inventory_count) {
        return 0;
    }
    return g->inventory[g->equipped_main_hand].spell_power_bonus;
}

static const Item *equipped_armor(const GameState *g) {
    if (g->equipped_armor < 0 ||
        g->equipped_armor >= g->inventory_count) {
        return NULL;
    }
    return &g->inventory[g->equipped_armor];
}

static int equipped_spell_cost(const GameState *g, const Spell *spell) {
    const Item *armor = equipped_armor(g);
    if (spell->mp_cost == 0) {
        return spell->mp_cost;
    }
    int reduction = armor ? armor->spell_cost_reduction_percent : 0;
    if (g->equipped_main_hand >= 0 &&
        g->equipped_main_hand < g->inventory_count) {
        reduction += g->inventory[g->equipped_main_hand]
            .spell_cost_reduction_percent;
    }
    if (reduction > 50) {
        reduction = 50;
    }
    int cost = spell->mp_cost * (100 - reduction) / 100;
    return cost < 1 ? 1 : cost;
}

static int apply_enemy_damage(GameState *g, int damage, CombatFeedbackArrival arrival) {
    const Item *armor = equipped_armor(g);
    if (armor && armor->evasion_chance > 0 &&
        rand() % 100 < armor->evasion_chance) {
        push_message_kind(g, "Dodged!", MESSAGE_DEFENDED);
        combat_feedback_add(g, FEEDBACK_DODGE, arrival, g->player.x, g->player.y, 0);
        return 0;
    }
    int blocked = -1;
    if (g->equipped_off_hand >= 0 &&
        g->equipped_off_hand < g->inventory_count) {
        const Item *shield = &g->inventory[g->equipped_off_hand];
        if (shield->type == ITEM_SHIELD && shield->block_chance > 0 &&
            rand() % 100 < shield->block_chance) {
            int full_damage = damage;
            damage = (damage * (100 - shield->block_reduction_percent) + 99)
                / 100;
            if (damage < 1) {
                damage = 1;
            }
            blocked = full_damage - damage;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Blocked %d of %d damage!", blocked, full_damage);
            push_message_kind(g, msg, MESSAGE_DEFENDED);
        }
    }
    g->player.hp -= damage;
    combat_feedback_add(g, FEEDBACK_PLAYER_DAMAGE, arrival, g->player.x, g->player.y, damage);
    if (blocked >= 0) {
        // Added after the damage so the label stacks above the number.
        combat_feedback_add(g, FEEDBACK_BLOCK, arrival, g->player.x, g->player.y, blocked);
    }
    return damage;
}

// A shot that hits nothing shows MISS where it came to rest: the end of its
// trail, or the tile ahead when a wall stopped it at once.
static void add_miss_feedback(GameState *g) {
    int x = g->player.x + g->player.last_dx;
    int y = g->player.y + g->player.last_dy;
    if (g->trail_count > 0) {
        x = g->trail[g->trail_count - 1].x;
        y = g->trail[g->trail_count - 1].y;
    }
    combat_feedback_add(g, FEEDBACK_MISS, FEEDBACK_AFTER_PLAYER_SHOT, x, y, 0);
}

static void set_trail(GameState *g, int sx, int sy,
                      int tx, int ty, int dx, int dy,
                      int range, Uint8 r, Uint8 gr, Uint8 b,
                      TrailEffect effect) {
    g->trail_count  = 0;
    g->trail_frames = 4;
    g->trail_effect = effect;
    #ifndef TEST_BUILD
    g->trail_started_at = SDL_GetTicks();
    #else
    g->trail_started_at = 0;
    #endif
    int cx = sx;
    int cy = sy;
    for (int step = 1; step <= range; step++) {
        cx = sx + dx * step;
        cy = sy + dy * step;
        if (cx < 0 || cx >= MAP_W || cy < 0 || cy >= MAP_H) break;
        if (!map_is_walkable(&g->map, cx, cy)) break;
        if (g->trail_count >= MAX_TRAIL) break;
        TrailTile *t      = &g->trail[g->trail_count++];
        t->active         = 1;
        t->x              = cx;
        t->y              = cy;
        t->r              = r;
        t->g              = gr;
        t->b              = b;
        t->is_impact      = (cx == tx && cy == ty);
        if (t->is_impact) break;
    }
}

// Changes a map tile, keeping floor-item underlays in sync and marking it
// explored. Mountain and Frostfell terrain both use it.
static void change_mountain_tile(GameState *g, int x, int y, TileType tile) {
    g->map.tiles[y][x] = tile;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
        }
    }
    map_mark_explored(&g->map, x, y);
}

static int enemy_position_occupied(const GameState *g, int skip, int x, int y);

// After a step onto slick ice the player keeps sliding the same way, all in
// one turn, until reaching snow or meeting a wall, an enemy or anything else
// such as an exit or an item. Enemies keep their footing and never slide.
static void frost_slide(GameState *g, int old_x, int old_y) {
    int dx = g->player.x - old_x;
    int dy = g->player.y - old_y;
    if (abs_int(dx) + abs_int(dy) != 1) {
        return;
    }
    int slid = 0;
    while (g->map.tiles[g->player.y][g->player.x] == TILE_FROST_ICE) {
        int nx = g->player.x + dx;
        int ny = g->player.y + dy;
        if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H ||
            (g->map.tiles[ny][nx] != TILE_FROST_ICE &&
            g->map.tiles[ny][nx] != TILE_FROST_FLOOR) ||
            enemy_position_occupied(g, -1, nx, ny)) {
            break;
        }
        game_move_player(g, dx, dy);
        slid = 1;
    }
    if (slid) {
        // The trail records the glide from the starting tile so the screen can
        // show the player sliding rather than jumping to the end.
        int distance = abs_int(g->player.x - old_x) + abs_int(g->player.y - old_y);
        set_trail(g, old_x, old_y, g->player.x, g->player.y, dx, dy, distance, 0, 0, 0, TRAIL_EFFECT_ICE_SLIDE);
        push_message(g, "You slide across the ice.");
    }
}

// Thin ice holds while the player stays on it. The moment the player steps off
// either end, the whole shortcut collapses into open water, so a shortcut is
// always either whole or gone and can never strand anyone halfway. Creatures
// still standing on it fall through and count as a normal kill.
static void frost_thin_ice_step(GameState *g, int old_x, int old_y) {
    TileType from_tile = g->map.tiles[old_y][old_x];
    TileType to_tile = g->map.tiles[g->player.y][g->player.x];
    if (to_tile == TILE_FROST_THIN_ICE && from_tile != TILE_FROST_THIN_ICE) {
        push_message(g, "The thin ice creaks. It will give way once you step off.");
    }
    if (from_tile != TILE_FROST_THIN_ICE || to_tile == TILE_FROST_THIN_ICE) {
        return;
    }
    static int queue[MAP_W * MAP_H];
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int head = 0;
    int tail = 0;
    change_mountain_tile(g, old_x, old_y, TILE_FROST_BROKEN_ICE);
    queue[tail++] = old_y * MAP_W + old_x;
    while (head < tail) {
        int x = queue[head] % MAP_W;
        int y = queue[head] / MAP_W;
        head++;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx >= 0 && nx < MAP_W && ny >= 0 && ny < MAP_H &&
                g->map.tiles[ny][nx] == TILE_FROST_THIN_ICE) {
                change_mountain_tile(g, nx, ny, TILE_FROST_BROKEN_ICE);
                queue[tail++] = ny * MAP_W + nx;
            }
        }
    }
    int drowned = 0;
    const char *victim = "";
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && !e->is_boss &&
            g->map.tiles[e->y][e->x] == TILE_FROST_BROKEN_ICE) {
            e->active = 0;
            drop_loot(g, e);
            player_gain_xp(g, e->experience);
            victim = e->name;
            drowned++;
        }
    }
    if (drowned > 0) {
        game_update_level_progress(g);
    }
    char msg[MAX_MESSAGE_LEN];
    if (drowned == 0) {
        snprintf(msg, sizeof(msg), "The thin ice collapses behind you!");
    } else if (drowned == 1) {
        snprintf(msg, sizeof(msg), "The thin ice collapses, drowning the %s!", victim);
    } else {
        snprintf(msg, sizeof(msg), "The thin ice collapses, drowning %d creatures!", drowned);
    }
    push_message(g, msg);
}

static int mountain_obstacle(TileType tile) {
    return tile == TILE_MOUNTAIN_GATE || tile == TILE_MOUNTAIN_ROCKFALL ||
        tile == TILE_MOUNTAIN_CHASM;
}

int game_has_regional_interaction(const GameState *g) {
    if (game_has_watchfire_interaction(g)) {
        return 1;
    }
    if (game_has_moonveil_interaction(g)) {
        return 1;
    }
    if (game_has_glassdeep_interaction(g)) {
        return 1;
    }
    if (game_has_frostfell_interaction(g)) {
        return 1;
    }
    if (game_has_emberforge_interaction(g)) {
        return 1;
    }
    if (castle_has_interaction(g)) {
        return 1;
    }
    if (g->location == LOCATION_CATACOMBS) {
        return catacombs_has_interaction(g);
    }
    if (g->location == LOCATION_LABYRINTH) {
        return game_has_labyrinth_interaction(g);
    }
    if (g->location == LOCATION_TEMPLE) {
        return game_has_temple_interaction(g);
    }
    if (g->location == LOCATION_ISLAND) {
        return game_has_island_interaction(g);
    }
    if (g->location == LOCATION_COAST) {
        TileType tile = g->map.tiles[g->player.y][g->player.x];
        return map_is_coast_object(tile) && tile != TILE_COAST_CACHE;
    }
    if (g->location == LOCATION_DESERT) {
        return g->map.tiles[g->player.y][g->player.x] == TILE_DESERT_LAMP;
    }
    if (g->location != LOCATION_MOUNTAINS) {
        return 0;
    }
    if (g->map.tiles[g->player.y][g->player.x] == TILE_MOUNTAIN_CACHE) {
        return 1;
    }
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
            mountain_obstacle(g->map.tiles[y][x])) {
            return 1;
        }
    }
    return 0;
}

static int interact_mountain(GameState *g) {
    if (g->location != LOCATION_MOUNTAINS) {
        return 0;
    }
    int px = g->player.x;
    int py = g->player.y;
    if (g->map.tiles[py][px] == TILE_MOUNTAIN_CACHE) {
        int gold = 15 + map_mountain_difficulty(g->level) * 4;
        g->gold += gold;
        g->score += gold;
        change_mountain_tile(g, px, py, TILE_MOUNTAIN_CAVE_FLOOR);
        push_message(g, "You recover the buried goblin hoard!");
        return 1;
    }
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        int x = px + offsets[i][0];
        int y = py + offsets[i][1];
        if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
            continue;
        }
        TileType tile = g->map.tiles[y][x];
        if (!mountain_obstacle(tile)) {
            continue;
        }
        if (tile == TILE_MOUNTAIN_GATE) {
            change_mountain_tile(g, x, y, TILE_MOUNTAIN_FORTRESS_FLOOR);
            push_message(g, "The goblin gate opens. Watch for traps!");
        } else if (tile == TILE_MOUNTAIN_CHASM) {
            change_mountain_tile(g, x, y, TILE_MOUNTAIN_BRIDGE);
            push_message(g, "You lash a new crossing into place.");
        } else {
            // Each buried treasure passage has two ends.
            int left = x;
            int right = x;
            while (left > 0 && (g->map.tiles[y][left - 1] == TILE_MOUNTAIN_HIDDEN_CAVE ||
                g->map.tiles[y][left - 1] == TILE_MOUNTAIN_ROCKFALL)) {
                left--;
            }
            while (right < MAP_W - 1 && (g->map.tiles[y][right + 1] == TILE_MOUNTAIN_HIDDEN_CAVE ||
                g->map.tiles[y][right + 1] == TILE_MOUNTAIN_ROCKFALL)) {
                right++;
            }
            for (int cx = left; cx <= right; cx++) {
                g->map.tiles[y][cx] = TILE_MOUNTAIN_CAVE_FLOOR;
                map_mark_explored(&g->map, cx, y);
            }
            g->map.tiles[y][(left + right) / 2] = TILE_MOUNTAIN_CACHE;
            int rock_damage = 4 + map_mountain_difficulty(g->level);
            g->player.hp -= rock_damage;
            combat_feedback_add(g, FEEDBACK_PLAYER_DAMAGE, FEEDBACK_NOW, px, py, rock_damage);
            push_message_kind(g, "Falling rocks hurt! A cave is exposed.", MESSAGE_DAMAGE_TAKEN);
        }
        map_mark_explored(&g->map, x, y);
        return 1;
    }
    return 0;
}

static void coast_find_bank(GameState *g, Enemy *enemy) {
    // A rising channel carries its occupants to the nearest unoccupied bank.
    unsigned char seen[MAP_H][MAP_W] = {{0}};
    int queue[MAP_W * MAP_H];
    int head = 0;
    int tail = 0;
    queue[tail++] = enemy->y * MAP_W + enemy->x;
    seen[enemy->y][enemy->x] = 1;
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % MAP_W;
        int y = cell / MAP_W;
        int occupied = g->player.x == x && g->player.y == y;
        for (int i = 0; i < g->enemy_count && !occupied; i++) {
            Enemy *other = &g->enemies[i];
            occupied = other->active && other != enemy && other->x == x && other->y == y;
        }
        if (map_is_walkable(&g->map, x, y) && !occupied) {
            enemy->x = x;
            enemy->y = y;
            return;
        }
        for (int i = 0; i < 4; i++) {
            int nx = x + offsets[i][0];
            int ny = y + offsets[i][1];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H || seen[ny][nx]) {
                continue;
            }
            if (!map_is_walkable(&g->map, nx, ny) && !map_is_coast_tidal_tile(g->map.tiles[ny][nx])) {
                continue;
            }
            seen[ny][nx] = 1;
            queue[tail++] = ny * MAP_W + nx;
        }
    }
}

static void coast_toggle_tide(GameState *g) {
    // Restore water covered by loot in older saves before swapping the basins.
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x >= 0 && item->x < MAP_W && item->y >= 0 &&
            item->y < MAP_H && g->map.tiles[item->y][item->x] == TILE_ITEM &&
            map_is_coast_tidal_tile(item->underlying_tile)) {
            g->map.tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    int blue_drained = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = g->map.tiles[y][x];
            TileType next = map_coast_swapped_tile(tile);
            if (next == tile) {
                continue;
            }
            g->map.tiles[y][x] = next;
            blue_drained |= next == TILE_COAST_DRAINED_WATER;
            // Operating the sluice reveals its connected channels and banks.
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    map_mark_explored(&g->map, x + dx, y + dy);
                }
            }
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            item->underlying_tile = map_coast_swapped_tile(item->underlying_tile);
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (enemy->active && map_is_coast_tidal_tile(g->map.tiles[enemy->y][enemy->x]) &&
            !map_is_walkable(&g->map, enemy->x, enemy->y)) {
            coast_find_bank(g, enemy);
        }
    }
    push_message(g, blue_drained ? "Blue channels drain; amber channels rise." :
        "Blue channels rise; amber channels drain.");
}

static void tick_player_poison(GameState *g) {
    if (g->player.poison_turns > 0) {
        int damage = 3;
        g->player.hp -= damage;
        combat_feedback_add(g, FEEDBACK_PLAYER_DAMAGE, FEEDBACK_NOW, g->player.x, g->player.y, damage);
        g->player.poison_turns--;
        char message[MAX_MESSAGE_LEN];
        snprintf(message, sizeof(message), "Poison! -%d HP (%d left)", damage, g->player.poison_turns);
        push_message_kind(g, message, MESSAGE_POISON);
    }
}

void action_resolve_player(GameState *g, Action a) {
    if (g->game_won || g->castle_prompt) {
        return;
    }
    game_repair_equipment_indices(g);
    if (g->location == LOCATION_TOWN ||
        g->location == LOCATION_TAVERN ||
        g->location == LOCATION_TOWN2 ||
        g->location == LOCATION_TOWN3 ||
        g->location == LOCATION_TOWN4 ||
        g->location == LOCATION_HIGH_PASS ||
        g->location == LOCATION_SWAMP_ROAD ||
        g->location == LOCATION_CASTLE ||
        g->location == LOCATION_FOREST_ROAD ||
        g->location == LOCATION_INN ||
        g->location == LOCATION_JAIL ||
        g->location == LOCATION_WORKSHOP ||
        g->location == LOCATION_TOWN_HALL ||
        g->location == LOCATION_GUILD ||
        g->location == LOCATION_ISLAND || town_life_is_interior(g->location)) {
        g->player.poison_turns = 0;
        g->player.frozen_turns = 0;
        g->player.freeze_recovery = 0;
    }
    if (a.type == ACTION_NONE) {
        return;
    }
    // Frozen players can drink potions; other inventory actions remain free.
    if (g->player.frozen_turns > 0 && a.type != ACTION_USE_ITEM &&
        a.type != ACTION_EQUIP_ITEM && a.type != ACTION_EQUIP_OFF_HAND &&
        a.type != ACTION_DROP_ITEM) {
        g->player.frozen_turns--;
        g->player.freeze_recovery = 1;
        push_message_kind(g, "You are frozen solid and lose a turn!", MESSAGE_DAMAGE_TAKEN);
        return;
    }
    if (a.type == ACTION_WAIT) {
        return;
    }

    if (g->location == LOCATION_CASTLE_INTERIOR && (a.type == ACTION_ASCEND || a.type == ACTION_DESCEND)) {
        TileType tile = g->map.tiles[g->player.y][g->player.x];
        if (a.type == ACTION_ASCEND && tile == TILE_STAIRS_UP) {
            castle_travel(g, 1);
        } else if (a.type == ACTION_DESCEND && tile == TILE_STAIRS_DOWN) {
            castle_travel(g, 0);
        }
        return;
    }

    if (a.type == ACTION_DESCEND) {
        TileType tile = g->map.tiles[g->player.y][g->player.x];
        if (g->location == LOCATION_TEMPLE && tile == TILE_STAIRS_DOWN) {
            if (g->level > 1) {
                game_ascend(g);
            }
            return;
        }
        if (g->location == LOCATION_CATACOMBS && tile == TILE_RETURN_EXIT) {
            if (g->defeated_bosses & (1 << LOCATION_CATACOMBS)) {
                game_leave_catacombs(g);
            } else {
                push_message(g, "The Grave Marshal seals this return passage.");
            }
            return;
        }
        if (tile == TILE_RETURN_EXIT && (g->defeated_bosses & (1 << g->location))) {
            int forest = g->location == LOCATION_FOREST;
            g->score += g->level * 100;
            game_return_to_town(g);
            push_message(g, forest ? "The forest is freed!" :
                "The Lich is defeated!");
        } else if (tile == TILE_STAIRS_DOWN) {
            if (g->level < DUNGEON_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            }
        }
        return;
    }

    if (a.type == ACTION_ASCEND) {
        TileType tile = g->map.tiles[g->player.y][g->player.x];
        TileType underlay = floor_drop_underlay(g, g->player.x, g->player.y);
        if (g->location == LOCATION_DUNGEON &&
            (underlay == TILE_DUNGEON_STAIRS_RETURN || underlay == TILE_DUNGEON_STAIRS_SEALED)) {
            if (g->defeated_bosses & (1 << LOCATION_DUNGEON)) {
                g->score += g->level * 100;
                game_return_to_town(g);
                push_message(g, "You climb the stairs back to Oakhaven.");
            } else {
                push_message(g, "The Lich King seals these stairs.");
            }
            return;
        }
        if (g->location == LOCATION_TEMPLE && underlay == TILE_STAIRS_UP) {
            if (game_temple_remaining_enemies(g) > 0) {
                push_message(g, "Stairs sealed: clear all Sun and Moon enemies. Use an altar to awaken dormant sentinels.");
                return;
            }
            if (g->level < TEMPLE_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            }
            return;
        }
        if (tile == TILE_STAIRS_UP) {
            if (g->level == 1 && g->location == LOCATION_CATACOMBS) {
                game_leave_catacombs(g);
            } else if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
        }
        return;
    }

    if (a.type == ACTION_INTERACT) {
        if (game_interact_watchfire(g)) {
            return;
        }
        if (game_interact_moonveil(g)) {
            return;
        }
        if (game_interact_glassdeep(g)) {
            return;
        }
        if (game_interact_frostfell(g)) {
            return;
        }
        if (game_interact_emberforge(g)) {
            return;
        }
        if (castle_interact(g)) {
            return;
        }
        if (catacombs_interact(g)) {
            return;
        }
        if (game_interact_labyrinth(g)) {
            return;
        }
        if (game_interact_temple(g)) {
            return;
        }
        if (game_interact_island(g)) {
            return;
        }
        if (interact_mountain(g)) {
            return;
        }
        TileType tile = g->map.tiles[g->player.y][g->player.x];
        if (tile == TILE_DESERT_LAMP) {
            game_collect_desert_lamp(g);
            return;
        }
        if (tile == TILE_BROKEN_BURIAL_SEAL) {
            if (g->elowen_quest_state != 1) {
                push_message(g, "A shattered burial seal lies here.");
                return;
            }
            int seal_index = g->level - 2;
            if (seal_index < 0 || seal_index >= 3) {
                return;
            }
            g->elowen_seals_restored |= 1 << seal_index;
            g->map.tiles[g->player.y][g->player.x] =
                TILE_RESTORED_BURIAL_SEAL;
            push_message(g, "Burial seal restored.");
            if ((g->elowen_seals_restored & 7) == 7) {
                g->elowen_quest_state = 2;
                push_message(g, "All seals restored. Return to Elowen in Oakhaven's Tavern.");
            }
            return;
        }
        if (tile == TILE_COAST_BEACON_UNLIT) {
            game_light_coast_beacon(g, g->player.x, g->player.y);
            return;
        }
        if (tile == TILE_COAST_BEACON_LIT) {
            push_message(g, "The beacon already burns brightly.");
            return;
        }
        if (tile == TILE_COAST_TIDE_CONTROL || tile == TILE_COAST_SLUICE_CONTROL) {
            coast_toggle_tide(g);
            return;
        }
        push_message(g, "There is nothing to interact with here.");
        return;
    }

    if (a.type == ACTION_PICK_UP) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_DRAGON_TREASURE) {
            game_collect_dragon_treasure(g);
            return;
        }
        if (g->map.tiles[g->player.y][g->player.x] == TILE_CRYPT_CACHE) {
            int gold = 10 + g->level * 2;
            g->gold += gold;
            g->score += gold;
            g->map.tiles[g->player.y][g->player.x] = TILE_FLOOR;
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message),
                "The crypt cache holds %d gold!", gold);
            push_message(g, message);
            return;
        }
        if (g->map.tiles[g->player.y][g->player.x] == TILE_COAST_CACHE) {
            int gold = 20 + g->level * 4;
            g->gold += gold;
            g->score += gold;
            g->map.tiles[g->player.y][g->player.x] = TILE_COAST_FLOOR;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == g->player.x && item->y == g->player.y) {
                    item->underlying_tile = TILE_COAST_FLOOR;
                    mark_item_tile(g, item->x, item->y);
                }
            }
            push_message(g, "You recover the sunken chamber's hoard!");
            return;
        }
        if (g->map.tiles[g->player.y][g->player.x] == TILE_CRYPT_KEY) {
            g->dungeon_crypt_keys++;
            g->map.tiles[g->player.y][g->player.x] = TILE_FLOOR;
            push_message(g, "Picked up a crypt key.");
            return;
        }
        if (g->map.tiles[g->player.y][g->player.x] == TILE_DUNGEON_KEY) {
            g->dungeon_key_found = 1;
            g->map.tiles[g->player.y][g->player.x] = TILE_FLOOR;
            push_message(g, "Picked up the Lich King's door key!");
            return;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *fi = &g->floor_items[i];
            if (!fi->active) continue;
            if (fi->x != g->player.x || fi->y != g->player.y) continue;
            if (fi->item.type == ITEM_GOLD) {
                g->gold += fi->item.value;
                g->score += fi->item.value;
                finish_floor_pickup(g, fi);
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Picked up %d gold", fi->item.value);
                push_message(g, msg);
                return;
            }
            if (g->inventory_count >= MAX_INVENTORY) {
                push_message(g, "Inventory full!");
                return;
            }
            g->inventory[g->inventory_count++] = fi->item;
            finish_floor_pickup(g, fi);
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Picked up %s", fi->item.name);
            push_message(g, msg);
            return;
        }
        push_message(g, "Nothing to pick up");
        return;
    }

    if (a.type == ACTION_USE_ITEM) {
        int idx = a.target_x;
        if (idx < 0 || idx >= g->inventory_count) {
            return;
        }
        Item *item = &g->inventory[idx];
        int drinking = item->type == ITEM_POTION_HEALTH || item->type == ITEM_POTION_MANA ||
            item->type == ITEM_POTION_STRENGTH || item->type == ITEM_POTION_INTELLIGENCE;
        char msg[MAX_MESSAGE_LEN];

        if (item->type == ITEM_TREASURE_MAP) {
            push_message(g, "The map charts a sea route to an island and marks treasure beneath a ruined temple.");
            return;
        }
        if (item->type == ITEM_POTION_HEALTH) {
            if (g->player.hp >= g->player.max_hp) {
                push_message(g, "HP is already full");
                return;
            }
            int healed = g->player.max_hp - g->player.hp;
            g->player.hp = g->player.max_hp;
            snprintf(msg, sizeof(msg), "Drank %s +%d HP", item->name, healed);
            push_message(g, msg);
        } else if (item->type == ITEM_POTION_MANA) {
            if (g->player.mp >= g->player.max_mp) {
                push_message(g, "MP is already full");
                return;
            }
            int restored = g->player.max_mp - g->player.mp;
            g->player.mp = g->player.max_mp;
            snprintf(msg, sizeof(msg), "Drank %s +%d MP", item->name, restored);
            push_message(g, msg);
        } else if (item->type == ITEM_POTION_STRENGTH) {
            g->player.attack += 1;
            push_message(g, "Potion of Strength: base attack permanently increased by 1.");
        } else if (item->type == ITEM_POTION_INTELLIGENCE) {
            g->player.max_mp += 1;
            g->player.mp += 1;
            push_message(g, "Potion of Intelligence: max MP permanently increased by 1.");
        } else if (item->type == ITEM_SPELL_TOME) {
            if (!item_class_allowed(item, g->player.player_class)) {
                push_message(g, "Only a Mage can study that tome");
                return;
            }
            Spell *known = NULL;
            for (int i = 0; i < g->player.known_spell_count; i++) {
                if (g->player.known_spells[i].id == item->spell_id) {
                    known = &g->player.known_spells[i];
                    break;
                }
            }
            if (!known) {
                push_message(g, "Learn the spell before upgrading it");
                return;
            }
            if (!spell_upgrade(known)) {
                push_message(g, "That spell cannot be upgraded further");
                return;
            }
            snprintf(msg, sizeof(msg), "%s reached rank %d!", known->name,
                known->rank);
            push_message(g, msg);
        } else if (item->type == ITEM_SCROLL) {
            if (!item_class_allowed(item, g->player.player_class)) {
                push_message(g, "Your class cannot learn that spell");
                return;
            }
            for (int i = 0; i < g->player.known_spell_count; i++) {
                if (g->player.known_spells[i].id == item->spell_id) {
                    push_message(g, "Already know that spell");
                    return;
                }
            }
            if (g->player.known_spell_count >= MAX_SPELLS) {
                push_message(g, "Cannot learn more spells");
                return;
            }
            Spell learned;
            switch (item->spell_id) {
                case SPELL_MAGIC_ARROW: learned = spell_make_magic_arrow(); break;
                case SPELL_FIREBALL:    learned = spell_make_fireball();    break;
                case SPELL_HEAL:        learned = spell_make_heal();        break;
                case SPELL_FROST_BOLT:
                    learned = spell_make_frost_bolt();
                    break;
                case SPELL_TELEPORT: learned = spell_make_teleport(); break;
                case SPELL_RETURN_TO_TOWN:
                    learned = spell_make_return_to_town();
                    break;
                default: return;
            }
            g->player.known_spells[g->player.known_spell_count++] = learned;
            snprintf(msg, sizeof(msg), "Learned %s!", learned.name);
            push_message(g, msg);
        } else {
            push_message(g, "Cannot use that item");
            return;
        }

        game_remove_inventory_item(g, idx);
        if (drinking) {
            if (g->player.frozen_turns > 0) {
                g->player.frozen_turns--;
                g->player.freeze_recovery = 1;
            }
            tick_player_poison(g);
        }
        return;
    }

    if (a.type == ACTION_EQUIP_ITEM) {
        int idx = a.target_x;
        if (idx < 0 || idx >= g->inventory_count) {
            return;
        }
        Item *item = &g->inventory[idx];
        char msg[MAX_MESSAGE_LEN];

        if (item->type == ITEM_WEAPON) {
            if (!game_equip_main_hand(g, idx)) {
                push_message(g, "Your class cannot equip that");
                return;
            }
            snprintf(msg, sizeof(msg), "Equipped %s", item->name);
            push_message(g, msg);
        } else if (item->type == ITEM_ARMOR) {
            if (!item_class_allowed(item, g->player.player_class)) {
                push_message(g, "Your class cannot equip that");
                return;
            }
            if (g->equipped_main_hand == idx) {
                g->equipped_main_hand = -1;
            }
            if (g->equipped_off_hand == idx) {
                g->equipped_off_hand = -1;
            }
            if (g->equipped_armor >= 0 &&
                g->equipped_armor < g->inventory_count) {
                game_remove_armor_bonuses(g,
                    &g->inventory[g->equipped_armor]);
            }
            g->equipped_armor  = idx;
            game_apply_armor_bonuses(g, item);
            snprintf(msg, sizeof(msg), "Equipped %s", item->name);
            push_message(g, msg);
        } else if (item->type == ITEM_SHIELD) {
            if (!game_equip_shield(g, idx)) {
                push_message(g, "Shield requires one-handed weapon");
                return;
            }
            snprintf(msg, sizeof(msg), "Off-hand: %s", item->name);
            push_message(g, msg);
        } else {
            push_message(g, "Cannot equip that item");
        }
        return;
    }
    if (a.type == ACTION_EQUIP_OFF_HAND) {
        int idx = a.target_x;
        if (game_equip_off_hand(g, idx)) {
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Off-hand: %s",
                g->inventory[idx].name);
            push_message(g, msg);
        } else {
            push_message(g, "Requires two one-handed weapons");
        }
        return;
    }
    if (a.type == ACTION_DROP_ITEM) {
        int idx = a.target_x;
        if (idx < 0 || idx >= g->inventory_count) return;
        if (g->inventory[idx].type == ITEM_TREASURE_MAP) {
            push_message(g, "Keep the treasure map for your island voyage.");
            return;
        }
        if (g->floor_item_count >= MAX_FLOOR_ITEMS) {
            push_message(g, "No room to drop item!");
            return;
        }
        // Refuse drops on thin ice: an item tile would split the shortcut, and the
        // item would sink when it collapses.
        if (g->map.tiles[g->player.y][g->player.x] == TILE_FROST_THIN_ICE) {
            push_message(g, "Anything dropped here would sink when the ice gives way.");
            return;
        }

        Item item = g->inventory[idx];

        // Place on floor
        FloorItem fi = {0};
        fi.active = 1;
        fi.x      = g->player.x;
        fi.y      = g->player.y;
        fi.underlying_tile = floor_drop_underlay(g, fi.x, fi.y);
        fi.item   = item;
        mark_item_tile(g, fi.x, fi.y);
        g->floor_items[g->floor_item_count++] = fi;

        game_remove_inventory_item(g, idx);

        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Dropped %s", fi.item.name);
        push_message(g, msg);
        return;
    }

    if (a.type == ACTION_CAST_SPELL) {
        if (g->player.equipped_spell < 0 ||
            g->player.equipped_spell >= g->player.known_spell_count) {
            push_message(g, "No spell equipped!");
            return;
        }

        Spell *sp = &g->player.known_spells[g->player.equipped_spell];
        int spell_power = equipped_spell_power(g);
        int mana_cost = equipped_spell_cost(g, sp);

        if (g->player.mp < mana_cost) {
            push_message(g, "Not enough MP!");
            return;
        }

        if (sp->id == SPELL_RETURN_TO_TOWN) {
            if (jail_blocks_portal(g)) {
                return;
            }
            if (g->location == LOCATION_FOREST_ROAD ||
                g->location == LOCATION_HIGH_PASS ||
                g->location == LOCATION_SWAMP_ROAD ||
                game_is_king_road(g)) {
                push_message(g, "A town is just ahead on the road.");
                return;
            }
            if (g->location == LOCATION_TOWN ||
                g->location == LOCATION_TOWN2 ||
                g->location == LOCATION_TOWN3 ||
                g->location == LOCATION_TOWN4 ||
                g->location == LOCATION_CASTLE ||
                g->location == LOCATION_TAVERN ||
                g->location == LOCATION_INN || g->location == LOCATION_WORKSHOP ||
                g->location == LOCATION_TOWN_HALL || g->location == LOCATION_GUILD ||
                town_life_is_interior(g->location)) {
                push_message(g, "Already in town!");
                return;
            }
            if (g->location == LOCATION_ISLAND) {
                game_leave_island(g);
                return;
            }
            game_open_town_portal(g);
            return;
        }

        if (sp->id == SPELL_TELEPORT) {
            int start_x = g->player.x;
            int start_y = g->player.y;
            int destination_x = start_x;
            int destination_y = start_y;
            for (int step = 1; step <= sp->range; step++) {
                int x = start_x + g->player.last_dx * step;
                int y = start_y + g->player.last_dy * step;
                if (!map_is_walkable(&g->map, x, y)) {
                    break;
                }
                int occupied = jail_prisoner_at(g, x, y);
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active && g->enemies[i].x == x &&
                        g->enemies[i].y == y) {
                        occupied = 1;
                        break;
                    }
                }
                if (occupied) {
                    break;
                }
                destination_x = x;
                destination_y = y;
            }
            if (destination_x == start_x && destination_y == start_y) {
                push_message(g, "Teleport path is blocked");
                return;
            }
            g->player.mp -= mana_cost;
            int distance = abs_int(destination_x - start_x) +
                abs_int(destination_y - start_y);
            set_trail(g, start_x, start_y, destination_x, destination_y,
                g->player.last_dx, g->player.last_dy, distance,
                155, 90, 235, TRAIL_EFFECT_GENERIC);
            g->player.x = destination_x;
            g->player.y = destination_y;
            push_message(g, "Teleported!");
            // Teleporting onto or off thin ice counts like a step on it.
            frost_thin_ice_step(g, start_x, start_y);
            return;
        }

        if (sp->type == SPELL_TYPE_HEAL &&
            g->player.hp >= g->player.max_hp) {
            push_message(g, "HP is already full");
            return;
        }

        if (g->player.last_dx == 0 && g->player.last_dy == 0) {
            push_message(g, "Move first to aim!");
            return;
        }

        g->player.mp -= mana_cost;

        // Set trail based on spell type
        if (sp->type == SPELL_TYPE_DAMAGE_RANGED) {
            #ifndef TEST_BUILD
            if (sp->id == SPELL_MAGIC_ARROW) {
                sfx_play_magic_arrow();
            } else if (sp->id == SPELL_FROST_BOLT) {
                sfx_play_frost_bolt();
            }
            #endif
            int ex = g->player.x + g->player.last_dx * sp->range;
            int ey = g->player.y + g->player.last_dy * sp->range;
            if (sp->id == SPELL_FROST_BOLT) {
                set_trail(g, g->player.x, g->player.y, ex, ey,
                    g->player.last_dx, g->player.last_dy,
                    sp->range, 120, 225, 255, TRAIL_EFFECT_MAGIC_ARROW);
            } else {
                set_trail(g, g->player.x, g->player.y, ex, ey,
                    g->player.last_dx, g->player.last_dy,
                    sp->range, 40, 120, 220, TRAIL_EFFECT_MAGIC_ARROW);
            }
        } else if (sp->type == SPELL_TYPE_DAMAGE_AREA) {
            #ifndef TEST_BUILD
            sfx_play_fireball();
            #endif
            int ex = g->player.x + g->player.last_dx * sp->range;
            int ey = g->player.y + g->player.last_dy * sp->range;
            set_trail(g, g->player.x, g->player.y,
                ex, ey,
                g->player.last_dx, g->player.last_dy,
                sp->range, 220, 100, 20, TRAIL_EFFECT_FIREBALL);
        } else if (sp->type == SPELL_TYPE_HEAL) {
            #ifndef TEST_BUILD
            sfx_play_heal();
            #endif
            // Heal — green ring on player tile
            g->trail_count  = 0;
            g->trail_frames = 4;
            g->trail_effect = TRAIL_EFFECT_GENERIC;
            TrailTile *t = &g->trail[g->trail_count++];
            t->active    = 1;
            t->x         = g->player.x;
            t->y         = g->player.y;
            t->r         = 40;
            t->g         = 180;
            t->b         = 80;
            t->is_impact = 1;
        }

        if (sp->type == SPELL_TYPE_DAMAGE_RANGED) {
            // Travel in last direction, hit first enemy
            int cx = g->player.x;
            int cy = g->player.y;
            int hit = 0;
            for (int step = 1; step <= sp->range && !hit; step++) {
                cx = g->player.x + g->player.last_dx * step;
                cy = g->player.y + g->player.last_dy * step;
                if (!map_is_walkable(&g->map, cx, cy)) break;
                for (int i = 0; i < g->enemy_count; i++) {
                    Enemy *e = &g->enemies[i];
                    if (!e->active) continue;
                    if (e->x == cx && e->y == cy) {
                        // Stop the projectile at the enemy it actually hits.
                        if (step <= g->trail_count) {
                            g->trail_count = step;
                            g->trail[step - 1].is_impact = 1;
                        }
                        int dmg = sp->damage + g->player.level * 2 +
                            spell_power;
                        dmg = catacombs_enemy_damage(g, e, dmg);
                        dmg = castle_enemy_damage(g, e, dmg);
                        e->hp -= dmg;
                        combat_feedback_add(g, FEEDBACK_ENEMY_DAMAGE, FEEDBACK_AFTER_PLAYER_SHOT, e->x, e->y, dmg);
                        char msg[MAX_MESSAGE_LEN];
                        if (e->hp <= 0) {
                            e->active = 0;
                            game_update_level_progress(g);
                            drop_loot(g, e);
                            player_gain_xp(g, e->experience);
                            g->score += e->revived ? 0 : enemy_score(e->type);
                            snprintf(msg, sizeof(msg), "%s killed %s!",
                                sp->name, e->name);
                        } else if (sp->id == SPELL_FROST_BOLT &&
                            (e->type == ENEMY_ICE_GOLEM ||
                            e->type == ENEMY_POLAR_KRAKEN)) {
                            snprintf(msg, sizeof(msg),
                                "%s shrugs off the frost: %d dmg", e->name,
                                dmg);
                        } else if (sp->id == SPELL_FROST_BOLT) {
                            e->frozen_turns = 2;
                            snprintf(msg, sizeof(msg),
                                "Frost Bolt froze %s: %d dmg", e->name,
                                dmg);
                        } else {
                            snprintf(msg, sizeof(msg), "%s hit %s: %d dmg",
                                sp->name, e->name, dmg);
                        }
                        push_message(g, msg);
                        hit = 1;
                    }
                }
            }
            if (!hit) {
                push_message(g, "Spell missed!");
                add_miss_feedback(g);
            }

        } else if (sp->type == SPELL_TYPE_HEAL) {
            int healed = g->player.max_hp - g->player.hp;
            g->player.hp = g->player.max_hp;
            combat_feedback_add(g, FEEDBACK_HEAL, FEEDBACK_NOW, g->player.x, g->player.y, healed);
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Healed %d HP!", healed);
            push_message(g, msg);

        } else if (sp->type == SPELL_TYPE_DAMAGE_AREA) {
            // Explode on the first enemy, or at the end of the visible path.
            if (g->trail_count == 0) {
                push_message(g, "Fireball path is blocked!");
                add_miss_feedback(g);
                return;
            }
            for (int step = 0; step < g->trail_count; step++) {
                int impact = 0;
                for (int i = 0; i < g->enemy_count; i++) {
                    const Enemy *enemy = &g->enemies[i];
                    if (enemy->active && enemy->x == g->trail[step].x &&
                        enemy->y == g->trail[step].y) {
                        impact = 1;
                        break;
                    }
                }
                if (impact) {
                    g->trail_count = step + 1;
                    break;
                }
            }
            TrailTile *impact = &g->trail[g->trail_count - 1];
            impact->is_impact = 1;
            int cx = impact->x;
            int cy = impact->y;
            int hits = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                if (!e->active) {
                    continue;
                }
                int dx = e->x - cx;
                int dy = e->y - cy;
                int dist = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
                if (dist <= sp->radius) {
                    int dmg = sp->damage + g->player.level * 2 + spell_power;
                    dmg = catacombs_enemy_damage(g, e, dmg);
                    dmg = castle_enemy_damage(g, e, dmg);
                    e->hp -= dmg;
                    combat_feedback_add(g, FEEDBACK_ENEMY_DAMAGE, FEEDBACK_AFTER_PLAYER_SHOT, e->x, e->y, dmg);
                    if (e->hp <= 0) {
                        e->active = 0;
                        drop_loot(g, e);
                        player_gain_xp(g, e->experience);
                    }
                    hits++;
                }
            }
            game_update_level_progress(g);
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Fireball hit %d enemies!", hits);
            push_message(g, msg);
            if (hits == 0) {
                add_miss_feedback(g);
            }
        }
        return;
    }

    if (a.type == ACTION_RANGED_ATTACK) {
        if (g->equipped_main_hand < 0 ||
            g->equipped_main_hand >= g->inventory_count) {
            push_message(g, "No weapon equipped!");
            return;
        }

        Item *wpn = &g->inventory[g->equipped_main_hand];
        if (!wpn->is_ranged) {
            push_message(g, "No ranged weapon equipped!");
            return;
        }

        if (g->player.last_dx == 0 && g->player.last_dy == 0) {
            push_message(g, "Move first to aim!");
            return;
        }

        if (wpn->weapon_family == WEAPON_FAMILY_BOW) {
            if (g->player.arrows <= 0) {
                push_message(g, "No arrows! Buy more at the Blacksmith or equip a melee weapon.");
                return;
            }
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                if (!e->active) {
                    continue;
                }
                int target_x = g->player.x + g->player.last_dx;
                int target_y = g->player.y + g->player.last_dy;
                if (e->x == target_x && e->y == target_y) {
                    push_message(g, "Too close to use bow!");
                    return;
                }
            }
            g->player.arrows--;
        }

        #ifndef TEST_BUILD
        if (wpn->visual_id == ITEM_VISUAL_DEMONIC_SWORD) {
            sfx_play_demonic_sword();
        } else if (wpn->weapon_family == WEAPON_FAMILY_SWORD) {
            sfx_play_attack();
        } else {
            sfx_play_arrow();
        }
        #endif

        int hit = 0;
        int impact_x = g->player.x + g->player.last_dx * wpn->range;
        int impact_y = g->player.y + g->player.last_dy * wpn->range;
        for (int step = 1;
            step <= wpn->range && (!hit || wpn->pierces_targets); step++) {
            int tx = g->player.x + g->player.last_dx * step;
            int ty = g->player.y + g->player.last_dy * step;
            if (!map_is_walkable(&g->map, tx, ty)) break;
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                if (!e->active) continue;
                if (e->x != tx || e->y != ty) continue;
                if (step < 2 && wpn->weapon_family == WEAPON_FAMILY_BOW) {
                    continue;
                }
                int dmg = g->player.attack - e->defense;
                if (dmg < 1) dmg = 1;
                int critical = wpn->weapon_family == WEAPON_FAMILY_BOW &&
                    rand() % 100 < 15;
                if (critical) dmg = dmg * 3 / 2;
                dmg = catacombs_enemy_damage(g, e, dmg);
                dmg = castle_enemy_damage(g, e, dmg);
                e->hp -= dmg;
                combat_feedback_add(g, critical ? FEEDBACK_ENEMY_CRITICAL : FEEDBACK_ENEMY_DAMAGE, FEEDBACK_AFTER_PLAYER_SHOT, e->x, e->y, dmg);
                char msg[MAX_MESSAGE_LEN];
                if (e->hp <= 0) {
                    e->active = 0;
                    game_update_level_progress(g);
                    drop_loot(g, e);
                    player_gain_xp(g, e->experience);
                    snprintf(msg, sizeof(msg), "Attack killed %s!", e->name);
                } else if (critical) {
                    snprintf(msg, sizeof(msg), "Critical hit %s: %d dmg",
                        e->name, dmg);
                } else {
                    snprintf(msg, sizeof(msg), "Attack hit %s: %d dmg",
                        e->name, dmg);
                }
                push_message_kind(g, msg, critical ? MESSAGE_CRITICAL : MESSAGE_NORMAL);
                if (!wpn->pierces_targets) {
                    impact_x = tx;
                    impact_y = ty;
                }
                hit = 1;
            }
        }
        int demonic = wpn->visual_id == ITEM_VISUAL_DEMONIC_SWORD;
        set_trail(g, g->player.x, g->player.y,
            impact_x, impact_y,
            g->player.last_dx, g->player.last_dy,
            wpn->range, demonic ? 220 : 160, demonic ? 58 : 160,
            demonic ? 67 : 160, demonic ? TRAIL_EFFECT_DEMONIC_SWORD :
            TRAIL_EFFECT_WEAPON_ARROW);
        if (!hit) {
            push_message(g, "Attack missed!");
            add_miss_feedback(g);
        }
        if (wpn->weapon_family == WEAPON_FAMILY_BOW && g->player.arrows == 0) {
            push_message(g, "Last arrow fired. Buy more at the Blacksmith or equip a melee weapon.");
        }
        return;
    }

    if (a.type == ACTION_MOVE) {
        int tx = a.target_x;
        int ty = a.target_y;

        if (g->location == LOCATION_ISLAND && tx >= 0 && tx < MAP_W &&
            ty >= 0 && ty < MAP_H &&
            g->map.tiles[ty][tx] == TILE_ISLAND_TEMPLE_GATE) {
            game_enter_temple(g);
            return;
        }

        if (g->location == LOCATION_TEMPLE && tx >= 0 && tx < MAP_W &&
            ty >= 0 && ty < MAP_H &&
            g->map.tiles[ty][tx] == TILE_TEMPLE_ENTRANCE) {
            game_leave_temple(g);
            return;
        }

        // Check for enemy at target
        if (jail_move_exit(g, tx, ty)) {
            return;
        }
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (!e->active) continue;
            if (e->x == tx && e->y == ty) {
                // Melee attack
                g->player.last_dx = tx - g->player.x;
                g->player.last_dy = ty - g->player.y;
                int melee_attack = g->player.attack;
                Item *melee_weapon = NULL;
                if (g->equipped_main_hand >= 0 &&
                    g->equipped_main_hand < g->inventory_count) {
                    melee_weapon = &g->inventory[g->equipped_main_hand];
                    if (melee_weapon->weapon_family == WEAPON_FAMILY_BOW) {
                        melee_attack -= melee_weapon->attack_bonus;
                    }
                }
                int effective_defense = e->defense;
                if (melee_weapon &&
                    melee_weapon->armor_penetration_percent > 0) {
                    effective_defense = effective_defense *
                        (100 - melee_weapon->armor_penetration_percent) / 100;
                }
                int dmg = melee_attack - effective_defense;
                if (dmg < 1) {
                    dmg = 1;
                }
                int critical_chance = melee_weapon &&
                    melee_weapon->weapon_family != WEAPON_FAMILY_BOW
                    ? melee_weapon->critical_chance_bonus : 0;
                if (g->equipped_off_hand >= 0 &&
                    g->equipped_off_hand < g->inventory_count) {
                    critical_chance +=
                        g->inventory[g->equipped_off_hand]
                            .critical_chance_bonus / 2;
                }
                int critical = critical_chance > 0 &&
                    rand() % 100 < critical_chance;
                if (critical) {
                    dmg = (dmg * 3 + 1) / 2;
                }
                dmg = catacombs_enemy_damage(g, e, dmg);
                dmg = castle_enemy_damage(g, e, dmg);
                e->hp -= dmg;
                combat_feedback_add(g, critical ? FEEDBACK_ENEMY_CRITICAL : FEEDBACK_ENEMY_DAMAGE, FEEDBACK_NOW, e->x, e->y, dmg);
                int cleave_hits = melee_weapon
                    ? apply_melee_cleave(g, e, melee_attack,
                        melee_weapon->cleave_percent)
                    : 0;
                #ifndef TEST_BUILD
                int melee_visual = melee_weapon ? melee_weapon->visual_id : ITEM_VISUAL_NONE;
                if (!melee_weapon) {
                    sfx_play_punch();
                } else if (melee_visual == ITEM_VISUAL_DEMONIC_SWORD) {
                    sfx_play_demonic_sword();
                } else if (melee_visual == ITEM_VISUAL_MAGIC_LONG_SWORD ||
                    melee_visual == ITEM_VISUAL_MAGIC_GREATSWORD) {
                    sfx_play_magic_sword();
                } else if (melee_visual == ITEM_VISUAL_SHORT_SWORD ||
                    melee_visual == ITEM_VISUAL_LONG_SWORD || melee_visual == ITEM_VISUAL_GREATSWORD) {
                    sfx_play_large_blade();
                } else if (melee_visual == ITEM_VISUAL_MAGIC_DAGGER) {
                    sfx_play_magic_dagger();
                } else if (melee_visual == ITEM_VISUAL_MAGIC_BATTLE_AXE) {
                    sfx_play_magic_axe();
                } else if (melee_visual == ITEM_VISUAL_BATTLE_AXE) {
                    sfx_play_axe();
                } else if (melee_weapon->weapon_family == WEAPON_FAMILY_STAFF ||
                    melee_weapon->weapon_family == WEAPON_FAMILY_BOW) {
                    sfx_play_punch();
                } else {
                    sfx_play_attack();
                }
                #endif
                if (e->hp <= 0) {
                    e->active = 0;
                    drop_loot(g, e);
                    player_gain_xp(g, e->experience);
                    game_update_level_progress(g);
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), critical ?
                        "Critical killed %s!" : "Killed %s!", e->name);
                    push_message_kind(g, msg, critical ? MESSAGE_CRITICAL : MESSAGE_NORMAL);
                } else {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), critical ?
                        "Critical hit %s: %d dmg" : "Hit %s: %d dmg",
                        e->name, dmg);
                    push_message_kind(g, msg, critical ? MESSAGE_CRITICAL : MESSAGE_NORMAL);
                }
                if (cleave_hits > 0) {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), "Cleave struck %d nearby!",
                        cleave_hits);
                    push_message(g, msg);
                }
                return;
            }
        }
        // Check for town exit
        if (g->map.tiles[ty][tx] == TILE_LOCAL_DOOR) {
            town_life_enter(g);
            return;
        }
        if (town_life_is_interior(g->location) && g->map.tiles[ty][tx] == TILE_TAVERN_EXIT) {
            town_life_leave(g);
            return;
        }
        if (g->location == LOCATION_TOWN4 && g->map.tiles[ty][tx] == TILE_TOWN_HALL_DOOR) {
            game_enter_town_hall(g);
            return;
        }
        if (g->location == LOCATION_TOWN_HALL && g->map.tiles[ty][tx] == TILE_TAVERN_EXIT) {
            game_leave_town_hall(g);
            return;
        }
        if (g->location == LOCATION_TOWN4 && g->map.tiles[ty][tx] == TILE_WORKSHOP_DOOR) {
            game_enter_workshop(g);
            return;
        }
        if (g->location == LOCATION_WORKSHOP && g->map.tiles[ty][tx] == TILE_TAVERN_EXIT) {
            game_leave_workshop(g);
            return;
        }
        if (g->location == LOCATION_TOWN3 && g->map.tiles[ty][tx] == TILE_GUILD_DOOR) {
            game_enter_guild(g);
            return;
        }
        if ((g->location == LOCATION_TOWN || g->location == LOCATION_TOWN2 ||
            g->location == LOCATION_TOWN3 || g->location == LOCATION_TOWN4) &&
            g->map.tiles[ty][tx] == TILE_PORTAL && g->portal_active) {
            game_use_town_portal(g);
            return;
        }

        if ((g->location == LOCATION_TOWN || g->location == LOCATION_TOWN2) &&
            g->map.tiles[ty][tx] == TILE_TAVERN_DOOR) {
            if (g->location == LOCATION_TOWN) {
                game_enter_tavern(g);
            } else {
                game_enter_inn(g);
            }
            return;
        }

        if (g->location == LOCATION_TOWN2 &&
            g->map.tiles[ty][tx] == TILE_LABYRINTH_ENTRANCE) {
            if (game_labyrinth_is_open(g)) {
                game_enter_labyrinth(g);
            } else {
                push_message(g, "The old labyrinth gate is sealed.");
            }
            return;
        }

        if (g->location == LOCATION_LABYRINTH &&
            g->map.tiles[ty][tx] == TILE_LABYRINTH_EXIT) {
            if (g->level == 1) {
                game_leave_labyrinth(g);
            } else {
                int false_stair = tx == LABYRINTH_FALSE_EXIT_X &&
                    ty == LABYRINTH_FALSE_EXIT_Y;
                game_change_labyrinth_floor(g, 0, false_stair);
            }
            return;
        }

        if (g->location == LOCATION_LABYRINTH &&
            g->map.tiles[ty][tx] == TILE_LABYRINTH_STAIRS) {
            int false_stair = tx != g->map.stairs_down_x ||
                ty != g->map.stairs_down_y;
            game_change_labyrinth_floor(g, 1, false_stair);
            return;
        }

        if (g->location == LOCATION_TAVERN &&
            g->map.tiles[ty][tx] == TILE_TAVERN_EXIT) {
            game_leave_tavern(g);
            return;
        }

        if (g->location == LOCATION_INN &&
            g->map.tiles[ty][tx] == TILE_TAVERN_EXIT) {
            game_leave_inn(g);
            return;
        }
        if (g->location == LOCATION_GUILD && g->map.tiles[ty][tx] == TILE_TAVERN_EXIT) {
            game_leave_guild(g);
            return;
        }

        if (g->location == LOCATION_TOWN2 &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT && ty == 0) {
            if (tx == TOWN3_ROAD_X) {
                if (g->defeated_bosses & (1 << LOCATION_SWAMP)) {
                    game_enter_swamp_road(g);
                }
            } else {
                game_enter_swamp(g);
            }
            return;
        }

        if (g->location == LOCATION_TOWN2 &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT && tx == TOWN_W - 1) {
            if (ty == TOWN_ROAD_EXIT_Y) {
                if (g->defeated_bosses & (1 << LOCATION_FOREST)) {
                    game_enter_forest_road(g);
                }
            } else {
                game_enter_forest(g);
            }
            return;
        }

        if (g->location == LOCATION_TOWN2 &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT && tx == 0) {
            game_enter_desert(g);
            return;
        }


        if (g->location == LOCATION_TOWN2 &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT && ty == TOWN_H - 1) {
            game_enter_glassdeep(g);
            return;
        }

        if (g->location == LOCATION_TOWN3 &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT) {
            if (ty == 0) {
                game_enter_frostfell(g);
            } else if (ty == TOWN_H - 1) {
                if (tx == ROSEMOOR_SWAMP_ROAD_X) {
                    if (g->defeated_bosses & (1 << LOCATION_SWAMP)) {
                        game_enter_swamp_road(g);
                    }
                } else {
                    game_enter_swamp(g);
                }
            } else if (tx == TOWN_W - 1) {
                game_enter_king_road(g, LOCATION_CROWNROAD, 0);
            } else if (tx == 0) {
                game_enter_moonveil(g);
            }
            return;
        }

        if (g->location == LOCATION_CASTLE &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT) {
            if (ty == TOWN_H - 1) {
                game_enter_catacombs(g);
            } else if (tx == 0) {
                game_enter_king_road(g, LOCATION_CROWNROAD, 1);
            } else if (tx == TOWN_W - 1) {
                game_enter_king_road(g, LOCATION_KING_ROAD_WEST, 1);
            } else {
                castle_request(g, 0);
            }
            return;
        }

        if (g->location == LOCATION_TOWN4 &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT) {
            if (ty == 0) {
                game_enter_ashen(g);
            } else if (tx == TOWN_W - 1) {
                game_enter_dragonspine(g);
            } else if (tx == 0) {
                game_enter_king_road(g, LOCATION_KING_ROAD_WEST, 0);
            } else if (ty == TOWN_H - 1) {
                if (tx == RIDGESHIRE_MOUNTAIN_ROAD_X) {
                    if (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)) {
                        game_enter_high_pass(g, 0);
                    }
                } else {
                    game_enter_mountains(g);
                }
            }
            return;
        }

        if (g->location == LOCATION_TOWN &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT) {
            if (tx == 0) {
                if (ty == TOWN_ROAD_EXIT_Y) {
                    if (g->defeated_bosses & (1 << LOCATION_FOREST)) {
                        game_enter_forest_road(g);
                    } else {
                        push_message(g, "The road to Stillbury is still blocked.");
                    }
                } else {
                    game_enter_forest(g);
                }
            } else if (tx == TOWN_W - 1) {
                game_enter_dungeon(g);
            } else if (ty == TOWN_H - 1) {
                game_enter_coast(g);
            } else if (tx == TOWN4_ROAD_X) {
                if (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)) {
                    game_enter_high_pass(g, 1);
                }
            } else {
                game_enter_mountains(g);
            }
            return;
        }

        if (g->location == LOCATION_HIGH_PASS &&
            g->map.tiles[ty][tx] == TILE_HIGH_PASS_ENTRANCE) {
            game_leave_high_pass(g, LOCATION_TOWN);
            return;
        }
        if (g->location == LOCATION_HIGH_PASS &&
            g->map.tiles[ty][tx] == TILE_HIGH_PASS_EXIT) {
            game_leave_high_pass(g, LOCATION_TOWN4);
            return;
        }
        if (g->location == LOCATION_DRAGONSPINE &&
            g->map.tiles[ty][tx] == TILE_DRAGON_ENTRANCE) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
            return;
        }
        if (g->location == LOCATION_DRAGONSPINE &&
            g->map.tiles[ty][tx] == TILE_DRAGON_EXIT) {
            if (g->level < DRAGONSPINE_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else {
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active &&
                        g->enemies[i].type == ENEMY_RED_DRAGON) {
                        push_message(g, "The dragon guards the summit pass!");
                        return;
                    }
                }
                game_return_to_town(g);
                push_message(g, "Dragonspine is free of its dragon.");
            }
            return;
        }

        if (g->location == LOCATION_FOREST_ROAD &&
            g->map.tiles[ty][tx] == TILE_FOREST_ENTRANCE && tx == 0) {
            game_leave_forest_road(g, LOCATION_TOWN2);
            return;
        }

        if (g->location == LOCATION_FOREST_ROAD &&
            g->map.tiles[ty][tx] == TILE_FOREST_EXIT &&
            tx == FOREST_ROAD_W - 1) {
            game_leave_forest_road(g, LOCATION_TOWN);
            return;
        }

        if (game_is_king_road(g) &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT) {
            Location destination;
            if (g->location == LOCATION_CROWNROAD) {
                destination = tx == 0 ? LOCATION_TOWN3 : LOCATION_CASTLE;
            } else {
                destination = tx == 0 ? LOCATION_CASTLE : LOCATION_TOWN4;
            }
            game_leave_crownroad(g, destination);
            return;
        }

        if (g->location == LOCATION_SWAMP_ROAD &&
            (g->map.tiles[ty][tx] == TILE_SWAMP_ENTRANCE ||
            g->map.tiles[ty][tx] == TILE_SWAMP_EXIT)) {
            game_leave_swamp_road(g, ty == 0 ? LOCATION_TOWN3 : LOCATION_TOWN2);
            return;
        }

        if (g->location == LOCATION_SWAMP &&
            g->map.tiles[ty][tx] == TILE_SWAMP_SHORTCUT) {
            if (g->defeated_bosses & (1 << LOCATION_SWAMP)) {
                game_leave_swamp(g, g->swamp_entry_town == LOCATION_TOWN3 ? LOCATION_TOWN2 : LOCATION_TOWN3, 1);
            }
            return;
        }

        if (g->location == LOCATION_SWAMP &&
            g->map.tiles[ty][tx] == TILE_SWAMP_ENTRANCE) {
            if (g->level == SWAMP_BOSS_LEVEL && g->swamp_entry_town == LOCATION_TOWN3) {
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active && g->enemies[i].type == ENEMY_SWAMP_DEMON) {
                        push_message(g, "The demon blocks the swamp trail!");
                        return;
                    }
                }
            }
            if (g->level == 1) {
                game_leave_swamp(g, LOCATION_TOWN2, 0);
            } else {
                int unvisited = !g->swamp_cache[g->level - 2].valid;
                game_ascend(g);
                if (unvisited) {
                    g->score += map_swamp_difficulty(g->level) * 100;
                }
            }
            return;
        }

        if (g->location == LOCATION_SWAMP &&
            g->map.tiles[ty][tx] == TILE_SWAMP_EXIT) {
            if (g->level == SWAMP_BOSS_LEVEL && g->swamp_entry_town != LOCATION_TOWN3) {
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active &&
                        g->enemies[i].type == ENEMY_SWAMP_DEMON) {
                        push_message(g, "The demon blocks the swamp trail!");
                        return;
                    }
                }
            }
            if (g->level < SWAMP_DEPTH) {
                int unvisited = !g->swamp_cache[g->level].valid;
                game_descend(g);
                if (unvisited) {
                    g->score += map_swamp_difficulty(g->level) * 100;
                }
            } else {
                game_leave_swamp(g, LOCATION_TOWN3, 0);
            }
            return;
        }

        if (g->location == LOCATION_FROSTFELL &&
            g->map.tiles[ty][tx] == TILE_FROST_ENTRANCE) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
            return;
        }

        if (g->location == LOCATION_FROSTFELL &&
            g->map.tiles[ty][tx] == TILE_FROST_EXIT) {
            if (g->level < FROSTFELL_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else {
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active &&
                        g->enemies[i].type == ENEMY_POLAR_KRAKEN) {
                        push_message(g, "The Polar Kraken bars the way west!");
                        return;
                    }
                }
                game_return_to_town(g);
                push_message(g, "The Frostfell Wastes are free of the Kraken.");
            }
            return;
        }

        if (g->location == LOCATION_DESERT &&
            g->map.tiles[ty][tx] == TILE_DESERT_ENTRANCE) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
            return;
        }

        if (g->location == LOCATION_DESERT &&
            g->map.tiles[ty][tx] == TILE_DESERT_EXIT) {
            if (g->level < DESERT_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else {
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active && g->enemies[i].is_boss) {
                        push_message(g, "The Desert Pharaoh bars the way west!");
                        return;
                    }
                }
                game_return_to_town(g);
                push_message(g, "You return to Stillbury.");
            }
            return;
        }

        if (g->location == LOCATION_MOONVEIL &&
            g->map.tiles[ty][tx] == TILE_MOONVEIL_ENTRANCE) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
            return;
        }

        if (g->location == LOCATION_MOONVEIL &&
            g->map.tiles[ty][tx] == TILE_MOONVEIL_EXIT) {
            if (g->level < MOONVEIL_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else {
                if (!(g->defeated_bosses & (1 << LOCATION_MOONVEIL))) {
                    push_message(g, "The Thorn Regent seals the path to Rosemoor!");
                    return;
                }
                game_return_to_town(g);
                push_message(g, "You return to Rosemoor through the moonlit gardens.");
            }
            return;
        }

        if (g->location == LOCATION_ASHEN &&
            g->map.tiles[ty][tx] == TILE_ASHEN_ENTRANCE) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
            return;
        }

        if (g->location == LOCATION_ASHEN &&
            g->map.tiles[ty][tx] == TILE_ASHEN_EXIT) {
            if (g->level < ASHEN_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else if (g->defeated_bosses & (1 << LOCATION_ASHEN)) {
                game_return_to_town(g);
                push_message(g, "You return to Ridgeshire through the volcanic pass.");
            } else {
                push_message(g, "The Cinder Lord seals the path to Ridgeshire!");
            }
            return;
        }

        if (g->location == LOCATION_GLASSDEEP &&
            g->map.tiles[ty][tx] == TILE_GLASSDEEP_ENTRANCE) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
            return;
        }

        if (g->location == LOCATION_GLASSDEEP &&
            g->map.tiles[ty][tx] == TILE_GLASSDEEP_EXIT) {
            if (g->level < GLASSDEEP_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else if (g->defeated_bosses & (1 << LOCATION_GLASSDEEP)) {
                game_return_to_town(g);
                push_message(g, "You emerge from Glassdeep at Stillbury's quarry gate.");
            } else {
                push_message(g, "The Prism Sovereign seals the path to Stillbury!");
            }
            return;
        }

        if (g->location == LOCATION_COAST &&
            g->map.tiles[ty][tx] == TILE_COAST_ENTRANCE) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
            return;
        }

        if (g->location == LOCATION_COAST &&
            g->map.tiles[ty][tx] == TILE_COAST_EXIT) {
            if (g->player.x != g->map.stairs_down_x ||
                g->player.y != g->map.stairs_down_y) {
                return;
            }
            if (g->level < COAST_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else {
                int queen_alive = 0;
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active &&
                        g->enemies[i].type == ENEMY_DROWNED_QUEEN) {
                        queen_alive = 1;
                        break;
                    }
                }
                if (queen_alive) {
                    push_message(g, "The Drowned Queen commands the tide!");
                    return;
                }
                g->score += g->level * 100;
                game_return_to_town(g);
                push_message(g, "The Sunken Coast is reclaimed!");
            }
            return;
        }

        if (g->location == LOCATION_FOREST &&
            g->map.tiles[ty][tx] == TILE_FOREST_SHORTCUT) {
            if (g->defeated_bosses & (1 << LOCATION_FOREST)) {
                game_leave_forest(g, g->forest_entry_town == LOCATION_TOWN2 ? LOCATION_TOWN : LOCATION_TOWN2, 1);
            }
            return;
        }

        if (g->location == LOCATION_FOREST &&
            g->map.tiles[ty][tx] == TILE_FOREST_ENTRANCE) {
            if (g->level == FOREST_BOSS_LEVEL && g->forest_entry_town == LOCATION_TOWN2) {
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active && g->enemies[i].type == ENEMY_FOREST_NECROMANCER) {
                        push_message(g, "The Necromancer seals the path!");
                        return;
                    }
                }
            }
            if (g->level == 1) {
                game_leave_forest(g, LOCATION_TOWN, 0);
            } else {
                int unvisited = !g->forest_cache[g->level - 2].valid;
                game_ascend(g);
                if (unvisited) {
                    g->score += map_forest_difficulty(g->level) * 100;
                }
            }
            return;
        }

        if (g->location == LOCATION_MOUNTAINS &&
            g->map.tiles[ty][tx] == TILE_MOUNTAIN_SHORTCUT) {
            if (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)) {
                game_leave_mountains(g, g->mountain_entry_town == LOCATION_TOWN4 ? LOCATION_TOWN : LOCATION_TOWN4, 1);
            }
            return;
        }

        if (g->location == LOCATION_MOUNTAINS &&
            (g->map.tiles[ty][tx] == TILE_MOUNTAIN_ENTRANCE ||
            g->map.tiles[ty][tx] == TILE_MOUNTAIN_EXIT)) {
            int reverse = g->map.tiles[ty][tx] == TILE_MOUNTAIN_ENTRANCE;
            int crossing_peak = reverse == (g->mountain_entry_town == LOCATION_TOWN4);
            if (g->level == MOUNTAIN_BOSS_LEVEL && crossing_peak) {
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active && g->enemies[i].type == ENEMY_MOUNTAIN_GOBLIN_KING) {
                        push_message(g, "The Goblin King bars the pass!");
                        return;
                    }
                }
            }
            if ((reverse && g->level == 1) || (!reverse && g->level == MOUNTAIN_DEPTH)) {
                game_leave_mountains(g, reverse ? LOCATION_TOWN : LOCATION_TOWN4, 0);
            } else {
                int next = g->level + (reverse ? -1 : 1);
                int unvisited = !g->mountain_cache[next - 1].valid;
                if (reverse) {
                    game_ascend(g);
                } else {
                    game_descend(g);
                }
                if (unvisited) {
                    g->score += map_mountain_difficulty(g->level) * 100;
                }
            }
            return;
        }

        if (g->location == LOCATION_FOREST &&
            g->map.tiles[ty][tx] == TILE_FOREST_EXIT) {
            if (g->level == FOREST_BOSS_LEVEL && g->forest_entry_town != LOCATION_TOWN2) {
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active && g->enemies[i].type == ENEMY_FOREST_NECROMANCER) {
                        push_message(g, "The Necromancer seals the path!");
                        return;
                    }
                }
            }
            if (g->level < FOREST_DEPTH) {
                int unvisited = !g->forest_cache[g->level].valid;
                game_descend(g);
                if (unvisited) {
                    g->score += map_forest_difficulty(g->level) * 100;
                }
            } else {
                game_leave_forest(g, LOCATION_TOWN2, 0);
            }
            return;
        }

        if (g->map.tiles[ty][tx] == TILE_LOCKED_DOOR) {
            if (!g->dungeon_key_found) {
                push_message(g, "The Lich King's door is locked.");
                return;
            }
            g->dungeon_key_found = 0;
            g->map.tiles[ty][tx] = TILE_FLOOR;
            push_message(g, "The dungeon key unlocks the door!");
        }

        if (g->map.tiles[ty][tx] == TILE_CRYPT_DOOR) {
            if (g->dungeon_crypt_keys < 1) {
                push_message(g, "The crypt is locked. Find its key.");
                return;
            }
            g->dungeon_crypt_keys--;
            g->map.tiles[ty][tx] = TILE_FLOOR;
            push_message(g, "The crypt key unlocks the door!");
        }

        if (g->location == LOCATION_COAST &&
            tx == g->map.stairs_down_x && ty == g->map.stairs_down_y &&
            g->map.tiles[ty][tx] == TILE_COAST_DEEP_WATER) {
            push_message(g, "Lower the tide to reach the exit.");
            return;
        }

        // Move if walkable
        // Track last direction for ranged attacks
        if (map_is_walkable(&g->map, tx, ty)) {
            g->player.last_dx = tx - g->player.x;
            g->player.last_dy = ty - g->player.y;
            int old_x = g->player.x;
            int old_y = g->player.y;
            if (jail_prisoner_at(g, tx, ty)) {
                g->prisoner_x = old_x;
                g->prisoner_y = old_y;
            }
            game_move_player(g, tx - g->player.x, ty - g->player.y);
            if ((old_x != g->player.x || old_y != g->player.y) &&
                g->map.tiles[old_y][old_x] == TILE_MOUNTAIN_WEAK_BRIDGE) {
                change_mountain_tile(g, old_x, old_y, TILE_MOUNTAIN_CHASM);
                push_message(g, "Bridge collapsed! Press A to repair.");
            }
            if (g->map.tiles[g->player.y][g->player.x] == TILE_MOUNTAIN_WEAK_BRIDGE) {
                push_message(g, "The bridge creaks beneath your feet!");
            }
            frost_thin_ice_step(g, old_x, old_y);
            frost_slide(g, old_x, old_y);
        }
        // Check for trap on new tile
        int px = g->player.x;
        int py = g->player.y;
        TileType tile = g->map.tiles[py][px];

        if (g->location == LOCATION_FOREST &&
            tile == TILE_FOREST_FALSE_MARKER) {
            g->map.tiles[py][px] = TILE_FOREST_FLOOR;
            push_message(g, "The broken marker points to a dead trail.");
            tile = TILE_FOREST_FLOOR;
        }

        if (g->location == LOCATION_TEMPLE &&
            tile == TILE_TEMPLE_SOLAR_TRAP && !g->temple_alignment) {
            int dmg = 8 + rand() % 7;
            g->player.hp -= dmg;
            combat_feedback_add(g, FEEDBACK_PLAYER_DAMAGE, FEEDBACK_NOW, px, py, dmg);
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Solar flame erupts! -%d HP", dmg);
            push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            if (g->player.hp <= 0) {
                return;
            }
        }

        if (g->location == LOCATION_FOREST &&
            tile == TILE_FOREST_LANDMARK) {
            g->map.tiles[py][px] = TILE_FOREST_FLOOR;
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (g->map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL) {
                        g->map.tiles[y][x] = TILE_FOREST_FLOOR;
                    }
                }
            }
            map_reveal_forest_exit(&g->map);
            map_reveal_forest_entrance(&g->map);
            push_message(g, "The landmark reveals hidden trails!");
            tile = TILE_FOREST_FLOOR;
        }

        if (tile == TILE_TRAP_HIDDEN || tile == TILE_TRAP_REVEALED) {
            int roll = rand() % 3;
            TileType trap_type;
            if (roll == 0)      trap_type = TILE_TRAP_SPIKE;
            else if (roll == 1) trap_type = TILE_TRAP_FIRE;
            else                trap_type = TILE_TRAP_POISON;
            g->map.tiles[py][px] = trap_type;

            int dmg = 0;
            char msg[MAX_MESSAGE_LEN];

            if (trap_type == TILE_TRAP_SPIKE) {
                dmg = 5 + rand() % 10;
                g->player.hp -= dmg;
                combat_feedback_add(g, FEEDBACK_PLAYER_DAMAGE, FEEDBACK_NOW, px, py, dmg);
                snprintf(msg, sizeof(msg), "Spike trap! -%d HP", dmg);
                // Red flash
                g->trail_count  = 0;
                g->trail_frames = 4;
                g->trail_effect = TRAIL_EFFECT_GENERIC;
                TrailTile *t = &g->trail[g->trail_count++];
                t->active = 1; t->x = px; t->y = py;
                t->r = 200; t->g = 20; t->b = 20;
                t->is_impact = 1;
            } else if (trap_type == TILE_TRAP_FIRE) {
                dmg = 4 + rand() % 8;
                g->player.hp -= dmg;
                combat_feedback_add(g, FEEDBACK_PLAYER_DAMAGE, FEEDBACK_NOW, px, py, dmg);
                snprintf(msg, sizeof(msg), "Fire trap! -%d HP", dmg);
                // Orange flash
                g->trail_count  = 0;
                g->trail_frames = 4;
                g->trail_effect = TRAIL_EFFECT_GENERIC;
                TrailTile *t = &g->trail[g->trail_count++];
                t->active = 1; t->x = px; t->y = py;
                t->r = 220; t->g = 100; t->b = 20;
                t->is_impact = 1;
            } else if (trap_type == TILE_TRAP_POISON) {
                g->player.poison_turns = 3;
                snprintf(msg, sizeof(msg), "Poison trap! 3 turns");
                // Green flash
                g->trail_count  = 0;
                g->trail_frames = 4;
                g->trail_effect = TRAIL_EFFECT_GENERIC;
                TrailTile *t = &g->trail[g->trail_count++];
                t->active = 1; t->x = px; t->y = py;
                t->r = 40; t->g = 180; t->b = 40;
                t->is_impact = 1;
            }
            push_message_kind(g, msg, trap_type == TILE_TRAP_POISON ? MESSAGE_POISON : MESSAGE_DAMAGE_TAKEN);
            if (g->player.hp <= 0) {
                return;
            }
        }

        tick_player_poison(g);
    }
}

int action_use_inventory_item(GameState *g, int index, EnemyProjectiles *shots) {
    if (index < 0 || index >= g->inventory_count) {
        return 0;
    }
    ItemType type = g->inventory[index].type;
    int count = g->inventory_count;
    action_resolve_player(g, (Action){ACTION_USE_ITEM, index, 0});
    if (g->inventory_count == count - 1 &&
        (type == ITEM_POTION_HEALTH || type == ITEM_POTION_MANA ||
        type == ITEM_POTION_STRENGTH || type == ITEM_POTION_INTELLIGENCE)) {
        action_resolve_enemies_with_projectiles(g, shots);
        return 1;
    }
    return 0;
}

static int enemy_position_occupied(const GameState *g, int skip, int x, int y) {
    if (jail_prisoner_at(g, x, y)) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (i == skip || !g->enemies[i].active) {
            continue;
        }
        if (g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static int enemy_distances[MAP_H][MAP_W];
static int enemy_path_queue[MAP_H * MAP_W];

#define ENEMY_NOTICE_DISTANCE 10
#define ENEMY_PROVOKED_DISTANCE 16
#define ENEMY_PURSUER_LIMIT 3

static void build_enemy_distance_map(const GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            enemy_distances[y][x] = -1;
        }
    }

    if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
        return;
    }

    int head = 0;
    int tail = 0;
    enemy_distances[g->player.y][g->player.x] = 0;
    enemy_path_queue[tail++] = g->player.y * MAP_W + g->player.x;
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};

    while (head < tail) {
        int cell = enemy_path_queue[head++];
        int x = cell % MAP_W;
        int y = cell / MAP_W;
        for (int direction = 0; direction < 4; direction++) {
            int nx = x + dx[direction];
            int ny = y + dy[direction];
            if (!map_is_walkable(&g->map, nx, ny) ||
                enemy_distances[ny][nx] >= 0) {
                continue;
            }
            enemy_distances[ny][nx] = enemy_distances[y][x] + 1;
            enemy_path_queue[tail++] = ny * MAP_W + nx;
        }
    }
}

static int enemy_is_major_boss(const Enemy *e) {
    return e->is_boss ||
        e->type == ENEMY_LICH_KING ||
        e->type == ENEMY_FOREST_NECROMANCER ||
        e->type == ENEMY_MOUNTAIN_GOBLIN_KING ||
        e->type == ENEMY_DROWNED_QUEEN ||
        e->type == ENEMY_FALLEN_SUN_GUARDIAN;
}

static int enemy_prefers_range(const Enemy *e);

static int enemy_is_support(const Enemy *e) {
    return e->type == ENEMY_BONE_CANTOR || e->type == ENEMY_CRYPT_CONJURER ||
        e->type == ENEMY_GOBLIN_SHAMAN ||
        e->type == ENEMY_SUN_PRIEST;
}

static int enemy_is_protector(const Enemy *e) {
    return e->type == ENEMY_BONE_SENTINEL || e->type == ENEMY_HOBGOBLIN_GUARD ||
        e->type == ENEMY_HORSEMAN ||
        e->type == ENEMY_ANIMATED_STATUE ||
        e->type == ENEMY_VINEBOUND_GUARDIAN ||
        e->type == ENEMY_LUNAR_EFFIGY;
}

static int enemy_has_backline_ally(const GameState *g, int index, int range) {
    const Enemy *e = &g->enemies[index];
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *ally = &g->enemies[i];
        if (i == index || !ally->active ||
            (!enemy_prefers_range(ally) && !enemy_is_support(ally))) {
            continue;
        }
        int distance = abs_int(ally->x - e->x) + abs_int(ally->y - e->y);
        if (distance <= range) {
            return 1;
        }
    }
    return 0;
}

static int enemy_has_nearby_ally(const GameState *g, int index, int range) {
    const Enemy *e = &g->enemies[index];
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *ally = &g->enemies[i];
        if (i == index || !ally->active) {
            continue;
        }
        int distance = abs_int(ally->x - e->x) + abs_int(ally->y - e->y);
        if (distance <= range) {
            return 1;
        }
    }
    return 0;
}

static void select_enemy_pursuers(const GameState *g, int pursuers[MAX_ENEMIES]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        pursuers[i] = 0;
    }

    for (int slot = 0; slot < ENEMY_PURSUER_LIMIT; slot++) {
        int best = -1;
        int best_distance = MAP_W * MAP_H;
        int best_provoked = 0;
        int best_role_priority = 0;
        for (int i = 0; i < g->enemy_count; i++) {
            const Enemy *e = &g->enemies[i];
            if (!e->active || pursuers[i] || enemy_is_major_boss(e) ||
                (game_is_king_road(g) &&
                e->type == ENEMY_ROAD_ARCHER)) {
                continue;
            }
            int distance = enemy_distances[e->y][e->x];
            if (distance <= 1) {
                continue;
            }
            int provoked = e->hp < e->max_hp;
            int role_priority = 0;
            if (slot == 0 && enemy_is_protector(e) &&
                enemy_has_backline_ally(g, i, 6)) {
                role_priority = 2;
            } else if (slot == 1 &&
                (enemy_prefers_range(e) || enemy_is_support(e))) {
                role_priority = 1;
            }
            int limit = provoked ? ENEMY_PROVOKED_DISTANCE :
                ENEMY_NOTICE_DISTANCE;
            if (distance < 0 || distance > limit) {
                continue;
            }
            if (best < 0 || provoked > best_provoked ||
                (provoked == best_provoked &&
                role_priority > best_role_priority) ||
                (provoked == best_provoked &&
                role_priority == best_role_priority &&
                distance < best_distance)) {
                best = i;
                best_distance = distance;
                best_provoked = provoked;
                best_role_priority = role_priority;
            }
        }
        if (best < 0) {
            break;
        }
        pursuers[best] = 1;
    }
}

static int enemy_prefers_range(const Enemy *e) {
    return e->type == ENEMY_GRAVE_ARCHER || e->type == ENEMY_BONE_CANTOR || e->type == ENEMY_CRYPT_CONJURER ||
        e->type == ENEMY_DARK_ELF ||
        e->type == ENEMY_GOBLIN_ARCHER ||
        e->type == ENEMY_ROAD_ARCHER ||
        e->type == ENEMY_FROST_ARCHER ||
        e->type == ENEMY_GOBLIN_BOMBER ||
        e->type == ENEMY_SIREN ||
        e->type == ENEMY_WATER_ELEMENTAL ||
        e->type == ENEMY_BLOWDART_HUNTER ||
        e->type == ENEMY_FIRE_ELEMENTAL ||
        e->type == ENEMY_DJINN ||
        e->type == ENEMY_FEY_TRICKSTER ||
        e->type == ENEMY_LIVING_FLOWER ||
        e->type == ENEMY_CINDER_IMP ||
        e->type == ENEMY_SUN_PRIEST ||
        e->type == ENEMY_SERPENT_SPIRIT ||
        e->type == ENEMY_MOONBOUND_SENTINEL;
}

static int enemy_prefers_flank(const Enemy *e) {
    return e->type == ENEMY_CRYPT_BAT ||
        e->type == ENEMY_CRYSTAL_SPIDER ||
        e->type == ENEMY_BLIND_STALKER ||
        e->type == ENEMY_ASH_HOUND ||
        e->type == ENEMY_GIANT_MOTH ||
        e->type == ENEMY_PIXIE ||
        e->type == ENEMY_BLIGHTED_WOLF ||
        e->type == ENEMY_GIANT_SPIDER ||
        e->type == ENEMY_GOBLIN_SCOUT ||
        e->type == ENEMY_TUNNEL_SPIDER ||
        e->type == ENEMY_RELIC_SCARABS ||
        e->type == ENEMY_SCARAB ||
        e->type == ENEMY_TEMPLE_STALKER;
}

static int enemy_blocks_line_of_sight(const GameState *g, int shooter_index, int x, int y);
static int clear_orthogonal_path(const GameState *g, int shooter_index, const Enemy *e);

static int enemy_move_toward(GameState *g, int index) {
    Enemy *e = &g->enemies[index];
    int current_distance = enemy_distances[e->y][e->x];
    if (current_distance <= 0) {
        return 0;
    }

    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int best_x = e->x;
    int best_y = e->y;
    int best_distance = current_distance;
    int best_has_firing_lane = 0;
    for (int direction = 0; direction < 4; direction++) {
        int tx = e->x + dx[direction];
        int ty = e->y + dy[direction];
        if (!map_is_walkable(&g->map, tx, ty) ||
            enemy_position_occupied(g, index, tx, ty) ||
            (tx == g->player.x && ty == g->player.y)) {
            continue;
        }
        int distance = enemy_distances[ty][tx];
        Enemy candidate = *e;
        candidate.x = tx;
        candidate.y = ty;
        int has_firing_lane = enemy_prefers_range(e) &&
            clear_orthogonal_path(g, index, &candidate);
        if (distance >= 0 &&
            (distance < best_distance ||
            (distance == best_distance &&
            has_firing_lane > best_has_firing_lane))) {
            best_x = tx;
            best_y = ty;
            best_distance = distance;
            best_has_firing_lane = has_firing_lane;
        }
    }
    if (best_x == e->x && best_y == e->y) {
        return 0;
    }
    e->x = best_x;
    e->y = best_y;
    return 1;
}

static int enemy_move_away(GameState *g, int index) {
    Enemy *e = &g->enemies[index];
    int current_distance = enemy_distances[e->y][e->x];
    int current_separation = abs_int(g->player.x - e->x);
    int vertical_separation = abs_int(g->player.y - e->y);
    if (vertical_separation > current_separation) {
        current_separation = vertical_separation;
    }

    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int best_x = e->x;
    int best_y = e->y;
    int best_distance = current_distance;
    int best_separation = current_separation;
    for (int direction = 0; direction < 4; direction++) {
        int tx = e->x + dx[direction];
        int ty = e->y + dy[direction];
        if (!map_is_walkable(&g->map, tx, ty) ||
            enemy_position_occupied(g, index, tx, ty) ||
            (tx == g->player.x && ty == g->player.y)) {
            continue;
        }
        int distance = enemy_distances[ty][tx];
        if (distance < 0) {
            continue;
        }
        int separation = abs_int(g->player.x - tx);
        int vertical = abs_int(g->player.y - ty);
        if (vertical > separation) {
            separation = vertical;
        }
        if (distance > best_distance ||
            (distance == best_distance && separation > best_separation)) {
            best_x = tx;
            best_y = ty;
            best_distance = distance;
            best_separation = separation;
        }
    }
    if (best_x == e->x && best_y == e->y) {
        return 0;
    }
    e->x = best_x;
    e->y = best_y;
    return 1;
}

static int enemy_move_to_flank(GameState *g, int index) {
    Enemy *e = &g->enemies[index];
    int current_distance = enemy_distances[e->y][e->x];
    if (current_distance <= 2) {
        return 0;
    }

    int step_x = 0;
    int step_y = 0;
    if (e->x == g->player.x) {
        step_x = index % 2 == 0 ? -1 : 1;
    } else if (e->y == g->player.y) {
        step_y = index % 2 == 0 ? -1 : 1;
    } else {
        return 0;
    }

    for (int side = 0; side < 2; side++) {
        int direction = side == 0 ? 1 : -1;
        int tx = e->x + step_x * direction;
        int ty = e->y + step_y * direction;
        if (!map_is_walkable(&g->map, tx, ty) ||
            enemy_position_occupied(g, index, tx, ty) ||
            (tx == g->player.x && ty == g->player.y)) {
            continue;
        }
        int distance = enemy_distances[ty][tx];
        if (distance < 0 || distance > current_distance + 1) {
            continue;
        }
        e->x = tx;
        e->y = ty;
        return 1;
    }
    return 0;
}

static int clear_orthogonal_path(const GameState *g, int shooter_index, const Enemy *e) {
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    if (dx != 0 && dy != 0) {
        return 0;
    }
    int distance = abs_int(dx) + abs_int(dy);
    if (distance < 2 || distance > 6) {
        return 0;
    }
    int sx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int sy = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    for (int step = 1; step < distance; step++) {
        int x = e->x + sx * step;
        int y = e->y + sy * step;
        if (!map_is_walkable(&g->map, x, y) ||
            enemy_blocks_line_of_sight(g, shooter_index, x, y)) {
            return 0;
        }
    }
    return 1;
}

static int necromancer_revive(GameState *g, int necromancer_index) {
    Enemy *caster = &g->enemies[necromancer_index];
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *dead = &g->enemies[i];
        if (dead->active || dead->type != ENEMY_SKELETON) continue;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int x = caster->x + dx;
                int y = caster->y + dy;
                if (!map_is_walkable(&g->map, x, y) ||
                    enemy_position_occupied(g, i, x, y) ||
                    (x == g->player.x && y == g->player.y)) continue;
                dead->x = x;
                dead->y = y;
                dead->active = 1;
                dead->hp = dead->max_hp;
                push_message(g, "Crypt Conjurer raises a Skeleton!");
                return 1;
            }
        }
    }
    return 0;
}

static int forest_necromancer_raise(GameState *g, int caster_index) {
    Enemy *caster = &g->enemies[caster_index];
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *dead = &g->enemies[i];
        if (dead->active || dead->is_boss ||
            dead->type < ENEMY_PIXIE ||
            dead->type > ENEMY_FOREST_TROLL) continue;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int x = caster->x + dx;
                int y = caster->y + dy;
                if (!map_is_walkable(&g->map, x, y) ||
                    enemy_position_occupied(g, i, x, y) ||
                    (x == g->player.x && y == g->player.y)) continue;
                dead->x = x;
                dead->y = y;
                dead->active = 1;
                dead->hp = dead->max_hp / 2;
                if (dead->hp < 1) dead->hp = 1;
                push_message(g, "Necromancer recalls a fallen servant!");
                return 1;
            }
        }
    }
    return 0;
}

static int enemy_blocks_line_of_sight(const GameState *g, int shooter_index, int x, int y) {
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *other = &g->enemies[i];
        if (i == shooter_index || !other->active) {
            continue;
        }
        if (other->x == x && other->y == y) {
            return 1;
        }
    }
    return 0;
}

static int clear_projectile_path(const GameState *g, int shooter_index, const Enemy *e) {
    int x = e->x;
    int y = e->y;
    int dx = abs_int(g->player.x - x);
    int dy = abs_int(g->player.y - y);
    int step_x = x < g->player.x ? 1 : -1;
    int step_y = y < g->player.y ? 1 : -1;
    int error = dx - dy;

    while (x != g->player.x || y != g->player.y) {
        int old_x = x;
        int old_y = y;
        int twice_error = error * 2;
        if (twice_error > -dy) {
            error -= dy;
            x += step_x;
        }
        if (twice_error < dx) {
            error += dx;
            y += step_y;
        }
        if (x == g->player.x && y == g->player.y) {
            break;
        }
        if (!map_is_walkable(&g->map, x, y) ||
            enemy_blocks_line_of_sight(g, shooter_index, x, y)) {
            return 0;
        }
        if (x != old_x && y != old_y &&
            (!map_is_walkable(&g->map, x, old_y) ||
            !map_is_walkable(&g->map, old_x, y))) {
            return 0;
        }
    }
    return 1;
}

static int apply_enemy_ranged_damage(GameState *g, int shooter_index, const Enemy *e, int damage, EnemyProjectiles *shots) {
    if (!clear_projectile_path(g, shooter_index, e)) {
        return 0;
    }
    if (shots && shots->count < MAX_ENEMIES) {
        shots->shots[shots->count++] = (EnemyProjectile){
            e->type, e->x, e->y, g->player.x, g->player.y
        };
    }
    return apply_enemy_damage(g, damage, FEEDBACK_AFTER_ENEMY_SHOT);
}

void action_resolve_enemies(GameState *g) {
    action_resolve_enemies_with_projectiles(g, NULL);
}

static int near_lake_hole(const GameState *g, int x, int y) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int tx = x + dx;
            int ty = y + dy;
            if (tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H &&
                g->map.tiles[ty][tx] == TILE_FROST_LAKE_HOLE) {
                return 1;
            }
        }
    }
    return 0;
}

// Shared by damage resolution and warning tiles. The saved target fixes the
// sweep's row/column even if the player moves during its wind-up.
int kraken_tile_threatened(const GameState *g, const Enemy *e, int x, int y) {
    if (e->move_timer % 3 == 0 || e->attack_target_x < 0 || e->attack_target_y < 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    if (x == e->attack_target_x && y == e->attack_target_y) {
        return 1;
    }
    if (e->move_timer % 6 >= 3) {
        const Room *lake = &g->map.rooms[g->map.room_count - 1];
        if (x < lake->x || x >= lake->x + lake->w || y < lake->y || y >= lake->y + lake->h) {
            return 0;
        }
        int dx = abs_int(e->attack_target_x - e->x);
        int dy = abs_int(e->attack_target_y - e->y);
        int lane = dx >= dy ? e->attack_target_y : e->attack_target_x;
        int center = dx >= dy ? e->y : e->x;
        int tile = dx >= dy ? y : x;
        int second_lane = lane + (lane > center ? -2 : 2);
        return tile == lane || (e->attack_phase == 1 && tile == second_lane);
    }
    return near_lake_hole(g, x, y);
}

// Alternate a targeted strike (phases 1-2) with a lake sweep (phases 4-5).
// Phases 3 and 0 provide recovery; retreat resets the cycle to the first strike.
static void polar_kraken_turn(GameState *g, Enemy *e) {
    const Room *lake = &g->map.rooms[g->map.room_count - 1];
    int on_lake = g->player.x >= lake->x && g->player.x < lake->x + lake->w &&
        g->player.y >= lake->y && g->player.y < lake->y + lake->h;
    int distance_x = abs_int(g->player.x - e->x);
    int distance_y = abs_int(g->player.y - e->y);
    if (!on_lake && (e->hp == e->max_hp || distance_x > 12 || distance_y > 12)) {
        e->move_timer = 0;
        e->attack_target_x = -1;
        e->attack_target_y = -1;
        return;
    }
    e->move_timer = (e->move_timer + 1) % 6;
    char msg[MAX_MESSAGE_LEN];
    if (e->move_timer % 3 == 0) {
        push_message(g, "The Kraken readies its next strike.");
        return;
    }
    if (e->move_timer % 3 == 1) {
        int enraged = e->hp <= e->max_hp / 2;
        if (enraged && e->attack_phase == 0) {
            push_message(g, "The Kraken thrashes! Its sweeps split into two lanes!");
        }
        e->attack_phase = enraged;
        e->attack_target_x = g->player.x;
        e->attack_target_y = g->player.y;
        int adjacent = abs_int(g->player.x - e->x) <= 1 &&
            abs_int(g->player.y - e->y) <= 1;
        if (adjacent) {
            int dmg = e->attack - g->player.defense;
            if (dmg < 1) {
                dmg = 1;
            }
            dmg = apply_enemy_damage(g, dmg, FEEDBACK_NOW);
            if (dmg > 0) {
                snprintf(msg, sizeof(msg), "%s bites: %d dmg", e->name, dmg);
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            }
        }
        const char *warning = "Tentacles rise! Move off the marked tile and away from holes!";
        if (e->move_timer == 4) {
            warning = e->attack_phase == 1 ? "Double sweep next turn! Step into a clear lane!" :
                "Tentacles sweep next turn! Step out of the marked row or column!";
        }
        push_message(g, warning);
        return;
    }
    int threatened = kraken_tile_threatened(g, e, g->player.x, g->player.y);
    e->attack_target_x = -1;
    e->attack_target_y = -1;
    if (!threatened) {
        push_message(g, e->move_timer == 5 ? "The tentacles sweep empty ice." : "The tentacles lash empty ice.");
        return;
    }
    int dmg = e->attack - g->player.defense / 2;
    if (dmg < 4) {
        dmg = 4;
    }
    dmg = apply_enemy_damage(g, dmg, FEEDBACK_NOW);
    if (dmg > 0) {
        snprintf(msg, sizeof(msg), e->move_timer == 5 ? "Kraken sweep: %d dmg" : "Kraken tentacle: %d dmg", dmg);
        push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
    }
}

static void prism_sovereign_turn(GameState *g, int index, EnemyProjectiles *shots) {
    Enemy *enemy = &g->enemies[index];
    enemy->move_timer++;
    if (enemy->move_timer % 2 != 0) {
        int dx = g->player.x - enemy->x;
        int dy = g->player.y - enemy->y;
        if (abs_int(dx) >= abs_int(dy)) {
            enemy->attack_target_x = enemy->x + (dx < 0 ? -8 : 8);
            enemy->attack_target_y = enemy->y;
        } else {
            enemy->attack_target_x = enemy->x;
            enemy->attack_target_y = enemy->y + (dy < 0 ? -8 : 8);
        }
        push_message(g, "The Sovereign gathers light. Step out of the marked beam!");
        return;
    }
    int dx = (enemy->attack_target_x > enemy->x) - (enemy->attack_target_x < enemy->x);
    int dy = (enemy->attack_target_y > enemy->y) - (enemy->attack_target_y < enemy->y);
    int x = enemy->x;
    int y = enemy->y;
    int hit = 0;
    for (int step = 1; step <= 8; step++) {
        int tx = enemy->x + dx * step;
        int ty = enemy->y + dy * step;
        if (!map_is_walkable(&g->map, tx, ty) || enemy_blocks_line_of_sight(g, index, tx, ty)) {
            break;
        }
        x = tx;
        y = ty;
        hit |= g->player.x == x && g->player.y == y;
    }
    enemy->attack_target_x = -1;
    enemy->attack_target_y = -1;
    if (shots && shots->count < MAX_ENEMIES && (x != enemy->x || y != enemy->y)) {
        shots->shots[shots->count++] = (EnemyProjectile){enemy->type, enemy->x, enemy->y, x, y};
    }
    if (hit) {
        int damage = enemy->attack - g->player.defense / 2;
        if (damage < 3) {
            damage = 3;
        }
        damage = apply_enemy_damage(g, damage, FEEDBACK_AFTER_ENEMY_SHOT);
        if (damage > 0) {
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message), "Prismatic beam: %d dmg", damage);
            push_message_kind(g, message, MESSAGE_DAMAGE_TAKEN);
        }
    } else {
        push_message(g, "The prismatic beam strikes empty stone.");
    }
}

static void grave_marshal_turn(GameState *g, int index) {
    Enemy *e = &g->enemies[index];
    int room = catacombs_room_at(&g->map, g->player.x, g->player.y);
    if (room != g->map.room_count - 1 && e->hp == e->max_hp && e->move_timer == 0) {
        return;
    }
    if (e->attack_target_x >= 0) {
        if (catacombs_sweep_marks(&g->map, e, g->player.x, g->player.y)) {
            int damage = e->attack - g->player.defense;
            if (damage < 3) {
                damage = 3;
            }
            apply_enemy_damage(g, damage, FEEDBACK_NOW);
            push_message_kind(g, "The Marshal's polearm sweeps the marked stones!", MESSAGE_DAMAGE_TAKEN);
        } else {
            push_message(g, "The Marshal's polearm sweeps empty stone.");
        }
        e->attack_target_x = -1;
        e->attack_target_y = -1;
        e->move_timer = 1;
        return;
    }
    if (e->move_timer == 1) {
        e->move_timer = 2;
        return;
    }
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    if (abs_int(dx) <= 1 && abs_int(dy) <= 1) {
        int sx = abs_int(dx) >= abs_int(dy) ? (dx < 0 ? -1 : 1) : 0;
        int sy = sx == 0 ? (dy < 0 ? -1 : 1) : 0;
        e->attack_target_x = e->x + sx;
        e->attack_target_y = e->y + sy;
        e->move_timer = 2;
        push_message(g, "The Grave Marshal raises its polearm. Leave the marked sweep!");
    } else {
        int old_x = e->x;
        int old_y = e->y;
        enemy_move_toward(g, index);
        if (catacombs_room_at(&g->map, e->x, e->y) != g->map.room_count - 1) {
            e->x = old_x;
            e->y = old_y;
        }
        e->move_timer = 2;
    }
}

void action_resolve_enemies_with_projectiles(GameState *g, EnemyProjectiles *shots) {
    if (shots) {
        shots->count = 0;
    }
    if (g->player.hp <= 0 || g->game_won || g->castle_prompt) {
        return;
    }
    jail_follow(g);
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        int damage = castle_tick(g);
        if (damage > 0) {
            apply_enemy_damage(g, damage, FEEDBACK_NOW);
        }
        game_update_level_progress(g);
        return;
    }
    int burial_damage = catacombs_tick(g);
    if (burial_damage > 0) {
        int damage = burial_damage - g->player.defense / 2;
        if (damage < 3) {
            damage = 3;
        }
        apply_enemy_damage(g, damage, FEEDBACK_NOW);
    }
    // Protect the whole enemy phase after a lost turn, including every wraith.
    int freeze_immune = g->player.freeze_recovery;
    g->player.freeze_recovery = 0;
    build_enemy_distance_map(g);
    int pursuers[MAX_ENEMIES];
    select_enemy_pursuers(g, pursuers);
    int boss_locked = 0;
    if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH) {
        for (int y = 0; y < MAP_H && !boss_locked; y++)
            for (int x = 0; x < MAP_W; x++)
                if (g->map.tiles[y][x] == TILE_LOCKED_DOOR) {
                    boss_locked = 1;
                    break;
                }
    }

    for (int i = 0; i < g->enemy_count; i++) {
        if (g->player.hp <= 0) {
            return;
        }
        Enemy *e = &g->enemies[i];
        if (!e->active) continue;
        if (e->is_boss && boss_locked) continue;
        if (e->frozen_turns > 0) {
            e->frozen_turns--;
            continue;
        }

        int dx = g->player.x - e->x;
        int dy = g->player.y - e->y;
        int adjacent = abs_int(dx) <= 1 && abs_int(dy) <= 1 &&
            !(dx == 0 && dy == 0);
        if (!enemy_is_major_boss(e) && !adjacent && !pursuers[i] &&
            !(game_is_king_road(g) &&
            e->type == ENEMY_ROAD_ARCHER &&
            clear_orthogonal_path(g, i, e))) {
            continue;
        }

        if (e->type == ENEMY_GRAVE_MARSHAL) {
            grave_marshal_turn(g, i);
            continue;
        }
        if (e->type == ENEMY_LICH_KING) {
            Room *chamber = &g->map.rooms[g->map.room_count - 1];
            int player_in_chamber =
                g->player.x > chamber->x &&
                g->player.x < chamber->x + chamber->w - 1 &&
                g->player.y > chamber->y &&
                g->player.y < chamber->y + chamber->h - 1;
            // Unlocking the door does not pull the boss into the corridor.
            // The encounter begins only when the player crosses the threshold.
            if (!player_in_chamber && e->move_timer == 0) continue;
        }
        if (e->type == ENEMY_FOREST_NECROMANCER) {
            Room *grove = &g->map.rooms[g->map.room_count - 1];
            int player_in_grove =
                g->player.x >= grove->x &&
                g->player.x < grove->x + grove->w &&
                g->player.y >= grove->y &&
                g->player.y < grove->y + grove->h;
            int distance = abs_int(g->player.x - e->x) +
                abs_int(g->player.y - e->y);
            // Ranged hits can start the fight before the player enters the grove.
            if (!player_in_grove &&
                ((e->move_timer == 0 && e->hp == e->max_hp) ||
                distance > 12)) {
                continue;
            }
        }
        if (e->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            Room *fortress = &g->map.rooms[g->map.room_count - 1];
            int player_in_fortress =
                g->player.x >= fortress->x &&
                g->player.x < fortress->x + fortress->w &&
                g->player.y >= fortress->y &&
                g->player.y < fortress->y + fortress->h;
            int range_x = abs_int(g->player.x - e->x);
            int range_y = abs_int(g->player.y - e->y);
            // Hits outside the fortress start the fight, including diagonal bow shots.
            if (!player_in_fortress &&
                ((e->move_timer == 0 && e->hp == e->max_hp) ||
                range_x > 12 || range_y > 12)) {
                continue;
            }
        }
        if (e->type == ENEMY_DROWNED_QUEEN) {
            Room *throne = &g->map.rooms[g->map.room_count - 1];
            int player_in_throne =
                g->player.x >= throne->x &&
                g->player.x < throne->x + throne->w &&
                g->player.y >= throne->y &&
                g->player.y < throne->y + throne->h;
            int distance = abs_int(g->player.x - e->x) +
                abs_int(g->player.y - e->y);
            if (!player_in_throne &&
                ((e->move_timer == 0 && e->hp == e->max_hp) ||
                distance > 12)) {
                continue;
            }
        }
        if (e->type == ENEMY_FALLEN_SUN_GUARDIAN) {
            int distance = abs_int(g->player.x - e->x) +
                abs_int(g->player.y - e->y);
            if (e->hp == e->max_hp && g->player.y > 12) {
                continue;
            }
            if (distance > 16) {
                continue;
            }
        }
        if (e->type == ENEMY_POLAR_KRAKEN) {
            polar_kraken_turn(g, e);
            continue;
        }
        if (e->type == ENEMY_DESERT_PHARAOH || e->type == ENEMY_THORN_REGENT ||
            e->type == ENEMY_CINDER_LORD || e->type == ENEMY_PRISM_SOVEREIGN) {
            Room *lair = &g->map.rooms[g->map.room_count - 1];
            int in_lair = g->player.x >= lair->x &&
                g->player.x < lair->x + lair->w &&
                g->player.y >= lair->y && g->player.y < lair->y + lair->h;
            int distance = abs_int(dx) + abs_int(dy);
            if (!in_lair && ((e->move_timer == 0 && e->hp == e->max_hp) || distance > 12)) {
                continue;
            }
        }

        if (e->type == ENEMY_PRISM_SOVEREIGN) {
            prism_sovereign_turn(g, i, shots);
            continue;
        }
        if (e->type == ENEMY_BLIND_STALKER && e->hp == e->max_hp &&
            e->move_timer == 0 && abs_int(dx) + abs_int(dy) > 4) {
            continue;
        }

        int path_distance = enemy_distances[e->y][e->x];
        // Leave an opening after retreating so melee attackers can catch up.
        if ((enemy_prefers_range(e) || enemy_is_support(e)) &&
            e->move_timer % 2 == 0 && path_distance > 0 &&
            path_distance < 3 && enemy_move_away(g, i)) {
            e->move_timer++;
            continue;
        }

        // Adjacent to player — melee attack
        if (adjacent) {
                int defense = g->player.defense;
                if (e->type == ENEMY_WRAITH) defense /= 2;
                int dmg = e->attack - defense;
                if (dmg < 1) dmg = 1;
                dmg = apply_enemy_damage(g, dmg, FEEDBACK_NOW);
                if (dmg == 0) {
                    continue;
                }
                if (e->type == ENEMY_GIANT_SPIDER ||
                    e->type == ENEMY_TUNNEL_SPIDER ||
                    e->type == ENEMY_VIPER) {
                    g->player.poison_turns = 3;
                    push_message_kind(g, e->type == ENEMY_VIPER
                        ? "Viper venom poisons you!" : "Giant Spider venom poisons you!",
                        MESSAGE_POISON);
                }
                if (e->type == ENEMY_WRAITH && g->player.mp > 0) {
                    int drained = g->player.mp < 3 ? g->player.mp : 3;
                    g->player.mp -= drained;
                    combat_feedback_add(g, FEEDBACK_MANA_LOSS, FEEDBACK_NOW, g->player.x, g->player.y, drained);
                }
                // One hit in four freezes the player for their next turn.
                int froze = e->type == ENEMY_FROST_WRAITH &&
                    !freeze_immune && g->player.frozen_turns == 0 && rand() % 100 < 25;
                if (froze) {
                    g->player.frozen_turns = 1;
                }
                char msg[MAX_MESSAGE_LEN];
                if (e->type == ENEMY_WRAITH) {
                    snprintf(msg, sizeof(msg), "Wraith: %d dmg, drains MP", dmg);
                } else if (froze) {
                    snprintf(msg, sizeof(msg), "%s: %d dmg, freezes you!", e->name, dmg);
                } else {
                    snprintf(msg, sizeof(msg), "%s: %d dmg", e->name, dmg);
                }
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
                continue;
        }

        e->move_timer++;

        if (e->type == ENEMY_RED_DRAGON && path_distance > 1 &&
            path_distance <= 8 && clear_orthogonal_path(g, i, e)) {
            if (e->move_timer % 2 != 0) {
                push_message(g, "The dragon draws a fiery breath!");
            } else {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 3) {
                    dmg = 3;
                }
                dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
                if (dmg > 0) {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), "Dragonfire: %d dmg", dmg);
                    push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
                }
            }
            continue;
        }

        if ((e->type == ENEMY_FIRE_ELEMENTAL || e->type == ENEMY_DJINN ||
            e->type == ENEMY_FEY_TRICKSTER || e->type == ENEMY_LIVING_FLOWER ||
            e->type == ENEMY_CINDER_IMP) &&
            e->move_timer % 2 == 0 &&
            clear_orthogonal_path(g, i, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 1) {
                dmg = 1;
            }
            dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
            if (dmg > 0) {
                char msg[MAX_MESSAGE_LEN];
                const char *attack = e->type == ENEMY_CINDER_IMP ? "Imp firebolt" :
                    (e->type == ENEMY_FEY_TRICKSTER ? "Fey sparkle" :
                    (e->type == ENEMY_LIVING_FLOWER ? "Carnivorous Flower" :
                    (e->type == ENEMY_DJINN ? "Djinn magic bolt" : "Elemental flame")));
                snprintf(msg, sizeof(msg), "%s: %d dmg", attack, dmg);
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            }
            continue;
        }

        if (e->type == ENEMY_LICH_KING) {
            // The Lich holds the center of his chamber and alternates a ranged
            // necrotic attack with a telegraphed recovery turn. This prevents
            // ordinary pathfinding from walking him out of his own arena.
            if (e->move_timer % 2 == 0) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 4) dmg = 4;
                dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
                if (dmg == 0) {
                    continue;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Lich necrotic bolt: %d dmg", dmg);
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            } else {
                push_message(g, "The Lich gathers dark power...");
            }
            continue;
        }

        if (e->type == ENEMY_FOREST_NECROMANCER) {
            if (e->move_timer % 4 == 0 &&
                forest_necromancer_raise(g, i)) continue;
            if (e->move_timer % 2 == 0) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 3) dmg = 3;
                dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
                if (dmg == 0) {
                    continue;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Necromancer spirit bolt: %d dmg", dmg);
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            } else {
                push_message(g, "The Necromancer invokes the forest...");
            }
            continue;
        }

        if (e->type == ENEMY_THORN_REGENT || e->type == ENEMY_CINDER_LORD) {
            if (e->move_timer % 2 != 0) {
                push_message(g, e->type == ENEMY_CINDER_LORD ?
                    "The Cinder Lord gathers molten fire..." :
                    "The Thorn Regent gathers moonlit thorns...");
            } else {
                int damage = e->attack - g->player.defense / 2;
                if (damage < 3) {
                    damage = 3;
                }
                damage = apply_enemy_ranged_damage(g, i, e, damage, shots);
                if (damage > 0) {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), e->type == ENEMY_CINDER_LORD ?
                        "Cinder Lord firebolt: %d dmg" : "Regent thorn bolt: %d dmg", damage);
                    push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
                }
            }
            continue;
        }

        if (e->type == ENEMY_DESERT_PHARAOH) {
            if (e->move_timer % 2 != 0) {
                push_message(g, "The Pharaoh gathers desert magic...");
            } else {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 3) {
                    dmg = 3;
                }
                dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
                if (dmg > 0) {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), "Pharaoh magic bolt: %d dmg", dmg);
                    push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
                }
            }
            continue;
        }

        if (e->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            if (e->move_timer % 2 == 0) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 4) dmg = 4;
                dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
                if (dmg == 0) {
                    continue;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Goblin King axe: %d dmg", dmg);
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            } else push_message(g, "The Goblin King raises his axe...");
            continue;
        }

        if (e->type == ENEMY_DROWNED_QUEEN) {
            if (e->move_timer % 3 == 0) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 5) {
                    dmg = 5;
                }
                dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
                if (dmg == 0) {
                    continue;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Queen's tidal wave: %d dmg", dmg);
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            } else {
                push_message(g, "The Drowned Queen summons the tide...");
            }
            continue;
        }

        if (e->type == ENEMY_FALLEN_SUN_GUARDIAN) {
            if (e->move_timer % 2 == 0) {
                int phase_bonus = e->hp <= e->max_hp / 2 ? 5 : 0;
                int dmg = e->attack + phase_bonus - g->player.defense / 2;
                if (dmg < 6) {
                    dmg = 6;
                }
                dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
                if (dmg > 0) {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), "Guardian sunburst: %d dmg", dmg);
                    push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
                }
            } else {
                push_message(g, e->hp <= e->max_hp / 2
                    ? "The broken guardian's core flares wildly!"
                    : "The Fallen Sun Guardian gathers light...");
            }
            continue;
        }

        if (e->type == ENEMY_SUN_PRIEST && e->move_timer % 3 == 0) {
            int healed = 0;
            for (int j = 0; j < g->enemy_count; j++) {
                Enemy *ally = &g->enemies[j];
                if (ally->active && ally != e && ally->hp < ally->max_hp &&
                    abs_int(ally->x - e->x) <= 4 &&
                    abs_int(ally->y - e->y) <= 4) {
                    int hp_before = ally->hp;
                    ally->hp += 8;
                    if (ally->hp > ally->max_hp) {
                        ally->hp = ally->max_hp;
                    }
                    combat_feedback_add(g, FEEDBACK_HEAL, FEEDBACK_NOW, ally->x, ally->y, ally->hp - hp_before);
                    push_message(g, "Sun Priest restores a guardian!");
                    healed = 1;
                    break;
                }
            }
            if (healed) {
                continue;
            }
        }

        if ((e->type == ENEMY_BLOWDART_HUNTER ||
            e->type == ENEMY_SUN_PRIEST ||
            e->type == ENEMY_SERPENT_SPIRIT ||
            e->type == ENEMY_MOONBOUND_SENTINEL) &&
            e->move_timer % 2 == 0 && clear_orthogonal_path(g, i, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 2) {
                dmg = 2;
            }
            dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
            if (dmg > 0) {
                if (e->type == ENEMY_BLOWDART_HUNTER) {
                    g->player.poison_turns = 3;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "%s ranged strike: %d dmg",
                    e->name, dmg);
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            }
            continue;
        }

        if ((e->type == ENEMY_SIREN ||
            e->type == ENEMY_WATER_ELEMENTAL) &&
            e->move_timer % 2 == 0 && clear_orthogonal_path(g, i, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 1) {
                dmg = 1;
            }
            dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
            if (dmg == 0) {
                continue;
            }
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), e->type == ENEMY_SIREN
                ? "Siren song: %d dmg" : "Water surge: %d dmg", dmg);
            push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            continue;
        }

        if ((e->type == ENEMY_GRAVE_ARCHER || e->type == ENEMY_GOBLIN_ARCHER ||
            e->type == ENEMY_ROAD_ARCHER ||
            e->type == ENEMY_FROST_ARCHER ||
            e->type == ENEMY_GOBLIN_BOMBER) &&
            e->move_timer % 2 == 0 && clear_orthogonal_path(g, i, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 1) dmg = 1;
            dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
            if (dmg == 0) {
                continue;
            }
            char msg[MAX_MESSAGE_LEN];
            const char *attack_name = e->type == ENEMY_GRAVE_ARCHER ? "Grave arrow" : e->type == ENEMY_GOBLIN_BOMBER ?
                "Goblin bomb" : e->type == ENEMY_ROAD_ARCHER ?
                "Road arrow" : e->type == ENEMY_FROST_ARCHER ?
                "Frost arrow" : "Goblin arrow";
            snprintf(msg, sizeof(msg), "%s: %d dmg", attack_name, dmg);
            push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            continue;
        }

        if (e->type == ENEMY_GOBLIN_SHAMAN && e->move_timer % 3 == 0) {
            int healed = 0;
            for (int j = 0; j < g->enemy_count; j++) {
                Enemy *ally = &g->enemies[j];
                if (ally->active && ally->hp < ally->max_hp &&
                    abs_int(ally->x - e->x) <= 4 &&
                    abs_int(ally->y - e->y) <= 4) {
                    int hp_before = ally->hp;
                    ally->hp += 6;
                    if (ally->hp > ally->max_hp) ally->hp = ally->max_hp;
                    combat_feedback_add(g, FEEDBACK_HEAL, FEEDBACK_NOW, ally->x, ally->y, ally->hp - hp_before);
                    healed = 1;
                    push_message(g, "Goblin Shaman heals an ally!");
                    break;
                }
            }
            if (healed) continue;
        }

        if (e->type == ENEMY_DARK_ELF && e->move_timer % 2 == 0 &&
            clear_orthogonal_path(g, i, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 1) dmg = 1;
            dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
            if (dmg == 0) {
                continue;
            }
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Dark Elf arrow: %d dmg", dmg);
            push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
            continue;
        }

        if (e->type == ENEMY_BONE_CANTOR) {
            if (e->move_timer % 4 == 0 && catacombs_cantor_raise(g, e)) {
                continue;
            }
            if (e->move_timer % 2 == 0 && clear_orthogonal_path(g, i, e)) {
                int damage = e->attack - g->player.defense / 2;
                if (damage < 1) {
                    damage = 1;
                }
                apply_enemy_ranged_damage(g, i, e, damage, shots);
                push_message_kind(g, "The Bone Cantor hurls a grave bolt!", MESSAGE_DAMAGE_TAKEN);
                continue;
            }
        }
        if (e->type == ENEMY_CRYPT_CONJURER) {
            if (e->move_timer % 4 == 0 && necromancer_revive(g, i)) continue;
            if (e->move_timer % 2 == 0 && clear_orthogonal_path(g, i, e)) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 1) dmg = 1;
                dmg = apply_enemy_ranged_damage(g, i, e, dmg, shots);
                if (dmg == 0) {
                    continue;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Conjurer bolt: %d dmg", dmg);
                push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
                continue;
            }
        }

        if (enemy_is_support(e) && enemy_has_nearby_ally(g, i, 4)) {
            continue;
        }

        if (enemy_prefers_range(e) && clear_orthogonal_path(g, i, e)) {
            continue;
        }

        if (e->type == ENEMY_ZOMBIE || e->type == ENEMY_GIANT_WURM ||
            e->type == ENEMY_FOREST_TROLL || e->type == ENEMY_CAVE_TROLL ||
            e->type == ENEMY_GIANT_CRAB ||
            e->type == ENEMY_ANIMATED_STATUE ||
            e->type == ENEMY_ICE_GOLEM ||
            e->type == ENEMY_MUMMY ||
            e->type == ENEMY_GOLEM ||
            e->type == ENEMY_THORN_GUARDIAN ||
            e->type == ENEMY_OBSIDIAN_GUARDIAN ||
            e->type == ENEMY_SHARD_GOLEM ||
            e->type == ENEMY_LIVING_FLOWER ||
            e->type == ENEMY_VINEBOUND_GUARDIAN ||
            e->type == ENEMY_LUNAR_EFFIGY ||
            e->type == ENEMY_MOONBOUND_SENTINEL) {
            if (e->move_timer % 2 != 0) continue;
        }

        int moved = enemy_prefers_flank(e) && enemy_move_to_flank(g, i);
        if (!moved) {
            moved = enemy_move_toward(g, i);
        }
        if ((e->type == ENEMY_CRYPT_BAT || e->type == ENEMY_GIANT_MOTH ||
            e->type == ENEMY_ASH_HOUND || e->type == ENEMY_CRYSTAL_SPIDER) && moved) {
            // Fast enemies close distance without attacking on their second move.
            enemy_move_toward(g, i);
        }
        // Ice Wolves skip flanking and charge two tiles a turn.
        if ((e->type == ENEMY_PIXIE || e->type == ENEMY_BLIGHTED_WOLF ||
            e->type == ENEMY_ICE_WOLF) && moved) {
            enemy_move_toward(g, i);
        }
        if (e->type == ENEMY_TEMPLE_STALKER && moved) {
            enemy_move_toward(g, i);
        }
        if (e->type == ENEMY_HORSEMAN && moved && path_distance > 3) {
            enemy_move_toward(g, i);
        }
    }
}
