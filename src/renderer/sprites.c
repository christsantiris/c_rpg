#include "sprites.h"

static void fill_rect(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

void draw_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = {18, 18, 35, 255};
    SDL_Color dot  = {30, 30, 55, 255};
    fill_rect(r, x,            y,            TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + TILE_SIZE/2, y + TILE_SIZE/2, 2, 2, dot);
}

void draw_wall(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color face  = {50, 45, 80, 255};
    SDL_Color top   = {70, 65, 105, 255};
    SDL_Color seam  = {35, 30, 60, 255};
    fill_rect(r, x,     y,     TILE_SIZE,     TILE_SIZE,     face);
    fill_rect(r, x,     y,     TILE_SIZE,     3,             top);
    fill_rect(r, x,     y,     2,             TILE_SIZE,     top);
    fill_rect(r, x + 2, y + 8, TILE_SIZE - 4, 1,             seam);
}

void draw_player(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body  = {60,  100, 220, 255};
    SDL_Color head  = {200, 160, 120, 255};
    SDL_Color sword = {190, 190, 210, 255};

    // body
    fill_rect(r, x + 7,  y + 11, 10, 10, body);
    // head
    fill_rect(r, x + 8,  y + 4,  8,  7,  head);
    // sword
    fill_rect(r, x + 17, y + 7,  2,  9,  sword);
}

void draw_stairs_up(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = {18,  18,  35,  255};
    SDL_Color step   = {180, 140, 60,  255};
    SDL_Color edge   = {220, 180, 80,  255};
    fill_rect(r, x,         y,         TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + 3,     y + 14,    18,        3,         step);
    fill_rect(r, x + 6,     y + 10,    12,        3,         step);
    fill_rect(r, x + 9,     y + 6,     6,         3,         step);
    fill_rect(r, x + 3,     y + 14,    18,        1,         edge);
    fill_rect(r, x + 6,     y + 10,    12,        1,         edge);
    fill_rect(r, x + 9,     y + 6,     6,         1,         edge);
}

void draw_stairs_down(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = {18,  18,  35,  255};
    SDL_Color step   = {100, 80,  160, 255};
    SDL_Color edge   = {140, 110, 200, 255};
    fill_rect(r, x,         y,         TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + 3,     y + 4,     18,        3,         step);
    fill_rect(r, x + 6,     y + 8,     12,        3,         step);
    fill_rect(r, x + 9,     y + 12,    6,         3,         step);
    fill_rect(r, x + 3,     y + 4,     18,        1,         edge);
    fill_rect(r, x + 6,     y + 8,     12,        1,         edge);
    fill_rect(r, x + 9,     y + 12,    6,         1,         edge);
}

void draw_goblin(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = { 40, 160,  40, 255};
    SDL_Color eye  = {220, 220,   0, 255};
    SDL_Color mouth= {180,  40,  40, 255};
    fill_rect(r, x+6,  y+4,  12, 10, body);
    fill_rect(r, x+5,  y+12, 14,  8, body);
    fill_rect(r, x+7,  y+7,   2,  2, eye);
    fill_rect(r, x+13, y+7,   2,  2, eye);
    fill_rect(r, x+8,  y+11,  6,  1, mouth);
}

void draw_skeleton(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color bone  = {210, 210, 190, 255};
    SDL_Color dark  = { 18,  18,  35, 255};
    fill_rect(r, x+7,  y+3,  10,  9, bone);
    fill_rect(r, x+6,  y+12, 12,  8, bone);
    fill_rect(r, x+8,  y+6,   2,  2, dark);
    fill_rect(r, x+13, y+6,   2,  2, dark);
    fill_rect(r, x+9,  y+10,  5,  1, dark);
}

void draw_orc(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = { 80, 120,  40, 255};
    SDL_Color eye  = {220,  20,  20, 255};
    SDL_Color tusk = {220, 200, 160, 255};
    fill_rect(r, x+5,  y+3,  14, 12, body);
    fill_rect(r, x+4,  y+13, 16,  7, body);
    fill_rect(r, x+7,  y+7,   2,  2, eye);
    fill_rect(r, x+13, y+7,   2,  2, eye);
    fill_rect(r, x+8,  y+13,  2,  3, tusk);
    fill_rect(r, x+13, y+13,  2,  3, tusk);
}

