#include "sfx.h"
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define SFX_ATTACK "assets/sfx/knifeSlice2.ogg"
#define SFX_ARROW  "assets/sfx/arrow.mp3"
#define SFX_MAGIC_ARROW "assets/sfx/magicArrow.mp3"

static Mix_Chunk *sfx_attack = NULL;
static Mix_Chunk *sfx_arrow = NULL;
static Mix_Chunk *sfx_magic_arrow = NULL;
static int sfx_on = 1;

void sfx_init(void) {
    sfx_attack = Mix_LoadWAV(SFX_ATTACK);
    if (!sfx_attack) {
        fprintf(stderr, "Failed to load attack sfx: %s\n", Mix_GetError());
    }
    sfx_arrow = Mix_LoadWAV(SFX_ARROW);
    if (!sfx_arrow) {
        fprintf(stderr, "Failed to load arrow sfx: %s\n", Mix_GetError());
    }
    sfx_magic_arrow = Mix_LoadWAV(SFX_MAGIC_ARROW);
    if (!sfx_magic_arrow) {
        fprintf(stderr, "Failed to load magic arrow sfx: %s\n", Mix_GetError());
    }
}

void sfx_play_arrow(void) {
    if (!sfx_arrow || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_arrow, 0);
}

void sfx_play_magic_arrow(void) {
    if (!sfx_magic_arrow || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_magic_arrow, 0);
}

void sfx_play_attack(void) {
    if (!sfx_attack || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_attack, 0);
}

void sfx_toggle(void) {
    sfx_on = !sfx_on;
}

int sfx_enabled(void) {
    return sfx_on;
}

void sfx_free(void) {
    if (sfx_attack) {
        Mix_FreeChunk(sfx_attack);
    }
    if (sfx_arrow) {
        Mix_FreeChunk(sfx_arrow);
    }
    if (sfx_magic_arrow) {
        Mix_FreeChunk(sfx_magic_arrow);
    }
    sfx_attack = NULL;
    sfx_arrow = NULL;
    sfx_magic_arrow = NULL;
}
