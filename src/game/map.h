#ifndef MAP_HEADER_H
#define MAP_HEADER_H

#define MAP_W 200
#define MAP_H 100
#define MAP_EXPLORED_BYTES ((MAP_W * MAP_H + 7) / 8)

#define MIN_ROOMS 6
#define MAX_ROOMS 10
#define MIN_ROOM_W 8 // dungeon room size
#define MAX_ROOM_W 20 // dungeon room size
#define MIN_ROOM_H 8 // dungeon room size
#define MAX_ROOM_H 14 // dungeon room size

#define TOWN_W 44 // town dimensions
#define TOWN_H 25 // town dimensions
#define TOWN_ROAD_GATE_Y 3
#define TOWN_ROAD_EXIT_Y 5
#define FOREST_ROAD_W 80
#define FOREST_ROAD_H 25
#define FOREST_ROAD_Y 12
#define CROWNROAD_LEGACY_W 48
#define CROWNROAD_LENGTH_SCALE 3
#define CROWNROAD_W ((CROWNROAD_LEGACY_W - 1) * CROWNROAD_LENGTH_SCALE + 1)
#define CROWNROAD_H 44
#define CROWNROAD_X 20
#define CROWNROAD_Y 20
#define TOWN3_ROAD_X 28
#define TOWN3_KING_GATE_Y 14
#define TOWN4_KING_GATE_X 4
#define TOWN4_KING_GATE_Y 12
#define CASTLE_ROAD_Y 14
#define SWAMP_ROAD_W TOWN_W
#define SWAMP_ROAD_H 48
#define SWAMP_ROAD_X 20
#define TOWN3_GUARD_X 39
#define TOWN3_GUARD_NORTH_Y (TOWN3_KING_GATE_Y - 1)
#define TOWN3_GUARD_SOUTH_Y (TOWN3_KING_GATE_Y + 1)
#define TOWN_CASTLE_X 13
#define TOWN_CASTLE_Y 3
#define TOWN_CASTLE_W 15
#define TOWN_CASTLE_H 8
#define TOWN_MOAT_X (TOWN_CASTLE_X - 1)
#define TOWN_MOAT_Y (TOWN_CASTLE_Y - 1)
#define TOWN_MOAT_W (TOWN_CASTLE_W + 2)
#define TOWN_MOAT_H (TOWN_CASTLE_H + 2)
#define TOWN_APOTHECARY_X 23
#define TOWN_APOTHECARY_Y 8
#define TOWN_APOTHECARY_W 5
#define TOWN_APOTHECARY_H 4
#define TOWN_APOTHECARY_DOOR_X (TOWN_APOTHECARY_X + 2)
#define TOWN_APOTHECARY_DOOR_Y (TOWN_APOTHECARY_Y + TOWN_APOTHECARY_H - 1)
#define TOWN_GUILD_X 11
#define TOWN_GUILD_Y 7
#define TOWN_GUILD_W 7
#define TOWN_GUILD_H 5
#define TOWN_GUILD_DOOR_X (TOWN_GUILD_X + TOWN_GUILD_W / 2)
#define TOWN_GUILD_DOOR_Y (TOWN_GUILD_Y + TOWN_GUILD_H - 1)
// Legacy Guild coordinates used by save migrations.
#define GUILD_ZARA_X 28
#define GUILD_ZARA_Y 7
#define GUILD_DAIN_X 18
#define GUILD_DAIN_Y 7
#define ZARA_INN_X 18
#define ZARA_INN_Y 18
#define DAIN_TOWN_X 18
#define DAIN_TOWN_Y 11
#define LIORA_TOWN_X 18
#define LIORA_TOWN_Y 13
#define ALDER_INN_X 31
#define ALDER_INN_Y 18
#define GUILD_ORIN_X 10
#define GUILD_ORIN_Y 18
#define GUILD_SELENE_X 28
#define GUILD_SELENE_Y 14
#define HUNT_FROSTFELL_LEVEL 3
#define HUNT_GLASSDEEP_LEVEL 4
#define HUNT_SUNSCAR_LEVEL 3
#define DESERT_LAMP_LEVEL 4
#define TAVERN_X 4
#define TAVERN_Y 2
#define TAVERN_W 32
#define TAVERN_H 21
#define ELOWEN_TAVERN_X 10
#define ELOWEN_TAVERN_Y 7
#define TOWN_CAIN_X 19
#define TOWN_CAIN_Y 11
#define TOWN_BRAM_X 17
#define TOWN_BRAM_Y 11
#define TOWN_HARBOR_W 5
#define TOWN_HARBOR_H 4
#define TOWN_HARBOR_X (TOWN_W - 1 - TOWN_HARBOR_W)
#define TOWN_HARBOR_Y (TOWN_H - 1 - TOWN_HARBOR_H)
#define TOWN_HARBOR_ENTRANCE_X (TOWN_HARBOR_X + 2)
#define TOWN_HARBOR_ENTRANCE_Y (TOWN_HARBOR_Y + 2)
#define TOWN_ROWAN_X (TOWN_HARBOR_X - 2)
#define TOWN_ROWAN_Y TOWN_HARBOR_Y
#define TOWN_BLACKSMITH_X 11
#define TOWN_BLACKSMITH_Y 7
#define TOWN_HEALER_X 4
#define TOWN_HEALER_Y 7
#define TOWN_HEALER_W 5
#define TOWN_HEALER_H 4
#define TOWN_HEALER_DOOR_X (TOWN_HEALER_X + 2)
#define TOWN_HEALER_DOOR_Y (TOWN_HEALER_Y + TOWN_HEALER_H - 1)
#define TOWN_INN_X 11
#define TOWN_INN_Y 6
#define TOWN_INN_W 7
#define TOWN_INN_H 5
#define TOWN_INN_DOOR_X (TOWN_INN_X + 3)
#define TOWN_INN_DOOR_Y (TOWN_INN_Y + TOWN_INN_H - 1)
#define TOWN_ALCHEMIST_X 29
#define TOWN_ALCHEMIST_Y 7
#define TOWN_TAVERN_X 21
#define TOWN_TAVERN_Y 6
#define TOWN_TAVERN_W 7
#define TOWN_TAVERN_H 5
#define TOWN_TAVERN_DOOR_X (TOWN_TAVERN_X + 3)
#define TOWN_TAVERN_DOOR_Y (TOWN_TAVERN_Y + TOWN_TAVERN_H - 1)
#define TOWN_WITCH_X 35
#define TOWN_WITCH_Y 7
#define TOWN_WITCH_W 5
#define TOWN_WITCH_H 4
#define TOWN_WITCH_DOOR_X (TOWN_WITCH_X + 2)
#define TOWN_WITCH_DOOR_Y (TOWN_WITCH_Y + TOWN_WITCH_H - 1)
#define TOWN_LABYRINTH_X TOWN_WITCH_DOOR_X
#define TOWN_LABYRINTH_Y 17

