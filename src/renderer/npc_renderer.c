#include "npc_renderer.h"

// Native pixel clusters; dots preserve the caller's terrain.
// v/A/a cloth, V/B/b headwear, T/t/h hair, F/f/s skin, o outline.
// p/P apron, W/w linen or fur, l/L leather, m/M/N metal, r/R trim.
enum { NPC_HAIR, NPC_BEARD, NPC_HOOD, NPC_CAP, NPC_HAT, NPC_HELM, NPC_BAKER, NPC_WINTER, NPC_WRAP };
static const char npc_heads[9][10][25] = {
    { // HAIR
        "........................",
        "........................",
        "........oooooo..........",
        ".......oTttTtho.........",
        "......oTtthhhtho........",
        "......oThFfffsho........",
        "......ohFffffsho........",
        "......ohfeffefho........",
        ".......ofFffso..........",
        "........osfso...........",
    },
    { // BEARD
        "........................",
        "........................",
        "........oooooo..........",
        ".......oTttTtho.........",
        "......oTthhhhtho........",
        "......ohFfffftho........",
        "......ohfeffefho........",
        "......ohfFffstho........",
        ".......oTttTtho.........",
        "........oTttho..........",
    },
    { // HOOD
        "........................",
        "...........oo...........",
        ".........oVVbo..........",
        "........oVVBBbo.........",
        ".......oVBBBBbbo........",
        "......oVBooooBbbo.......",
        "......oVboFffsbbo.......",
        "......oBbfefesbbo.......",
        "......obbFffsbbo........",
        ".......obbssbbo.........",
    },
    { // CAP
        "........................",
        "........................",
        "........ooooooo.........",
        ".......oVVBBBbbo........",
        "......oVBBBBBBbbo.......",
        "......oBBBBBBbbbbbo.....",
        ".......ohFfffsho........",
        ".......ohfefeso.........",
        ".......ohFffssho........",
        "........osfso...........",
    },
    { // HAT
        "........................",
        "........oooooo..........",
        ".......oVVBBBbo.........",
        "......oVVBBBBBbo........",
        "......oBGGGGGGbbo.......",
        "....oVBBBBBBBBBbbbo.....",
        ".......ohFfffsho........",
        ".......ohfefeso.........",
        ".......ohFffssho........",
        "........osfso...........",
    },
    { // HELM
        "..........rro...........",
        ".........rrro...........",
        ".......oNNMMmo..........",
        "......oNMMMMMmmo........",
        ".....oNMMMMMMMmmo.......",
        ".....omooooooooo........",
        "......omFffffsmo........",
        "......omfeffefmo........",
        "......omfFffssmo........",
        ".......omsssmo..........",
    },
    { // BAKER
        "........................",
        "........oooooo..........",
        "......oWWWWWWWWo........",
        ".....oWWWwWWwwwwwo......",
        ".....oWWWWWWwwwwwo......",
        "......owwwwwwwwo........",
        ".......ohFfffsho........",
        ".......ohfefeso.........",
        ".......ohFffssho........",
        "........osfso...........",
    },
    { // WINTER
        "........................",
        "........................",
        ".........oooooo.........",
        ".......oVVBBBbbo........",
        "......oVBBBBBBbbo.......",
        ".....oWwWWwwWwwbbo......",
        ".....oWboFffsbwwbo......",
        ".....owbfefesbwwo.......",
        "......owbFffsbwo........",
        ".......owbssbwo.........",
    },
    { // WRAP
        "........................",
        "........oooooo..........",
        ".......oVVBBBbo.........",
        "......oVBBVVBBbbo.......",
        "......oBBVVBBBbbbo......",
        "......obGGBBBbbbo.......",
        ".......ohFfffsho........",
        ".......ohfefeso.........",
        ".......ohFffssho........",
        "........osfso...........",
    },
};

