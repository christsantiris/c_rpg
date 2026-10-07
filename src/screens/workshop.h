#ifndef WORKSHOP_HEADER_H
#define WORKSHOP_HEADER_H

#include <SDL2/SDL.h>

typedef enum {
    WORKSHOP_NONE,
    WORKSHOP_CLOSED,
    WORKSHOP_SHARPEN
} WorkshopResult;

typedef struct {
    int selected;
} WorkshopScreen;

WorkshopResult workshop_handle_key(WorkshopScreen *s, int key, int count);
WorkshopResult workshop_handle_click(WorkshopScreen *s, int x, int y, int width, int height, int count);
int workshop_visible_rows(int height);
int workshop_list_start(int selected, int rows);
SDL_Rect workshop_button_rect(int width, int height);

#endif
