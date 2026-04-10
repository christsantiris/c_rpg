#include "sfx.h"
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define SFX_ATTACK "assets/sfx/knifeSlice2.ogg"

static Mix_Chunk *sfx_attack = NULL;

void sfx_init(void) {
    sfx_attack = Mix_LoadWAV(SFX_ATTACK);
    if (!sfx_attack) {
        fprintf(stderr, "Failed to load attack sfx: %s\n", Mix_GetError());
    }
}

void sfx_play_attack(void) {
    if (!sfx_attack) {
        return;
    }
    Mix_PlayChannel(-1, sfx_attack, 0);
}

void sfx_free(void) {
    if (sfx_attack) {
        Mix_FreeChunk(sfx_attack);
    }
    sfx_attack = NULL;
}