#define LABYRINTH_W 43
#define LABYRINTH_H 25
#define LABYRINTH_DEPTH 5
#define LABYRINTH_SWITCH_COUNT 5
#define LABYRINTH_FALSE_EXIT_X 38
#define LABYRINTH_FALSE_EXIT_Y 4

#define ISLAND_W 40
#define ISLAND_H 25
#define ISLAND_GATE_X 19
#define ISLAND_GATE_Y 7
#define ISLAND_CAMP_X 12
#define ISLAND_CAMP_Y 16
#define ISLAND_MARKER_X 10
#define ISLAND_MARKER_Y 10
#define ISLAND_STATUE_X 29
#define ISLAND_STATUE_Y 10
#define ISLAND_LAGOON_X 29
#define ISLAND_LAGOON_Y 15
#define ISLAND_SHIP_X 25
#define ISLAND_SHIP_Y 22
#define ISLAND_CAPTAIN_X 18
#define ISLAND_CAPTAIN_Y 21
#define ISLAND_NAHLA_X 15
#define ISLAND_NAHLA_Y 14
#define ISLAND_SPAWN_X 20
#define ISLAND_SPAWN_Y 21

#define TEMPLE_W 64
#define TEMPLE_H 36
#define TEMPLE_ENTRANCE_X 32
#define TEMPLE_ENTRANCE_Y 34
#define TEMPLE_TREASURE_X 32
#define TEMPLE_TREASURE_Y 3
#define TEMPLE_DEPTH 5

