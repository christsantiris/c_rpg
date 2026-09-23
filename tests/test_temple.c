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

void test_temple(void) {
    printf("Ruined Temple tests:\n");
    static GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_island(&g);
    g.player.x = ISLAND_GATE_X;
    g.player.y = ISLAND_GATE_Y + 1;
    action_resolve_player(&g, (Action){ACTION_MOVE, ISLAND_GATE_X,
        ISLAND_GATE_Y});

    ASSERT("walking through the island gate enters the handcrafted temple",
        g.location == LOCATION_TEMPLE &&
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
    const Enemy *guardian = find_temple_enemy(&balanced,
        ENEMY_FALLEN_SUN_GUARDIAN);
    ASSERT("level 18 temple enemies avoid duplicate region-order scaling",
        stalker && stalker->max_hp == 58 && stalker->attack == 21 &&
        stalker->defense == 5 && guardian && guardian->max_hp == 408 &&
        guardian->attack == 31 && guardian->defense == 12);

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

    Enemy *boss = NULL;
    for (int i = 0; i < g.enemy_count; i++) {
        if (g.enemies[i].type == ENEMY_FALLEN_SUN_GUARDIAN) {
            boss = &g.enemies[i];
            break;
        }
    }
    ASSERT("Fallen Sun Guardian waits in the northern sanctuary", boss != NULL);
    if (boss) {
        g.player.x = boss->x;
        g.player.y = boss->y + 5;
        boss->hp--;
        boss->move_timer = 1;
        EnemyProjectiles shots = {0};
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("a ranged hit wakes the boss and produces a visible counterattack",
            shots.count > 0 && shots.shots[0].type ==
            ENEMY_FALLEN_SUN_GUARDIAN);
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
        g.player.y == 18 && g.temple_alignment == 0 &&
        count_temple_enemies(&g, ENEMY_MOONBOUND_SENTINEL) == 4);

    game_record_temple_enemy_defeated(&g, ENEMY_FALLEN_SUN_GUARDIAN);
    g.player.x = TEMPLE_TREASURE_X;
    g.player.y = TEMPLE_TREASURE_Y + 1;
    int old_gold = g.gold;
    ASSERT("defeating the guardian opens and completes the buried treasure quest",
        g.map.tiles[6][32] == TILE_TEMPLE_FLOOR &&
        game_interact_temple(&g) && g.temple_treasure_state == 3 &&
        g.gold == old_gold + 150 &&
        g.map.tiles[TEMPLE_TREASURE_Y][TEMPLE_TREASURE_X] ==
        TILE_TEMPLE_RUBBLE);
    QuestJournalEntry quest;
    ASSERT("the journal records The Buried Sun as completed",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 1 &&
        quest_journal_get_entry(&g, QUEST_TAB_COMPLETED, 0, &quest) &&
        strcmp(quest.title, "The Buried Sun") == 0 &&
        quest.objective_count == 1 && quest.objective_complete[0]);

    const int slot = 99014;
    ASSERT("temple state saves successfully", save_game(&g, slot));
    static GameState loaded;
    ASSERT("temple alignment, treasure and encounter state survive loading",
        load_game(&loaded, slot) && loaded.location == LOCATION_TEMPLE &&
        loaded.temple_alignment == 0 && loaded.temple_treasure_state == 3 &&
        loaded.map.tiles[TEMPLE_TREASURE_Y][TEMPLE_TREASURE_X] ==
        TILE_TEMPLE_RUBBLE);
    remove("saves/savegame_99014.json");
}
