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

void draw_forest_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE, y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){10, 28, 18, 255});
    fill_rect(r, x+3, y+5, 3, 2, (SDL_Color){25, 58, 31, 255});
    fill_rect(r, x+16, y+15, 2, 4, (SDL_Color){34, 74, 40, 255});
    fill_rect(r, x+8, y+20, 6, 1, (SDL_Color){18, 48, 27, 255});
}

void draw_forest_wall(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE, y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){7, 20, 12, 255});
    fill_rect(r, x+2, y, 7, TILE_SIZE, (SDL_Color){25, 48, 25, 255});
    fill_rect(r, x+4, y, 3, TILE_SIZE, (SDL_Color){49, 66, 35, 255});
    fill_rect(r, x+14, y+2, 8, TILE_SIZE-2, (SDL_Color){18, 42, 22, 255});
    fill_rect(r, x+16, y+2, 3, TILE_SIZE-2, (SDL_Color){42, 62, 31, 255});
    fill_rect(r, x, y+2, TILE_SIZE, 4, (SDL_Color){22, 65, 30, 255});
}

void draw_forest_edge(Renderer *r, int tile_x, int tile_y, int forward) {
    int x = tile_x * TILE_SIZE, y = tile_y * TILE_SIZE;
    draw_forest_floor(r, tile_x, tile_y);
    SDL_Color trunk = {60, 43, 25, 255};
    SDL_Color leaf = {25, 80, 38, 255};
    SDL_Color glow = forward ? (SDL_Color){112, 224, 92, 255} :
        (SDL_Color){76, 155, 120, 255};
    fill_rect(r, x+2, y, 4, TILE_SIZE, trunk);
    fill_rect(r, x+18, y, 4, TILE_SIZE, trunk);
    fill_rect(r, x+3, y+1, 18, 5, leaf);
    fill_rect(r, x+10, y+8, 4, 9, glow);
    fill_rect(r, x+8, y+11, 8, 3, glow);
}

void draw_forest_landmark(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_forest_floor(r, tile_x, tile_y);
    fill_rect(r, x + 5, y + 17, 15, 5, (SDL_Color){45, 58, 48, 255});
    fill_rect(r, x + 8, y + 5, 9, 14, (SDL_Color){84, 103, 91, 255});
    fill_rect(r, x + 10, y + 2, 5, 5, (SDL_Color){109, 132, 113, 255});
    fill_rect(r, x + 11, y + 8, 3, 7, (SDL_Color){94, 224, 126, 255});
    fill_rect(r, x + 9, y + 10, 7, 3, (SDL_Color){94, 224, 126, 255});
}

void draw_mountain_floor(Renderer *r, int tile_x, int tile_y) {
    int x=tile_x*TILE_SIZE, y=tile_y*TILE_SIZE;
    fill_rect(r,x,y,TILE_SIZE,TILE_SIZE,(SDL_Color){20,12,15,255});
    fill_rect(r,x+2,y+4,9,2,(SDL_Color){48,25,28,255});
    fill_rect(r,x+13,y+15,8,2,(SDL_Color){72,28,25,255});
    fill_rect(r,x+5,y+21,3,2,(SDL_Color){126,39,22,255});
}

void draw_mountain_wall(Renderer *r, int tile_x, int tile_y) {
    int x=tile_x*TILE_SIZE, y=tile_y*TILE_SIZE;
    fill_rect(r,x,y,TILE_SIZE,TILE_SIZE,(SDL_Color){13,10,13,255});
    fill_rect(r,x+1,y+2,22,7,(SDL_Color){42,31,35,255});
    fill_rect(r,x+4,y+10,18,11,(SDL_Color){31,23,27,255});
    fill_rect(r,x,y+8,TILE_SIZE,2,(SDL_Color){86,28,27,255});
    fill_rect(r,x+14,y+11,2,9,(SDL_Color){104,31,23,255});
}

void draw_mountain_edge(Renderer *r, int tile_x, int tile_y, int forward) {
    int x=tile_x*TILE_SIZE, y=tile_y*TILE_SIZE;
    draw_mountain_floor(r,tile_x,tile_y);
    fill_rect(r,x+2,y+1,5,22,(SDL_Color){40,31,32,255});
    fill_rect(r,x+18,y+1,5,22,(SDL_Color){40,31,32,255});
    fill_rect(r,x+2,y+1,21,4,(SDL_Color){91,32,28,255});
    fill_rect(r,x+10,y+8,5,9,forward ?
        (SDL_Color){239,65,25,255} : (SDL_Color){145,47,32,255});
}

void draw_mountain_bridge(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){12, 8, 11, 255});
    fill_rect(r, x, y + 3, TILE_SIZE, 18, (SDL_Color){81, 48, 31, 255});
    fill_rect(r, x, y + 5, TILE_SIZE, 2, (SDL_Color){136, 76, 39, 255});
    fill_rect(r, x, y + 17, TILE_SIZE, 2, (SDL_Color){48, 29, 24, 255});
    fill_rect(r, x + 4, y + 3, 2, 18, (SDL_Color){35, 25, 24, 255});
    fill_rect(r, x + 17, y + 3, 2, 18, (SDL_Color){35, 25, 24, 255});
}

void draw_mountain_cave_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){15, 13, 18, 255});
    fill_rect(r, x + 2, y + 5, 8, 3, (SDL_Color){49, 43, 51, 255});
    fill_rect(r, x + 14, y + 15, 7, 4, (SDL_Color){65, 48, 50, 255});
    fill_rect(r, x + 7, y + 21, 3, 2, (SDL_Color){202, 57, 28, 255});
}

void draw_mountain_fortress_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){34, 27, 31, 255});
    fill_rect(r, x, y, TILE_SIZE, 2, (SDL_Color){91, 62, 60, 255});
    fill_rect(r, x, y + 11, TILE_SIZE, 2, (SDL_Color){19, 15, 19, 255});
    fill_rect(r, x + 11, y, 2, 11, (SDL_Color){19, 15, 19, 255});
    fill_rect(r, x + 5, y + 13, 2, 11, (SDL_Color){19, 15, 19, 255});
}

void draw_coast_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){8, 31, 43, 255});
    fill_rect(r, x + 2, y + 5, 10, 2, (SDL_Color){18, 67, 78, 255});
    fill_rect(r, x + 13, y + 16, 8, 2, (SDL_Color){34, 103, 109, 255});
    fill_rect(r, x + 6, y + 21, 4, 2, (SDL_Color){157, 132, 82, 255});
}

void draw_coast_wall(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){7, 20, 30, 255});
    fill_rect(r, x + 1, y + 2, 22, 8, (SDL_Color){25, 68, 73, 255});
    fill_rect(r, x + 4, y + 12, 18, 9, (SDL_Color){20, 51, 61, 255});
    fill_rect(r, x, y + 10, TILE_SIZE, 2, (SDL_Color){74, 121, 112, 255});
    fill_rect(r, x + 15, y + 13, 2, 8, (SDL_Color){125, 103, 66, 255});
}

