// Controlled encounter probe, not a full expedition playtest.
// Run with python3 tools/act2_combat_probe.py after building test_runner.
#include "game/game.h"
#include "game/aldric.h"
#include "screens/shop.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GameState g;
static const int dx[] = {0, 1, 0, -1};
static const int dy[] = {-1, 0, 1, 0};

static int enemy_at(int x, int y) {
    for (int i = 0; i < g.enemy_count; i++) {
        if (g.enemies[i].active && g.enemies[i].x == x && g.enemies[i].y == y) {
            return i;
        }
    }
    return -1;
}

static void buy(Item item, int slot) {
    int index = g.inventory_count;
    assert(shop_purchase(&g, &item));
    if (slot >= 0) {
        assert(game_equip_item(&g, index, slot));
    } else if (item.type == ITEM_SCROLL) {
        action_resolve_player(&g, (Action){ACTION_USE_ITEM, index, 0});
        assert(g.player.known_spell_count == 1);
        g.player.equipped_spell = 0;
    }
}

static int find_potion(ItemType type) {
    for (int i = 0; i < g.inventory_count; i++) {
        if (g.inventory[i].type == type) {
            return i;
        }
    }
    return -1;
}

static int alive(void) {
    int count = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        count += g.enemies[i].active;
    }
    return count;
}

static int shot_target(int range, int *sx, int *sy) {
    for (int side = 0; side < 4; side++) {
        for (int step = 1; step <= range; step++) {
            int x = g.player.x + dx[side] * step;
            int y = g.player.y + dy[side] * step;
            if (!map_is_walkable(&g.map, x, y) || aldric_at(&g, x, y)) {
                break;
            }
            if (enemy_at(x, y) >= 0) {
                if (step > 1 || g.player.player_class == CLASS_MAGE) {
                    *sx = dx[side];
                    *sy = dy[side];
                    return 1;
                }
                break;
            }
        }
    }
    return 0;
}

static Action close_target(void) {
    static int previous[MAP_H][MAP_W];
    static int queue[MAP_H * MAP_W];
    memset(previous, -1, sizeof(previous));
    int start = g.player.y * MAP_W + g.player.x;
    queue[0] = start;
    previous[g.player.y][g.player.x] = start;
    int head = 0;
    int tail = 1;
    while (head < tail) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + dx[side];
            int y = cell / MAP_W + dy[side];
            if (!map_is_walkable(&g.map, x, y) || previous[y][x] >= 0 || aldric_at(&g, x, y)) {
                continue;
            }
            previous[y][x] = cell;
            int next = y * MAP_W + x;
            if (enemy_at(x, y) >= 0) {
                while (previous[next / MAP_W][next % MAP_W] != start) {
                    next = previous[next / MAP_W][next % MAP_W];
                }
                return (Action){ACTION_MOVE, next % MAP_W, next / MAP_W};
            }
            queue[tail++] = next;
        }
    }
    return (Action){ACTION_WAIT, 0, 0};
}

static void setup(PlayerClass cls, int prepared, int companion, Location area, int seed) {
    memset(&g, 0, sizeof(g));
    g.player.player_class = cls;
    game_init(&g);
    if (prepared) {
        player_gain_xp(&g, 1000);
        assert(g.player.level == 5);
        g.defeated_bosses = 1 << LOCATION_DUNGEON;
        g.gold = 400;
        Item weapon = cls == CLASS_WARRIOR ? item_make_short_sword() : cls == CLASS_MAGE ? item_make_staff() : item_make_bow();
        Item armor = cls == CLASS_WARRIOR ? item_make_chain_mail() : cls == CLASS_MAGE ? item_make_apprentice_robes() : item_make_leather_armor();
        if (cls != CLASS_MAGE) {
            buy(weapon, EQUIPMENT_STRONG_HAND);
        }
        buy(armor, EQUIPMENT_ARMOR);
        if (cls == CLASS_MAGE) {
            buy(item_make_scroll_magic_arrow(), -1);
            for (int i = 0; i < 2; i++) {
                buy(item_make_mana_potion(), -1);
            }
        }
        for (int i = 0; i < 3; i++) {
            buy(item_make_health_potion(), -1);
        }
        Item ring = item_make_crypt_ring();
        g.inventory[g.inventory_count++] = ring;
        assert(game_equip_item(&g, g.inventory_count - 1, EQUIPMENT_ACCESSORY_1));
    }
    g.location = area;
    g.level = 2;
    g.enemy_count = 0;
    g.map.room_count = 1;
    g.map.rooms[0] = (Room){4, 4, 34, 34};
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.map.tiles[y][x] = x >= 4 && x < 38 && y >= 4 && y < 38 ? TILE_FLOOR : TILE_WALL;
        }
    }
    g.player.x = 20;
    g.player.y = 20;
    // Fully rested departure, not a heal between waves.
    g.player.hp = g.player.max_hp;
    g.player.mp = g.player.max_mp;
    if (companion) {
        g.aldric_party_state = ALDRIC_FOLLOWING;
        g.aldric_x = 21;
        g.aldric_y = 20;
        g.aldric_hp = ALDRIC_MAX_HP;
    }
    srand(seed);
}

