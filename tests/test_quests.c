#include "test_utils.h"
#include "../src/game/game.h"
#include <string.h>

static int find_tile(const Map *map, TileType type, int *found_x, int *found_y) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map->tiles[y][x] == type) {
                *found_x = x;
                *found_y = y;
                return 1;
            }
        }
    }
    return 0;
}

void test_elowen_quest(void) {
    printf("Elowen quest tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    game_talk_to_elowen(&g);
    ASSERT("Elowen offers The Broken Seals", g.elowen_quest_state == 1);
    ASSERT("Elowen speaks through dialogue state", g.dialogue_active &&
        strstr(g.dialogue_text, "burial seals") != NULL);
    ASSERT("event log records only the quest assignment",
        strcmp(g.messages[g.message_count - 1],
            "Quest assigned: The Broken Seals.") == 0);
    ASSERT("new quest begins with no restored seals",
        g.elowen_seals_restored == 0);

    game_enter_dungeon(&g);
    ASSERT("new dungeon expedition starts on floor one", g.level == 1);
    int expected_levels[3] = {2, 4, 6};
    for (int seal = 0; seal < 3; seal++) {
        while (g.level < expected_levels[seal]) {
            game_descend(&g);
        }
        int seal_x = 0;
        int seal_y = 0;
        ASSERT("quest floor contains a broken burial seal",
            find_tile(&g.map, TILE_BROKEN_BURIAL_SEAL, &seal_x, &seal_y));
        g.player.x = seal_x;
        g.player.y = seal_y;
        Action restore = {ACTION_PICK_UP, 0, 0};
        action_resolve_player(&g, restore);
        ASSERT("P restores the burial seal",
            g.map.tiles[seal_y][seal_x] == TILE_RESTORED_BURIAL_SEAL);
    }
    ASSERT("three restored seals make the quest ready",
        g.elowen_quest_state == 2 && g.elowen_seals_restored == 7);

    game_return_to_town(&g);
    int gold_before = g.gold;
    game_talk_to_elowen(&g);
    ASSERT("Elowen completes the quest", g.elowen_quest_state == 3);
    ASSERT("Elowen awards 100 gold", g.gold == gold_before + 100);

    game_enter_dungeon(&g);
    ASSERT("replayed dungeon begins again on floor one", g.level == 1);
    int living_enemies = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        living_enemies += g.enemies[i].active;
    }
    ASSERT("new expedition repopulates regular enemies", living_enemies > 0);

    g.defeated_bosses |= 1 << LOCATION_DUNGEON;
    g.level = DUNGEON_DEPTH;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    int liches = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        liches += g.enemies[i].type == ENEMY_LICH_KING;
    }
    ASSERT("defeated Lich King does not respawn", liches == 0);
    game_return_to_town(&g);
    game_enter_dungeon(&g);
    while (g.level < DUNGEON_DEPTH) {
        game_descend(&g);
    }
    ASSERT("replayed finale keeps its return passage open",
        g.map.tiles[g.map.stairs_down_y][g.map.stairs_down_x] ==
            TILE_RETURN_EXIT);
}

void test_tavern_interior(void) {
    printf("Tavern interior tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    g.player.x = 8;
    g.player.y = 21;
    Action enter = {ACTION_MOVE, 8, 20};
    action_resolve_player(&g, enter);
    ASSERT("walking into the Tavern door enters its interior",
        g.location == LOCATION_TAVERN);
    ASSERT("Tavern spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));

    int elowen_x = 0;
    int elowen_y = 0;
    ASSERT("Elowen has an in-world Tavern tile",
        find_tile(&g.map, TILE_NPC_ELOWEN, &elowen_x, &elowen_y));
    ASSERT("player cannot overlap Elowen",
        !map_is_walkable(&g.map, elowen_x, elowen_y));
    g.player.x = elowen_x;
    g.player.y = elowen_y + 1;
    game_talk_to_elowen(&g);
    ASSERT("Elowen quest interaction works inside the Tavern",
        g.elowen_quest_state == 1);
    ASSERT("talking opens Elowen's dialogue bubble", g.dialogue_active);
    game_move_player(&g, 1, 0);
    ASSERT("moving dismisses the dialogue bubble", !g.dialogue_active);

    int exit_x = 0;
    int exit_y = 0;
    ASSERT("Tavern has a south doorway",
        find_tile(&g.map, TILE_TAVERN_EXIT, &exit_x, &exit_y));
    g.player.x = exit_x;
    g.player.y = exit_y - 1;
    Action leave = {ACTION_MOVE, exit_x, exit_y};
    action_resolve_player(&g, leave);
    ASSERT("walking through the Tavern doorway returns to town",
        g.location == LOCATION_TOWN);
    ASSERT("Tavern returns player outside its front door",
        g.player.x == 8 && g.player.y == 21);
    ASSERT("Tavern transition preserves Elowen quest state",
        g.elowen_quest_state == 1);
}