void draw_coast_edge(Renderer *r, int tile_x, int tile_y, int forward) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_coast_floor(r, tile_x, tile_y);
    fill_rect(r, x + 2, y + 2, 5, 21, (SDL_Color){42, 84, 82, 255});
    fill_rect(r, x + 18, y + 2, 5, 21, (SDL_Color){42, 84, 82, 255});
    fill_rect(r, x + 2, y + 2, 21, 4, (SDL_Color){93, 139, 117, 255});
    fill_rect(r, x + 9, y + 8, 7, 9, forward ?
        (SDL_Color){62, 220, 216, 255} : (SDL_Color){47, 130, 145, 255});
}

void draw_coast_shallow_water(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){18, 73, 88, 255});
    fill_rect(r, x + 1, y + 5, 10, 2, (SDL_Color){51, 145, 151, 255});
    fill_rect(r, x + 12, y + 14, 11, 2, (SDL_Color){70, 174, 166, 255});
    fill_rect(r, x + 5, y + 21, 8, 1, (SDL_Color){122, 181, 156, 255});
}

void draw_coast_deep_water(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){3, 18, 42, 255});
    fill_rect(r, x, y + 4, 13, 2, (SDL_Color){14, 58, 91, 255});
    fill_rect(r, x + 9, y + 13, 15, 2, (SDL_Color){20, 75, 105, 255});
    fill_rect(r, x + 2, y + 21, 9, 1, (SDL_Color){29, 95, 116, 255});
}

void draw_coast_tide_control(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_coast_floor(r, tile_x, tile_y);
    fill_rect(r, x + 5, y + 15, 14, 7, (SDL_Color){43, 91, 89, 255});
    fill_rect(r, x + 8, y + 7, 8, 10, (SDL_Color){91, 151, 132, 255});
    fill_rect(r, x + 10, y + 3, 4, 8, (SDL_Color){196, 158, 69, 255});
    fill_rect(r, x + 12, y + 2, 7, 3, (SDL_Color){90, 231, 207, 255});
}

void draw_player(Renderer *r, int tile_x, int tile_y, PlayerClass player_class) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outline = {16, 18, 30, 255};
    SDL_Color skin = {210, 158, 112, 255};
    SDL_Color leather = {78, 48, 30, 255};
    SDL_Color steel = {184, 194, 204, 255};

    if (player_class == CLASS_MAGE) {
        SDL_Color robe = {42, 74, 154, 255};
        SDL_Color robe_hi = {72, 112, 220, 255};
        SDL_Color magic = {62, 224, 232, 255};
        fill_rect(r, x+7, y+3, 10, 3, outline);
        fill_rect(r, x+5, y+6, 14, 7, robe);
        fill_rect(r, x+8, y+6, 8, 6, skin);
        fill_rect(r, x+6, y+12, 12, 9, robe);
        fill_rect(r, x+9, y+13, 6, 7, robe_hi);
        fill_rect(r, x+5, y+20, 5, 2, outline);
        fill_rect(r, x+14,y+20, 5, 2, outline);
        fill_rect(r, x+2, y+7, 2, 14, leather);
        fill_rect(r, x+1, y+4, 4, 4, magic);
        fill_rect(r, x+2, y+3, 2, 2, (SDL_Color){188, 255, 250, 255});
    } else if (player_class == CLASS_ROGUE) {
        SDL_Color hood = {48, 88, 56, 255};
        SDL_Color cloth = {72, 104, 62, 255};
        fill_rect(r, x+6, y+3, 12, 3, hood);
        fill_rect(r, x+5, y+6, 14, 7, hood);
        fill_rect(r, x+8, y+7, 8, 5, skin);
        fill_rect(r, x+6, y+12, 12, 8, cloth);
        fill_rect(r, x+9, y+12, 3, 8, leather);
        fill_rect(r, x+5, y+20, 5, 2, outline);
        fill_rect(r, x+14,y+20, 5, 2, outline);
        fill_rect(r, x+19,y+6, 2, 13, leather);
        fill_rect(r, x+17,y+5, 4, 2, leather);
        fill_rect(r, x+17,y+18,4, 2, leather);
        fill_rect(r, x+18,y+8, 1, 9, steel);
    } else {
        SDL_Color blue = {48, 86, 184, 255};
        fill_rect(r, x+7, y+3, 10, 8, skin);
        fill_rect(r, x+6, y+2, 12, 3, steel);
        fill_rect(r, x+5, y+10, 14, 10, blue);
        fill_rect(r, x+5, y+10, 4, 7, steel);
        fill_rect(r, x+15,y+10, 4, 7, steel);
        fill_rect(r, x+8, y+20, 4, 2, outline);
        fill_rect(r, x+14,y+20, 4, 2, outline);
        // Raised sword: pointed blade, crossguard, grip, and pommel.
        fill_rect(r, x+20,y,   1,  2, (SDL_Color){232, 238, 242, 255});
        fill_rect(r, x+19,y+2, 3, 14, steel);
        fill_rect(r, x+20,y+2, 1, 13, (SDL_Color){224, 230, 232, 255});
        fill_rect(r, x+17,y+16,7,  2, (SDL_Color){202, 158, 62, 255});
        fill_rect(r, x+19,y+18,3,  4, leather);
        fill_rect(r, x+18,y+22,5,  2, (SDL_Color){202, 158, 62, 255});
    }
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

void draw_return_exit(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = {12, 14, 28, 255};
    SDL_Color glow = {96, 210, 190, 255};
    SDL_Color core = {190, 255, 232, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + 3, y + 3, 18, 18, glow);
    fill_rect(r, x + 6, y + 5, 12, 16, base);
    fill_rect(r, x + 9, y + 7, 6, 12, core);
}

void draw_broken_burial_seal(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x + 3, y + 3, 18, 18, (SDL_Color){42, 38, 52, 255});
    fill_rect(r, x + 5, y + 5, 14, 14, (SDL_Color){91, 83, 99, 255});
    fill_rect(r, x + 8, y + 8, 8, 8, (SDL_Color){54, 46, 67, 255});
    fill_rect(r, x + 11, y + 5, 2, 6, (SDL_Color){185, 55, 192, 255});
    fill_rect(r, x + 8, y + 10, 5, 2, (SDL_Color){185, 55, 192, 255});
    fill_rect(r, x + 12, y + 11, 2, 8, (SDL_Color){28, 24, 36, 255});
    fill_rect(r, x + 14, y + 14, 5, 2, (SDL_Color){28, 24, 36, 255});
}

void draw_restored_burial_seal(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x + 3, y + 3, 18, 18, (SDL_Color){63, 59, 66, 255});
    fill_rect(r, x + 5, y + 5, 14, 14, (SDL_Color){118, 111, 112, 255});
    fill_rect(r, x + 8, y + 8, 8, 8, (SDL_Color){72, 67, 72, 255});
    fill_rect(r, x + 11, y + 6, 2, 12, (SDL_Color){226, 191, 82, 255});
    fill_rect(r, x + 7, y + 11, 10, 2, (SDL_Color){246, 220, 126, 255});
    fill_rect(r, x + 10, y + 10, 4, 4, (SDL_Color){255, 236, 158, 255});
}

void draw_locked_door(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color stone = {50, 45, 80, 255};
    SDL_Color wood = {66, 38, 54, 255};
    SDL_Color edge = {116, 72, 94, 255};
    SDL_Color iron = {146, 142, 164, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, stone);
    fill_rect(r, x+3, y+2, 18, 22, edge);
    fill_rect(r, x+5, y+4, 14, 20, wood);
    fill_rect(r, x+7, y+4, 2, 20, edge);
    fill_rect(r, x+15, y+4, 2, 20, edge);
    fill_rect(r, x+10, y+11, 5, 6, iron);
    fill_rect(r, x+11, y+9, 3, 4, iron);
}

