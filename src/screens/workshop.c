#include "workshop.h"

int workshop_visible_rows(int height) {
    int rows = (height - 260) / 32;
    return rows > 0 ? rows : 1;
}

int workshop_list_start(int selected, int rows) {
    return selected >= rows ? selected - rows + 1 : 0;
}

SDL_Rect workshop_button_rect(int width, int height) {
    return (SDL_Rect){width / 2 - 180, height - 96, 360, 32};
}

WorkshopResult workshop_handle_key(WorkshopScreen *s, int key, int count) {
    if (key == SDL_SCANCODE_ESCAPE) {
        return WORKSHOP_CLOSED;
    }
    if (count <= 0) {
        s->selected = 0;
        return WORKSHOP_NONE;
    }
    if (key == SDL_SCANCODE_UP && s->selected > 0) {
        s->selected--;
    } else if (key == SDL_SCANCODE_DOWN && s->selected < count - 1) {
        s->selected++;
    } else if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER) {
        return WORKSHOP_SHARPEN;
    }
    return WORKSHOP_NONE;
}

WorkshopResult workshop_handle_click(WorkshopScreen *s, int x, int y, int width, int height, int count) {
    SDL_Rect button = workshop_button_rect(width, height);
    if (x >= button.x && x < button.x + button.w && y >= button.y && y < button.y + button.h && count > 0) {
        return WORKSHOP_SHARPEN;
    }
    int rows = workshop_visible_rows(height);
    int start = workshop_list_start(s->selected, rows);
    if (x >= width / 2 - 300 && x < width / 2 + 300 && y >= 150 && y < 150 + rows * 32) {
        int index = start + (y - 150) / 32;
        if (index < count) {
            s->selected = index;
        }
    }
    return WORKSHOP_NONE;
}
