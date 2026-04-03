#include "music.h"
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define TOWN_MUSIC    "assets/music/Woodland Fantasy.mp3"
#define DUNGEON_MUSIC "assets/music/the_march_upon_the_red_mountain.mp3"

static Mix_Music *town_track   = NULL;
static Mix_Music *dungeon_track = NULL;
static GameScreen last_screen  = -1;

void music_init(void) {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer error: %s\n", Mix_GetError());
        return;
    }
    town_track    = Mix_LoadMUS(TOWN_MUSIC);
    dungeon_track = Mix_LoadMUS(DUNGEON_MUSIC);
    if (!town_track)
        fprintf(stderr, "Failed to load town music: %s\n", Mix_GetError());
    if (!dungeon_track)
        fprintf(stderr, "Failed to load dungeon music: %s\n", Mix_GetError());
}

void music_update(GameScreen screen, int dungeon_level) {
    if (screen == last_screen) return;
    last_screen = screen;

    if (screen == SCREEN_PLAYING) {
        Mix_Music *track = dungeon_track;
        // Use town track when in town
        if (dungeon_level == 0) track = town_track;
        if (track) {
            Mix_HaltMusic();
            Mix_PlayMusic(track, -1);
        }
    } else if (screen == SCREEN_LANDING ||
               screen == SCREEN_NAME_ENTRY) {
        if (town_track) {
            Mix_HaltMusic();
            Mix_PlayMusic(town_track, -1);
        }
    }
}

void music_free(void) {
    Mix_HaltMusic();
    if (town_track)    Mix_FreeMusic(town_track);
    if (dungeon_track) Mix_FreeMusic(dungeon_track);
    Mix_CloseAudio();
}