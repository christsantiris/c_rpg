#include "approach_checks.h"
#include "game/western_arc.h"
#include "game/crown_quest.h"
#include "game/giants_crown.h"
#include "game/royal_evidence.h"
#include "game/eastern_arc.h"
#include "game/aldric.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define WESTERN_ARC_SLOT 99271

static void visit_camp_npc(GameState *g, int x, int y) {
    g->player.x = x;
    g->player.y = y - 1;
    g->dialogue_active = 0;
    ASSERT("the caravan camp NPC can be spoken to beside the road", western_talk(g));
}

static void enter_convoy(GameState *g) {
    game_enter_desert(g);
    game_descend(g);
    g->player.x = TRIBUTE_X;
    g->player.y = TRIBUTE_Y;
}

static int count_escorts(const GameState *g, int alive) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        count += western_convoy_enemy(g, &g->enemies[i]) && (!alive || g->enemies[i].active);
    }
    return count;
}

static int rewrite_legacy(void) {
    const char *path = "saves/savegame_99271.json";
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
    size_t read = fread(data, 1, (size_t)length, file);
    data[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 239);
    cJSON_AddNumberToObject(root, "sunscar_lamp_quest_state", 2);
    const char *fields[] = {"tribute_quest_state", "tribute_progress", "jarl_quest_state", "zara_at_camp"};
    for (int i = 0; i < 4; i++) {
        cJSON_DeleteItemFromObject(root, fields[i]);
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    int ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

void test_western_arc(void) {
    printf("Western Act 3 quest tests:\n");
    static GameState g;
    static GameState loaded;
    game_init(&g);
    g.defeated_bosses |= (1u << LOCATION_DUNGEON) | (1u << LOCATION_MOUNTAINS);
    g.dain_quest_state = REGIONAL_CHAIN_COMPLETE;
    game_enter_tavern(&g);
    g.player.x = DAIN_OAKHAVEN_X;
    g.player.y = DAIN_OAKHAVEN_Y - 1;
    game_talk_to_dain(&g);
    ASSERT("Dain refers the player to Zara in Oakhaven", strstr(g.dialogue_text, "Zara") && strstr(g.dialogue_text, "Oakhaven's Inn"));
    ASSERT("Zara occupies the retired Rook place without displacing Dain", g.map.tiles[ZARA_OAKHAVEN_Y][ZARA_OAKHAVEN_X] == TILE_NPC_GUILD_SEEKER && g.map.tiles[DAIN_OAKHAVEN_Y][DAIN_OAKHAVEN_X] == TILE_NPC_DAIN);
    g.player.x = ZARA_OAKHAVEN_X;
    g.player.y = ZARA_OAKHAVEN_Y - 1;
    ASSERT("Zara offers only the tribute quest in Oakhaven", western_talk(&g) && game_quest_offer_active(&g) && !g.tribute_quest_state && strstr(g.dialogue_text, "Sunscar 2"));
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("accepting in the Inn uses one active slot", g.tribute_quest_state == 1 && game_active_quest_count(&g) == 1);
    approach_enter_region(&g, APPROACH_SUNSCAR_ROAD);
    ASSERT("Zara and the two additional NPCs meet at the caravan camp", g.zara_at_camp && g.map.tiles[CAMP_ZARA_Y][CAMP_ZARA_X] == TILE_NPC_GUILD_SEEKER && g.map.tiles[CAMP_GARRAN_Y][CAMP_GARRAN_X] == TILE_NPC_GARRAN && g.map.tiles[CAMP_LIORA_Y][CAMP_LIORA_X] == TILE_NPC_LIORA);
    visit_camp_npc(&g, CAMP_GARRAN_X, CAMP_GARRAN_Y);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    visit_camp_npc(&g, CAMP_LIORA_X, CAMP_LIORA_Y);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("the western arc has exactly three independently assigned active quests", g.jarl_quest_state == 1 && g.crown_quest_state == 1 && game_active_quest_count(&g) == 3 && quest_journal_count(&g, QUEST_TAB_ACTIVE) == 3);
    game_enter_tavern(&g);
    ASSERT("Zara no longer duplicates herself in Oakhaven after moving to camp", g.map.tiles[ZARA_OAKHAVEN_Y][ZARA_OAKHAVEN_X] != TILE_NPC_GUILD_SEEKER);
    enter_convoy(&g);
    ASSERT("the optional convoy has seven escorts and accessible cargo on Sunscar 2", g.level == 2 && count_escorts(&g, 1) == 7 && map_is_walkable(&g.map, TRIBUTE_X, TRIBUTE_Y));
    g.player.hp = g.player.max_hp = 100000;
    int hp = g.player.hp;
    Enemy before[TRIBUTE_GUARDS];
    int next = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        if (western_convoy_enemy(&g, &g.enemies[i])) {
            before[next++] = g.enemies[i];
        } else {
            g.enemies[i].active = 0;
        }
    }
    g.aldric_party_state = ALDRIC_FOLLOWING;
    g.aldric_hp = ALDRIC_MAX_HP;
    g.aldric_x = before[0].x + 1;
    g.aldric_y = before[0].y;
    for (int turn = 0; turn < 3; turn++) {
        action_resolve_enemies(&g);
    }
    next = 0;
    int unchanged = 1;
    for (int i = 0; i < g.enemy_count; i++) {
        if (western_convoy_enemy(&g, &g.enemies[i])) {
            unchanged &= g.enemies[i].hp == before[next].hp && g.enemies[i].x == before[next].x && g.enemies[i].y == before[next].y;
            next++;
        }
    }
    ASSERT("nearby escorts and Aldric do not initiate an anti-Crown battle", unchanged && g.player.hp == hp && !g.royal_reported_actions);
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("cargo prompts an explicit reclaim-or-pass choice and advertises the Action key", western_convoy_prompt(&g) && game_has_regional_interaction(&g));
    western_handle_key(&g, SDL_SCANCODE_Y, 1);
    ASSERT("held Y cannot accidentally start the convoy fight", !g.royal_reported_actions);
    western_handle_key(&g, SDL_SCANCODE_N, 0);
    ASSERT("passing the convoy leaves cargo and Crown standing unchanged", !g.dialogue_active && !g.tribute_progress && !g.royal_reported_actions);
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    western_handle_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("intervening is reported to Stillbury and activates the escort", g.royal_reported_actions == 2 && g.enemies[g.enemy_count - 1].boss_aux_x);
    int wounded = g.enemy_count - 1;
    g.enemies[wounded].hp -= 9;
    ASSERT("quest progress, convoy wounds and reported actions survive save/load", save_game(&g, WESTERN_ARC_SLOT) && load_game(&loaded, WESTERN_ARC_SLOT) && loaded.tribute_quest_state == 1 && loaded.jarl_quest_state == 1 && loaded.crown_quest_state == 1 && loaded.zara_at_camp && loaded.royal_reported_actions == 2 && loaded.enemies[wounded].hp == g.enemies[wounded].hp);
    g = loaded;
    for (int i = 0; i < g.enemy_count; i++) {
        if (western_convoy_enemy(&g, &g.enemies[i])) {
            g.enemies[i].active = 0;
        }
    }
    g.inventory_count = MAX_INVENTORY;
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("cargo requires defeating the escort and uses no pack slot", g.tribute_progress == 1 && g.tribute_quest_state == 1 && g.inventory_count == MAX_INVENTORY);
    g.defeated_bosses |= 1u << LOCATION_DESERT;
    game_update_level_progress(&g);
    ASSERT("cargo plus the Pharaoh's agreement completes tribute before reward collection", g.tribute_progress == 3 && g.tribute_quest_state == 2 && game_active_quest_count(&g) == 2 && royal_evidence_document(ROYAL_EVIDENCE_TRIBUTE));
    approach_enter_region(&g, APPROACH_SUNSCAR_ROAD);
    int gold = g.gold;
    g.player.experience_next = 10000000;
    int xp = g.player.experience;
    visit_camp_npc(&g, CAMP_ZARA_X, CAMP_ZARA_Y);
    ASSERT("Zara pays at the camp immediately outside Sunscar", g.tribute_quest_state == 3 && g.gold == gold + TRIBUTE_GOLD && g.player.experience == xp + TRIBUTE_XP);
    western_talk(&g);
    ASSERT("Enter closes Zara's finished dialogue without leaving play", western_handle_key(&g, SDL_SCANCODE_RETURN, 0) && !g.dialogue_active);
    ASSERT("Zara's reward cannot be claimed twice", g.gold == gold + TRIBUTE_GOLD);
    giants_defeat(&g);
    ASSERT("the Jarl boss quest finishes immediately and yields his crown", g.jarl_quest_state == 3 && g.crown_quest_state == 2 && (g.crown_quest_progress & CROWN_TAKEN) && game_active_quest_count(&g) == 0);
    ASSERT("defeating the Jarl adds evidence without creating a further Crown offense", royal_evidence_found(&g) & ROYAL_EVIDENCE_CHARTER && g.royal_reported_actions == 2);
    approach_enter_region(&g, APPROACH_PEAKS_NORTH_ROAD);
    ASSERT("Edda is visible on the first bridge-road frame without erasing the road", g.map.tiles[EDDA_STILLBURY_Y][EDDA_STILLBURY_X] == TILE_NPC_EDDA && g.map.tiles[APPROACH_BRIDGE_EXIT_Y][EDDA_STILLBURY_X] == TILE_TOWN_PATH);
    walk_approach_to(&g, EDDA_STILLBURY_X, EDDA_STILLBURY_Y - 1);
    gold = g.gold;
    ASSERT("wanted travelers can deliver the crown outside Stillbury's checkpoint", crown_quest_talk(&g) && g.crown_quest_state == 3 && g.crown_reward_state == 1 && g.gold == gold + CROWN_REWARD_GOLD);
    g.inventory_count--;
    crown_quest_talk(&g);
    crown_quest_talk(&g);
    ASSERT("Edda reserves the shield for a full pack and pays each reward once", g.crown_reward_state == 2 && g.inventory_count == MAX_INVENTORY && g.gold == gold + CROWN_REWARD_GOLD);
    ASSERT("completed western quests and shield delivery persist", save_game(&g, WESTERN_ARC_SLOT) && load_game(&loaded, WESTERN_ARC_SLOT) && loaded.tribute_quest_state == 3 && loaded.jarl_quest_state == 3 && loaded.crown_quest_state == 3 && loaded.crown_reward_state == 2);

    for (int seed = 0; seed < 4; seed++) {
        game_init(&g);
        srand(8700 + seed);
        enter_convoy(&g);
        ASSERT("each generated convoy preserves actual entrance and onward transition tiles", g.map.tiles[g.map.stairs_up_y][g.map.stairs_up_x] == TILE_DESERT_ENTRANCE && g.map.tiles[g.map.stairs_down_y][g.map.stairs_down_x] == TILE_DESERT_EXIT && count_escorts(&g, 1) == TRIBUTE_GUARDS);
        int i = g.enemy_count - 1;
        int x = g.enemies[i].x;
        int y = g.enemies[i].y;
        g.player.x = x;
        g.player.y = y + 1;
        g.player.attack = 10000;
        g.aldric_party_state = ALDRIC_WAITING;
        action_resolve_player(&g, (Action){ACTION_MOVE, x, y});
        ASSERT("attacking an escort directly also reports the public offense", g.royal_reported_actions == 2 && !g.enemies[i].active);
        game_ascend(&g);
        game_enter_desert(&g);
        game_descend(&g);
        ASSERT("a fresh expedition never revives the named tribute escorts", count_escorts(&g, 0) == TRIBUTE_GUARDS && count_escorts(&g, 1) == TRIBUTE_GUARDS - 1);
    }
    for (int method = 0; method < 4; method++) {
        memset(&g, 0, sizeof(g));
        g.player.player_class = method == 0 ? CLASS_ROGUE : CLASS_MAGE;
        game_init(&g);
        g.location = LOCATION_DESERT;
        g.level = 2;
        g.player.x = 20;
        g.player.y = 20;
        g.player.last_dx = 1;
        g.player.last_dy = 0;
        g.player.mp = g.player.max_mp = 1000;
        g.enemy_count = 1;
        for (int y = 18; y <= 22; y++) {
            for (int x = 18; x <= 25; x++) {
                g.map.tiles[y][x] = TILE_DESERT_FLOOR;
            }
        }
        game_spawn_enemy(&g, &g.enemies[0], ENEMY_ROYAL_GUARD, 22, 20);
        g.enemies[0].boss_aux_x = 0;
        g.enemies[0].boss_aux_y = TRIBUTE_SQUAD;
        g.enemies[0].hp = g.enemies[0].max_hp = 1000;
        g.enemies[0].defense = 0;
        if (method == 0) {
            g.inventory[0] = item_make_shortbow();
            game_equip_main_hand(&g, 0);
            action_resolve_player(&g, (Action){ACTION_RANGED_ATTACK, 0, 0});
        } else {
            g.player.known_spell_count = 1;
            g.player.equipped_spell = 0;
            g.player.known_spells[0] = method == 1 ? spell_make_magic_arrow() : method == 2 ? spell_make_fireball() : spell_make_frost_bolt();
            action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
        }
        ASSERT("arrows and offensive spells against the convoy are reported like melee", g.royal_reported_actions == 2 && g.enemies[0].boss_aux_x);
    }
    game_init(&g);
    g.defeated_bosses |= 1u << LOCATION_DESERT;
    western_update(&g);
    ASSERT("bypassing the convoy and defeating the Pharaoh leaves tribute incomplete and standing clean", g.tribute_progress == 2 && !g.tribute_quest_state && !g.royal_reported_actions);
    giants_defeat(&g);
    approach_enter_region(&g, APPROACH_SUNSCAR_ROAD);
    visit_camp_npc(&g, CAMP_GARRAN_X, CAMP_GARRAN_Y);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    visit_camp_npc(&g, CAMP_LIORA_X, CAMP_LIORA_Y);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("finding the Jarl and crown before meeting the camp NPCs counts retroactively", g.jarl_quest_state == 3 && g.crown_quest_state == 2 && !g.royal_reported_actions);
    game_init(&g);
    g.location = LOCATION_DESERT;
    g.level = 4;
    map_generate_desert(&g.map, 4);
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;
    g.map.tiles[20][30] = TILE_RETIRED_DESERT_LAMP;
    g.desert_cache[3].valid = 1;
    g.desert_cache[3].map = g.map;
    g.gold = 319;
    ASSERT("old lamp progress is retired from both live and cached maps without a reward", save_game(&g, WESTERN_ARC_SLOT) && rewrite_legacy() && load_game(&loaded, WESTERN_ARC_SLOT) && loaded.map.tiles[20][30] == TILE_DESERT_FLOOR && loaded.desert_cache[3].map.tiles[20][30] == TILE_DESERT_FLOOR && loaded.gold == 319 && !quest_journal_count(&loaded, QUEST_TAB_ACTIVE) && !quest_journal_count(&loaded, QUEST_TAB_COMPLETED));
    remove("saves/savegame_99271.json");
}
