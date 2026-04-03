#include "music.h"
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define TOWN_MUSIC    "assets/music/Woodland Fantasy.mp3"
#define DUNGEON_MUSIC "assets/music/the_march_upon_the_red_mountain.mp3"

static Mix_Music *town_track    = NULL;
static Mix_Music *dungeon_track = NULL;
static Mix_Music *current_track = NULL;

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

static void play_track(Mix_Music *track) {
    if (!track) return;
    if (track == current_track) return;
    current_track = track;
    Mix_HaltMusic();
    Mix_PlayMusic(track, -1);
}

void music_update(GameScreen screen, int in_town) {
    if (screen == SCREEN_PLAYING) {
        if (in_town)
            play_track(town_track);
        else
            play_track(dungeon_track);
    } else if (screen == SCREEN_LANDING ||
               screen == SCREEN_NAME_ENTRY) {
        play_track(town_track);
    }
    // All other screens (shop, inventory, spellbook) keep current track
}

void music_free(void) {
    Mix_HaltMusic();
    if (town_track)    Mix_FreeMusic(town_track);
    if (dungeon_track) Mix_FreeMusic(dungeon_track);
    Mix_CloseAudio();
}