#define DUNGEON_DEPTH 5
#define FOREST_DEPTH 7
#define FOREST_BOSS_LEVEL 4
#define STILLBURY_FOREST_ROAD_X (TOWN_W - 3)
#define MOUNTAIN_DEPTH 7
#define MOUNTAIN_BOSS_LEVEL 4
#define COAST_DEPTH 5
#define MAX_REGION_DEPTH 8
#define SWAMP_DEPTH 7
#define SWAMP_BOSS_LEVEL 4
#define SWAMP_RESCUE_LEVEL 3
#define ROSEMOOR_SWAMP_ROAD_X 28
// Frostfell reuses the swamp's map size, mirrored so it runs east to west.
#define FROSTFELL_DEPTH 5
#define FROSTFELL_JOURNAL_LEVEL 2
#define FROSTFELL_SURVIVOR_LEVEL 4
// Legacy Tavern coordinates used by save migrations.
#define BRENNA_X 10
#define BRENNA_Y 18
#define BRENNA_INN_X 23
#define BRENNA_INN_Y 18
// Legacy Inn coordinates used by save migrations.
#define LIORA_INN_X 18
#define LIORA_INN_Y 18
#define MARA_TAVERN_X 31
#define MARA_TAVERN_Y 18
#define ILYA_TAVERN_X 28
#define ILYA_TAVERN_Y 18
#define DESERT_DEPTH 5
#define MOONVEIL_DEPTH 5
#define ASHEN_DEPTH 5
#define GLASSDEEP_DEPTH 5
#define CATACOMBS_DEPTH 5
#define CASTLE_DEPTH 6
#define CASTLE_W 64
#define CASTLE_H 64
#define CATACOMBS_W 60
#define CATACOMBS_H 80
#define STILLBURY_GLASSDEEP_GATE_X 20
#define RIDGESHIRE_ASHEN_GATE_X 20
#define ROSEMOOR_MOONVEIL_GATE_Y 14
#define SWAMP_MAP_W 72
#define SWAMP_MAP_H 64
#define DESERT_MAP_W SWAMP_MAP_W
#define DESERT_MAP_H SWAMP_MAP_H
#define DRAGONSPINE_DEPTH 5
#define HIGH_PASS_W TOWN_W
#define HIGH_PASS_H 64
#define HIGH_PASS_X 20
#define TOWN4_ROAD_X 40
#define RIDGESHIRE_MOUNTAIN_ROAD_X 28
#define TOWN4_SQUARE_X 20
#define TOWN4_SQUARE_Y 12
#define TOWN4_SQUARE_W (RIDGESHIRE_MOUNTAIN_ROAD_X - TOWN4_SQUARE_X + 1)
#define TOWN4_SQUARE_H 3
#define TOWN4_DRAGON_GATE_Y 12
#define TOWN4_PORTAL_X (TOWN_W - 3)
#define TOWN4_PORTAL_Y 13
#define TOWN4_WORKSHOP_X 7
#define TOWN4_WORKSHOP_Y 8
#define TOWN4_WORKSHOP_W 5
#define TOWN4_WORKSHOP_H 4
#define TOWN4_WORKSHOP_DOOR_X (TOWN4_WORKSHOP_X + TOWN4_WORKSHOP_W / 2)
#define TOWN4_WORKSHOP_DOOR_Y (TOWN4_WORKSHOP_Y + TOWN4_WORKSHOP_H - 1)
#define WORKSHOP_SMITH_X 18
#define WORKSHOP_SMITH_Y 7
#define WORKSHOP_SHARPEN_PRICE 50
#define TOWN4_HALL_X 28
#define TOWN4_HALL_Y 6
#define TOWN4_HALL_W 7
#define TOWN4_HALL_H 5
#define TOWN4_HALL_DOOR_X (TOWN4_HALL_X + TOWN4_HALL_W / 2)
#define TOWN4_HALL_DOOR_Y (TOWN4_HALL_Y + TOWN4_HALL_H - 1)
#define HALL_STEWARD_X 20
#define HALL_STEWARD_Y 8
// Mara's former position is retained for save migrations.
#define HALL_MARA_X 10
#define HALL_MARA_Y 18
#define HALL_OSWIN_X 28
#define HALL_OSWIN_Y 8
#define HALL_VEYRA_X 14
#define HALL_VEYRA_Y 8
#define WATCHFIRE_MOUNTAINS_LEVEL 6
#define WATCHFIRE_ASHEN_LEVEL 3
#define WATCHFIRE_DRAGONSPINE_LEVEL 3
#define EMBERFORGE_MECHANISM_LEVEL 2
#define EMBERFORGE_FURNACE_LEVEL 4

