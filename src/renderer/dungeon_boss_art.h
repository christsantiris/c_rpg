#ifndef DUNGEON_BOSS_ART_H
#define DUNGEON_BOSS_ART_H

// Rendering indices only; persistent EnemyType values are unchanged.
enum { BOSS_ART_LICH_KING, BOSS_ART_RED_DRAGON, BOSS_ART_COUNT };

// Native 24-pixel static poses. Dots preserve the underlying terrain.
// o outline; s/S/H skin or bone; c/C/v mantle or shell; e/y sockets/eyes;
// m/M/N steel; f/F horns, teeth and claws; a/A/h wing facets;
// g/G brass or belly plates; l/L/K leather; k boots; R gems; p/P magic or fire.
static const char dungeon_boss_art[BOSS_ART_COUNT][24][25] = {
    { // LICH_KING
        "..........F....F........",
        "...P.....oFG.oGFo.......",
        "..pPp...oMGGGGGMMo......",
        ".PpPpP..oMGMGMGMmo......",
        "..pPp...oFSSSSSSmo......",
        "...G....oFSSSSSSSo......",
        "...g....oFSeySSySo......",
        "...G.....oFSeSSsSo......",
        "...g.....oSfffSSo.......",
        "...G......oSfefSo.......",
        "...g.......oSSo.........",
        "...G....ovCCoSooCCco....",
        "...g...ovCCGGGCCccco....",
        "...G..ovCoCvGCCCcoSoo...",
        "..SS..oSSoCvGCCCcoSSo...",
        "...g..oSSoCvGCCCcoSSSo..",
        "...G...oooCvGCCCcooSSo..",
        "...g...ovCCCvGCCCCcooo..",
        "...G..ovCCCvGCCcCCCco...",
        "...g..ovCCvGCCcCCCCcco..",
        "...G.ovCCvGCCcCCvCCCcco.",
        "...g.oGCCGCCCoGCCCoGGGo.",
        "...oo.oooCCooCCooCCooo..",
        ".........oo..oo..oo.....",
    },
    { // RED_DRAGON
        ".......F.....F..........",
        "......oFfo..oFfo........",
        "......oFfSo.oFfo........",
        "..oo..oHSSSSSSSo..oo....",
        ".oHAo.oHSSSSSSSSo.oHAo..",
        "oHAAaoHSSeySSeySoaAAHo..",
        "oHAaaoHSSHHSSSSSoaAAHo..",
        "oHAaaoSHSSSeSSSSoaAAHo..",
        "oHAaaoHSffffSSSoaAaAHo..",
        "oHAAAaooSSSSSSooaAAAHo..",
        ".oHAAo.oSSGGGSo.oAAHo...",
        ".oHAAoHSoGggGSoSoAAHo...",
        "..oAoHSSoGGGGGSoSoAo....",
        "...oHSSSoGggGSoSSSo.....",
        "...oFSSoSGGGGSoSSFo.....",
        "....oooSGggGGSSooo...o..",
        "......oSGGGGGSSo....oSo.",
        "..P...oSGggGGSSo...oHSo.",
        ".pPp..oSSGGGSSSo..oHSo..",
        ".pPp.oHSSoSSSSSSooSSo...",
        "pPPp.oHSo.oSSSoSSSoo....",
        "pPp.oFSSo.oSSSo.oo......",
        ".p..oFSSFooFSSFo........",
        "....oooooooooooo........",
    },
};

#endif
