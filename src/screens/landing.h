#ifndef LANDING_HEADER_H
#define LANDING_HEADER_H

typedef enum {
    SCREEN_LANDING,
    SCREEN_NAME_ENTRY,
    SCREEN_PLAYING
} GameScreen;

typedef enum {
    LANDING_NONE = 0,
    LANDING_NEW_GAME,
    LANDING_LOAD_GAME,
    LANDING_QUIT
} LandingResult;

typedef struct {
    int selected;
} LandingScreen;

void          landing_init(LandingScreen *s);
LandingResult landing_handle_key(LandingScreen *s, int scancode);
LandingResult landing_handle_click(LandingScreen *s, int mouse_x, int mouse_y,
                                   int screen_w, int screen_h);

#endif