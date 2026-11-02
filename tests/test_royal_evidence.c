#include "test_utils.h"
#include "game/actions.h"
#include "game/boss_gates.h"
#include "game/royal_evidence.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define EVIDENCE_SLOT 99248

static void boss_arena(GameState *g, Location area) {
    g->location = area;
    g->level = 5;
    g->max_forest_level_reached = FOREST_BOSS_LEVEL;
    g->max_mountain_level_reached = MOUNTAIN_BOSS_LEVEL;
    memset(&g->map, 0, sizeof(g->map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = area == LOCATION_FOREST ? TILE_FOREST_FLOOR : TILE_MOUNTAIN_FORTRESS_FLOOR;
        }
    }
    g->map.room_count = 1;
    g->map.rooms[0] = (Room){2, 2, 16, 14};
    g->map.stairs_up_x = 3;
    g->map.stairs_up_y = 3;
    g->map.stairs_down_x = 15;
    g->map.stairs_down_y = 13;
    g->player.x = 9;
    g->player.y = 10;
    g->player.attack = 1000;
    g->floor_item_count = 0;
    g->loot_container_count = 0;
    g->enemy_count = 2;
    g->enemies[0] = (Enemy){.type = area == LOCATION_FOREST ? ENEMY_FOREST_NECROMANCER : ENEMY_MOUNTAIN_GOBLIN_KING,
        .active = 1, .is_boss = 1, .x = 10, .y = 10, .hp = 1, .max_hp = 180, .experience = 400};
    snprintf(g->enemies[0].name, sizeof(g->enemies[0].name), "%s", area == LOCATION_FOREST ? "Necromancer" : "Goblin King");
    g->enemies[1] = (Enemy){.type = ENEMY_GIANT_SPIDER, .active = 1, .x = 40, .y = 40, .hp = 30, .max_hp = 30};
}

static void victory_orders(GameState *g) {
    for (int order = 0; order < 2; order++) {
        game_init(g);
        ASSERT("new characters have no evidence or pending parchment", !royal_evidence_count(g) && !royal_evidence_pending(g) && !g->royal_evidence_seen);
        for (int i = 0; i < MAX_INVENTORY; i++) {
            g->inventory[i] = item_make_health_potion();
        }
        g->inventory_count = MAX_INVENTORY;
        for (int stage = 0; stage < 2; stage++) {
            Location area = stage == order ? LOCATION_FOREST : LOCATION_MOUNTAINS;
            int expected = area == LOCATION_FOREST ? ROYAL_EVIDENCE_PROTECTION : ROYAL_EVIDENCE_FUNDING;
            boss_arena(g, area);
            action_resolve_player(g, (Action){ACTION_MOVE, 10, 10});
            ASSERT("either boss grants its parchment without an accepted quest, full clear or free inventory slot", !g->enemies[0].active && g->enemies[1].active &&
                !g->alder_quest_state && !g->dain_quest_state && g->inventory_count == MAX_INVENTORY && royal_evidence_pending(g) == expected && royal_evidence_count(g) == stage + 1);
            ASSERT("the boss's road opens independently of reading the parchment or killing the other boss", boss_gate_open(g, area) &&
                (stage || !boss_gate_open(g, area == LOCATION_FOREST ? LOCATION_MOUNTAINS : LOCATION_FOREST)));
            const RoyalEvidence *document = royal_evidence_document(expected);
            ASSERT("each document implicates the King without naming the Royal Arcanist", document && strstr(document->text, "King") &&
                !strstr(document->text, "Arcanist") && !strstr(document->text, "Malakar"));
            int seen = g->royal_evidence_seen;
            ASSERT("holding a confirmation key cannot skip a newly revealed parchment", royal_evidence_handle_key(g, SDL_SCANCODE_RETURN, 1) && g->royal_evidence_seen == seen);
            ASSERT("movement and journal keys are captured while the parchment is open", royal_evidence_handle_key(g, SDL_SCANCODE_W, 0) &&
                royal_evidence_handle_key(g, SDL_SCANCODE_Q, 0) && royal_evidence_pending(g) == expected && g->player.x == 9);
            int xp = g->player.experience;
            int gold = g->gold;
            int hp = g->player.hp;
            g->key_bindings[CONTROL_QUEST_JOURNAL] = SDL_SCANCODE_J;
            ASSERT("closing keeps evidence without spending a turn, gold, HP or an active quest slot", royal_evidence_handle_key(g, stage ? SDL_SCANCODE_ESCAPE : SDL_SCANCODE_SPACE, 0) &&
                !royal_evidence_pending(g) && g->player.experience == xp && g->gold == gold && g->player.hp == hp && game_active_quest_count(g) == 0);
            ASSERT("the reopening hint follows the remapped journal key", strstr(g->messages[g->message_count - 1], "Press J") != NULL);
            ASSERT("normal input resumes after a single acknowledgment", !royal_evidence_handle_key(g, SDL_SCANCODE_RETURN, 0));
            ASSERT("evidence has its own journal tab and cannot appear as a quest", quest_journal_count(g, QUEST_TAB_EVIDENCE) == stage + 1 &&
                !quest_journal_count(g, QUEST_TAB_ACTIVE) && !quest_journal_count(g, QUEST_TAB_COMPLETED));
        }
        ASSERT("both independent records strengthen the conclusion in either victory order", g->royal_evidence_seen == 3 &&
            strstr(royal_evidence_conclusion(g), "Both parchments") && strstr(royal_evidence_conclusion(g), "funds the raids") &&
            strstr(royal_evidence_conclusion(g), "protects the Necromancer"));
        ASSERT("journal rereading exposes only discovered documents and validates selection bounds", royal_evidence_entry(g, 0)->id == ROYAL_EVIDENCE_FUNDING &&
            royal_evidence_entry(g, 1)->id == ROYAL_EVIDENCE_PROTECTION && !royal_evidence_entry(g, -1) && !royal_evidence_entry(g, 2) && !royal_evidence_document(3));
    }
}

