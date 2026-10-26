#ifndef GAME_HEADER_H
#define GAME_HEADER_H

#include "map.h"
#include "enemy.h"
#include "item.h"
#include "actions.h"
#include <stdio.h>
#include "spell.h"
#include "controls.h"
#include <SDL2/SDL.h>

#define MAX_MESSAGES 3
#define MAX_ARROWS 100
#define MAX_MESSAGE_LEN 128
#define MAX_DIALOGUE_LEN 192
#define MAX_SPEAKER_LEN 24
#define ROOK_QUEST_REWARD 40
#define EMBERFORGE_REWARD_GOLD 100
#define EMBERFORGE_REWARD_SCORE 800
#define EMBERFORGE_MECHANISM_RECOVERED 1
#define EMBERFORGE_RESTORED 2
#define FROSTFELL_REWARD_GOLD 100
#define FROSTFELL_REWARD_SCORE 800
#define FROSTFELL_JOURNAL_RECOVERED 1
#define FROSTFELL_SURVIVOR_RESCUED 2
#define GLASSDEEP_REWARD_GOLD 120
#define GLASSDEEP_REWARD_SCORE 1000
#define WATCHFIRE_REWARD_GOLD 180
#define WATCHFIRE_REWARD_SCORE 1400
#define HUNT_REWARD_GOLD 200
#define HUNT_REWARD_SCORE 1500

#define DAIN_FRAGMENT_ARCHER 1
#define DAIN_FRAGMENT_BOMBER 2
#define DAIN_FRAGMENT_SHAMAN 4

#define ALDER_WARDEN_STAGE_2 1
#define ALDER_WARDEN_STAGE_3 2
#define ALDER_WARDEN_STAGE_4 4

#define MARA_BEACON_STAGE_2 1
#define MARA_BEACON_STAGE_3 2
#define MARA_BEACON_STAGE_4 4

#define MAX_TRAIL 16
#define RETIRED_LOOT_STAGES 6

// Picks the message bar colour for a message.
typedef enum {
    MESSAGE_NORMAL = 0,
    MESSAGE_DAMAGE_TAKEN,
    MESSAGE_CRITICAL,
    MESSAGE_DEFENDED,
    MESSAGE_POISON,
    MESSAGE_KIND_COUNT
} MessageKind;

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
    TRAIL_EFFECT_FIREBALL,
    TRAIL_EFFECT_DEMONIC_SWORD,
    TRAIL_EFFECT_ICE_SLIDE
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
    int frozen_turns;
    int freeze_recovery;
    PlayerClass player_class;
    int arrows;
} Player;

typedef struct {
    Map   map;
    Enemy enemies[LEVEL_CACHE_ENEMY_CAPACITY];
    int   enemy_count;
    int   valid;
    int   level_cleared;
} LevelCache;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int enemy_count;
    int valid;
    int level_cleared;
} CrownroadCache;

typedef enum {
    LOCATION_TOWN,
    LOCATION_DUNGEON,
    LOCATION_FOREST,
    LOCATION_MOUNTAINS,
    LOCATION_COAST,
    LOCATION_TAVERN,
    LOCATION_ISLAND,
    LOCATION_TEMPLE,
    LOCATION_LABYRINTH,
    LOCATION_TOWN2,
    LOCATION_INN,
    LOCATION_FOREST_ROAD,
    LOCATION_SWAMP,
    LOCATION_HIGH_PASS,
    LOCATION_DRAGONSPINE,
    LOCATION_CROWNROAD,
    LOCATION_TOWN3,
    LOCATION_FROSTFELL,
    LOCATION_TOWN4,
    LOCATION_SWAMP_ROAD,
    LOCATION_KING_ROAD_WEST,
    LOCATION_CASTLE,
    LOCATION_DESERT,
    LOCATION_GUILD,
    LOCATION_ASHEN = 25,
    LOCATION_GLASSDEEP,
    LOCATION_CATACOMBS,
    LOCATION_CASTLE_INTERIOR,
    LOCATION_WORKSHOP,
    LOCATION_TOWN_HALL,
    LOCATION_JAIL,
    LOCATION_ESCAPE_TUNNEL,
    LOCATION_BAKERY,
    LOCATION_BUTCHER,
    LOCATION_MONASTERY,
    LOCATION_SPICE_SHOP,
    LOCATION_APPROACH,
    LOCATION_HEALER,
    LOCATION_WITCH,
    LOCATION_DAWNMERE,
    LOCATION_TOWN_ROAD,
    LOCATION_ELDERWOOD,
    LOCATION_JAIL_ARMORY,
    // Preserve the retired coastal location IDs 44-45.
    LOCATION_MANGROVES = 46,
    LOCATION_MANGROVE_PATH,
    LOCATION_DAWNMERE_MERCHANT,
    LOCATION_STORMWATCH,
    LOCATION_IRONFANG
} Location;