void draw_troll(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = { 60,  90,  60, 255};
    SDL_Color eye  = {255, 140,   0, 255};
    SDL_Color club = {120,  80,  40, 255};
    fill_rect(r, x+4,  y+2,  16, 14, body);
    fill_rect(r, x+3,  y+14, 18,  6, body);
    fill_rect(r, x+7,  y+6,   3,  3, eye);
    fill_rect(r, x+13, y+6,   3,  3, eye);
    fill_rect(r, x+18, y+8,   3,  10, club);
}

void draw_zombie(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body  = { 80, 110,  60, 255};
    SDL_Color skin  = {140, 160, 100, 255};
    SDL_Color eye   = {220,  20,  20, 255};
    SDL_Color wound = {160,  30,  30, 255};
    fill_rect(r, x+6,  y+11, 12, 10, body);
    fill_rect(r, x+7,  y+4,  10,  7, skin);
    fill_rect(r, x+8,  y+7,   2,  2, eye);
    fill_rect(r, x+14, y+7,   2,  2, eye);
    fill_rect(r, x+9,  y+10,  6,  1, wound);
    fill_rect(r, x+4,  y+13,  3,  6, skin);
    fill_rect(r, x+17, y+13,  3,  6, skin);
}

void draw_giant(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body  = { 90,  70,  50, 255};
    SDL_Color skin  = {160, 120,  80, 255};
    SDL_Color eye   = {255, 160,   0, 255};
    SDL_Color club  = {100,  60,  20, 255};
    fill_rect(r, x+3,  y+2,  18, 16, body);
    fill_rect(r, x+6,  y+4,  12,  8, skin);
    fill_rect(r, x+7,  y+6,   3,  3, eye);
    fill_rect(r, x+14, y+6,   3,  3, eye);
    fill_rect(r, x+2,  y+10,  3, 10, body);
    fill_rect(r, x+19, y+10,  3, 10, body);
    fill_rect(r, x+20, y+6,   3, 14, club);
    fill_rect(r, x+19, y+6,   4,  3, club);
}

void draw_goblin_king(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body  = { 60, 120,  40, 255};
    SDL_Color crown = {220, 180,  40, 255};
    SDL_Color eye   = {255,   0,   0, 255};
    fill_rect(r, x+4,  y+8,  16, 14, body);
    fill_rect(r, x+7,  y+4,  10, 8,  body);
    fill_rect(r, x+6,  y+2,  12, 4,  crown);
    fill_rect(r, x+8,  y,    3,  4,  crown);
    fill_rect(r, x+13, y,    3,  4,  crown);
    fill_rect(r, x+8,  y+7,  3,  3,  eye);
    fill_rect(r, x+13, y+7,  3,  3,  eye);
}

void draw_lich_king(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color robe  = { 40,  20,  80, 255};
    SDL_Color bone  = {200, 200, 180, 255};
    SDL_Color crown = {140,  60, 200, 255};
    SDL_Color glow  = {160,  80, 255, 255};
    fill_rect(r, x+4,  y+8,  16, 14, robe);
    fill_rect(r, x+7,  y+4,  10, 8,  bone);
    fill_rect(r, x+6,  y+2,  12, 3,  crown);
    fill_rect(r, x+8,  y+6,  3,  3,  glow);
    fill_rect(r, x+13, y+6,  3,  3,  glow);
    fill_rect(r, x+3,  y+10, 4,  2,  bone);
    fill_rect(r, x+17, y+10, 4,  2,  bone);
}

void draw_demon_lord(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body  = {140,  20,  20, 255};
    SDL_Color horn  = {100,  10,  10, 255};
    SDL_Color eye   = {255, 200,   0, 255};
    SDL_Color wing  = { 80,  10,  10, 255};
    fill_rect(r, x+4,  y+6,  16, 16, body);
    fill_rect(r, x+7,  y+3,  10, 8,  body);
    fill_rect(r, x+6,  y,    4,  6,  horn);
    fill_rect(r, x+14, y,    4,  6,  horn);
    fill_rect(r, x+8,  y+5,  3,  3,  eye);
    fill_rect(r, x+13, y+5,  3,  3,  eye);
    fill_rect(r, x,    y+8,  4,  8,  wing);
    fill_rect(r, x+20, y+8,  4,  8,  wing);
}