enum { NPC_COAT, NPC_ROBE, NPC_APRON, NPC_DRESS, NPC_ARMOR, NPC_HUNCHED, NPC_FUR, NPC_FORMAL };
static const char npc_bodies[8][14][25] = {
    { // COAT
        ".....ovvAoWwovAao.......",
        "....ovvAAoWoAvAAao......",
        "...ovvAAAvovAAAAaao.....",
        "...ovAAoAvoAAAAoAao.....",
        "...ovAAoAvoAAAAoAao.....",
        "...oFFooAvGAAAoosso.....",
        "....oooAvlllLlaooo......",
        "......ovvAoAAaao........",
        "......ovAAoAAaao........",
        "......ovAAoAAaao........",
        ".......oAaoAao..........",
        ".......oLkoLko..........",
        "......okLkolLkko........",
        "......oooo.ooooo........",
    },
    { // ROBE
        ".....ovvAoWwovAao.......",
        "....ovvAAAorAvAAao......",
        "...ovvAAAAvrAvAAaao.....",
        "...ovAAoAvvrAAAoAao.....",
        "...ovAAoAvvAAAoAao......",
        "...oFFooAvvAAAoosso.....",
        "....oooAvllrLlaooo......",
        "......ovvAvAAaao........",
        ".....ovvAAvAAAaao.......",
        ".....ovAAAvAAaaao.......",
        "....ovvAAvAAAAaaao......",
        "....ovAAAvAAaaaao.......",
        "....orrrRrrrrrrro.......",
        ".....ooooo.ooooo........",
    },
    { // APRON
        ".....ovvAoWwovAao.......",
        "....ovvAAoWoAvAAao......",
        "...ovvAAAopPoAAAaao.....",
        "...oFFAoopPPPoAosso.....",
        "...oFfooopPPPoosso......",
        "....oooApPPPPpAoo.......",
        "......ovLLLLLlao........",
        "......ovpPPPPpao........",
        "......ovpPPPPpao........",
        "......oApPPPPpAo........",
        ".......oAaoAao..........",
        ".......oLkoLko..........",
        "......okLkolLkko........",
        "......oooo.ooooo........",
    },
    { // DRESS
        ".....ovvAoWwovAao.......",
        "....ovvAAorAvAAao.......",
        "....ovAAAAvAAAaao.......",
        "....ovAAooFFoAAao.......",
        "....oFFoFFFsoosso.......",
        ".....oFFFFssoooo........",
        "......ovllrLlao.........",
        "......ovvAvAAaao........",
        ".....ovvAAvAAAaao.......",
        ".....ovAAAvAAaaao.......",
        "....ovvAAvAAAAaaao......",
        "....opAAAvAAaaapoo......",
        "....oppppAApppppo.......",
        ".....ooooo.ooooo........",
    },
    { // ARMOR
        "...oNNMoommmooMmo.......",
        "..oNMMMovArvAoMMmo......",
        "..oNMMmovAGGAoMmmo......",
        "...ommoAvAGGAaoommo.....",
        "....oMovAvGGAaoMmo......",
        "....oFovAGGGAaosso......",
        ".....oovlllLlaooo.......",
        "......ovAoAaaao.........",
        "......oAAoAAaao.........",
        "......oMmo.oMmo.........",
        "......oNmo.oMmo.........",
        "......oMko.oMko.........",
        ".....okLko.okLko........",
        ".....ooooo.ooooo........",
    },
    { // HUNCHED
        "....ovvAooooovAao.......",
        "...ovvAAAowwAvAAao......",
        "...ovvAAAvwwAAAAaao.....",
        "...ovAAoAvwAAAAoAao.....",
        "...ovAAoAvAAAAAoAao.....",
        "...oFFooAvvAAAoosso.....",
        "....oooAvlllLlaooo......",
        "......ovvAoAAaao........",
        "......ovAAoAAaao........",
        "......ovAAoAppaao.......",
        ".......oAaoAppo.........",
        ".......oLkoLko..........",
        "......okLkolLkko........",
        "......oooo.ooooo........",
    },
    { // FUR
        "....oWWwWooWwwwo........",
        "...ovWwwWWWwwwAAao......",
        "..ovvAAAAorAAAAAaao.....",
        "..ovAAAooAvAAAAoAAao....",
        "..ovAAAoAvAAAAAoAAao....",
        "..oLlloAvvvAAAolLlo.....",
        "...oFfooLLLLLlooFfo.....",
        "....ooovvAoAAaaooo......",
        "......ovAAoAAaao........",
        "......ovAAoAAaao........",
        "......oWwo.oWwo.........",
        "......oLko.oLko.........",
        ".....okLko.okLko........",
        ".....ooooo.ooooo........",
    },
    { // FORMAL
        "....ovvAoWWwoAao........",
        "...ovvAAAorAvAAao.......",
        "..ovvAAAAvrrvAAAaao.....",
        "..ovAAAoAvrrAAAoAAao....",
        "..ovAAAoAvAApAAoAAao....",
        "..oFFfooAvAAPAoossso....",
        "...oooovllrLLlaoooo.....",
        "......ovvAvAAaao........",
        "......ovAAAvAAaao.......",
        "......ovAAAvAAaao.......",
        "......ovAAAvAAaao.......",
        "......orrrRrrrro........",
        "......oLko.oLko.........",
        ".....ooooo.ooooo........",
    },
};