// Elderwood uses only its own bounds; compact caches avoid allocating the
// unused 200x100 map rectangle for each of its five stages.
typedef struct {
    unsigned short tiles[ELDERWOOD_H][ELDERWOOD_W];
    unsigned char explored[(ELDERWOOD_W * ELDERWOOD_H + 7) / 8];
    Enemy enemies[AREA_ENEMY_LIMIT];
    int enemy_count;
    int valid;
    int level_cleared;
} ElderwoodCache;

typedef struct {
    int initialized;
    int stage;
    int boat_x;
    int boat_y;
    int boat_dx;
    int boat_dy;
    int aboard;
    unsigned int searched;
    unsigned char explored[(MANGROVE_W * MANGROVE_H + 7) / 8];
    FloorItem loot[MAX_FLOOR_ITEMS];
    int loot_count;
    Enemy enemies[AREA_ENEMY_LIMIT];
    int enemy_count;
} MangroveState;

#define STORM_W 64
#define STORM_H 48
#define STORM_DEPTH 5
#define STORM_MASK_BYTES ((STORM_W * STORM_H + 7) / 8)

typedef struct {
    unsigned short tiles[STORM_H][STORM_W];
    unsigned char explored[STORM_MASK_BYTES];
    Enemy enemies[AREA_ENEMY_LIMIT];
    FloorItem loot[MAX_FLOOR_ITEMS];
    int enemy_count;
    int loot_count;
    int valid;
    int phase[2];
    int direction[2];
    int next_direction[2];
    unsigned char warned[2][STORM_MASK_BYTES];
    int wind_elapsed_ms;
    int braced;
} StormwatchStage;

#define IRON_W 64
#define IRON_H 48
#define IRON_DEPTH 5

typedef struct {
    unsigned short tiles[IRON_H][IRON_W];
    unsigned char explored[(IRON_W * IRON_H + 7) / 8];
    Enemy enemies[AREA_ENEMY_LIMIT];
    FloorItem loot[MAX_FLOOR_ITEMS];
    int valid;
    int enemy_count;
    int loot_count;
    int cart_x;
    int cart_y;
    int aboard;
    int route;
    int ore_loads;
    int ore_fuse;
} IronfangStage;