typedef enum {
    TILE_FLOOR = 0,
    TILE_WALL,
    TILE_STAIRS_UP,
    TILE_STAIRS_DOWN,
    TILE_TOWN_FLOOR,
    TILE_TOWN_PATH,
    TILE_TOWN_EXIT,
    TILE_SHOP_BLACKSMITH,
    TILE_SHOP_ALCHEMIST,
    TILE_ITEM,
    TILE_GOLD,
    TILE_TRAP_HIDDEN,
    TILE_TRAP_REVEALED,
    TILE_TRAP_SPIKE,
    TILE_TRAP_FIRE,
    TILE_TRAP_POISON,
    TILE_RETURN_EXIT,
    TILE_LOCKED_DOOR,
    TILE_DUNGEON_KEY,
    TILE_CRYPT_DOOR,
    TILE_CRYPT_KEY,
    TILE_CRYPT_CACHE,
    // Retired with the portcullis shortcut. They keep their ids so older saves
    // load, and loading turns them into floor.
    TILE_DUNGEON_GATE,
    TILE_DUNGEON_SWITCH_OFF,
    TILE_DUNGEON_SWITCH_ON,
    TILE_PORTAL,
    TILE_FOREST_FLOOR,
    TILE_FOREST_WALL,
    TILE_FOREST_HIDDEN_TRAIL,
    TILE_FOREST_ENTRANCE,
    TILE_FOREST_EXIT,
    TILE_FOREST_FALSE_MARKER,
    TILE_MOUNTAIN_FLOOR,
    TILE_MOUNTAIN_WALL,
    TILE_MOUNTAIN_ENTRANCE,
    TILE_MOUNTAIN_EXIT,
    TILE_TAVERN,
    TILE_COAST_FLOOR,
    TILE_COAST_WALL,
    TILE_COAST_ENTRANCE,
    TILE_COAST_EXIT,
    TILE_FOREST_LANDMARK,
    TILE_MOUNTAIN_BRIDGE,
    TILE_MOUNTAIN_CAVE_FLOOR,
    TILE_MOUNTAIN_FORTRESS_FLOOR,
    TILE_COAST_SHALLOW_WATER,
    TILE_COAST_DEEP_WATER,
    TILE_COAST_TIDE_CONTROL,
    TILE_BROKEN_BURIAL_SEAL,
    TILE_RESTORED_BURIAL_SEAL,
    TILE_TAVERN_DOOR,
    TILE_TAVERN_FLOOR,
    TILE_TAVERN_WALL,
    TILE_TAVERN_EXIT,
    TILE_TAVERN_TABLE,
    TILE_NPC_ELOWEN,
    TILE_NPC_DAIN,
    TILE_NPC_ALDER,
    TILE_FOREST_WARDEN,
    TILE_NPC_MARA,
    TILE_COAST_BEACON_UNLIT,
    TILE_COAST_BEACON_LIT,
    TILE_COAST_DRAINED_WATER,
    TILE_BLACKSMITH_DOOR,
    TILE_ALCHEMIST_DOOR,
    TILE_WATCHTOWER,
    // Append terrain IDs: existing saves store these numeric values.
    TILE_MOUNTAIN_WEAK_BRIDGE,
    TILE_MOUNTAIN_CHASM,
    TILE_MOUNTAIN_GATE,
    TILE_MOUNTAIN_ROCKFALL,
    TILE_MOUNTAIN_HIDDEN_CAVE,
    TILE_MOUNTAIN_CACHE,
    TILE_COAST_CHANNEL_WATER,
    TILE_COAST_CHANNEL_DRY,
    TILE_COAST_SLUICE_CONTROL,
    TILE_COAST_CACHE,
    TILE_NPC_CAIN,
    TILE_NPC_ROWAN,
    TILE_HEALER,
    TILE_HEALER_DOOR,
    // Append island IDs: saved maps store these numeric values.
    TILE_ISLAND_WATER,
    TILE_ISLAND_SAND,
    TILE_ISLAND_GRASS,
    TILE_ISLAND_JUNGLE,
    TILE_ISLAND_PATH,
    TILE_ISLAND_DOCK,
    TILE_ISLAND_SHIP,
    TILE_ISLAND_CAMP,
    TILE_ISLAND_MARKER,
    TILE_ISLAND_STATUE,
    TILE_ISLAND_LAGOON,
    TILE_ISLAND_TEMPLE_GATE,
    TILE_NPC_ISLAND_CAPTAIN,
    // Append temple IDs: saved maps store these numeric values.
    TILE_TEMPLE_FLOOR,
    TILE_TEMPLE_WALL,
    TILE_TEMPLE_ENTRANCE,
    TILE_TEMPLE_ALTAR,
    TILE_TEMPLE_MOON_DOOR_CLOSED,
    TILE_TEMPLE_MOON_DOOR_OPEN,
    TILE_TEMPLE_SOLAR_TRAP,
    TILE_TEMPLE_DORMANT_SENTINEL,
    TILE_TEMPLE_VAULT_DOOR,
    TILE_TEMPLE_TREASURE,
    TILE_TEMPLE_WATER,
    TILE_TEMPLE_RUBBLE,
    TILE_NPC_ISLAND_NAHLA,
    TILE_WITCH,
    TILE_WITCH_DOOR,
    TILE_NPC_ROOK,
    TILE_LABYRINTH_ENTRANCE,
    TILE_LABYRINTH_FLOOR,
    TILE_LABYRINTH_WALL,
    TILE_LABYRINTH_EXIT,
    TILE_LABYRINTH_SWITCH_OFF,
    TILE_LABYRINTH_SWITCH_ON,
    TILE_LABYRINTH_GATE,
    TILE_LABYRINTH_RELIC,
    TILE_LABYRINTH_STAIRS,
    TILE_SWAMP_FLOOR,
    TILE_SWAMP_WALL,
    TILE_SWAMP_ENTRANCE,
    TILE_SWAMP_EXIT,
    TILE_NPC_INNKEEPER,
    TILE_SWAMP_DAUGHTER,
    TILE_DRAGON_FLOOR,
    TILE_DRAGON_WALL,
    TILE_DRAGON_ASH,
    TILE_DRAGON_HOARD,
    TILE_DRAGON_ENTRANCE,
    TILE_DRAGON_EXIT,
    TILE_HIGH_PASS_ENTRANCE,
    TILE_HIGH_PASS_EXIT,
    TILE_NPC_DRAGON_SEEKER,
    TILE_DRAGON_TREASURE,
    TILE_NPC_ROYAL_GUARD,
    TILE_FROST_FLOOR,
    TILE_FROST_WALL,
    TILE_FROST_ENTRANCE,
    TILE_FROST_EXIT,
    TILE_FROST_LAKE,
    TILE_FROST_LAKE_HOLE,
    TILE_FROST_ICE,
    TILE_FROST_THIN_ICE,
    TILE_FROST_BROKEN_ICE,
    TILE_DESERT_FLOOR,
    TILE_DESERT_WALL,
    TILE_DESERT_ENTRANCE,
    TILE_DESERT_EXIT,
    TILE_GUILD_DOOR,
    TILE_NPC_GUILD_SEEKER,
    TILE_DESERT_LAMP,
    TILE_FOREST_SHORTCUT,
    TILE_SWAMP_SHORTCUT,
    TILE_MOUNTAIN_SHORTCUT,
    TILE_MOONVEIL_FLOOR,
    TILE_MOONVEIL_WALL,
    TILE_MOONVEIL_ENTRANCE,
    TILE_MOONVEIL_EXIT,
    TILE_MOONVEIL_POOL,
    TILE_MOONVEIL_CIRCLE,
    TILE_ASHEN_FLOOR,
    TILE_ASHEN_WALL,
    TILE_ASHEN_ENTRANCE,
    TILE_ASHEN_EXIT,
    TILE_ASHEN_LAVA,
    TILE_ASHEN_RUIN,
    TILE_GLASSDEEP_FLOOR,
    TILE_GLASSDEEP_WALL,
    TILE_GLASSDEEP_ENTRANCE,
    TILE_GLASSDEEP_EXIT,
    TILE_GLASSDEEP_POOL,
    TILE_GLASSDEEP_RUIN,
    TILE_CATACOMBS_FLOOR,
    TILE_CATACOMBS_WALL,
    TILE_OSSUARY_BRAZIER,
    TILE_OSSUARY_COLD,
    TILE_BURIAL_PLATE,
    TILE_CATACOMBS_SARCOPHAGUS,
    TILE_CASTLE_FLOOR,
    TILE_CASTLE_WALL,
    TILE_CASTLE_CARPET,
    TILE_CASTLE_PILLAR,
    TILE_CASTLE_BANNER,
    TILE_CASTLE_GATE,
    TILE_CASTLE_GATE_OPEN,
    TILE_CASTLE_LEVER,
    TILE_CASTLE_TRAP_HIDDEN,
    TILE_CASTLE_TRAP_OPEN,
    TILE_CASTLE_PASSAGE, // Retired; keep its numeric ID for save migration.
    TILE_CASTLE_SEAL, // Retired; keep its numeric ID for save migration.
    TILE_CASTLE_TABLE,
    TILE_CASTLE_BOOKCASE,
    TILE_CASTLE_THRONE,
    TILE_WORKSHOP_DOOR,
    TILE_NPC_SHARPENER,
    TILE_TOWN_HALL_DOOR,
    TILE_NPC_STEWARD,
    TILE_EMBERFORGE_MECHANISM,
    TILE_EMBERFORGE_COLD,
    TILE_EMBERFORGE_LIT,
    TILE_NPC_BRENNA,
    TILE_FROST_JOURNAL,
    TILE_NPC_FROST_SURVIVOR,
    TILE_NPC_ORIN,
    TILE_GLASSDEEP_RESONATOR,
    TILE_GLASSDEEP_RESONATOR_LIT,
    TILE_NPC_LIORA,
    TILE_MOONVEIL_SEED_POD,
    TILE_MOONVEIL_SPRING,
    TILE_MOONVEIL_PLANTING_CIRCLE,
    TILE_MOONVEIL_MOONFLOWER,
    TILE_MOONVEIL_BLOSSOMS,
    TILE_NPC_OSWIN,
    TILE_MEMORIAL_BRAZIER,
    TILE_MEMORIAL_COLD,
    TILE_BURIAL_LEDGER,
    TILE_JAIL_BUILDING,
    TILE_JAIL_DOOR,
    TILE_NPC_INFORMANT,
    TILE_NPC_PRISONER,
    TILE_JAIL_BARS,
    TILE_JAIL_HATCH,
    TILE_TUNNEL_EXIT,
    TILE_NPC_BRAM,
    TILE_DUNGEON_STAIRS_SEALED,
    TILE_DUNGEON_STAIRS_RETURN,
    TILE_LOCAL_BUILDING,
    TILE_LOCAL_DOOR,
    TILE_NPC_RESIDENT,
    TILE_CASTLE_WARD,
    TILE_CASTLE_WARD_SPENT,
    TILE_CASTLE_MUSTER,
    TILE_CASTLE_FIRE_RUNE,
    TILE_NPC_VEYRA,
    TILE_WATCHFIRE_COLD,
    TILE_WATCHFIRE_LIT,
    TILE_NPC_SELENE
} TileType;