typedef struct {
    int head;
    int body;
    SDL_Color cloth;
    SDL_Color hat;
    SDL_Color hair;
    SDL_Color skin;
    SDL_Color trim;
} NpcLook;

#define RGB(r, g, b) {r, g, b, 255}
static const NpcLook npc_looks[NPC_ART_COUNT] = {
    [NPC_ART_ELOWEN] = {NPC_BEARD, NPC_ROBE, RGB(48, 76, 132), RGB(48, 76, 132), RGB(213, 208, 186), RGB(202, 154, 119), RGB(107, 195, 211)},
    [NPC_ART_DAIN] = {NPC_CAP, NPC_COAT, RGB(116, 67, 35), RGB(72, 67, 62), RGB(101, 65, 43), RGB(189, 132, 91), RGB(173, 105, 50)},
    [NPC_ART_GARRICK] = {NPC_BEARD, NPC_APRON, RGB(87, 101, 114), RGB(72, 67, 62), RGB(66, 43, 34), RGB(189, 132, 91), RGB(107, 68, 38)},
    [NPC_ART_ALDER] = {NPC_HOOD, NPC_ROBE, RGB(48, 91, 55), RGB(48, 91, 55), RGB(91, 57, 32), RGB(191, 142, 101), RGB(154, 177, 118)},
    [NPC_ART_WARDEN] = {NPC_HOOD, NPC_HUNCHED, RGB(41, 84, 49), RGB(41, 84, 49), RGB(80, 57, 31), RGB(199, 151, 108), RGB(157, 174, 132)},
    [NPC_ART_MARA] = {NPC_CAP, NPC_COAT, RGB(29, 91, 111), RGB(30, 54, 66), RGB(67, 43, 31), RGB(188, 137, 101), RGB(69, 207, 196)},
    [NPC_ART_CAIN] = {NPC_BEARD, NPC_ROBE, RGB(112, 78, 47), RGB(112, 78, 47), RGB(223, 218, 200), RGB(198, 151, 113), RGB(204, 173, 113)},
    [NPC_ART_ROWAN] = {NPC_HAT, NPC_COAT, RGB(43, 66, 101), RGB(43, 66, 101), RGB(206, 205, 191), RGB(195, 148, 109), RGB(208, 167, 85)},
    [NPC_ART_NAHLA] = {NPC_HAT, NPC_COAT, RGB(47, 112, 104), RGB(181, 132, 64), RGB(54, 35, 29), RGB(173, 112, 72), RGB(214, 190, 127)},
    [NPC_ART_ROOK] = {NPC_HAT, NPC_FORMAL, RGB(91, 30, 39), RGB(77, 42, 32), RGB(83, 50, 34), RGB(187, 127, 84), RGB(218, 172, 55)},
    [NPC_ART_BRAM_INN] = {NPC_BEARD, NPC_APRON, RGB(113, 73, 48), RGB(68, 39, 27), RGB(96, 53, 35), RGB(196, 142, 96), RGB(211, 185, 132)},
    [NPC_ART_BRAM_TOWN] = {NPC_BEARD, NPC_COAT, RGB(113, 73, 48), RGB(68, 39, 27), RGB(96, 53, 35), RGB(196, 142, 96), RGB(211, 185, 132)},
    [NPC_ART_MIRA] = {NPC_HAIR, NPC_DRESS, RGB(126, 83, 135), RGB(126, 83, 135), RGB(66, 39, 39), RGB(213, 164, 129), RGB(199, 162, 205)},
    [NPC_ART_ILYA] = {NPC_CAP, NPC_COAT, RGB(110, 65, 52), RGB(87, 73, 60), RGB(70, 48, 36), RGB(204, 159, 120), RGB(225, 181, 80)},
    [NPC_ART_GUARD] = {NPC_HELM, NPC_ARMOR, RGB(38, 60, 140), RGB(146, 152, 166), RGB(89, 59, 41), RGB(196, 146, 106), RGB(176, 38, 44)},
    [NPC_ART_ZARA] = {NPC_HOOD, NPC_ROBE, RGB(57, 106, 135), RGB(57, 106, 135), RGB(80, 54, 36), RGB(191, 133, 86), RGB(224, 184, 79)},
    [NPC_ART_HADRIN] = {NPC_HAIR, NPC_FORMAL, RGB(78, 54, 101), RGB(78, 54, 101), RGB(145, 142, 130), RGB(191, 142, 101), RGB(218, 175, 78)},
    [NPC_ART_BRENNA] = {NPC_HAIR, NPC_FUR, RGB(53, 77, 110), RGB(53, 77, 110), RGB(125, 66, 40), RGB(208, 164, 126), RGB(228, 182, 78)},
    [NPC_ART_FEN] = {NPC_WINTER, NPC_HUNCHED, RGB(67, 93, 118), RGB(53, 77, 110), RGB(130, 121, 109), RGB(186, 146, 119), RGB(175, 197, 209)},
    [NPC_ART_ORIN] = {NPC_CAP, NPC_COAT, RGB(53, 91, 114), RGB(103, 82, 55), RGB(88, 53, 36), RGB(208, 164, 126), RGB(70, 148, 174)},
    [NPC_ART_OSWIN] = {NPC_HOOD, NPC_ROBE, RGB(89, 85, 103), RGB(89, 85, 103), RGB(87, 62, 46), RGB(208, 168, 137), RGB(224, 216, 186)},
    [NPC_ART_INFORMANT] = {NPC_CAP, NPC_COAT, RGB(130, 96, 65), RGB(67, 53, 44), RGB(67, 53, 44), RGB(208, 165, 126), RGB(168, 142, 97)},
    [NPC_ART_TOMAS] = {NPC_HAIR, NPC_HUNCHED, RGB(112, 128, 143), RGB(67, 53, 44), RGB(67, 53, 44), RGB(208, 165, 126), RGB(183, 163, 127)},
    [NPC_ART_NESSA] = {NPC_BAKER, NPC_APRON, RGB(168, 122, 65), RGB(229, 216, 185), RGB(71, 48, 33), RGB(203, 156, 118), RGB(229, 216, 185)},
    [NPC_ART_HOLT] = {NPC_CAP, NPC_APRON, RGB(115, 65, 57), RGB(198, 187, 160), RGB(71, 48, 33), RGB(203, 156, 118), RGB(131, 111, 89)},
    [NPC_ART_EDRIC] = {NPC_BEARD, NPC_ROBE, RGB(91, 76, 52), RGB(91, 76, 52), RGB(188, 182, 164), RGB(203, 156, 118), RGB(148, 119, 60)},
    [NPC_ART_SURI] = {NPC_WRAP, NPC_ROBE, RGB(125, 67, 145), RGB(201, 150, 78), RGB(71, 48, 33), RGB(203, 156, 118), RGB(223, 186, 97)},
    [NPC_ART_VEYRA] = {NPC_HELM, NPC_ARMOR, RGB(102, 38, 61), RGB(146, 152, 166), RGB(88, 53, 36), RGB(196, 146, 106), RGB(227, 181, 78)},
    [NPC_ART_SELENE] = {NPC_HAIR, NPC_FUR, RGB(65, 93, 59), RGB(65, 93, 59), RGB(130, 74, 41), RGB(202, 153, 112), RGB(180, 198, 135)},
    [NPC_ART_ALDRIC] = {NPC_BEARD, NPC_COAT, RGB(64, 91, 119), RGB(64, 91, 119), RGB(167, 163, 153), RGB(196, 146, 106), RGB(192, 157, 87)},
    [NPC_ART_TESSA] = {NPC_HAIR, NPC_APRON, RGB(65, 103, 66), RGB(139, 106, 64), RGB(120, 69, 37), RGB(203, 156, 118), RGB(191, 202, 142)},
    [NPC_ART_LYSA] = {NPC_HAIR, NPC_ROBE, RGB(217, 222, 200), RGB(217, 222, 200), RGB(83, 51, 36), RGB(208, 164, 126), RGB(73, 150, 110)},
    [NPC_ART_MORWEN] = {NPC_HAT, NPC_ROBE, RGB(72, 42, 100), RGB(72, 42, 100), RGB(42, 32, 30), RGB(196, 152, 119), RGB(120, 195, 137)}
};
#undef RGB