void draw_dungeon_key(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color gold = {226, 184, 54, 255};
    SDL_Color shine = {255, 232, 126, 255};
    draw_town_path(r, tile_x, tile_y);
    fill_rect(r, x+4, y+7, 8, 8, gold);
    fill_rect(r, x+6, y+9, 4, 4, (SDL_Color){18, 18, 35, 255});
    fill_rect(r, x+11, y+10, 10, 3, gold);
    fill_rect(r, x+17, y+13, 3, 4, gold);
    fill_rect(r, x+5, y+7, 3, 2, shine);
}

void draw_portal(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outer = {68, 72, 218, 255};
    SDL_Color glow = {86, 214, 244, 255};
    SDL_Color core = {188, 252, 255, 255};
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x+3, y+9, 18, 7, outer);
    fill_rect(r, x+6, y+7, 12, 10, glow);
    fill_rect(r, x+9, y+9, 6, 6, core);
    fill_rect(r, x+7, y+11, 10, 3, (SDL_Color){24, 30, 82, 255});
}

void draw_goblin(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {78, 142, 38, 255};
    SDL_Color dark = {28, 48, 26, 255};
    SDL_Color cloth = {92, 58, 30, 255};
    fill_rect(r, x+4, y+6, 16, 3, skin);
    fill_rect(r, x+7, y+4, 10, 9, skin);
    fill_rect(r, x+8, y+7, 2, 2, (SDL_Color){232, 186, 38, 255});
    fill_rect(r, x+14,y+7, 2, 2, (SDL_Color){232, 186, 38, 255});
    fill_rect(r, x+7, y+13, 10, 7, cloth);
    fill_rect(r, x+4, y+14, 3, 5, skin);
    fill_rect(r, x+17,y+13, 3, 6, skin);
    fill_rect(r, x+7, y+20, 4, 2, dark);
    fill_rect(r, x+14,y+20, 4, 2, dark);
    fill_rect(r, x+19,y+10, 2, 8, (SDL_Color){174, 184, 188, 255});
}

void draw_skeleton(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color bone  = {210, 210, 190, 255};
    SDL_Color dark  = { 18,  18,  35, 255};
    fill_rect(r, x+7, y+3, 10, 9, bone);
    fill_rect(r, x+8, y+6, 2, 2, dark);
    fill_rect(r, x+14,y+6, 2, 2, dark);
    fill_rect(r, x+10,y+10,4, 2, dark);
    fill_rect(r, x+11,y+12,3, 8, bone);
    fill_rect(r, x+7, y+14,10, 2, bone);
    fill_rect(r, x+7, y+17,10, 2, bone);
    fill_rect(r, x+7, y+19,3, 4, bone);
    fill_rect(r, x+15,y+19,3, 4, bone);
    fill_rect(r, x+2, y+9, 2, 12, (SDL_Color){174, 184, 192, 255});
    fill_rect(r, x+1, y+8, 4, 3, (SDL_Color){212, 218, 220, 255});
    fill_rect(r, x+18,y+12,5, 8, (SDL_Color){92, 62, 38, 255});
    fill_rect(r, x+19,y+13,3, 6, (SDL_Color){132, 132, 132, 255});
}

void draw_orc(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {92, 116, 48, 255};
    SDL_Color dark = {30, 38, 26, 255};
    SDL_Color armor = {82, 82, 86, 255};
    fill_rect(r, x+6, y+3, 12, 10, skin);
    fill_rect(r, x+5, y+5, 2, 5, skin);
    fill_rect(r, x+17,y+5, 2, 5, skin);
    fill_rect(r, x+8, y+7, 2, 2, (SDL_Color){232, 166, 34, 255});
    fill_rect(r, x+14,y+7, 2, 2, (SDL_Color){232, 166, 34, 255});
    fill_rect(r, x+8, y+11,2, 3, (SDL_Color){224, 210, 170, 255});
    fill_rect(r, x+15,y+11,2, 3, (SDL_Color){224, 210, 170, 255});
    fill_rect(r, x+4, y+13,16, 7, armor);
    fill_rect(r, x+8, y+14,8, 6, (SDL_Color){84, 48, 28, 255});
    fill_rect(r, x+5, y+20,5, 3, dark);
    fill_rect(r, x+15,y+20,5, 3, dark);
    fill_rect(r, x+20,y+7, 3, 13, (SDL_Color){172, 180, 182, 255});
}

void draw_troll(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {72, 94, 58, 255};
    SDL_Color shade = {42, 56, 40, 255};
    fill_rect(r, x+5, y+3, 14, 10, skin);
    fill_rect(r, x+7, y+6, 3, 2, (SDL_Color){224, 154, 36, 255});
    fill_rect(r, x+14,y+6, 3, 2, (SDL_Color){224, 154, 36, 255});
    fill_rect(r, x+3, y+12,18, 8, skin);
    fill_rect(r, x+2, y+13,4, 8, skin);
    fill_rect(r, x+18,y+12,4, 9, skin);
    fill_rect(r, x+8, y+13,8, 7, (SDL_Color){90, 58, 32, 255});
    fill_rect(r, x+4, y+20,6, 3, shade);
    fill_rect(r, x+15,y+20,6, 3, shade);
    fill_rect(r, x+20,y+5, 3, 15, (SDL_Color){104, 76, 48, 255});
    fill_rect(r, x+19,y+4, 5, 6, (SDL_Color){126, 126, 116, 255});
}

void draw_zombie(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {132, 146, 108, 255};
    SDL_Color cloth = {100, 48, 54, 255};
    SDL_Color dark = {42, 34, 42, 255};
    fill_rect(r, x+8, y+3, 10, 9, skin);
    fill_rect(r, x+7, y+5, 2, 6, skin);
    fill_rect(r, x+10,y+6, 2, 2, (SDL_Color){214, 198, 90, 255});
    fill_rect(r, x+15,y+7, 2, 2, dark);
    fill_rect(r, x+7, y+12,12, 9, cloth);
    fill_rect(r, x+3, y+11,4, 3, skin);
    fill_rect(r, x+18,y+14,4, 3, skin);
    fill_rect(r, x+8, y+20,4, 3, dark);
    fill_rect(r, x+15,y+20,4, 3, dark);
    fill_rect(r, x+11,y+14,6, 2, (SDL_Color){150, 48, 48, 255});
}

void draw_crypt_bat(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color wing = {92, 42, 126, 255};
    SDL_Color edge = {46, 22, 70, 255};
    SDL_Color body = {54, 32, 72, 255};
    SDL_Color eye = {244, 48, 62, 255};
    fill_rect(r, x+1, y+7, 7, 3, edge);
    fill_rect(r, x+3, y+5, 6, 8, wing);
    fill_rect(r, x+15, y+5, 6, 8, wing);
    fill_rect(r, x+16, y+7, 7, 3, edge);
    fill_rect(r, x+7, y+8, 10, 9, body);
    fill_rect(r, x+8, y+6, 3, 3, edge);
    fill_rect(r, x+14, y+6, 3, 3, edge);
    fill_rect(r, x+9, y+11, 2, 2, eye);
    fill_rect(r, x+14, y+11, 2, 2, eye);
    fill_rect(r, x+11, y+17, 3, 3, edge);
}

