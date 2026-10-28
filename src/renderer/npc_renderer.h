#ifndef NPC_RENDERER_H
#define NPC_RENDERER_H

#include "renderer.h"

// Rendering identities only; these are not saved actor IDs.
typedef enum {
    NPC_ART_ELOWEN, NPC_ART_DAIN, NPC_ART_GARRICK, NPC_ART_ALDER,
    NPC_ART_WARDEN, NPC_ART_MARA, NPC_ART_CAIN, NPC_ART_ROWAN,
    NPC_ART_NAHLA, NPC_ART_ROOK, NPC_ART_BRAM_INN, NPC_ART_BRAM_TOWN,
    NPC_ART_MIRA, NPC_ART_ILYA, NPC_ART_GUARD, NPC_ART_ZARA,
    NPC_ART_HADRIN, NPC_ART_BRENNA, NPC_ART_FEN, NPC_ART_ORIN,
    NPC_ART_OSWIN, NPC_ART_INFORMANT, NPC_ART_TOMAS,
    NPC_ART_NESSA, NPC_ART_HOLT, NPC_ART_EDRIC, NPC_ART_SURI,
    NPC_ART_VEYRA, NPC_ART_SELENE, NPC_ART_ALDRIC, NPC_ART_TESSA, NPC_ART_LYSA, NPC_ART_MORWEN, NPC_ART_TAMSIN, NPC_ART_SEREN, NPC_ART_EDDA, NPC_ART_COUNT
} NpcArt;

// Transparent character overlay. Use game_renderer.c's draw_npc_ground for town
// tiles so the location supplies paving or grass independently of the character.
void npc_draw(Renderer *r, int tx, int ty, NpcArt art, int mirror);

#endif
