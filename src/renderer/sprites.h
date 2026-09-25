#ifndef SPRITES_H
#define SPRITES_H
#include "../game/enemy.h"
#include "../game/game.h"
#include "renderer.h"

#define AMBIENT_FRAME_MS 350u

// Map draw functions
void draw_floor(Renderer *r, int tile_x, int tile_y);
void draw_wall(Renderer *r, int tile_x, int tile_y);
void draw_dungeon_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_dungeon_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_dungeon_torch(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
enum {
    DUNGEON_EDGE_NORTH = 1,
    DUNGEON_EDGE_EAST = 2,
    DUNGEON_EDGE_SOUTH = 4,
    DUNGEON_EDGE_WEST = 8
};
void draw_dungeon_wall_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, unsigned int edges);
void draw_forest_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_forest_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
enum {
    FOREST_EDGE_NORTH = 1,
    FOREST_EDGE_EAST = 2,
    FOREST_EDGE_SOUTH = 4,
    FOREST_EDGE_WEST = 8
};
void draw_forest_tree_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, unsigned int edges);
void draw_forest_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int forward);
void draw_forest_landmark(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_forest_ruin(Renderer *r, int tile_x, int tile_y, int offset_x, int offset_y);
void draw_forest_false_marker(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_mountain_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_mountain_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_mountain_gate(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_mountain_gate_support(Renderer *r, int tile_x, int tile_y, int above);
void draw_mountain_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int forward);
void draw_mountain_rockfall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
enum {
    MOUNTAIN_EDGE_NORTH = 1,
    MOUNTAIN_EDGE_EAST = 2,
    MOUNTAIN_EDGE_SOUTH = 4,
    MOUNTAIN_EDGE_WEST = 8
};
void draw_mountain_chasm(Renderer *r, int tile_x, int tile_y, unsigned int bridges);
void draw_mountain_bridge(Renderer *r, int tile_x, int tile_y, unsigned int paths);
void draw_mountain_cave_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_mountain_fortress_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_coast_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_coast_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_coast_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int forward);
void draw_coast_shallow_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_coast_deep_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
enum {
    COAST_SHORE_NORTH = 1,
    COAST_SHORE_EAST = 2,
    COAST_SHORE_SOUTH = 4,
    COAST_SHORE_WEST = 8
};
void draw_coast_shore(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, unsigned int edges, TileType water);
void draw_coast_channel(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int amber, int flooded);
void draw_coast_sluice(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_coast_sluice_intake(Renderer *r, int tile_x, int tile_y);
void draw_coast_sluice_conduit(Renderer *r, int tile_x, int tile_y);
void draw_coast_cache(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_coast_tide_control(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_coast_beacon(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int lit);
void draw_player(Renderer *r, int tile_x, int tile_y, PlayerClass player_class, const Item *main_hand, const Item *off_hand, const Item *armor, int facing_dx, int facing_dy);
void draw_stairs_up(Renderer *r, int tile_x, int tile_y);
void draw_stairs_down(Renderer *r, int tile_x, int tile_y);
void draw_return_exit(Renderer *r, int tile_x, int tile_y);
void draw_locked_door(Renderer *r, int tile_x, int tile_y);
void draw_dungeon_door_support(Renderer *r, int tile_x, int tile_y, unsigned int side, int crypt);
void draw_dungeon_key(Renderer *r, int tile_x, int tile_y);
void draw_crypt_door(Renderer *r, int tile_x, int tile_y);
void draw_crypt_key(Renderer *r, int tile_x, int tile_y);
void draw_crypt_cache(Renderer *r, int tile_x, int tile_y);
void draw_dungeon_gate(Renderer *r, int tile_x, int tile_y);
void draw_dungeon_switch(Renderer *r, int tile_x, int tile_y, int active);
void draw_portal(Renderer *r, int tile_x, int tile_y);
void draw_broken_burial_seal(Renderer *r, int tile_x, int tile_y);
void draw_restored_burial_seal(Renderer *r, int tile_x, int tile_y);

// Enemey draw functions
void draw_skeleton(Renderer *r, int tile_x, int tile_y);
void draw_goblin(Renderer *r, int tile_x, int tile_y);
void draw_zombie(Renderer *r, int tile_x, int tile_y);
void draw_crypt_bat(Renderer *r, int tile_x, int tile_y);
void draw_wraith(Renderer *r, int tile_x, int tile_y);
void draw_necromancer(Renderer *r, int tile_x, int tile_y);
void draw_orc(Renderer *r, int tile_x, int tile_y);
void draw_troll(Renderer *r, int tile_x, int tile_y);
void draw_giant(Renderer *r, int tile_x, int tile_y);
void draw_enemy(Renderer *r, int tile_x, int tile_y, EnemyType type);
void draw_fallen_sun_guardian_broken(Renderer *r, int tile_x, int tile_y);

void draw_goblin_king(Renderer *r, int tile_x, int tile_y);
void draw_lich_king(Renderer *r, int tile_x, int tile_y);
void draw_demon_lord(Renderer *r, int tile_x, int tile_y);
void draw_red_dragon(Renderer *r, int tile_x, int tile_y);
void draw_tarrasque(Renderer *r, int tile_x, int tile_y);

// Town Draw functions
typedef enum {
    TOWN_EXIT_DUNGEON,
    TOWN_EXIT_FOREST,
    TOWN_EXIT_MOUNTAINS,
    TOWN_EXIT_COAST
} TownExitStyle;

void draw_town_floor(Renderer *r, int tile_x, int tile_y);
void draw_town_path(Renderer *r, int tile_x, int tile_y);
void draw_town_gate(Renderer *r, int tile_x, int tile_y, TownExitStyle style);
void draw_dungeon_transition(Renderer *r, int covered_width);
void draw_forest_transition(Renderer *r, int covered_width);
void draw_mountain_transition(Renderer *r, int covered_width);
void draw_coast_transition(Renderer *r, int covered_width);
void draw_shop_blacksmith(Renderer *r, int tile_x, int tile_y);
void draw_shop_alchemist(Renderer *r, int tile_x, int tile_y);
void draw_tavern(Renderer *r, int tile_x, int tile_y);
void draw_harbor(Renderer *r, int tile_x, int tile_y);
void draw_tavern_floor(Renderer *r, int tile_x, int tile_y);
void draw_tavern_wall(Renderer *r, int tile_x, int tile_y);
void draw_tavern_exit(Renderer *r, int tile_x, int tile_y);
void draw_tavern_table(Renderer *r, int tile_x, int tile_y);
void draw_elowen(Renderer *r, int tile_x, int tile_y);
void draw_healer_house(Renderer *r, int tile_x, int tile_y);
void draw_witch_hut(Renderer *r, int tile_x, int tile_y);
void draw_witch_portrait(Renderer *r, int x, int y, int scale);
void draw_healer_portrait(Renderer *r, int x, int y, int scale);
void draw_dain(Renderer *r, int tile_x, int tile_y);
void draw_alder(Renderer *r, int tile_x, int tile_y);
void draw_mara(Renderer *r, int tile_x, int tile_y);
void draw_gambler(Renderer *r, int tile_x, int tile_y);
void draw_gambler_portrait(Renderer *r, int x, int y, int scale);
void draw_labyrinth_entrance(Renderer *r, int tile_x, int tile_y, int open);
void draw_labyrinth_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_labyrinth_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_labyrinth_exit(Renderer *r, int tile_x, int tile_y);
void draw_labyrinth_switch(Renderer *r, int tile_x, int tile_y, int active);
void draw_labyrinth_gate(Renderer *r, int tile_x, int tile_y);
void draw_labyrinth_relic(Renderer *r, int tile_x, int tile_y);
void draw_cain(Renderer *r, int tile_x, int tile_y);
void draw_rowan(Renderer *r, int tile_x, int tile_y);
void draw_nahla(Renderer *r, int tile_x, int tile_y);
void draw_forest_warden(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);

// Ruined Isle draw functions
void draw_island_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_island_sand(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_island_grass(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_island_jungle(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_island_path(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_island_temple(Renderer *r, int tile_x, int tile_y);
void draw_island_camp(Renderer *r, int tile_x, int tile_y);
void draw_island_marker(Renderer *r, int tile_x, int tile_y);
void draw_island_statue(Renderer *r, int tile_x, int tile_y);
void draw_island_lagoon(Renderer *r, int tile_x, int tile_y);
void draw_island_dock(Renderer *r, int tile_x, int tile_y);
void draw_island_ship(Renderer *r, int tile_x, int tile_y);
void draw_temple_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_temple_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_temple_entrance(Renderer *r, int tile_x, int tile_y);
void draw_temple_altar(Renderer *r, int tile_x, int tile_y, int moon);
void draw_temple_moon_door(Renderer *r, int tile_x, int tile_y, int open);
void draw_temple_solar_trap(Renderer *r, int tile_x, int tile_y, int active);
void draw_temple_dormant_sentinel(Renderer *r, int tile_x, int tile_y);
void draw_temple_vault_door(Renderer *r, int tile_x, int tile_y);
void draw_temple_treasure(Renderer *r, int tile_x, int tile_y);
void draw_temple_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y);
void draw_temple_rubble(Renderer *r, int tile_x, int tile_y);

void draw_floor_item(Renderer *r, int tile_x, int tile_y);
void draw_floor_gold(Renderer *r, int tile_x, int tile_y);

void draw_trap_warning(Renderer *r, int tile_x, int tile_y);
void draw_trap_spike(Renderer *r, int tile_x, int tile_y);
void draw_trap_fire(Renderer *r, int tile_x, int tile_y);
void draw_trap_poison(Renderer *r, int tile_x, int tile_y);

#endif