void draw_wraith(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color glow = {112, 224, 232, 255};
    SDL_Color pale = {176, 242, 238, 255};
    SDL_Color shade = {54, 112, 132, 255};
    SDL_Color voidc = {12, 20, 38, 255};
    fill_rect(r, x+7, y+2, 10, 4, pale);
    fill_rect(r, x+5, y+6, 14, 8, glow);
    fill_rect(r, x+8, y+6, 8, 6, voidc);
    fill_rect(r, x+9, y+8, 2, 2, pale);
    fill_rect(r, x+14, y+8, 2, 2, pale);
    fill_rect(r, x+4, y+13, 16, 5, shade);
    fill_rect(r, x+6, y+18, 4, 3, glow);
    fill_rect(r, x+12, y+17, 4, 5, glow);
    fill_rect(r, x+17, y+18, 3, 2, glow);
    fill_rect(r, x+1, y+13, 4, 3, pale);
    fill_rect(r, x+19, y+12, 4, 3, pale);
}

void draw_necromancer(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color robe = {62, 28, 92, 255};
    SDL_Color trim = {116, 54, 148, 255};
    SDL_Color skin = {172, 166, 134, 255};
    SDL_Color magic = {78, 232, 76, 255};
    SDL_Color bone = {210, 204, 170, 255};
    fill_rect(r, x+7, y+2, 10, 4, trim);
    fill_rect(r, x+5, y+5, 14, 8, robe);
    fill_rect(r, x+8, y+7, 8, 5, skin);
    fill_rect(r, x+9, y+8, 2, 2, magic);
    fill_rect(r, x+14, y+8, 2, 2, magic);
    fill_rect(r, x+5, y+13, 14, 9, robe);
    fill_rect(r, x+9, y+14, 6, 7, trim);
    fill_rect(r, x+2, y+6, 2, 16, bone);
    fill_rect(r, x+1, y+3, 4, 4, bone);
    fill_rect(r, x+2, y+2, 2, 2, magic);
    fill_rect(r, x+19, y+13, 4, 4, magic);
}

void draw_giant(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {166, 122, 82, 255};
    SDL_Color shade = {104, 72, 50, 255};
    fill_rect(r, x+5, y+2, 14, 10, skin);
    fill_rect(r, x+8, y+5, 2, 2, (SDL_Color){34, 28, 28, 255});
    fill_rect(r, x+15,y+5, 2, 2, (SDL_Color){34, 28, 28, 255});
    fill_rect(r, x+3, y+11,18, 10, skin);
    fill_rect(r, x+1, y+12,4, 9, skin);
    fill_rect(r, x+19,y+11,4, 10, skin);
    fill_rect(r, x+7, y+13,10, 8, (SDL_Color){92, 62, 38, 255});
    fill_rect(r, x+4, y+20,6, 3, shade);
    fill_rect(r, x+15,y+20,6, 3, shade);
    fill_rect(r, x+20,y+3, 3, 17, (SDL_Color){92, 58, 30, 255});
    fill_rect(r, x+19,y+2, 5, 6, (SDL_Color){116, 76, 40, 255});
}

void draw_goblin_king(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {82, 128, 38, 255};
    SDL_Color crown = {230, 184, 42, 255};
    SDL_Color cloak = {132, 34, 42, 255};
    fill_rect(r, x+5, y+1, 14, 4, crown);
    fill_rect(r, x+6, y, 3, 3, crown);
    fill_rect(r, x+11,y, 3, 3, crown);
    fill_rect(r, x+16,y, 3, 3, crown);
    fill_rect(r, x+4, y+7, 16, 3, skin);
    fill_rect(r, x+7, y+4, 10, 9, skin);
    fill_rect(r, x+8, y+7, 2, 2, (SDL_Color){250, 56, 36, 255});
    fill_rect(r, x+14,y+7, 2, 2, (SDL_Color){250, 56, 36, 255});
    fill_rect(r, x+3, y+12,18, 9, cloak);
    fill_rect(r, x+7, y+13,10, 8, (SDL_Color){72, 50, 28, 255});
    fill_rect(r, x+5, y+20,5, 3, (SDL_Color){26, 34, 24, 255});
    fill_rect(r, x+15,y+20,5, 3, (SDL_Color){26, 34, 24, 255});
    fill_rect(r, x+20,y+8, 2, 12, (SDL_Color){194, 202, 204, 255});
}

void draw_lich_king(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color robe = {42, 24, 76, 255};
    SDL_Color bone = {208, 204, 170, 255};
    SDL_Color glow = {142, 80, 248, 255};
    fill_rect(r, x+7, y+1, 10, 4, (SDL_Color){92, 76, 120, 255});
    fill_rect(r, x+8, y, 2, 3, bone);
    fill_rect(r, x+14,y, 2, 3, bone);
    fill_rect(r, x+7, y+4, 10, 8, bone);
    fill_rect(r, x+9, y+6, 2, 2, glow);
    fill_rect(r, x+14,y+6, 2, 2, glow);
    fill_rect(r, x+5, y+12,14, 9, robe);
    fill_rect(r, x+3, y+14,4, 8, robe);
    fill_rect(r, x+17,y+14,4, 8, robe);
    fill_rect(r, x+4, y+21,5, 2, robe);
    fill_rect(r, x+15,y+21,5, 2, robe);
    fill_rect(r, x+2, y+7, 2, 14, bone);
    fill_rect(r, x+1, y+3, 4, 5, glow);
    fill_rect(r, x+2, y+1, 2, 3, (SDL_Color){206, 166, 255, 255});
}

void draw_demon_lord(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = {142, 38, 34, 255};
    SDL_Color dark = {54, 20, 26, 255};
    SDL_Color horn = {126, 116, 94, 255};
    fill_rect(r, x, y+7, 5, 12, dark);
    fill_rect(r, x+19,y+7,5, 12, dark);
    fill_rect(r, x+5, y+2, 4, 6, horn);
    fill_rect(r, x+15,y+2,4, 6, horn);
    fill_rect(r, x+7, y, 2, 4, horn);
    fill_rect(r, x+15,y, 2, 4, horn);
    fill_rect(r, x+6, y+5,12, 9, body);
    fill_rect(r, x+8, y+8,2, 2, (SDL_Color){248, 160, 28, 255});
    fill_rect(r, x+14,y+8,2, 2, (SDL_Color){248, 160, 28, 255});
    fill_rect(r, x+4, y+13,16, 8, body);
    fill_rect(r, x+8, y+14,8, 6, dark);
    fill_rect(r, x+4, y+20,6, 3, dark);
    fill_rect(r, x+15,y+20,6, 3, dark);
}

