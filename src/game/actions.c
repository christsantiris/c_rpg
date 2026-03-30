#include "actions.h"
#include "game.h"
#include <stdlib.h>

static int abs_int(int n) { return n < 0 ? -n : n; }

static Action make_move_or_attack(int tx, int ty) {
    Action a;
    a.target_x = tx;
    a.target_y = ty;
    a.type     = ACTION_MOVE;
    return a;
}

void action_resolve_player(GameState *g, Action a) {
    if (a.type == ACTION_NONE) return;

    if (a.type == ACTION_DESCEND) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_STAIRS_DOWN)
            game_descend(g);
        return;
    }

    if (a.type == ACTION_ASCEND) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_STAIRS_UP)
            game_ascend(g);
        return;
    }

    if (a.type == ACTION_MOVE) {
        int tx = a.target_x;
        int ty = a.target_y;

        // Check for enemy at target
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (!e->active) continue;
            if (e->x == tx && e->y == ty) {
                // Melee attack
                int dmg = g->player.attack - e->defense;
                if (dmg < 1) dmg = 1;
                e->hp -= dmg;
                if (e->hp <= 0) e->active = 0;
                return;
            }
        }

        // Move if walkable
        if (map_is_walkable(&g->map, tx, ty))
            game_move_player(g, tx - g->player.x, ty - g->player.y);
    }
}

void action_resolve_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->active) continue;

        int dx = g->player.x - e->x;
        int dy = g->player.y - e->y;

        // Adjacent to player — melee attack
        if (abs_int(dx) <= 1 && abs_int(dy) <= 1 &&
            !(dx == 0 && dy == 0)) {
            int dmg = e->attack - g->player.defense;
            if (dmg < 1) dmg = 1;
            g->player.hp -= dmg;
            continue;
        }

        // Move toward player
        int mx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
        int my = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
        int tx = e->x + mx;
        int ty = e->y + my;

        if (map_is_walkable(&g->map, tx, ty) &&
            !(tx == g->player.x && ty == g->player.y)) {
            e->x = tx;
            e->y = ty;
        }
    }
}