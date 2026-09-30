#include "sfx.h"
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define SFX_ATTACK "assets/sfx/smallblade.ogg"
#define SFX_LARGE_BLADE "assets/sfx/largeblade.ogg"
#define SFX_GREATSWORD "assets/sfx/greatsword.mp3"
#define SFX_ARROW  "assets/sfx/arrow.mp3"
#define SFX_MAGIC_ARROW "assets/sfx/magicArrow.mp3"
#define SFX_FIREBALL "assets/sfx/fireball.mp3"
#define SFX_HEAL "assets/sfx/heal.mp3"

static Mix_Chunk *sfx_attack = NULL;
static Mix_Chunk *sfx_large_blade = NULL;
static Mix_Chunk *sfx_greatsword = NULL;
static Mix_Chunk *sfx_arrow = NULL;
static Mix_Chunk *sfx_magic_arrow = NULL;
static Mix_Chunk *sfx_fireball = NULL;
static Mix_Chunk *sfx_heal = NULL;
static int sfx_on = 1;

void sfx_init(void) {
    sfx_attack = Mix_LoadWAV(SFX_ATTACK);
    if (!sfx_attack) {
        fprintf(stderr, "Failed to load attack sfx: %s\n", Mix_GetError());
    }
    sfx_large_blade = Mix_LoadWAV(SFX_LARGE_BLADE);
    if (!sfx_large_blade) {
        fprintf(stderr, "Failed to load large blade sfx: %s\n", Mix_GetError());
    }
    sfx_greatsword = Mix_LoadWAV(SFX_GREATSWORD);
    if (!sfx_greatsword) {
        fprintf(stderr, "Failed to load greatsword sfx: %s\n", Mix_GetError());
    }
    sfx_arrow = Mix_LoadWAV(SFX_ARROW);
    if (!sfx_arrow) {
        fprintf(stderr, "Failed to load arrow sfx: %s\n", Mix_GetError());
    }
    sfx_magic_arrow = Mix_LoadWAV(SFX_MAGIC_ARROW);
    if (!sfx_magic_arrow) {
        fprintf(stderr, "Failed to load magic arrow sfx: %s\n", Mix_GetError());
    }
    sfx_fireball = Mix_LoadWAV(SFX_FIREBALL);
    if (!sfx_fireball) {
        fprintf(stderr, "Failed to load fireball sfx: %s\n", Mix_GetError());
    }
    sfx_heal = Mix_LoadWAV(SFX_HEAL);
    if (!sfx_heal) {
        fprintf(stderr, "Failed to load heal sfx: %s\n", Mix_GetError());
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

void sfx_play_fireball(void) {
    if (!sfx_fireball || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_fireball, 0);
}

void sfx_play_heal(void) {
    if (!sfx_heal || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_heal, 0);
}

void sfx_play_attack(void) {
    if (!sfx_attack || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_attack, 0);
}

void sfx_play_large_blade(void) {
    if (!sfx_large_blade || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_large_blade, 0);
}

void sfx_play_greatsword(void) {
    if (!sfx_greatsword || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_greatsword, 0);
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
    if (sfx_large_blade) {
        Mix_FreeChunk(sfx_large_blade);
    }
    if (sfx_greatsword) {
        Mix_FreeChunk(sfx_greatsword);
    }
    if (sfx_arrow) {
        Mix_FreeChunk(sfx_arrow);
    }
    if (sfx_magic_arrow) {
        Mix_FreeChunk(sfx_magic_arrow);
    }
    if (sfx_fireball) {
        Mix_FreeChunk(sfx_fireball);
    }
    if (sfx_heal) {
        Mix_FreeChunk(sfx_heal);
    }
    sfx_attack = NULL;
    sfx_large_blade = NULL;
    sfx_greatsword = NULL;
    sfx_arrow = NULL;
    sfx_magic_arrow = NULL;
    sfx_fireball = NULL;
    sfx_heal = NULL;
}