void draw_red_dragon(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = {174, 42, 24, 255};
    SDL_Color dark = {92, 24, 20, 255};
    SDL_Color gold = {224, 154, 42, 255};
    fill_rect(r, x, y+6, 5, 12, dark);
    fill_rect(r, x+19,y+6,5, 12, dark);
    fill_rect(r, x+6, y+3,12, 9, body);
    fill_rect(r, x+7, y+1,3, 4, (SDL_Color){196, 174, 126, 255});
    fill_rect(r, x+14,y+1,3, 4, (SDL_Color){196, 174, 126, 255});
    fill_rect(r, x+8, y+6,2, 2, gold);
    fill_rect(r, x+14,y+6,2, 2, gold);
    fill_rect(r, x+5, y+11,14, 10, body);
    fill_rect(r, x+9, y+12,6, 9, gold);
    fill_rect(r, x+4, y+20,6, 3, dark);
    fill_rect(r, x+15,y+20,6, 3, dark);
    fill_rect(r, x+19,y+18,4, 2, body);
    fill_rect(r, x+21,y+16,2, 3, body);
    fill_rect(r, x+1, y+18,3, 3, (SDL_Color){255, 110, 16, 255});
    fill_rect(r, x, y+20,2, 2, (SDL_Color){255, 210, 54, 255});
}

void draw_tarrasque(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = {78, 58, 36, 255};
    SDL_Color shell = {112, 88, 52, 255};
    SDL_Color spike = {176, 156, 112, 255};
    fill_rect(r, x+3, y+4, 18, 15, shell);
    fill_rect(r, x+5, y+2, 4, 5, spike);
    fill_rect(r, x+10,y, 4, 6, spike);
    fill_rect(r, x+16,y+2,4, 5, spike);
    fill_rect(r, x+5, y+9, 14, 11, body);
    fill_rect(r, x+7, y+9, 3, 2, (SDL_Color){238, 48, 26, 255});
    fill_rect(r, x+15,y+9, 3, 2, (SDL_Color){238, 48, 26, 255});
    fill_rect(r, x+10,y+12,5, 5, (SDL_Color){48, 36, 28, 255});
    fill_rect(r, x, y+14,6, 7, body);
    fill_rect(r, x+18,y+14,6, 7, body);
    fill_rect(r, x+1, y+20,7, 3, spike);
    fill_rect(r, x+16,y+20,7, 3, spike);
}

static void draw_forest_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int x = tx * TILE_SIZE, y = ty * TILE_SIZE;
    SDL_Color dark = {12, 24, 18, 255};
    SDL_Color moss = {52, 92, 42, 255};
    SDL_Color glow = {92, 238, 126, 255};
    SDL_Color pale = {154, 210, 176, 255};
    if (type == ENEMY_PIXIE) {
        fill_rect(r, x+3,y+7,7,9,pale); fill_rect(r,x+14,y+7,7,9,pale);
        fill_rect(r,x+9,y+6,6,13,moss); fill_rect(r,x+10,y+4,4,4,glow);
        fill_rect(r,x+7,y+20,3,2,glow); fill_rect(r,x+15,y+20,3,2,glow);
    } else if (type == ENEMY_BLIGHTED_WOLF) {
        fill_rect(r,x+3,y+10,17,9,dark); fill_rect(r,x+16,y+7,7,9,moss);
        fill_rect(r,x+17,y+5,3,4,dark); fill_rect(r,x+22,y+5,2,4,dark);
        fill_rect(r,x+18,y+10,2,2,glow); fill_rect(r,x+3,y+18,3,5,moss);
        fill_rect(r,x+15,y+18,3,5,moss); fill_rect(r,x,y+8,5,3,moss);
    } else if (type == ENEMY_GIANT_SPIDER) {
        fill_rect(r,x+8,y+6,9,13,dark); fill_rect(r,x+10,y+4,5,5,glow);
        for (int i=0;i<4;i++) { fill_rect(r,x+2,y+5+i*5,7,2,moss); fill_rect(r,x+16,y+5+i*5,7,2,moss); }
        fill_rect(r,x+11,y+8,2,2,pale); fill_rect(r,x+14,y+8,2,2,pale);
    } else if (type == ENEMY_DARK_ELF) {
        fill_rect(r,x+7,y+4,11,8,dark); fill_rect(r,x+9,y+7,7,4,(SDL_Color){118,98,138,255});
        fill_rect(r,x+7,y+12,11,10,moss); fill_rect(r,x+3,y+5,2,17,(SDL_Color){91,58,34,255});
        fill_rect(r,x+1,y+7,5,2,glow); fill_rect(r,x+10,y+8,2,1,glow);
    } else if (type == ENEMY_GIANT_WURM) {
        fill_rect(r,x+6,y+3,13,19,(SDL_Color){73,76,46,255});
        fill_rect(r,x+4,y+4,17,5,moss); fill_rect(r,x+7,y+6,11,8,dark);
        fill_rect(r,x+9,y+8,2,4,pale); fill_rect(r,x+14,y+8,2,4,pale);
        fill_rect(r,x+4,y+17,17,3,(SDL_Color){101,86,48,255});
    } else if (type == ENEMY_FOREST_TROLL) {
        fill_rect(r,x+5,y+6,14,15,(SDL_Color){70,86,65,255});
        fill_rect(r,x+3,y+11,4,11,moss); fill_rect(r,x+18,y+10,4,12,moss);
        fill_rect(r,x+8,y+8,3,2,glow); fill_rect(r,x+14,y+8,3,2,glow);
        fill_rect(r,x+20,y+2,3,20,(SDL_Color){94,59,30,255});
    } else if (type == ENEMY_FOREST_NECROMANCER) {
        fill_rect(r,x+4,y+6,16,16,dark); fill_rect(r,x+7,y+4,10,8,(SDL_Color){29,54,36,255});
        fill_rect(r,x+9,y+8,2,2,glow); fill_rect(r,x+14,y+8,2,2,glow);
        fill_rect(r,x+3,y+2,3,8,(SDL_Color){104,91,63,255});
        fill_rect(r,x+18,y+1,3,9,(SDL_Color){104,91,63,255});
        fill_rect(r,x+21,y+3,2,19,(SDL_Color){76,51,27,255});
        fill_rect(r,x+19,y+1,6,6,glow); fill_rect(r,x+8,y+15,8,5,(SDL_Color){45,83,49,255});
    }
}

