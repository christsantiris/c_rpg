#include "sfx.h"
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define SFX_ATTACK "assets/sfx/smallblade.ogg"
#define SFX_LARGE_BLADE "assets/sfx/largeblade.ogg"
#define SFX_MAGIC_SWORD "assets/sfx/studio-sword.mp3"
#define SFX_DEMONIC_SWORD "assets/sfx/swordsound1.mp3"
#define SFX_STAFF "assets/sfx/staffswing.mp3"
#define SFX_MAGIC_DAGGER "assets/sfx/slice1.mp3"
#define SFX_MAGIC_AXE "assets/sfx/magicsword.mp3"
#define SFX_AXE "assets/sfx/axehit.mp3"
#define SFX_PUNCH "assets/sfx/punch.mp3"
#define SFX_ARROW  "assets/sfx/arrow.mp3"
#define SFX_MAGIC_ARROW "assets/sfx/magicArrow.mp3"
#define SFX_FROST_BOLT "assets/sfx/icemagic.mp3"
#define SFX_FIREBALL "assets/sfx/fireball.mp3"
#define SFX_HEAL "assets/sfx/heal.mp3"

static Mix_Chunk *sfx_attack = NULL;
static Mix_Chunk *sfx_large_blade = NULL;
static Mix_Chunk *sfx_magic_sword = NULL;
static Mix_Chunk *sfx_demonic_sword = NULL;
static Mix_Chunk *sfx_staff = NULL;
static Mix_Chunk *sfx_magic_dagger = NULL;
static Mix_Chunk *sfx_magic_axe = NULL;
static Mix_Chunk *sfx_axe = NULL;
static Mix_Chunk *sfx_punch = NULL;
static Mix_Chunk *sfx_arrow = NULL;
static Mix_Chunk *sfx_magic_arrow = NULL;
static Mix_Chunk *sfx_frost_bolt = NULL;
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
    sfx_magic_sword = Mix_LoadWAV(SFX_MAGIC_SWORD);
    if (!sfx_magic_sword) {
        fprintf(stderr, "Failed to load magic sword sfx: %s\n", Mix_GetError());
    }
    sfx_demonic_sword = Mix_LoadWAV(SFX_DEMONIC_SWORD);
    if (!sfx_demonic_sword) {
        fprintf(stderr, "Failed to load demonic sword sfx: %s\n", Mix_GetError());
    }
    sfx_staff = Mix_LoadWAV(SFX_STAFF);
    if (!sfx_staff) {
        fprintf(stderr, "Failed to load staff sfx: %s\n", Mix_GetError());
    }
    sfx_magic_dagger = Mix_LoadWAV(SFX_MAGIC_DAGGER);
    if (!sfx_magic_dagger) {
        fprintf(stderr, "Failed to load magic dagger sfx: %s\n", Mix_GetError());
    }
    sfx_magic_axe = Mix_LoadWAV(SFX_MAGIC_AXE);
    if (!sfx_magic_axe) {
        fprintf(stderr, "Failed to load magic axe sfx: %s\n", Mix_GetError());
    }
    sfx_axe = Mix_LoadWAV(SFX_AXE);
    if (!sfx_axe) {
        fprintf(stderr, "Failed to load axe sfx: %s\n", Mix_GetError());
    }
    sfx_punch = Mix_LoadWAV(SFX_PUNCH);
    if (!sfx_punch) {
        fprintf(stderr, "Failed to load punch sfx: %s\n", Mix_GetError());
    }
    sfx_arrow = Mix_LoadWAV(SFX_ARROW);
    if (!sfx_arrow) {
        fprintf(stderr, "Failed to load arrow sfx: %s\n", Mix_GetError());
    }
    sfx_magic_arrow = Mix_LoadWAV(SFX_MAGIC_ARROW);
    if (!sfx_magic_arrow) {
        fprintf(stderr, "Failed to load magic arrow sfx: %s\n", Mix_GetError());
    }
    sfx_frost_bolt = Mix_LoadWAV(SFX_FROST_BOLT);
    if (!sfx_frost_bolt) {
        fprintf(stderr, "Failed to load frost bolt sfx: %s\n", Mix_GetError());
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

void sfx_play_frost_bolt(void) {
    if (!sfx_frost_bolt || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_frost_bolt, 0);
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

void sfx_play_magic_sword(void) {
    if (!sfx_magic_sword || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_magic_sword, 0);
}

void sfx_play_demonic_sword(void) {
    if (!sfx_demonic_sword || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_demonic_sword, 0);
}

void sfx_play_staff(void) {
    if (!sfx_staff || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_staff, 0);
}

void sfx_play_magic_dagger(void) {
    if (!sfx_magic_dagger || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_magic_dagger, 0);
}

void sfx_play_magic_axe(void) {
    if (!sfx_magic_axe || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_magic_axe, 0);
}

void sfx_play_axe(void) {
    if (!sfx_axe || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_axe, 0);
}

void sfx_play_punch(void) {
    if (!sfx_punch || !sfx_on) {
        return;
    }
    Mix_PlayChannel(-1, sfx_punch, 0);
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
    if (sfx_magic_sword) {
        Mix_FreeChunk(sfx_magic_sword);
    }
    if (sfx_demonic_sword) {
        Mix_FreeChunk(sfx_demonic_sword);
    }
    if (sfx_staff) {
        Mix_FreeChunk(sfx_staff);
    }
    if (sfx_magic_dagger) {
        Mix_FreeChunk(sfx_magic_dagger);
    }
    if (sfx_magic_axe) {
        Mix_FreeChunk(sfx_magic_axe);
    }
    if (sfx_axe) {
        Mix_FreeChunk(sfx_axe);
    }
    if (sfx_punch) {
        Mix_FreeChunk(sfx_punch);
    }
    if (sfx_arrow) {
        Mix_FreeChunk(sfx_arrow);
    }
    if (sfx_magic_arrow) {
        Mix_FreeChunk(sfx_magic_arrow);
    }
    if (sfx_frost_bolt) {
        Mix_FreeChunk(sfx_frost_bolt);
    }
    if (sfx_fireball) {
        Mix_FreeChunk(sfx_fireball);
    }
    if (sfx_heal) {
        Mix_FreeChunk(sfx_heal);
    }
    sfx_attack = NULL;
    sfx_large_blade = NULL;
    sfx_magic_sword = NULL;
    sfx_demonic_sword = NULL;
    sfx_staff = NULL;
    sfx_magic_dagger = NULL;
    sfx_magic_axe = NULL;
    sfx_axe = NULL;
    sfx_punch = NULL;
    sfx_arrow = NULL;
    sfx_magic_arrow = NULL;
    sfx_frost_bolt = NULL;
    sfx_fireball = NULL;
    sfx_heal = NULL;
}
