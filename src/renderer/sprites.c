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
    SDL_Color outline = { 18,  14,  14, 255};
    SDL_Color stone   = { 72,  70,  68, 255};
    SDL_Color mortar  = { 42,  40,  40, 255};
    SDL_Color timber  = { 82,  48,  28, 255};
    SDL_Color wood_hi = {116,  70,  36, 255};
    SDL_Color roof    = { 42,  42,  48, 255};
    SDL_Color roof_hi = { 68,  66,  70, 255};
    SDL_Color brick   = {104,  48,  34, 255};
    SDL_Color ember   = {255, 112,  16, 255};
    SDL_Color flame   = {255, 202,  54, 255};
    SDL_Color iron    = {132, 138, 142, 255};

    /* Foundation and timber facade. */
    fill_rect(r, x+4, y+78, 112, 16, outline);
    fill_rect(r, x+6, y+80, 108, 12, stone);
    for (int sx = 8; sx < 112; sx += 18)
        fill_rect(r, x+sx, y+86, 1, 6, mortar);
    fill_rect(r, x+8, y+55, 104, 27, timber);
    fill_rect(r, x+8, y+55, 104, 4, wood_hi);
    fill_rect(r, x+12, y+56, 5, 26, outline);
    fill_rect(r, x+103, y+56, 5, 26, outline);

    /* Broad charcoal roof with staggered shingle highlights. */
    fill_rect(r, x+5,  y+25, 110, 34, outline);
    fill_rect(r, x+9,  y+22, 102, 35, roof);
    fill_rect(r, x+13, y+18,  94,  7, roof_hi);
    for (int row = 0; row < 3; row++) {
        int sy = y + 28 + row * 9;
        int offset = (row & 1) ? 8 : 0;
        fill_rect(r, x+10, sy+6, 100, 2, outline);
        for (int sx = 14 + offset; sx < 108; sx += 16)
            fill_rect(r, x+sx, sy, 2, 7, outline);
    }

    /* Sooted brick chimney and live forge. */
    fill_rect(r, x+10, y+4, 24, 31, outline);
    fill_rect(r, x+13, y+7, 18, 27, brick);
    fill_rect(r, x+13, y+15, 18, 2, outline);
    fill_rect(r, x+20, y+7,  2, 27, outline);
    fill_rect(r, x+7,  y+59, 39, 25, outline);
    fill_rect(r, x+11, y+63, 31, 18, (SDL_Color){56, 24, 16, 255});
    fill_rect(r, x+16, y+73, 21,  7, ember);
    fill_rect(r, x+21, y+67, 11, 12, flame);

    /* Workshop door, iron braces, woodpile, and anvil sign. */
    fill_rect(r, x+52, y+60, 29, 33, outline);
    fill_rect(r, x+56, y+64, 21, 29, (SDL_Color){58, 34, 22, 255});
    fill_rect(r, x+59, y+69, 15, 2, wood_hi);
    fill_rect(r, x+59, y+80, 15, 2, wood_hi);
    fill_rect(r, x+62, y+65, 2, 26, iron);
    fill_rect(r, x+72, y+77, 3, 3, iron);
    fill_rect(r, x+87, y+70, 18, 4, iron);
    fill_rect(r, x+91, y+74, 10, 3, iron);
    fill_rect(r, x+94, y+77,  4, 4, outline);
    for (int i = 0; i < 3; i++) {
        fill_rect(r, x+85+i*9, y+84, 8, 7, (SDL_Color){92, 52, 26, 255});
        fill_rect(r, x+87+i*9, y+84, 4, 2, wood_hi);
    }
}

void draw_shop_alchemist(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outline = { 14,  14,  24, 255};
    SDL_Color stone   = { 66,  70,  76, 255};
    SDL_Color wall    = { 48,  38,  66, 255};
    SDL_Color timber  = { 28,  24,  42, 255};
    SDL_Color roof    = { 24,  72,  70, 255};
    SDL_Color roof_hi = { 42, 104,  94, 255};
    SDL_Color purple  = {176,  68, 220, 255};
    SDL_Color violet  = {108,  62, 188, 255};
    SDL_Color green   = {112, 214,  92, 255};
    SDL_Color glass   = {202, 228, 210, 255};
    SDL_Color herb    = { 76, 116,  48, 255};

    /* Stone base and crooked indigo timber facade. */
    fill_rect(r, x+5, y+79, 110, 15, outline);
    fill_rect(r, x+8, y+81, 104, 11, stone);
    fill_rect(r, x+9, y+52, 102, 31, wall);
    fill_rect(r, x+13, y+53, 5, 30, timber);
    fill_rect(r, x+102,y+53, 5, 30, timber);
    fill_rect(r, x+14, y+58, 92, 4, timber);

    /* Steep teal roof and central gable. */
    fill_rect(r, x+5,  y+27, 110, 31, outline);
    fill_rect(r, x+9,  y+24, 102, 31, roof);
    for (int row = 0; row < 3; row++) {
        int sy = y + 28 + row * 8;
        int offset = (row & 1) ? 7 : 0;
        fill_rect(r, x+10, sy+6, 100, 2, outline);
        for (int sx = 13 + offset; sx < 109; sx += 14)
            fill_rect(r, x+sx, sy, 2, 7, roof_hi);
    }
    fill_rect(r, x+44, y+22, 32, 36, outline);
    fill_rect(r, x+48, y+25, 24, 33, (SDL_Color){42, 32, 58, 255});
    fill_rect(r, x+56, y+17,  8,  9, roof_hi);

    /* Bottle-shaped glowing gable window. */
    fill_rect(r, x+56, y+31, 8, 4, glass);
    fill_rect(r, x+53, y+35,14,14, glass);
    fill_rect(r, x+55, y+37,10,10, violet);
    fill_rect(r, x+58, y+39, 6, 6, purple);
    fill_rect(r, x+61, y+39, 2, 2, (SDL_Color){244, 184, 255, 255});

    /* South door, luminous window, herbs, and potion display. */
    fill_rect(r, x+48, y+59, 27, 34, outline);
    fill_rect(r, x+52, y+63, 19, 30, (SDL_Color){54, 34, 30, 255});
    fill_rect(r, x+55, y+70, 13, 2, (SDL_Color){102, 66, 42, 255});
    fill_rect(r, x+55, y+81, 13, 2, (SDL_Color){102, 66, 42, 255});
    fill_rect(r, x+66, y+77, 3, 3, glass);
    fill_rect(r, x+18, y+63, 22, 18, outline);
    fill_rect(r, x+21, y+66, 16, 12, green);
    fill_rect(r, x+28, y+66, 2, 12, outline);
    fill_rect(r, x+21, y+72, 16, 2, outline);
    for (int i = 0; i < 3; i++) {
        fill_rect(r, x+78+i*7, y+68, 4, 6, herb);
        fill_rect(r, x+79+i*7, y+74, 2, 7, herb);
    }
    fill_rect(r, x+80, y+83, 27, 3, timber);
    fill_rect(r, x+83, y+78, 5, 5, glass);
    fill_rect(r, x+84, y+81, 3, 4, purple);
    fill_rect(r, x+92, y+76, 5, 7, glass);
    fill_rect(r, x+93, y+80, 3, 5, green);
    fill_rect(r, x+101,y+79, 5, 6, glass);
    fill_rect(r, x+102,y+82, 3, 3, violet);
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