static void wave(int area, int deep) {
    const EnemyType forest[] = {ENEMY_DARK_ELF_GUARD, ENEMY_DARK_ELF_ARCHER, ENEMY_DARK_ELF_RANGER};
    const EnemyType mountains[] = {ENEMY_HOBGOBLIN_GUARD, ENEMY_GOBLIN_ARCHER, ENEMY_GOBLIN_SHAMAN};
    g.enemy_count = 3;
    for (int i = 0; i < 3; i++) {
        EnemyType type = deep ? (area ? mountains[i] : forest[i]) : area ?
            (i == 0 ? ENEMY_GOBLIN_WARCHIEF : i == 1 ? ENEMY_GOBLIN_ARCHER : ENEMY_GIANT_SPIDER) : ENEMY_GIANT_SPIDER;
        game_spawn_enemy(&g, &g.enemies[i], type, 14 - (i > 0), 20 + (i == 1 ? 1 : i == 2 ? -1 : 0));
    }
}

int main(void) {
    printf("class,prepared,companion,region,encounter,wins,runs,health_used,mana_used,remaining_hp,turns\n");
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        for (int prepared = 0; prepared <= 1; prepared++) {
            for (int companion = 0; companion <= prepared; companion++) {
                for (int area = 0; area < 2; area++) {
                    for (int deep = 0; deep <= 1; deep++) {
                        int wins = 0;
                        int health = 0;
                        int mana = 0;
                        int remaining = 0;
                        int turns = 0;
                        for (int seed = 9200; seed < 9220; seed++) {
                            setup(cls, prepared, companion, area ? LOCATION_MOUNTAINS : LOCATION_FOREST, seed);
                            wave(area, deep);
                            int back_x = -1;
                            int back_y = -1;
                            int waits = 0;
                            for (int turn = 0; turn < 250 && g.player.hp > 0 && alive(); turn++) {
                                int potion = find_potion(ITEM_POTION_HEALTH);
                                if (g.player.hp * 100 < g.player.max_hp * 50 && potion >= 0) {
                                    assert(action_use_inventory_item(&g, potion, NULL));
                                    health++;
                                    turns++;
                                    continue;
                                }
                                int sx = 0;
                                int sy = 0;
                                int magic = cls == CLASS_MAGE && g.player.known_spell_count;
                                const Item *weapon = game_equipped_item(&g, EQUIPMENT_STRONG_HAND);
                                int range = magic ? 6 : weapon && weapon->is_ranged ? weapon->range : 0;
                                Action action = close_target();
                                if (range && shot_target(range, &sx, &sy)) {
                                    if (magic && g.player.mp < g.player.known_spells[0].mp_cost) {
                                        potion = find_potion(ITEM_POTION_MANA);
                                        if (potion >= 0) {
                                            assert(action_use_inventory_item(&g, potion, NULL));
                                            mana++;
                                            turns++;
                                            continue;
                                        }
                                    } else {
                                        g.player.last_dx = sx;
                                        g.player.last_dy = sy;
                                        action = (Action){magic ? ACTION_CAST_SPELL : ACTION_RANGED_ATTACK, 0, 0};
                                    }
                                }
                                if (action.type == ACTION_MOVE && enemy_at(action.target_x, action.target_y) < 0) {
                                    if (action.target_x == back_x && action.target_y == back_y && waits < 3) {
                                        action.type = ACTION_WAIT;
                                        waits++;
                                    } else {
                                        back_x = g.player.x;
                                        back_y = g.player.y;
                                        waits = 0;
                                    }
                                }
                                action_resolve_player(&g, action);
                                action_resolve_enemies(&g);
                                turns++;
                            }
                            wins += g.player.hp > 0 && !alive();
                            remaining += g.player.hp > 0 ? g.player.hp : 0;
                        }
                        printf("%d,%d,%d,%s,%s,%d,20,%.2f,%.2f,%.1f,%.1f\n", cls, prepared, companion, area ? "mountain" : "forest", deep ? "squad" : "early", wins, health / 20.0, mana / 20.0, remaining / 20.0, turns / 20.0);
                    }
                }
            }
        }
    }
    return 0;
}