typedef struct {
    Renderer *r;
    int x;
    int y;
    int mirror;
    SDL_Color palette[128];
} NpcCanvas;

static SDL_Color npc_shade(SDL_Color color, int light) {
    if (light) {
        return (SDL_Color){color.r + (255 - color.r) / 3,
            color.g + (255 - color.g) / 3, color.b + (255 - color.b) / 3, 255};
    }
    return (SDL_Color){color.r * 2 / 3, color.g * 2 / 3, color.b * 2 / 3, 255};
}

static void npc_ink(NpcCanvas *c, int x, int y, int width, char ink) {
    if (ink == '.') {
        return;
    }
    SDL_Color color = c->palette[(unsigned char)ink];
    SDL_SetRenderDrawColor(c->r->sdl, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {c->x + (c->mirror ? 24 - x - width : x), c->y + y, width, 1};
    SDL_RenderFillRect(c->r->sdl, &rect);
}

static void npc_row(NpcCanvas *c, int y, const char *row) {
    for (int col = 0; col < 24;) {
        int end = col + 1;
        while (end < 24 && row[end] == row[col]) {
            end++;
        }
        npc_ink(c, col, y, end - col, row[col]);
        col = end;
    }
}

// Slash-separated miniatures stay on the native pixel grid, including when mirrored.
static void npc_prop(NpcCanvas *c, int x, int y, const char *pixels) {
    int start = x;
    for (const char *p = pixels; *p; p++) {
        if (*p == '/') {
            x = start;
            y++;
        } else {
            npc_ink(c, x, y, 1, *p);
            x++;
        }
    }
}

static void npc_accessories(NpcCanvas *c, NpcArt art) {
    switch (art) {
        case NPC_ART_ELOWEN:
            npc_prop(c, 19, 2, "..o/oro/rRro/oro/.Lo/.lo/.Lo/.lo/.Lo/.lo/.Lo/.lo/oFF/.lo/.Lo/.lo/.Lo/.lo/.Lo/.lo/.oo");
            npc_prop(c, 9, 9, "TTtth/.TTth/..Th");
            break;
        case NPC_ART_DAIN:
            npc_prop(c, 1, 2, ".oo/oNMo/NMMmo/MMmmo/.Loo/.lo/.Lo/.lo/.Lo/.lo/.Lo/oFF/.Lo/.lo/.Lo/.lo/.Lo/.lo/.Lo/.lo/.oo");
            npc_prop(c, 17, 16, "oooo/oLLlo/oLGlo/olllo/oooo");
            npc_prop(c, 9, 8, "tTtth/.tth");
            break;
        case NPC_ART_GARRICK:
            npc_prop(c, 1, 11, "oooooo/oNMMmo/ommmoo/..Lo/..Lo/..FF/..Lo/..lo/..oo");
            npc_prop(c, 8, 18, "PPpPP/PpPPP/PPPPp");
            break;
        case NPC_ART_ALDER:
            npc_prop(c, 20, 2, "..n/.Jn/nJLo/.nLo/..lo/..Lo/..lo/..Lo/..lo/..Lo/..lo/.FFo/..lo/..Lo/..lo/..Lo/..lo/..Lo/..lo/..Lo/..oo");
            npc_prop(c, 11, 10, ".J/Jnn/.n");
            break;
        case NPC_ART_WARDEN:
            npc_prop(c, 2, 8, "l/Lo/.lo/.Lo/.lnJ/..lnJ/..lo/.lLo/.lo/.Lo/.lo/Lo/lLo/lo/lo");
            npc_prop(c, 20, 5, "lo/lLo/.lo/.Lo/lo/ln/lnJ/.lo/lo/Lo/lo/.Lo/.lo/Lo/lo/lLo/lo/lo");
            npc_prop(c, 4, 12, "nn..nJJnn..nn/.nJJJn..nJJn");
            npc_prop(c, 5, 17, "nJJn...nJJnn/..nJJnnn..n");
            break;
        case NPC_ART_MARA:
            npc_prop(c, 17, 13, ".ooo/.G.G/oGGGo/oGrGo/oGRGo/oGrGo/ogGgo/ooooo");
            break;
        case NPC_ART_CAIN:
            npc_prop(c, 20, 5, "ooo/LLo/.lo/.Lo/.lo/.Lo/.lo/.Lo/.lo/oFF/.lo/.Lo/.lo/.Lo/.lo/.Lo/.lo/.oo");
            npc_prop(c, 2, 14, ".oooo/oWWwo/oWwwo/oWwwo/oWWwo/.oooo");
            npc_prop(c, 9, 9, "TTtth/.TTth/..Th");
            break;
        case NPC_ART_ROWAN:
            npc_prop(c, 4, 10, "GGG........GGG/Gg..........Gg");
            npc_prop(c, 10, 8, "tTtth/.tth");
            npc_prop(c, 11, 13, ".G/GGG/.G");
            break;
        case NPC_ART_NAHLA:
            npc_prop(c, 7, 16, "wwWwwWwwWw");
            npc_prop(c, 16, 17, "ooooo/oLLlo/oLrlo/olllo/ooooo");
            npc_prop(c, 7, 7, "h/.t/h/.t/h");
            break;
        case NPC_ART_ROOK:
            npc_prop(c, 17, 0, ".W/WW/Ww/w");
            npc_prop(c, 18, 16, ".oo/oGYo/oGGo/.oo");
            break;
        case NPC_ART_BRAM_INN:
            npc_prop(c, 17, 15, "oooo/oWWoo/oMm.o/oMm.o/omm.o/oooo");
            break;
        case NPC_ART_BRAM_TOWN:
            npc_prop(c, 17, 17, "ooooo/oLLlo/oLrlo/olllo/ooooo");
            break;
        case NPC_ART_MIRA:
            npc_prop(c, 6, 8, "Th/hT/Th/hT/Th/.r");
            npc_prop(c, 7, 20, "..l....l/ll......ll");
            break;
        case NPC_ART_ILYA:
            npc_prop(c, 9, 10, "rrRrr/rRrrr/..rr/..rr");
            npc_prop(c, 17, 17, "ooooo/oLLlo/oLrlo/olllo/ooooo");
            break;
        case NPC_ART_GUARD:
            npc_prop(c, 18, 0, "..N/..M/..M/..LoM/..LNMM/..LMMM/omLMM/..Lm/..Lo/..Lo/..Lo/..Lo/..Lo/.FFo/..Lo/..Lo/..Lo/..Lo/..Lo/..Lo/..Lo/..Lo/..Lo/..oo");
            npc_prop(c, 4, 12, "M.m/.mM/M.m");
            break;
        case NPC_ART_ZARA:
            npc_prop(c, 11, 11, ".G/GYG/GrG/GYG/.G");
            npc_prop(c, 7, 18, "r..r..r..r");
            break;
        case NPC_ART_HADRIN:
            npc_prop(c, 11, 10, "GGG/GRG/GGG");
            npc_prop(c, 18, 13, "ooo/oGo/oWo/oWo/oWo/oWo/oGo/ooo");
            break;
        case NPC_ART_BRENNA:
            npc_prop(c, 17, 13, "oooooo/oLWWwo/oLwwwo/oLWwwo/oLwwwo/oLGGgo/oooooo");
            npc_prop(c, 16, 4, "th/th/.th/.th");
            break;
        case NPC_ART_FEN:
            npc_prop(c, 20, 7, "oo/Lo/lo/Lo/lo/Lo/lo/FF/lo/Lo/lo/Lo/lo/Lo/lo/oo");
            npc_prop(c, 7, 11, "wwWWww/..wwWw/....ww/....ww");
            npc_prop(c, 15, 19, "aa.o/a..o");
            break;
        case NPC_ART_ORIN:
            npc_prop(c, 8, 7, "MeeM/....M");
            npc_prop(c, 16, 14, "ooooooo/oWwwWWo/oWrwWwo/oWrwWro/oWrrWWo/oWwWWwo/ooooooo");
            npc_prop(c, 8, 17, "oGo/GRG/oGo");
            break;
            npc_prop(c, 17, 12, ".J/nJn/.Jn/..J/ooooo/oLLlo/oLrlo/olllo/ooooo");
            npc_prop(c, 10, 10, ".J/Jnn/.n");
            break;
        case NPC_ART_OSWIN:
        case NPC_ART_EDRIC:
            npc_prop(c, 16, 14, "ooooooo/oLWWwo/oLwwwo/oLWwwo/oLwwwo/oLGGgo/ooooooo");
            npc_prop(c, 7, 16, "wwWwwWwwWw");
            break;
        case NPC_ART_INFORMANT:
            npc_prop(c, 7, 14, "vvAoFF/oAAFFF/..oAAAaa/....AAaa");
            break;
        case NPC_ART_TOMAS:
            npc_prop(c, 9, 8, "shtsh/.hhh");
            npc_prop(c, 13, 19, "pPp/pPp/ppp");
            npc_prop(c, 5, 14, "pPp");
            break;
        case NPC_ART_NESSA:
            npc_prop(c, 16, 15, ".oooooo/oLYLYLLo/oGGGGGgo/.oooooo");
            npc_prop(c, 9, 17, "W..P/.P..W");
            break;
        case NPC_ART_HOLT:
            npc_prop(c, 18, 11, "oooooo/oNMMMo/oMMMMo/ommmoo/.Lo/.Lo/oFF/.Lo/.oo");
            npc_prop(c, 9, 16, "LLrLL/L..rL");
            break;
        case NPC_ART_SURI:
            npc_prop(c, 18, 15, ".GG/.LL/olLlo/oLrlo/olllo/.ooo");
            npc_prop(c, 8, 16, "rrRrrRrrr");
            break;
        case NPC_ART_VEYRA:
            npc_prop(c, 10, 0, "rRrr/rRrr/.rr");
            npc_prop(c, 3, 10, "cCcc/ccCcc/ccCcc/ccCcc/.ccc/.ccc");
            npc_prop(c, 17, 16, "ooooo/oLLlo/oLrlo/olllo/ooooo");
            npc_prop(c, 18, 8, "N/M/M/L/L/L/FF/L/L/L/L/L/L/L/oo");
            break;
        case NPC_ART_SELENE:
            npc_prop(c, 2, 4, "N.N/N.N/LoL/LlL/LlL/LlL/LlL/olo/olo/ooo");
            npc_prop(c, 18, 5, "L/LL/L.L/L..L/L..L/L...L/L...L/L...L/L..FF/L...L/L...L/L..L/L..L/L.L/LL/L");
            npc_prop(c, 16, 7, "Th/hT/Th/hT/Th/.r");
            break;
        default:
            break;
    }
}

void npc_draw(Renderer *r, int tx, int ty, NpcArt art, int mirror) {
    if (art < 0 || art >= NPC_ART_COUNT) {
        return;
    }
    const NpcLook *look = &npc_looks[art];
    NpcCanvas c = {.r = r, .x = tx * TILE_SIZE, .y = ty * TILE_SIZE, .mirror = mirror};
    c.palette['o'] = (SDL_Color){22, 25, 34, 255};
    c.palette['e'] = (SDL_Color){25, 27, 31, 255};
    c.palette['k'] = (SDL_Color){47, 34, 31, 255};
    c.palette['l'] = (SDL_Color){92, 57, 33, 255};
    c.palette['L'] = (SDL_Color){156, 106, 59, 255};
    c.palette['m'] = (SDL_Color){79, 94, 116, 255};
    c.palette['M'] = (SDL_Color){151, 174, 191, 255};
    c.palette['N'] = (SDL_Color){225, 237, 240, 255};
    c.palette['n'] = (SDL_Color){58, 98, 49, 255};
    c.palette['J'] = (SDL_Color){148, 185, 105, 255};
    c.palette['c'] = (SDL_Color){112, 34, 53, 255};
    c.palette['C'] = (SDL_Color){173, 66, 76, 255};
    c.palette['g'] = (SDL_Color){130, 88, 33, 255};
    c.palette['G'] = (SDL_Color){212, 163, 65, 255};
    c.palette['Y'] = (SDL_Color){252, 222, 131, 255};
    c.palette['w'] = (SDL_Color){179, 170, 145, 255};
    c.palette['W'] = (SDL_Color){239, 231, 204, 255};
    c.palette['a'] = npc_shade(look->cloth, 0);
    c.palette['A'] = look->cloth;
    c.palette['v'] = npc_shade(look->cloth, 1);
    c.palette['b'] = npc_shade(look->hat, 0);
    c.palette['B'] = look->hat;
    c.palette['V'] = npc_shade(look->hat, 1);
    c.palette['h'] = npc_shade(look->hair, 0);
    c.palette['t'] = look->hair;
    c.palette['T'] = npc_shade(look->hair, 1);
    c.palette['s'] = npc_shade(look->skin, 0);
    c.palette['f'] = look->skin;
    c.palette['F'] = npc_shade(look->skin, 1);
    c.palette['p'] = npc_shade(look->trim, 0);
    c.palette['P'] = look->trim;
    c.palette['r'] = look->trim;
    c.palette['R'] = npc_shade(look->trim, 1);
    for (int row = 0; row < 10; row++) {
        npc_row(&c, row, npc_heads[look->head][row]);
    }
    for (int row = 0; row < 14; row++) {
        npc_row(&c, row + 10, npc_bodies[look->body][row]);
    }
    npc_accessories(&c, art);
}