typedef struct {
    Player     player;
    IronfangStage ironfang[IRON_DEPTH];
    int ironfang_defeated;
    StormwatchStage stormwatch[STORM_DEPTH];
    int stormwatch_defeated;
    MangroveState mangroves;
    MangroveState mangrove_stages[MANGROVE_DEPTH];
    int mangrove_defeated;
    int debug_passages_unlocked;
    ElderwoodCache elderwood_cache[ELDERWOOD_DEPTH];
    int max_elderwood_level_reached;
    int elderwood_defeated;
    Item retired_coast_items[RETIRED_LOOT_STAGES * MAX_FLOOR_ITEMS];
    int retired_coast_item_count; // Migration overflow, reclaimed as pack slots become free.
    Location last_town_visited;
    int return_portal_town; // -1 retains the anchor used by a legacy portal.
    Location elderwood_entry_town;
    Location catacombs_entry_town;
    Location northern_portal_town;
    Map        map;
    int        level;
    Enemy      enemies[MAX_ENEMIES];
    int        enemy_count;
    LevelCache level_cache[MAX_REGION_DEPTH];
    LevelCache forest_cache[MAX_REGION_DEPTH];
    LevelCache mountain_cache[MAX_REGION_DEPTH];
    LevelCache coast_cache[MAX_REGION_DEPTH];
    // Retain the old seven-map capacity for save migration.
    LevelCache swamp_cache[SWAMP_CACHE_DEPTH];
    LevelCache dragonspine_cache[DRAGONSPINE_DEPTH];
    LevelCache frostfell_cache[FROSTFELL_DEPTH];
    LevelCache desert_cache[DESERT_DEPTH];
    LevelCache ashen_cache[ASHEN_DEPTH];
    LevelCache glassdeep_cache[GLASSDEEP_DEPTH];
    LevelCache catacombs_cache[CATACOMBS_DEPTH];
    LevelCache castle_cache[CASTLE_DEPTH];
    FloorItem castle_loot[CASTLE_DEPTH][MAX_FLOOR_ITEMS];
    int castle_loot_count[CASTLE_DEPTH];
    int castle_fire_phase[CASTLE_DEPTH];
    int castle_minibosses;
    int castellan_reward_state; // 0: not earned, 1: reserved in Iron Keep, 2: claimed.
    int arcanist_reward_state; // 0: not earned, 1: reserved in Crown Chapel, 2: claimed.
    int castle_prompt;
    int game_won;
    int crown_of_unbound; // Permanent victory trophy; no inventory slot required.
    int jail_quest_state; // 0: not arrested, 1: jailed, 2: escorting, 3: rewarded.
    int prisoner_x;
    int prisoner_y;
    Item jail_gear[MAX_INVENTORY];
    int jail_gear_count;
    int jail_gear_claimed;
    int jail_gear_state; // 0: none taken, 1: recover gear, 3: recovered.
    int jail_main_hand;
    int jail_off_hand;
    int jail_armor;
    int jail_supplies_taken;
    int jail_armory_initialized;
    Enemy jail_guards[3];
    FloorItem jail_armory_loot[MAX_FLOOR_ITEMS];
    int jail_armory_loot_count;
    CrownroadCache crownroad_cache;
    CrownroadCache kingroad_west_cache;
    LevelCache temple_cache[TEMPLE_DEPTH];
    LevelCache labyrinth_cache[LABYRINTH_DEPTH];
    char       messages[MAX_MESSAGES][MAX_MESSAGE_LEN];
    int        message_count;
    MessageKind message_kinds[MAX_MESSAGES];
    int        level_cleared;
    Location   location;
    Location road_town;
    int approach_route; // Index into the append-only town approach routes, or -1.
    int max_level_reached;
    Location dungeon_entry_town;
    Location dungeon_portal_town;
    int max_forest_level_reached;
    Location forest_entry_town;
    Location forest_portal_town;
    int max_mountain_level_reached;
    Location mountain_entry_town;
    Location mountain_portal_town;
    int max_coast_level_reached;
    int max_swamp_level_reached;
    Location swamp_entry_town;
    Location swamp_portal_town;
    int max_dragonspine_level_reached;
    int max_frostfell_level_reached;
    int max_desert_level_reached;
    int max_ashen_level_reached;
    int max_glassdeep_level_reached;
    int max_catacombs_level_reached;
    int catacombs_mantle_unclaimed;
    int max_temple_level_reached;
    Item      inventory[MAX_INVENTORY];
    int       inventory_count;
    int       equipped_main_hand;
    int       equipped_off_hand;
    int       equipped_armor;
    int       gold;
    Location labyrinth_entry_town;
    int       rook_quest_state;
    int       innkeeper_quest_state;
    int       rook_labyrinth_switches;
    int       rook_quest_completions;
    FloorItem floor_items[MAX_FLOOR_ITEMS];
    int       floor_item_count;
    TrailTile trail[MAX_TRAIL];
    int       trail_count;
    int       trail_frames;
    TrailEffect trail_effect;
    Uint32    trail_started_at;
    int score;
    int dungeon_key_found;
    int dungeon_crypt_keys;
    int grave_marshal_reward_state; // 0: not earned, 1: unclaimed, 2: claimed.
    int minotaur_reward_state; // 0: none, 1: unclaimed Aegis, 2: claimed, 3: unclaimed legacy Magic Shield.
    int swamp_demon_reward_state; // 0: not earned, 1: unclaimed, 2: claimed.
    int drowned_queen_reward_state; // 0: not earned, 1: unclaimed, 2: claimed.
    int goblin_king_reward_state; // 0: not earned, 1: unclaimed, 2: claimed.
    int necromancer_reward_state; // 0: not earned, 1: unclaimed, 2: claimed.
    int lich_reward_state; // 0: not earned, 1: unclaimed, 2: claimed.
    int portal_active;
    int portal_level;
    Location portal_location;
    int portal_x, portal_y;
    TileType portal_origin_tile;
    int defeated_bosses;
    int kraken_bow_unclaimed;
    int sandstorm_staff_unclaimed;
    int prism_sovereign_reward_pending; // Bits: 1 Prismweave Robes, 2 Strength Potion.
    int stonewake_warhammer_state; // 0 unearned, 1 reserved, 2 claimed.
    int sun_guardian_reward_state; // 0: not earned, 1: unclaimed, 2: claimed.
    int red_dragon_reward_state; // 0: not earned, 1: unclaimed mantle, 2: claimed.
    int cinder_lord_reward_pending; // Bits: 1 Embercleaver, 2 Strength Potion.
    int desert_pharaoh_reward_state; // 0: not earned, 1: unclaimed, 2: claimed.
    int elowen_quest_state;
    int elowen_seals_restored;
    int dain_quest_state;
    int dain_map_fragments;
    int alder_quest_state;
    int alder_wardens_rescued;
    int mara_quest_state;
    int mara_beacons_lit;
    int cain_scroll_given;
    int island_travel_unlocked;
    int dragon_treasure_quest_state;
    int sunscar_lamp_quest_state;
    int emberforge_quest_state;
    int emberforge_progress;
    int emberforge_encounters;
    int frostfell_quest_state;
    int frostfell_quest_progress;
    int frostfell_quest_encounters;
    int glassdeep_quest_state;
    int glassdeep_quest_progress;
    int glassdeep_quest_encounters;
    int catacombs_quest_state;
    int catacombs_quest_progress;
    int catacombs_quest_encounters;
    int watchfire_quest_state;
    int watchfire_quest_progress;
    int watchfire_quest_encounters;
    int hunt_quest_state;
    int hunt_quest_progress;
    // One placement bit per hunt member: four Wolves/Wraith, three Golem/Stalkers, three Djinn/Mummies.
    int hunt_quest_encounters;
    int temple_alignment;
    int temple_sentinels_awakened;
    int temple_treasure_state;
    int dialogue_active;
    char dialogue_speaker[MAX_SPEAKER_LEN];
    char dialogue_text[MAX_DIALOGUE_LEN];
    int dialogue_x;
    int dialogue_y;
    // Scancode for each ControlAction; belongs to this character's session.
    int key_bindings[CONTROL_COUNT];
} GameState;