static int rewrite_save(int legacy, double seen) {
    const char *path = "saves/savegame_99248.json";
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *data = malloc((size_t)length + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t count = fread(data, 1, (size_t)length, file);
    data[count] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    if (legacy) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 234);
        cJSON_DeleteItemFromObject(root, "royal_evidence_seen");
    } else {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "royal_evidence_seen"), seen);
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void saved_evidence(GameState *g, GameState *loaded) {
    ASSERT("evidence save test slot is unused", !save_exists(EVIDENCE_SLOT));
    if (save_exists(EVIDENCE_SLOT)) {
        return;
    }
    game_init(g);
    g->defeated_bosses = 1u << LOCATION_FOREST;
    g->gold = 789;
    g->player.hp = 43;
    ASSERT("saving before acknowledgment resumes the pending parchment with existing progress intact", save_game(g, EVIDENCE_SLOT) && load_game(loaded, EVIDENCE_SLOT) &&
        royal_evidence_pending(loaded) == ROYAL_EVIDENCE_PROTECTION && loaded->gold == 789 && loaded->player.hp == 43);
    royal_evidence_handle_key(loaded, SDL_SCANCODE_RETURN, 0);
    ASSERT("saving after acknowledgment keeps journal evidence without a repeated popup", save_game(loaded, EVIDENCE_SLOT) && load_game(g, EVIDENCE_SLOT) &&
        !royal_evidence_pending(g) && royal_evidence_count(g) == 1 && royal_evidence_entry(g, 0)->id == ROYAL_EVIDENCE_PROTECTION);
    g->defeated_bosses |= 1u << LOCATION_MOUNTAINS;
    ASSERT("a second victory remains pending across a save without reopening the first document", save_game(g, EVIDENCE_SLOT) && load_game(loaded, EVIDENCE_SLOT) &&
        royal_evidence_pending(loaded) == ROYAL_EVIDENCE_FUNDING && loaded->royal_evidence_seen == ROYAL_EVIDENCE_PROTECTION);
    ASSERT("older victories gain both journal records without interrupting a resumed game", save_game(g, EVIDENCE_SLOT) && rewrite_save(1, 0) && load_game(loaded, EVIDENCE_SLOT) &&
        royal_evidence_count(loaded) == 2 && !royal_evidence_pending(loaded) && loaded->gold == 789 && loaded->player.hp == 43);
    ASSERT("the migrated evidence persists after saving in the current format", save_game(loaded, EVIDENCE_SLOT) && load_game(g, EVIDENCE_SLOT) && g->royal_evidence_seen == 3);
    const double invalid[] = {-1, 4, 1.5};
    for (int i = 0; i < 3; i++) {
        ASSERT("invalid evidence acknowledgment data is rejected", save_game(g, EVIDENCE_SLOT) && rewrite_save(0, invalid[i]) && !load_game(loaded, EVIDENCE_SLOT));
    }
    g->player.hp = 0;
    g->royal_evidence_seen = 0;
    ASSERT("unread evidence never blocks game-over controls", !royal_evidence_pending(g) && !royal_evidence_handle_key(g, SDL_SCANCODE_RETURN, 0));
    remove("saves/savegame_99248.json");
}

static void story_followups(GameState *g) {
    for (int both = 0; both < 2; both++) {
        for (int alder = 0; alder < 2; alder++) {
            game_init(g);
            g->location = LOCATION_TAVERN;
            g->defeated_bosses = (1u << LOCATION_DUNGEON) | (1u << (alder ? LOCATION_FOREST : LOCATION_MOUNTAINS));
            if (both) {
                g->defeated_bosses |= (1u << LOCATION_FOREST) | (1u << LOCATION_MOUNTAINS);
            }
            g->alder_quest_state = REGIONAL_CHAIN_COMPLETE;
            g->dain_quest_state = REGIONAL_CHAIN_COMPLETE;
            g->player.x = alder ? ALDER_OAKHAVEN_X : DAIN_OAKHAVEN_X;
            g->player.y = alder ? ALDER_OAKHAVEN_Y + 1 : DAIN_OAKHAVEN_Y + 1;
            if (alder) {
                game_talk_to_alder(g);
            } else {
                game_talk_to_dain(g);
            }
            ASSERT("optional Inn followups interpret royal evidence and retain the proper onward destination", g->dialogue_active &&
                strstr(g->dialogue_text, "King") && strstr(g->dialogue_text, alder ? "Dawnmere" : "Stillbury") &&
                strlen(g->dialogue_text) < MAX_DIALOGUE_LEN - 1 && !strstr(g->dialogue_text, "Arcanist"));
            ASSERT("finding both records changes the NPC's interpretation without another reward", (!both || strstr(g->dialogue_text, alder ? "both records" : "match")) && g->gold == 0);
        }
    }
}

void test_royal_evidence(void) {
    printf("Act 2 royal evidence:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    if (g && loaded) {
        victory_orders(g);
        saved_evidence(g, loaded);
        story_followups(g);
    } else {
        ASSERT("evidence fixtures allocate", 0);
    }
    free(g);
    free(loaded);
}