static void draw_mountain_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int x=tx*TILE_SIZE, y=ty*TILE_SIZE;
    SDL_Color skin={91,121,45,255}, dark={23,17,19,255};
    SDL_Color rust={128,48,30,255}, iron={103,91,88,255};
    SDL_Color ember={244,67,20,255};
    if (type == ENEMY_TUNNEL_SPIDER) {
        fill_rect(r,x+7,y+7,11,12,iron); fill_rect(r,x+9,y+9,7,7,dark);
        for(int i=0;i<4;i++){fill_rect(r,x+1,y+4+i*5,7,2,rust);fill_rect(r,x+17,y+4+i*5,7,2,rust);}
        fill_rect(r,x+10,y+10,2,2,ember); fill_rect(r,x+14,y+10,2,2,ember);
        return;
    }
    if (type == ENEMY_CAVE_TROLL) {
        fill_rect(r,x+4,y+5,16,17,iron); fill_rect(r,x+7,y+8,3,2,ember);
        fill_rect(r,x+15,y+8,3,2,ember); fill_rect(r,x+1,y+10,5,12,dark);
        fill_rect(r,x+19,y+2,4,21,(SDL_Color){85,62,45,255}); return;
    }
    fill_rect(r,x+6,y+5,13,8,skin); fill_rect(r,x+4,y+7,4,3,skin);
    fill_rect(r,x+17,y+7,4,3,skin); fill_rect(r,x+8,y+8,2,2,ember);
    fill_rect(r,x+15,y+8,2,2,ember); fill_rect(r,x+6,y+13,13,9,rust);
    if (type == ENEMY_GOBLIN_ARCHER) {
        fill_rect(r,x+20,y+4,2,18,(SDL_Color){101,67,34,255});
        fill_rect(r,x+18,y+4,5,2,iron); fill_rect(r,x+18,y+20,5,2,iron);
    } else if (type == ENEMY_GOBLIN_BOMBER) {
        fill_rect(r,x+18,y+2,6,6,dark); fill_rect(r,x+20,y,2,3,ember);
    } else if (type == ENEMY_HOBGOBLIN_GUARD) {
        fill_rect(r,x+1,y+8,7,14,iron); fill_rect(r,x+20,y+1,2,22,iron);
    } else if (type == ENEMY_GOBLIN_SHAMAN) {
        fill_rect(r,x+20,y+2,3,21,(SDL_Color){86,51,27,255});
        fill_rect(r,x+18,y,7,6,ember); fill_rect(r,x+5,y+3,15,3,dark);
    } else if (type == ENEMY_MOUNTAIN_GOBLIN_KING) {
        fill_rect(r,x+4,y+12,17,11,dark); fill_rect(r,x+6,y+1,13,5,iron);
        fill_rect(r,x+7,y,3,4,ember); fill_rect(r,x+15,y,3,4,ember);
        fill_rect(r,x+20,y+5,4,18,(SDL_Color){92,58,32,255});
    } else {
        fill_rect(r,x+20,y+5,3,17,iron); fill_rect(r,x+19,y+3,5,5,iron);
    }
}

static void draw_coast_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    SDL_Color deep = {10, 33, 48, 255};
    SDL_Color aqua = {55, 192, 187, 255};
    SDL_Color pale = {139, 231, 213, 255};
    SDL_Color coral = {214, 78, 70, 255};
    SDL_Color stone = {75, 125, 116, 255};
    if (type == ENEMY_ILLUSION) {
        fill_rect(r, x + 8, y + 4, 9, 7, pale);
        fill_rect(r, x + 6, y + 11, 13, 9, (SDL_Color){91, 81, 181, 255});
        fill_rect(r, x + 4, y + 20, 17, 2, aqua);
    } else if (type == ENEMY_MERFOLK) {
        fill_rect(r, x + 7, y + 4, 11, 9, aqua);
        fill_rect(r, x + 8, y + 13, 9, 7, stone);
        fill_rect(r, x + 4, y + 20, 7, 3, aqua);
        fill_rect(r, x + 15, y + 20, 7, 3, aqua);
        fill_rect(r, x + 20, y + 2, 2, 21, pale);
    } else if (type == ENEMY_SIREN) {
        fill_rect(r, x + 7, y + 4, 11, 8, pale);
        fill_rect(r, x + 5, y + 2, 15, 5, deep);
        fill_rect(r, x + 7, y + 12, 11, 10, (SDL_Color){36, 107, 145, 255});
        fill_rect(r, x + 20, y + 5, 3, 3, aqua);
        fill_rect(r, x + 21, y + 11, 2, 2, aqua);
    } else if (type == ENEMY_GIANT_CRAB) {
        fill_rect(r, x + 5, y + 8, 15, 11, coral);
        fill_rect(r, x, y + 5, 7, 6, coral);
        fill_rect(r, x + 18, y + 5, 6, 6, coral);
        fill_rect(r, x + 2, y + 19, 6, 3, pale);
        fill_rect(r, x + 16, y + 19, 6, 3, pale);
    } else if (type == ENEMY_ANIMATED_STATUE) {
        fill_rect(r, x + 7, y + 3, 11, 8, stone);
        fill_rect(r, x + 5, y + 11, 15, 11, stone);
        fill_rect(r, x + 9, y + 7, 2, 2, pale);
        fill_rect(r, x + 15, y + 7, 2, 2, pale);
        fill_rect(r, x + 12, y + 11, 2, 8, deep);
    } else if (type == ENEMY_WATER_ELEMENTAL) {
        fill_rect(r, x + 8, y + 3, 9, 7, pale);
        fill_rect(r, x + 5, y + 10, 15, 10, aqua);
        fill_rect(r, x + 2, y + 18, 20, 4, (SDL_Color){38, 121, 169, 255});
        fill_rect(r, x + 10, y + 6, 2, 2, deep);
        fill_rect(r, x + 15, y + 6, 2, 2, deep);
    } else if (type == ENEMY_SEA_SERPENT) {
        fill_rect(r, x + 5, y + 15, 17, 7, aqua);
        fill_rect(r, x + 13, y + 5, 8, 13, aqua);
        fill_rect(r, x + 16, y + 3, 7, 7, pale);
        fill_rect(r, x + 20, y + 5, 2, 2, coral);
    } else {
        fill_rect(r, x + 5, y + 9, 15, 13, (SDL_Color){27, 91, 112, 255});
        fill_rect(r, x + 7, y + 4, 11, 8, pale);
        fill_rect(r, x + 6, y + 1, 3, 5, coral);
        fill_rect(r, x + 11, y, 3, 5, coral);
        fill_rect(r, x + 16, y + 1, 3, 5, coral);
        fill_rect(r, x + 21, y + 3, 2, 20, stone);
    }
}

