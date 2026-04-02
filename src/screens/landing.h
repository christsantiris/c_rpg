#ifndef LANDING_HEADER_H
#define LANDING_HEADER_H

typedef enum {
    SCREEN_LANDING,
    SCREEN_NAME_ENTRY,
    SCREEN_PLAYING,
    SCREEN_GAME_OVER,
    SCREEN_SAVE_SLOT,
    SCREEN_LOAD_SLOT,
    SCREEN_INVENTORY,
    SCREEN_SPELLBOOK
} GameScreen;

typedef enum {
    LANDING_NONE = 0,
    LANDING_NEW_GAME,
    LANDING_LOAD_GAME,
    LANDING_CONTINUE,
    LANDING_QUIT,
    LANDING_SAVE_GAME
} LandingResult;

typedef struct {
    int selected;
    int has_active_game;
    int confirming_new_game;
} LandingScreen;

void          landing_init(LandingScreen *s);
LandingResult landing_handle_key(LandingScreen *s, int scancode);
LandingResult landing_handle_click(LandingScreen *s, int mouse_x, int mouse_y, int screen_w, int screen_h);
int landing_item_count(const LandingScreen *s);
LandingResult landing_handle_confirm(LandingScreen *s, int scancode);

#endif