void game_init(GameState *g);
int game_quest_offer_active(const GameState *g);
int game_handle_quest_offer_key(GameState *g, int key, int repeat);
void game_request_shortcut(GameState *g);
void game_enter_shortcut_boss(GameState *g);
int game_shortcut_prompt_active(const GameState *g);
int game_handle_shortcut_prompt_key(GameState *g, int key, int repeat);
void game_migrate_boss_shortcuts(GameState *g);
int game_has_regional_interaction(const GameState *g);
void game_move_player(GameState *g, int dx, int dy);
void game_descend(GameState *g);
void game_ascend(GameState *g);
void enemies_spawn(GameState *g);
Enemy *game_spawn_marshal_guard(GameState *g, int x, int y, int boss_index);
void game_add_lich_minions(GameState *g);
Enemy *game_raise_grove_servant(GameState *g, int x, int y, int boss_index);
Enemy *game_spawn_lich_shade(GameState *g, int x, int y, int boss_index);
void game_refresh_quest_encounters(GameState *g);
void game_repair_forest_enemy_positions(Map *m, Enemy *actors, int count, int px, int py);
void game_enter_dungeon(GameState *g);
void game_repair_dungeon_maps(GameState *g);
void game_enter_forest(GameState *g);
void game_leave_forest(GameState *g, Location town, int shortcut);
void game_reveal_forest_shortcut(GameState *g);
void game_enter_mountains(GameState *g);
void game_leave_mountains(GameState *g, Location town, int shortcut);
void game_reveal_mountain_shortcut(GameState *g);
void game_enter_town4(GameState *g);
void game_enter_coast(GameState *g);
void game_enter_swamp(GameState *g);
void game_leave_swamp(GameState *g, Location town, int shortcut);
void game_reveal_swamp_shortcut(GameState *g);
void game_enter_frostfell(GameState *g);
void game_enter_desert(GameState *g);
void game_enter_ashen(GameState *g);
void game_enter_glassdeep(GameState *g);
void game_enter_catacombs(GameState *g);
void game_leave_catacombs(GameState *g);
void game_enter_high_pass(GameState *g, int from_town);
void game_leave_high_pass(GameState *g, Location destination);
void game_enter_dragonspine(GameState *g);
void game_enter_tavern(GameState *g);
void game_leave_tavern(GameState *g);
void game_enter_town2(GameState *g);
void game_enter_town_gate(GameState *g, Location town, int x, int y);
void game_enter_forest_road(GameState *g);
void game_leave_forest_road(GameState *g, Location destination);
int game_is_king_road(const GameState *g);
void game_enter_king_road(GameState *g, Location road, int from_castle);
void game_migrate_crownroad_density(GameState *g);
void game_enter_town3(GameState *g);
void game_enter_swamp_road(GameState *g);
void game_leave_swamp_road(GameState *g, Location destination);
void game_leave_crownroad(GameState *g, Location destination);
void game_enter_inn(GameState *g);
void game_leave_inn(GameState *g);
void game_enter_guild(GameState *g);
void game_enter_workshop(GameState *g);
void game_leave_workshop(GameState *g);
void game_enter_town_hall(GameState *g);
void game_leave_town_hall(GameState *g);
void game_talk_to_steward(GameState *g);
void game_talk_to_veyra(GameState *g);
void game_talk_to_selene(GameState *g);
int game_hunt_enemy_index(const Enemy *enemy);
int game_is_hunt_leader(const Enemy *enemy);
int game_has_watchfire_interaction(const GameState *g);
int game_interact_watchfire(GameState *g);
void game_talk_to_brenna(GameState *g);
void game_talk_to_orin(GameState *g);
int game_has_glassdeep_interaction(const GameState *g);
int game_interact_glassdeep(GameState *g);
int game_glassdeep_prompt_active(const GameState *g);
int game_handle_glassdeep_prompt_key(GameState *g, int key, int repeat);
void game_talk_to_frost_survivor(GameState *g, int x, int y);
int game_has_frostfell_interaction(const GameState *g);
int game_interact_frostfell(GameState *g);
int game_has_emberforge_interaction(const GameState *g);
int game_interact_emberforge(GameState *g);
int game_workshop_near_smith(const GameState *g);
int game_workshop_near_fletcher(const GameState *g);
int game_sharpen_weapon(GameState *g, int index);
void game_leave_guild(GameState *g);
void game_talk_to_guild_seeker(GameState *g);
void game_collect_desert_lamp(GameState *g);
void game_talk_to_innkeeper(GameState *g);
void game_rescue_innkeeper_daughter(GameState *g, int x, int y);
void game_enter_labyrinth(GameState *g);
void game_leave_labyrinth(GameState *g);
int game_labyrinth_is_open(const GameState *g);
void game_change_labyrinth_floor(GameState *g, int descending, int false_stair);
int game_has_labyrinth_interaction(const GameState *g);
int game_interact_labyrinth(GameState *g);
void game_enter_island(GameState *g);
void game_leave_island(GameState *g);
void game_enter_temple(GameState *g);
void game_leave_temple(GameState *g);
int game_has_temple_interaction(const GameState *g);
int game_interact_temple(GameState *g);
int game_temple_dormant_sentinels(const GameState *g);
int game_temple_remaining_enemies(const GameState *g);
void game_record_temple_enemy_defeated(GameState *g, EnemyType type);
int game_has_island_interaction(const GameState *g);
int game_interact_island(GameState *g);