typedef struct {
    int x, y, w, h;
} Room;

typedef struct {
    int x, y;
    int vertical;
    int timer;
    int spent;
} BurialTrap;

typedef struct {
    TileType tiles[MAP_H][MAP_W];
    unsigned char explored[MAP_EXPLORED_BYTES];
    Room     rooms[MAX_ROOMS];
    int      room_count;
    int      stairs_up_x,   stairs_up_y;
    int      stairs_down_x, stairs_down_y;
    BurialTrap burial_traps[MAX_ROOMS];
    int burial_trap_count;
} Map;

void map_generate(Map *m, int level);
void map_repair_dungeon_boss_access(Map *m);
int map_ensure_dungeon_connectivity(Map *m, int level);
void map_remove_dungeon_gates(Map *m);
int  map_is_walkable(const Map *m, int x, int y);
void map_room_center(const Room *r, int *cx, int *cy);
void map_generate_town(Map *m, int *spawn_x, int *spawn_y);
void map_set_town2_road(Map *m, int unlocked);
void map_set_stillbury_forest_road(Map *m, int unlocked);
void map_generate_town4(Map *m, int *spawn_x, int *spawn_y);
void map_set_town4_road(Map *m, int unlocked);
void map_set_ridgeshire_mountain_road(Map *m, int unlocked);
void map_place_town4_square(Map *m);
void map_place_town4_workshop(Map *m);
void map_place_town4_hall(Map *m);
void map_generate_town2(Map *m, int *spawn_x, int *spawn_y);
void map_generate_town3(Map *m, int *spawn_x, int *spawn_y);
void map_generate_crownroad(Map *m);
void map_generate_castle(Map *m, int *spawn_x, int *spawn_y);
void map_generate_swamp_road(Map *m);
void map_set_town3_road(Map *m, int unlocked);
void map_set_rosemoor_swamp_road(Map *m, int unlocked);
void map_generate_forest_road(Map *m);
void map_place_town_labyrinth(Map *m);
void map_place_town2_center(Map *m);
void map_place_town3_guards(Map *m, int avoid_x, int avoid_y);
void map_place_town4_guards(Map *m, int avoid_x, int avoid_y);
void map_place_town3_frost_gate(Map *m);
void map_place_town3_moonveil_gate(Map *m);
void map_place_town4_ashen_gate(Map *m);
void map_place_town2_glassdeep_gate(Map *m);
void map_place_town_harbor(Map *m);
void map_place_town_tavern(Map *m);
void map_place_town_inn(Map *m);
void map_place_town_apothecary(Map *m);
void map_place_town3_guild(Map *m);
void map_generate_tavern(Map *m, int *spawn_x, int *spawn_y);
void map_place_tavern_brenna(Map *m);
void map_generate_inn(Map *m, int *spawn_x, int *spawn_y);
void map_place_tavern_elowen(Map *m);
void map_generate_island(Map *m, int *spawn_x, int *spawn_y);
void map_generate_temple(Map *m, int level, int *spawn_x, int *spawn_y);
void map_generate_labyrinth(Map *m, int level, int switches, int *spawn_x, int *spawn_y);
void map_generate_forest(Map *m, int level);
void map_reveal_forest_exit(Map *m);
void map_reveal_forest_entrance(Map *m);
int map_forest_difficulty(int level);
void map_generate_mountains(Map *m, int level);
int map_mountain_difficulty(int level);
void map_generate_coast(Map *m, int level);
void map_generate_swamp(Map *m, int level);
int map_swamp_difficulty(int level);
void map_generate_high_pass(Map *m);
void map_generate_dragonspine(Map *m, int level);
void map_generate_frostfell(Map *m, int level);
void map_generate_desert(Map *m, int level);
void map_generate_moonveil(Map *m, int level);
void map_generate_ashen(Map *m, int level);
void map_generate_glassdeep(Map *m, int level);
void map_generate_guild(Map *m, int *sx, int *sy);
void map_generate_workshop(Map *m, int *sx, int *sy);
void map_generate_town_hall(Map *m, int *sx, int *sy);
int map_remove_coast_sluice(Map *m);
TileType map_coast_trap_underlay(const Map *m, int x, int y);
int map_is_coast_tidal_tile(TileType tile);
int map_is_coast_object(TileType tile);
TileType map_coast_swapped_tile(TileType tile);
void map_clear_exploration(Map *m);
void map_mark_explored(Map *m, int x, int y);
int map_is_explored(const Map *m, int x, int y);

#endif