void draw_enemy(Renderer *r, int tile_x, int tile_y, EnemyType type) {
    switch (type) {
        case ENEMY_SKELETON: draw_skeleton(r, tile_x, tile_y); break;
        case ENEMY_GOBLIN:   draw_goblin(r, tile_x, tile_y);   break;
        case ENEMY_ZOMBIE:   draw_zombie(r, tile_x, tile_y);   break;
        case ENEMY_CRYPT_BAT: draw_crypt_bat(r, tile_x, tile_y); break;
        case ENEMY_WRAITH: draw_wraith(r, tile_x, tile_y); break;
        case ENEMY_CRYPT_CONJURER: draw_necromancer(r, tile_x, tile_y); break;
        case ENEMY_PIXIE:
        case ENEMY_BLIGHTED_WOLF:
        case ENEMY_GIANT_SPIDER:
        case ENEMY_DARK_ELF:
        case ENEMY_GIANT_WURM:
        case ENEMY_FOREST_TROLL:
        case ENEMY_FOREST_NECROMANCER:
            draw_forest_enemy(r, tile_x, tile_y, type); break;
        case ENEMY_GOBLIN_SCOUT:
        case ENEMY_GOBLIN_ARCHER:
        case ENEMY_GOBLIN_BOMBER:
        case ENEMY_TUNNEL_SPIDER:
        case ENEMY_CAVE_TROLL:
        case ENEMY_HOBGOBLIN_GUARD:
        case ENEMY_GOBLIN_SHAMAN:
        case ENEMY_MOUNTAIN_GOBLIN_KING:
            draw_mountain_enemy(r, tile_x, tile_y, type); break;
        case ENEMY_ILLUSION:
        case ENEMY_MERFOLK:
        case ENEMY_SIREN:
        case ENEMY_GIANT_CRAB:
        case ENEMY_ANIMATED_STATUE:
        case ENEMY_WATER_ELEMENTAL:
        case ENEMY_SEA_SERPENT:
        case ENEMY_DROWNED_QUEEN:
            draw_coast_enemy(r, tile_x, tile_y, type); break;
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

void draw_tavern_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color oak = {77, 45, 27, 255};
    SDL_Color plank = {102, 59, 31, 255};
    SDL_Color seam = {44, 27, 20, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, oak);
    fill_rect(r, x, y + 2, TILE_SIZE, 9, plank);
    fill_rect(r, x, y + 13, TILE_SIZE, 9, plank);
    fill_rect(r, x, y + 11, TILE_SIZE, 2, seam);
    fill_rect(r, x + 7, y + 2, 2, 9, seam);
    fill_rect(r, x + 17, y + 13, 2, 9, seam);
}

void draw_tavern_wall(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color timber = {50, 29, 22, 255};
    SDL_Color plaster = {122, 84, 52, 255};
    SDL_Color highlight = {152, 105, 62, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, timber);
    fill_rect(r, x + 3, y + 3, 18, 18, plaster);
    fill_rect(r, x + 3, y + 3, 18, 3, highlight);
    fill_rect(r, x + 10, y + 3, 4, 18, timber);
}

void draw_tavern_exit(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    fill_rect(r, x + 3, y, 18, 5, (SDL_Color){38, 23, 18, 255});
    fill_rect(r, x + 5, y + 5, 14, 15, (SDL_Color){92, 50, 27, 255});
    fill_rect(r, x + 8, y + 18, 8, 4, (SDL_Color){151, 46, 34, 255});
    fill_rect(r, x + 16, y + 11, 2, 2, (SDL_Color){235, 186, 70, 255});
}

void draw_tavern_table(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    fill_rect(r, x + 2, y + 5, 20, 14, (SDL_Color){48, 27, 18, 255});
    fill_rect(r, x + 3, y + 3, 18, 13, (SDL_Color){119, 67, 32, 255});
    fill_rect(r, x + 4, y + 4, 16, 3, (SDL_Color){157, 91, 42, 255});
    fill_rect(r, x + 7, y + 16, 3, 6, (SDL_Color){59, 34, 22, 255});
    fill_rect(r, x + 15, y + 16, 3, 6, (SDL_Color){59, 34, 22, 255});
}

void draw_elowen(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    fill_rect(r, x + 6, y + 3, 12, 7, (SDL_Color){225, 216, 190, 255});
    fill_rect(r, x + 8, y + 7, 8, 6, (SDL_Color){202, 154, 119, 255});
    fill_rect(r, x + 6, y + 12, 12, 10, (SDL_Color){48, 76, 132, 255});
    fill_rect(r, x + 4, y + 13, 3, 8, (SDL_Color){67, 101, 165, 255});
    fill_rect(r, x + 17, y + 13, 3, 8, (SDL_Color){67, 101, 165, 255});
    fill_rect(r, x + 10, y + 9, 2, 2, (SDL_Color){44, 37, 51, 255});
    fill_rect(r, x + 14, y + 9, 2, 2, (SDL_Color){44, 37, 51, 255});
    fill_rect(r, x + 20, y + 4, 2, 18, (SDL_Color){94, 61, 38, 255});
    fill_rect(r, x + 18, y + 3, 6, 3, (SDL_Color){112, 194, 210, 255});
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

void draw_town_exit(Renderer *r, int tile_x, int tile_y, TownExitStyle style, int segment) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    if (style == TOWN_EXIT_COAST) {
        draw_coast_floor(r, tile_x, tile_y);
        fill_rect(r, x, y, TILE_SIZE, 6, (SDL_Color){38, 89, 91, 255});
        fill_rect(r, x + 4, y + 6, 16, 18, (SDL_Color){9, 42, 56, 255});
        if (segment == 0 || segment == 4) {
            fill_rect(r, x, y, 5, TILE_SIZE, (SDL_Color){93, 139, 117, 255});
            fill_rect(r, x + 19, y, 5, TILE_SIZE, (SDL_Color){93, 139, 117, 255});
        }
        return;
    }
    if (style == TOWN_EXIT_FOREST) {
        draw_forest_floor(r, tile_x, tile_y);
        SDL_Color trunk = {72, 48, 25, 255};
        SDL_Color leaf = {28, 78, 38, 255};
        SDL_Color dark = {7, 24, 15, 255};
        fill_rect(r, x + 7, y, 17, TILE_SIZE, dark);
        fill_rect(r, x, y, 5, TILE_SIZE, trunk);
        fill_rect(r, x + 3, y, 5, TILE_SIZE, leaf);
        if (segment == 0 || segment == 4) {
            fill_rect(r, x, y, TILE_SIZE, 6, leaf);
            fill_rect(r, x, y + 18, TILE_SIZE, 6, leaf);
        } else {
            fill_rect(r, x + 18, y + 5, 3, 5, (SDL_Color){58, 132, 62, 255});
        }
        return;
    }
    if (style == TOWN_EXIT_MOUNTAINS) {
        draw_mountain_floor(r, tile_x, tile_y);
        SDL_Color basalt = {43, 32, 38, 255};
        SDL_Color ridge = {82, 39, 39, 255};
        SDL_Color ember = {218, 61, 26, 255};
        fill_rect(r, x, y, 7, TILE_SIZE, basalt);
        fill_rect(r, x + 7, y + 4, 17, 20, (SDL_Color){18, 13, 17, 255});
        fill_rect(r, x + 5, y, 3, TILE_SIZE, ridge);
        if (segment == 0 || segment == 4) {
            fill_rect(r, x, y, TILE_SIZE, 6, basalt);
            fill_rect(r, x, y + 18, TILE_SIZE, 6, basalt);
        } else {
            fill_rect(r, x + 10, y + 18, 7, 2, ember);
        }
        return;
    }

    SDL_Color stone = {69, 66, 91, 255};
    SDL_Color highlight = {99, 94, 125, 255};
    SDL_Color shadow = {18, 17, 32, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, shadow);
    fill_rect(r, x, y, TILE_SIZE, 5, stone);
    fill_rect(r, x, y, 3, TILE_SIZE, stone);
    fill_rect(r, x + 21, y, 3, TILE_SIZE, stone);
    if (segment == 0 || segment == 4) {
        fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, stone);
        fill_rect(r, x + 4, y + 3, 16, 4, highlight);
    } else {
        fill_rect(r, x + 2, y + 3, 20, 3, highlight);
        if (segment == 2) {
            fill_rect(r, x + 9, y + 5, 7, 5, (SDL_Color){188, 151, 54, 255});
        }
    }
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

void draw_tavern(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outline = {24, 16, 14, 255};
    SDL_Color plaster = {128, 91, 58, 255};
    SDL_Color plaster_hi = {158, 112, 67, 255};
    SDL_Color timber = {66, 38, 24, 255};
    SDL_Color timber_hi = {103, 58, 30, 255};
    SDL_Color roof = {52, 33, 35, 255};
    SDL_Color roof_hi = {82, 48, 43, 255};
    SDL_Color window = {255, 177, 54, 255};
    SDL_Color glow = {255, 220, 105, 255};
    SDL_Color stone = {76, 70, 67, 255};

    // Stone foundation and broad half-timbered facade.
    fill_rect(r, x + 4, y + 101, 160, 15, outline);
    fill_rect(r, x + 7, y + 104, 154, 10, stone);
    fill_rect(r, x + 9, y + 55, 150, 51, outline);
    fill_rect(r, x + 13, y + 59, 142, 43, plaster);
    fill_rect(r, x + 13, y + 59, 142, 5, plaster_hi);
    fill_rect(r, x + 13, y + 78, 142, 5, timber);
    for (int beam_x = 16; beam_x < 154; beam_x += 28) {
        fill_rect(r, x + beam_x, y + 59, 5, 45, timber);
    }

    // Deep shingled roof and off-center chimney.
    fill_rect(r, x + 2, y + 29, 164, 34, outline);
    fill_rect(r, x + 7, y + 25, 154, 34, roof);
    fill_rect(r, x + 14, y + 20, 140, 8, roof_hi);
    for (int row = 0; row < 3; row++) {
        int roof_y = y + 31 + row * 9;
        int offset = row % 2 == 0 ? 0 : 10;
        fill_rect(r, x + 9, roof_y + 6, 150, 2, outline);
        for (int shingle_x = 14 + offset; shingle_x < 156;
            shingle_x += 20) {
            fill_rect(r, x + shingle_x, roof_y, 2, 7, outline);
        }
    }
    fill_rect(r, x + 29, y + 5, 22, 31, outline);
    fill_rect(r, x + 33, y + 8, 14, 27, (SDL_Color){91, 55, 43, 255});
    fill_rect(r, x + 30, y + 4, 20, 6, stone);

    // Warm leaded windows.
    for (int window_x = 28; window_x <= 124; window_x += 96) {
        fill_rect(r, x + window_x - 4, y + 68, 30, 25, outline);
        fill_rect(r, x + window_x, y + 72, 22, 17, window);
        fill_rect(r, x + window_x + 10, y + 72, 3, 17, timber);
        fill_rect(r, x + window_x, y + 79, 22, 3, timber);
        fill_rect(r, x + window_x + 3, y + 74, 5, 4, glow);
    }

    // Elowen waits inside the left window in a pale hood and blue robes.
    fill_rect(r, x + 32, y + 72, 12, 5, (SDL_Color){224, 216, 190, 255});
    fill_rect(r, x + 34, y + 76, 8, 5, (SDL_Color){201, 154, 119, 255});
    fill_rect(r, x + 31, y + 81, 14, 8, (SDL_Color){52, 81, 135, 255});
    fill_rect(r, x + 36, y + 78, 2, 2, (SDL_Color){45, 38, 55, 255});
    fill_rect(r, x + 40, y + 78, 2, 2, (SDL_Color){45, 38, 55, 255});

    // Central oak door with an iron handle and lit transom.
    fill_rect(r, x + 68, y + 65, 34, 49, outline);
    fill_rect(r, x + 73, y + 70, 24, 44, timber);
    fill_rect(r, x + 77, y + 74, 16, 8, window);
    fill_rect(r, x + 77, y + 85, 16, 3, timber_hi);
    fill_rect(r, x + 77, y + 96, 16, 3, timber_hi);
    fill_rect(r, x + 91, y + 91, 3, 3, glow);

    // Hanging tavern sign with a simple golden mug emblem.
    fill_rect(r, x + 135, y + 45, 4, 24, timber);
    fill_rect(r, x + 126, y + 46, 13, 4, timber);
    fill_rect(r, x + 114, y + 49, 24, 20, outline);
    fill_rect(r, x + 117, y + 52, 18, 14, timber_hi);
    fill_rect(r, x + 121, y + 55, 8, 8, glow);
    fill_rect(r, x + 129, y + 56, 4, 6, glow);
    fill_rect(r, x + 123, y + 53, 4, 3, window);
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
    SDL_Color floor = {18, 18, 35, 255};
    SDL_Color rim = {64, 62, 82, 255};
    SDL_Color recess = {28, 27, 42, 255};
    SDL_Color steel = {152, 158, 170, 255};
    SDL_Color shine = {214, 218, 220, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, floor);
    fill_rect(r, x+3, y+4, 18, 17, rim);
    fill_rect(r, x+5, y+6, 14, 13, recess);
    fill_rect(r, x+4, y+19, 16, 2, (SDL_Color){38, 36, 54, 255});

    fill_rect(r, x+6,  y+12, 3, 7, steel);
    fill_rect(r, x+7,  y+8,  1, 4, shine);
    fill_rect(r, x+11, y+9,  3, 10, steel);
    fill_rect(r, x+12, y+5,  1, 4, shine);
    fill_rect(r, x+16, y+12, 3, 7, steel);
    fill_rect(r, x+17, y+8,  1, 4, shine);
    fill_rect(r, x+7,  y+7,  1, 1, shine);
    fill_rect(r, x+12, y+4,  1, 1, shine);
    fill_rect(r, x+17, y+7,  1, 1, shine);
}

void draw_trap_fire(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color floor = {18, 18, 35, 255};
    SDL_Color iron = {58, 54, 66, 255};
    SDL_Color soot = {34, 25, 30, 255};
    SDL_Color red = {174, 42, 20, 255};
    SDL_Color orange = {244, 98, 20, 255};
    SDL_Color yellow = {255, 204, 62, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, floor);
    fill_rect(r, x+3, y+15, 18, 6, iron);
    fill_rect(r, x+5, y+17, 14, 3, soot);
    fill_rect(r, x+5, y+14, 14, 2, (SDL_Color){92, 76, 70, 255});
    fill_rect(r, x+7, y+10, 11, 7, red);
    fill_rect(r, x+9, y+6, 7, 11, orange);
    fill_rect(r, x+12,y+3, 3, 8, orange);
    fill_rect(r, x+7, y+8, 3, 7, red);
    fill_rect(r, x+11,y+10,4, 7, yellow);
    fill_rect(r, x+12,y+7, 2, 5, (SDL_Color){255, 238, 150, 255});
    fill_rect(r, x+6, y+4, 2, 2, (SDL_Color){116, 52, 34, 255});
    fill_rect(r, x+18,y+7, 2, 2, (SDL_Color){206, 70, 24, 255});
}

void draw_trap_poison(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color floor = {18, 18, 35, 255};
    SDL_Color rim = {54, 62, 70, 255};
    SDL_Color recess = {24, 34, 34, 255};
    SDL_Color toxic = {54, 174, 70, 255};
    SDL_Color glow = {122, 236, 92, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, floor);
    fill_rect(r, x+3, y+4, 18, 17, rim);
    fill_rect(r, x+5, y+6, 14, 13, recess);
    fill_rect(r, x+6, y+7, 3, 3, toxic);
    fill_rect(r, x+15,y+7, 3, 3, toxic);
    fill_rect(r, x+8, y+9, 8, 7, toxic);
    fill_rect(r, x+9, y+10,2, 2, glow);
    fill_rect(r, x+14,y+10,2, 2, glow);
    fill_rect(r, x+11,y+13,3, 2, recess);
    fill_rect(r, x+8, y+16,3, 3, toxic);
    fill_rect(r, x+12,y+16,2, 3, toxic);
    fill_rect(r, x+15,y+16,3, 3, toxic);
    fill_rect(r, x+4, y+2, 2, 2, glow);
    fill_rect(r, x+18,y+1, 2, 2, toxic);
    fill_rect(r, x+20,y+5, 1, 1, glow);
}
