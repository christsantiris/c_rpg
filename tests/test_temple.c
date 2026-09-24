#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/screens/quest_journal.h"
#include "../src/systems/save_load.h"
#include <string.h>

static int count_temple_enemies(const GameState *g, EnemyType type) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == type) {
            count++;
        }
    }
    return count;
}

static const Enemy *find_temple_enemy(const GameState *g, EnemyType type) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == type) {
            return &g->enemies[i];
        }
    }
    return NULL;
}

static int temple_enemies_are_on_walkable_tiles(const GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active &&
            !map_is_walkable(&g->map, g->enemies[i].x, g->enemies[i].y)) {
            return 0;
        }
    }
    return 1;
}

void test_temple(void) {
    printf("Ruined Temple tests:\n");
    static GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_island(&g);
    ASSERT("Nahla waits on the island near the temple route",
        g.map.tiles[ISLAND_NAHLA_Y][ISLAND_NAHLA_X] ==
        TILE_NPC_ISLAND_NAHLA);
    game_talk_to_nahla(&g);
    ASSERT("Nahla assigns The Buried Sun before the climb",
        g.temple_treasure_state == 1 &&
        strstr(g.dialogue_text, "four tiers"));
    g.dialogue_active = 0;
    g.player.x = ISLAND_GATE_X;
    g.player.y = ISLAND_GATE_Y + 1;
    action_resolve_player(&g, (Action){ACTION_MOVE, ISLAND_GATE_X,
        ISLAND_GATE_Y});

    ASSERT("walking through the island gate enters the first pyramid tier",
        g.location == LOCATION_TEMPLE && g.level == 1 &&
        g.player.x == TEMPLE_ENTRANCE_X &&
        g.player.y == TEMPLE_ENTRANCE_Y &&
        g.map.tiles[27][25] == TILE_TEMPLE_MOON_DOOR_CLOSED &&
        g.temple_treasure_state == 1);
    ASSERT("temple begins in Sun state with dormant moon sentinels",
        g.temple_alignment == 0 &&
        g.map.tiles[27][22] == TILE_TEMPLE_DORMANT_SENTINEL &&
        count_temple_enemies(&g, ENEMY_MOONBOUND_SENTINEL) == 0);

    static GameState balanced;
    balanced.player.player_class = CLASS_ROGUE;
    game_init(&balanced);
    balanced.player.level = 18;
    balanced.defeated_bosses = (1 << LOCATION_DUNGEON) |
        (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS) |
        (1 << LOCATION_COAST);
    game_enter_temple(&balanced);
    const Enemy *stalker = find_temple_enemy(&balanced,
        ENEMY_TEMPLE_STALKER);
    const Enemy *guardian = NULL;
    ASSERT("level 18 temple enemies avoid duplicate region-order scaling",
        stalker && stalker->max_hp == 58 && stalker->attack == 21 &&
        stalker->defense == 5);
    ASSERT("the Guardian does not appear on the lower pyramid tiers",
        find_temple_enemy(&balanced, ENEMY_FALLEN_SUN_GUARDIAN) == NULL);
    ASSERT("first-tier enemies occupy walkable floor tiles",
        temple_enemies_are_on_walkable_tiles(&balanced));
    for (int floor = 2; floor <= TEMPLE_DEPTH; floor++) {
        game_descend(&balanced);
        ASSERT("each higher tier places enemies on walkable floor tiles",
            temple_enemies_are_on_walkable_tiles(&balanced));
    }
    guardian = find_temple_enemy(&balanced, ENEMY_FALLEN_SUN_GUARDIAN);
    ASSERT("the balanced Guardian waits only at the pyramid summit",
        balanced.level == TEMPLE_DEPTH && guardian &&
        guardian->max_hp == 408 && guardian->attack == 31 &&
        guardian->defense == 12);

    static GameState completed;
    completed.player.player_class = CLASS_ROGUE;
    game_init(&completed);
    completed.defeated_bosses |= 1 << LOCATION_TEMPLE;
    completed.temple_treasure_state = 3;
    game_enter_temple(&completed);
    for (int floor = 2; floor <= TEMPLE_DEPTH; floor++) {
        game_descend(&completed);
    }
    ASSERT("a completed summit does not respawn the Guardian",
        find_temple_enemy(&completed, ENEMY_FALLEN_SUN_GUARDIAN) == NULL);
    ASSERT("a completed summit keeps the vault open and treasure removed",
        completed.map.tiles[6][32] == TILE_TEMPLE_FLOOR &&
        completed.map.tiles[TEMPLE_TREASURE_Y][TEMPLE_TREASURE_X] ==
        TILE_TEMPLE_RUBBLE);

    g.player.x = 32;
    g.player.y = 29;
    ASSERT("altar interaction raises the Moon and animates sentinels",
        game_interact_temple(&g) && g.temple_alignment == 1 &&
        g.map.tiles[27][25] == TILE_TEMPLE_MOON_DOOR_OPEN &&
        count_temple_enemies(&g, ENEMY_MOONBOUND_SENTINEL) == 4);
    ASSERT("returning to Sun closes doors without petrifying awakened enemies",
        game_interact_temple(&g) && g.temple_alignment == 0 &&
        g.map.tiles[27][25] == TILE_TEMPLE_MOON_DOOR_CLOSED &&
        count_temple_enemies(&g, ENEMY_MOONBOUND_SENTINEL) == 4);

    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    ASSERT("the first tier exits by stairs that climb the pyramid",
        g.map.tiles[g.player.y][g.player.x] == TILE_STAIRS_UP);
    action_resolve_player(&g, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("climbing reaches tier two and its downward return stairs",
        g.level == 2 &&
        g.map.tiles[g.player.y][g.player.x] == TILE_STAIRS_DOWN &&
        find_temple_enemy(&g, ENEMY_FALLEN_SUN_GUARDIAN) == NULL);
    action_resolve_player(&g, (Action){ACTION_DESCEND, 0, 0});
    ASSERT("descending returns to the previous pyramid tier", g.level == 1);
    for (int floor = 2; floor <= TEMPLE_DEPTH; floor++) {
        g.player.x = g.map.stairs_down_x;
        g.player.y = g.map.stairs_down_y;
        action_resolve_player(&g, (Action){ACTION_ASCEND, 0, 0});
    }

    Enemy *boss = NULL;
    for (int i = 0; i < g.enemy_count; i++) {
        if (g.enemies[i].type == ENEMY_FALLEN_SUN_GUARDIAN) {
            boss = &g.enemies[i];
            break;
        }
    }
    ASSERT("Fallen Sun Guardian waits in the summit sanctuary",
        g.level == TEMPLE_DEPTH && boss != NULL);
    if (boss) {
        g.player.x = boss->x;
        g.player.y = boss->y + 5;
        boss->hp--;
        boss->move_timer = 1;
        EnemyProjectiles shots = {0};
        int hp_before = g.player.hp;
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("the closed sanctuary door blocks the boss's counterattack",
            shots.count == 0 && g.player.hp == hp_before);
        g.map.tiles[12][32] = TILE_TEMPLE_MOON_DOOR_OPEN;
        boss->move_timer = 1;
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("an open sanctuary gives the boss a visible firing path",
            shots.count > 0 && shots.shots[0].type ==
            ENEMY_FALLEN_SUN_GUARDIAN);
        g.map.tiles[12][32] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        g.temple_alignment = 0;
    }

    g.player.x = 31;
    g.player.y = 18;
    game_open_town_portal(&g);
    ASSERT("Return to Town leaves a temple portal beside the harbor",
        g.location == LOCATION_TOWN && g.portal_active &&
        g.portal_location == LOCATION_TEMPLE);
    game_use_town_portal(&g);
    ASSERT("town portal restores the exact temple position and state",
        g.location == LOCATION_TEMPLE && g.player.x == 31 &&
        g.player.y == 18 && g.level == TEMPLE_DEPTH &&
        g.temple_alignment == 0);

    game_record_temple_enemy_defeated(&g, ENEMY_FALLEN_SUN_GUARDIAN);
    g.player.x = TEMPLE_TREASURE_X;
    g.player.y = TEMPLE_TREASURE_Y + 1;
    int old_gold = g.gold;
    ASSERT("defeating the Guardian opens the summit vault",
        g.map.tiles[6][32] == TILE_TEMPLE_FLOOR &&
        game_interact_temple(&g) && g.temple_treasure_state == 2 &&
        g.gold == old_gold &&
        g.map.tiles[TEMPLE_TREASURE_Y][TEMPLE_TREASURE_X] ==
        TILE_TEMPLE_RUBBLE);
    game_leave_temple(&g);
    game_talk_to_nahla(&g);
    ASSERT("Nahla completes the quest and pays its reward",
        g.temple_treasure_state == 3 && g.gold == old_gold + 150);
    QuestJournalEntry quest;
    ASSERT("the journal records The Buried Sun as completed",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 1 &&
        quest_journal_get_entry(&g, QUEST_TAB_COMPLETED, 0, &quest) &&
        strcmp(quest.title, "The Buried Sun") == 0 &&
        quest.objective_count == 1 && quest.objective_complete[0]);

    const int slot = 99014;
    ASSERT("temple state saves successfully", save_game(&g, slot));
    static GameState loaded;
    ASSERT("all temple floors and quest progress survive loading",
        load_game(&loaded, slot) && loaded.location == LOCATION_ISLAND &&
        loaded.max_temple_level_reached == TEMPLE_DEPTH &&
        loaded.temple_treasure_state == 3 &&
        loaded.temple_cache[TEMPLE_DEPTH - 1].valid &&
        loaded.temple_cache[TEMPLE_DEPTH - 1]
            .map.tiles[TEMPLE_TREASURE_Y][TEMPLE_TREASURE_X] ==
            TILE_TEMPLE_RUBBLE);
    remove("saves/savegame_99014.json");
}