void draw_red_dragon(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body  = {180,  30,  10, 255};
    SDL_Color scale = {140,  20,   5, 255};
    SDL_Color eye   = {255, 220,   0, 255};
    SDL_Color wing  = {120,  15,   5, 255};
    SDL_Color flame = {255, 140,   0, 255};
    fill_rect(r, x+3,  y+6,  18, 14, body);
    fill_rect(r, x+6,  y+3,  12, 8,  body);
    fill_rect(r, x+4,  y+8,  16, 2,  scale);
    fill_rect(r, x+4,  y+12, 16, 2,  scale);
    fill_rect(r, x+8,  y+5,  3,  3,  eye);
    fill_rect(r, x+13, y+5,  3,  3,  eye);
    fill_rect(r, x,    y+4,  4,  10, wing);
    fill_rect(r, x+20, y+4,  4,  10, wing);
    fill_rect(r, x+10, y+16, 4,  6,  flame);
    fill_rect(r, x+9,  y+19, 6,  3,  flame);
}

void draw_tarrasque(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body  = { 60,  40,  20, 255};
    SDL_Color shell = { 80,  60,  30, 255};
    SDL_Color eye   = {255,   0,   0, 255};
    SDL_Color spike = { 40,  20,  10, 255};
    SDL_Color claw  = {120, 100,  50, 255};
    fill_rect(r, x+2,  y+4,  20, 18, body);
    fill_rect(r, x+4,  y+2,  16, 18, shell);
    fill_rect(r, x+6,  y,    4,  4,  spike);
    fill_rect(r, x+14, y,    4,  4,  spike);
    fill_rect(r, x+10, y+1,  4,  3,  spike);
    fill_rect(r, x+7,  y+5,  4,  4,  eye);
    fill_rect(r, x+13, y+5,  4,  4,  eye);
    fill_rect(r, x,    y+12, 4,  6,  claw);
    fill_rect(r, x+20, y+12, 4,  6,  claw);
    fill_rect(r, x+4,  y+20, 4,  4,  claw);
    fill_rect(r, x+16, y+20, 4,  4,  claw);
}

void draw_enemy(Renderer *r, int tile_x, int tile_y, EnemyType type) {
    switch (type) {
        case ENEMY_SKELETON: draw_skeleton(r, tile_x, tile_y); break;
        case ENEMY_GOBLIN:   draw_goblin(r, tile_x, tile_y);   break;
        case ENEMY_ZOMBIE:   draw_zombie(r, tile_x, tile_y);   break;
        case ENEMY_ORC:      draw_orc(r, tile_x, tile_y);      break;
        case ENEMY_TROLL:    draw_troll(r, tile_x, tile_y);    break;
        case ENEMY_GIANT:    draw_giant(r, tile_x, tile_y);    break;
        case ENEMY_GOBLIN_KING: draw_goblin_king(r, tile_x, tile_y); break;
        case ENEMY_LICH_KING:   draw_lich_king(r, tile_x, tile_y);   break;
        case ENEMY_DEMON_LORD:  draw_demon_lord(r, tile_x, tile_y);  break;
        case ENEMY_RED_DRAGON:  draw_red_dragon(r, tile_x, tile_y);  break;
        case ENEMY_TARRASQUE:   draw_tarrasque(r, tile_x, tile_y);   break;
    }
}

void draw_town_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base  = { 14, 26, 14, 255};
    SDL_Color blade = { 20, 36, 20, 255};
    SDL_Color dot   = { 26, 46, 26, 255};
    fill_rect(r, x,    y,    TILE_SIZE,   TILE_SIZE,   base);
    fill_rect(r, x+2,  y+2,  TILE_SIZE-4, TILE_SIZE-4, blade);
    fill_rect(r, x+4,  y+8,  2,           2,           dot);
    fill_rect(r, x+12, y+4,  2,           2,           dot);
}

void draw_town_path(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = { 58,  46,  30, 255};
    SDL_Color stone  = { 74,  58,  40, 255};
    SDL_Color mortar = { 42,  30,  16, 255};
    fill_rect(r, x,            y,            TILE_SIZE,     TILE_SIZE,     base);
    fill_rect(r, x+1,          y+1,          TILE_SIZE/2-2, TILE_SIZE/2-2, stone);
    fill_rect(r, x+TILE_SIZE/2+1, y+1,       TILE_SIZE/2-2, TILE_SIZE/2-2, stone);
    fill_rect(r, x+1,          y+TILE_SIZE/2+1, TILE_SIZE/2-2, TILE_SIZE/2-2, stone);
    fill_rect(r, x+TILE_SIZE/2+1, y+TILE_SIZE/2+1, TILE_SIZE/2-2, TILE_SIZE/2-2, stone);
    fill_rect(r, x,            y+TILE_SIZE/2, TILE_SIZE,     1,             mortar);
    fill_rect(r, x+TILE_SIZE/2, y,            1,             TILE_SIZE,     mortar);
}