void action_resolve_player(GameState *g, Action a);
int action_use_inventory_item(GameState *g, int index, EnemyProjectiles *shots);
void action_resolve_enemies(GameState *g);
void action_resolve_enemies_with_projectiles(GameState *g, EnemyProjectiles *shots);
int kraken_tile_threatened(const GameState *g, const Enemy *e, int x, int y);

void player_gain_xp(GameState *g, int xp);
void push_message(GameState *g, const char *msg);
void push_message_kind(GameState *g, const char *msg, MessageKind kind);
void game_mark_level_cleared(GameState *g);
void game_update_level_progress(GameState *g);

void game_return_to_town(GameState *g);
void game_return_to_approach(GameState *g);
void game_open_town_portal(GameState *g);
void game_use_town_portal(GameState *g);
void game_hide_portal_destination(GameState *g);
void game_talk_to_elowen(GameState *g);
void game_talk_to_dain(GameState *g);
void game_record_dain_kill(GameState *g, EnemyType type);
void game_talk_to_alder(GameState *g);
void game_rescue_forest_warden(GameState *g, int x, int y);
void game_talk_to_mara(GameState *g);
void game_talk_to_oswin(GameState *g);
void game_talk_to_rook(GameState *g);
void game_talk_to_cain(GameState *g);
void game_talk_to_bram(GameState *g);
void game_talk_to_aldric(GameState *g);
void game_talk_to_rowan(GameState *g);
void game_talk_to_dragon_seeker(GameState *g);
void game_talk_to_royal_guard(GameState *g, int x, int y);
void game_collect_dragon_treasure(GameState *g);
void game_talk_to_nahla(GameState *g);
int game_harbor_unlocked(const GameState *g);
void game_refresh_debug_passages(GameState *g);
int game_has_treasure_map(const GameState *g);
int game_can_sail_to_island(const GameState *g);
void game_light_coast_beacon(GameState *g, int x, int y);
void game_repair_equipment_indices(GameState *g);
int game_equip_main_hand(GameState *g, int index);
int game_equip_off_hand(GameState *g, int index);
int game_equip_shield(GameState *g, int index);
int game_off_hand_attack_bonus(const Item *weapon);
void game_unequip_main_hand(GameState *g);
void game_unequip_off_hand(GameState *g);
void game_remove_inventory_item(GameState *g, int index);
void game_recover_retired_items(GameState *g);
void game_apply_armor_bonuses(GameState *g, const Item *armor);
void game_remove_armor_bonuses(GameState *g, const Item *armor);

#endif