void draw_town_exit(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = { 26, 48, 32, 255};
    SDL_Color glow = { 48, 160, 96, 255};
    fill_rect(r, x,   y,   TILE_SIZE,   TILE_SIZE,   base);
    fill_rect(r, x+2, y+2, TILE_SIZE-4, TILE_SIZE-4, glow);
}

void draw_shop_blacksmith(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color wall   = { 58,  40,  16, 255};
    SDL_Color roof   = { 80,  32,  16, 255};
    SDL_Color door   = { 26,  14,   6, 255};
    SDL_Color window = {255, 136,  32, 255};
    SDL_Color anvil  = {136, 136, 136, 255};
    fill_rect(r, x,    y,    TILE_SIZE, TILE_SIZE, wall);
    fill_rect(r, x,    y,    TILE_SIZE, 3,         roof);
    fill_rect(r, x+8,  y+14, 8,         10,        door);
    fill_rect(r, x+2,  y+6,  5,         5,         window);
    fill_rect(r, x+14, y+8,  6,         4,         anvil);
    fill_rect(r, x+12, y+11, 10,        3,         anvil);
}

void draw_shop_alchemist(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color wall   = { 26,  42,  58, 255};
    SDL_Color roof   = { 16,  32,  48, 255};
    SDL_Color door   = { 10,  16,  24, 255};
    SDL_Color window = { 96,  64, 192, 255};
    SDL_Color potion = {160,  64, 192, 255};
    fill_rect(r, x,    y,    TILE_SIZE, TILE_SIZE, wall);
    fill_rect(r, x,    y,    TILE_SIZE, 3,         roof);
    fill_rect(r, x+8,  y+14, 8,         10,        door);
    fill_rect(r, x+2,  y+6,  5,         5,         window);
    fill_rect(r, x+14, y+6,  4,         8,         potion);
    fill_rect(r, x+13, y+9,  6,         2,         potion);
}

void draw_floor_item(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = { 18,  18,  35, 255};
    SDL_Color glow = {100, 180, 255, 255};
    fill_rect(r, x,   y,   TILE_SIZE,   TILE_SIZE,   base);
    fill_rect(r, x+8, y+6, 8,           10,          glow);
    fill_rect(r, x+6, y+8, 12,          6,           glow);
}

void draw_floor_gold(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = { 18,  18,  35, 255};
    SDL_Color gold = {220, 180,  40, 255};
    fill_rect(r, x,   y,   TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x+7, y+7, 10,        10,        gold);
    fill_rect(r, x+9, y+9, 6,         6,         gold);
}

void draw_trap_spike(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base  = { 18,  18,  35, 255};
    SDL_Color metal = {160, 160, 180, 255};
    SDL_Color dark  = { 80,  80, 100, 255};
    fill_rect(r, x,    y,    TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x+10, y+14, 4,         8,         metal);
    fill_rect(r, x+4,  y+16, 4,         6,         metal);
    fill_rect(r, x+16, y+16, 4,         6,         metal);
    fill_rect(r, x+7,  y+18, 10,        4,         dark);
}

void draw_trap_fire(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = { 18,  18,  35, 255};
    SDL_Color flame  = {220,  80,  20, 255};
    SDL_Color bright = {255, 160,  40, 255};
    fill_rect(r, x,    y,    TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x+8,  y+10, 8,         12,        flame);
    fill_rect(r, x+10, y+6,  4,         8,         bright);
    fill_rect(r, x+6,  y+14, 12,        4,         flame);
}

void draw_trap_poison(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = { 18,  18,  35, 255};
    SDL_Color green  = { 40, 160,  40, 255};
    SDL_Color bright = { 80, 220,  80, 255};
    fill_rect(r, x,    y,    TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x+6,  y+8,  12,        12,        green);
    fill_rect(r, x+9,  y+6,  6,         6,         bright);
    fill_rect(r, x+8,  y+16, 8,         4,         green);
}