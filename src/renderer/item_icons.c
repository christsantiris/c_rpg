#include "item_icons.h"
#include "item_icon_art.h"
#include <string.h>

static void fill_rect_px(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

static SDL_Color lighter(SDL_Color c) {
    return (SDL_Color){c.r + (255 - c.r) / 3, c.g + (255 - c.g) / 3, c.b + (255 - c.b) / 3, 255};
}

static SDL_Color darker(SDL_Color c) {
    return (SDL_Color){c.r * 2 / 3, c.g * 2 / 3, c.b * 2 / 3, 255};
}

static void icon_palette(SDL_Color *pal, const Item *item) {
    SDL_Color metal = {139, 157, 177, 255};
    SDL_Color cloth = {98, 65, 43, 255};
    SDL_Color wood = {103, 64, 36, 255};
    SDL_Color trim = {197, 155, 66, 255};
    SDL_Color accent = {93, 219, 243, 255};
    if (item) {
        if (item->type == ITEM_SHIELD) {
            cloth = (SDL_Color){92, 101, 111, 255};
        }
        switch (item->visual_id) {
            case ITEM_VISUAL_RUSTY_SWORD:
            case ITEM_VISUAL_RUSTY_DAGGER: metal = (SDL_Color){161, 85, 47, 255}; break;
            case ITEM_VISUAL_DEMONIC_SWORD:
                metal = (SDL_Color){27, 19, 24, 255};
                accent = (SDL_Color){235, 55, 65, 255};
                wood = (SDL_Color){27, 19, 24, 255};
                trim = (SDL_Color){104, 30, 42, 255};
                break;
            case ITEM_VISUAL_POISON_SWORD:
                metal = (SDL_Color){39, 67, 48, 255};
                accent = (SDL_Color){111, 239, 90, 255};
                trim = (SDL_Color){134, 67, 99, 255};
                break;
            case ITEM_VISUAL_MAGIC_LONG_SWORD:
            case ITEM_VISUAL_MAGIC_GREATSWORD:
            case ITEM_VISUAL_MAGIC_DAGGER:
            case ITEM_VISUAL_MAGIC_BATTLE_AXE:
            case ITEM_VISUAL_MAGIC_STAFF:
            case ITEM_VISUAL_MAGIC_LONGBOW:
                metal = (SDL_Color){120, 147, 210, 255};
                wood = (SDL_Color){96, 56, 124, 255};
                trim = (SDL_Color){172, 98, 218, 255};
                break;
            case ITEM_VISUAL_RUNED_STAFF:
                accent = (SDL_Color){110, 221, 135, 255};
                wood = (SDL_Color){126, 107, 50, 255};
                break;
            case ITEM_VISUAL_SANDSTORM_STAFF:
                accent = (SDL_Color){247, 151, 43, 255};
                trim = (SDL_Color){234, 184, 73, 255};
                break;
            case ITEM_VISUAL_LONGBOW: wood = (SDL_Color){112, 141, 63, 255}; break;
            case ITEM_VISUAL_SHORTBOW: wood = (SDL_Color){126, 83, 48, 255}; break;
            case ITEM_VISUAL_CROSSBOW:
                wood = (SDL_Color){91, 57, 36, 255};
                metal = (SDL_Color){156, 170, 185, 255};
                break;
            case ITEM_VISUAL_CHAIN_MAIL: cloth = (SDL_Color){111, 134, 156, 255}; break;
            case ITEM_VISUAL_SCALE_MAIL: cloth = (SDL_Color){171, 131, 73, 255}; break;
            case ITEM_VISUAL_PLATE_ARMOR: cloth = (SDL_Color){169, 182, 192, 255}; break;
            case ITEM_VISUAL_MAGIC_PLATE: cloth = (SDL_Color){99, 177, 211, 255}; break;
            case ITEM_VISUAL_STUDDED_LEATHER: cloth = (SDL_Color){132, 89, 48, 255}; break;
            case ITEM_VISUAL_PRISMWEAVE_ROBES:
                cloth = (SDL_Color){91, 107, 173, 255};
                trim = (SDL_Color){158, 223, 231, 255};
                accent = (SDL_Color){206, 155, 237, 255};
                break;
            case ITEM_VISUAL_CROWN_CHAPEL_VESTMENTS:
                cloth = (SDL_Color){100, 64, 135, 255};
                trim = (SDL_Color){232, 193, 106, 255};
                accent = (SDL_Color){141, 234, 239, 255};
                break;
            case ITEM_VISUAL_SUNWARDEN_MANTLE:
                cloth = (SDL_Color){172, 119, 55, 255};
                trim = (SDL_Color){244, 207, 112, 255};
                accent = (SDL_Color){117, 218, 228, 255};
                break;
            case ITEM_VISUAL_EMBERCLEAVER:
                metal = (SDL_Color){98, 100, 109, 255};
                wood = (SDL_Color){88, 51, 34, 255};
                trim = (SDL_Color){246, 143, 62, 255};
                accent = (SDL_Color){255, 193, 95, 255};
                break;
            case ITEM_VISUAL_BRIARWEAVE_CLOAK: cloth = (SDL_Color){61, 129, 112, 255}; trim = (SDL_Color){213, 157, 211, 255}; break;
            case ITEM_VISUAL_RANGER_CLOAK: cloth = (SDL_Color){57, 114, 63, 255}; break;
            case ITEM_VISUAL_SHADOW_ARMOR:
                cloth = (SDL_Color){75, 49, 111, 255};
                trim = (SDL_Color){174, 117, 211, 255};
                break;
            case ITEM_VISUAL_APPRENTICE_ROBES: cloth = (SDL_Color){54, 98, 177, 255}; break;
            case ITEM_VISUAL_RUNED_ROBES: cloth = (SDL_Color){42, 124, 144, 255}; break;
            case ITEM_VISUAL_ENCHANTER_ROBES: cloth = (SDL_Color){84, 77, 168, 255}; break;
            case ITEM_VISUAL_ARCHMAGE_ROBES:
                cloth = (SDL_Color){119, 74, 181, 255};
                accent = (SDL_Color){226, 167, 255, 255};
                break;
            case ITEM_VISUAL_BUCKLER: cloth = (SDL_Color){126, 88, 51, 255}; break;
            case ITEM_VISUAL_KITE_SHIELD: cloth = (SDL_Color){62, 91, 151, 255}; break;
            case ITEM_VISUAL_TOWER_SHIELD:
                cloth = (SDL_Color){106, 119, 133, 255};
                trim = (SDL_Color){199, 69, 59, 255};
                break;
            case ITEM_VISUAL_CASTELLAN_BULWARK:
                cloth = (SDL_Color){88, 48, 65, 255};
                metal = (SDL_Color){151, 157, 174, 255};
                trim = (SDL_Color){222, 181, 91, 255};
                break;
            case ITEM_VISUAL_LABYRINTH_AEGIS:
                cloth = (SDL_Color){61, 71, 102, 255};
                metal = (SDL_Color){191, 160, 91, 255};
                trim = (SDL_Color){214, 160, 70, 255};
                accent = (SDL_Color){132, 221, 230, 255};
                break;
            case ITEM_VISUAL_MAGIC_SHIELD:
                cloth = (SDL_Color){86, 69, 165, 255};
                metal = accent;
                break;
            default: break;
        }
        if (item->type == ITEM_ARMOR && item->armor_family == ARMOR_FAMILY_HEAVY) {
            metal = cloth;
        }
        // Named rewards keep their saved visual IDs but get recognizable finishes.
        if (strcmp(item->name, "Krakenbone Bow") == 0) {
            wood = (SDL_Color){192, 193, 158, 255};
            trim = (SDL_Color){99, 179, 191, 255};
        } else if (strcmp(item->name, "Cryptblade") == 0) {
            metal = (SDL_Color){142, 144, 180, 255};
            accent = (SDL_Color){184, 134, 242, 255};
        } else if (strcmp(item->name, "Necromancer's Cloak") == 0) {
            cloth = (SDL_Color){62, 46, 86, 255};
            trim = (SDL_Color){128, 198, 124, 255};
        } else if (strcmp(item->name, "Gravekeeper's Mantle") == 0) {
            cloth = (SDL_Color){81, 91, 102, 255};
        } else if (strcmp(item->name, "Dragon Scale Mantle") == 0) {
            cloth = (SDL_Color){165, 84, 55, 255};
            metal = cloth;
        } else if (strcmp(item->name, "Tidecaller Robes") == 0) {
            cloth = (SDL_Color){38, 132, 146, 255};
        } else if (strcmp(item->name, "Stoneguard Shield") == 0) {
            cloth = (SDL_Color){84, 100, 77, 255};
            metal = (SDL_Color){173, 180, 151, 255};
            trim = (SDL_Color){226, 193, 105, 255};
        } else if (strcmp(item->name, "Goblin King's Shield") == 0) {
            cloth = (SDL_Color){74, 115, 59, 255};
        }
        if (item->type == ITEM_POTION_HEALTH) {
            accent = (SDL_Color){211, 60, 71, 255};
        } else if (item->type == ITEM_POTION_MANA) {
            accent = (SDL_Color){58, 135, 233, 255};
        } else if (item->type == ITEM_POTION_STRENGTH) {
            accent = (SDL_Color){238, 164, 51, 255};
        } else if (item->type == ITEM_POTION_INTELLIGENCE) {
            accent = (SDL_Color){173, 101, 234, 255};
        }
    }
    pal['o'] = (SDL_Color){10, 13, 23, 255};
    pal['m'] = darker(metal);
    pal['M'] = metal;
    pal['N'] = lighter(metal);
    pal['l'] = wood;
    pal['L'] = lighter(wood);
    pal['g'] = darker(trim);
    pal['G'] = trim;
    pal['c'] = darker(cloth);
    pal['C'] = cloth;
    pal['v'] = lighter(cloth);
    pal['f'] = (SDL_Color){220, 204, 158, 255};
    pal['p'] = accent;
    pal['P'] = lighter(accent);
}

static void draw_item_art(Renderer *r, int x, int y, int art, SDL_Color *pal, const Item *item) {
    int enchanted = item && (item->visual_id == ITEM_VISUAL_MAGIC_LONG_SWORD ||
        item->visual_id == ITEM_VISUAL_MAGIC_GREATSWORD || item->visual_id == ITEM_VISUAL_MAGIC_DAGGER ||
        item->visual_id == ITEM_VISUAL_MAGIC_BATTLE_AXE || item->visual_id == ITEM_VISUAL_MAGIC_STAFF ||
        item->visual_id == ITEM_VISUAL_MAGIC_LONGBOW || item->visual_id == ITEM_VISUAL_DEMONIC_SWORD ||
        item->visual_id == ITEM_VISUAL_POISON_SWORD);
    for (int row = 0; row < ICON_SIZE; row++) {
        for (int col = 0; col < ICON_SIZE; col++) {
            char ink = item_icon_art[art][row][col];
            if (ink == '.') {
                continue;
            }
            // Ring holes, staggered scales, leather rivets and robe embroidery.
            if (item && ink == 'C') {
                if (item->visual_id == ITEM_VISUAL_CHAIN_MAIL) {
                    ink = (row + col) % 2 ? 'm' : 'N';
                } else if (item->visual_id == ITEM_VISUAL_SCALE_MAIL) {
                    ink = (col + row / 3) % 3 == 0 ? 'g' : row % 3 == 0 ? 'v' : 'C';
                } else if (item->visual_id == ITEM_VISUAL_STUDDED_LEATHER && row % 4 == 0 && col % 4 == 0) {
                    ink = 'N';
                } else if (item->armor_family == ARMOR_FAMILY_ROBE && item->rarity != ITEM_RARITY_COMMON && row % 4 == 1 && col % 5 == 0) {
                    ink = 'p';
                }
            }
            if (enchanted && ink == 'M' && (row + col) % 6 == 0) {
                ink = 'p';
            }
            fill_rect_px(r, x + col, y + row, 1, 1, pal[(unsigned char)ink]);
        }
    }
}

void draw_icon_weapon(Renderer *r, int px, int py, const Item *item) {
    if (item && item->visual_id == ITEM_VISUAL_STONE_WARHAMMER) {
        fill_rect_px(r, px + 10, py + 7, 4, 15, (SDL_Color){39, 34, 28, 255});
        fill_rect_px(r, px + 11, py + 7, 2, 14, (SDL_Color){140, 96, 54, 255});
        fill_rect_px(r, px + 3, py + 2, 18, 9, (SDL_Color){38, 40, 34, 255});
        fill_rect_px(r, px + 4, py + 3, 16, 6, (SDL_Color){173, 169, 140, 255});
        fill_rect_px(r, px + 4, py + 3, 16, 1, (SDL_Color){221, 213, 180, 255});
        fill_rect_px(r, px + 6, py + 3, 2, 7, (SDL_Color){65, 77, 65, 255});
        fill_rect_px(r, px + 16, py + 3, 2, 7, (SDL_Color){65, 77, 65, 255});
        fill_rect_px(r, px + 11, py + 4, 2, 4, (SDL_Color){139, 212, 110, 255});
        fill_rect_px(r, px + 9, py + 5, 6, 1, (SDL_Color){139, 212, 110, 255});
        return;
    }
    if (!item) {
        return;
    }
    int art = ICON_ART_LONG_SWORD;
    switch (item->visual_id) {
        case ITEM_VISUAL_RUSTY_SWORD:
        case ITEM_VISUAL_SHORT_SWORD: art = ICON_ART_SHORT_SWORD; break;
        case ITEM_VISUAL_RUSTY_DAGGER:
        case ITEM_VISUAL_DAGGER:
        case ITEM_VISUAL_MAGIC_DAGGER: art = ICON_ART_DAGGER; break;
        case ITEM_VISUAL_GREATSWORD:
        case ITEM_VISUAL_MAGIC_GREATSWORD: art = ICON_ART_GREATSWORD; break;
        case ITEM_VISUAL_SHORTBOW: art = ICON_ART_SHORTBOW; break;
        case ITEM_VISUAL_CROSSBOW: art = ICON_ART_CROSSBOW; break;
        default:
            if (item->weapon_family == WEAPON_FAMILY_AXE) {
                art = ICON_ART_AXE;
            } else if (item->weapon_family == WEAPON_FAMILY_STAFF) {
                art = ICON_ART_STAFF;
            } else if (item->weapon_family == WEAPON_FAMILY_BOW) {
                art = item->visual_id == ITEM_VISUAL_BOW ? ICON_ART_BOW : ICON_ART_LONGBOW;
            } else if (item->weapon_family == WEAPON_FAMILY_DAGGER) {
                art = ICON_ART_DAGGER;
            }
            break;
    }
    SDL_Color palette[128] = {{0}};
    icon_palette(palette, item);
    if (item->visual_id == ITEM_VISUAL_DEMONIC_SWORD || item->visual_id == ITEM_VISUAL_POISON_SWORD) {
        palette['N'] = palette['p'];
    }
    draw_item_art(r, px, py, art, palette, item);
}

void draw_icon_armor(Renderer *r, int px, int py, const Item *item) {
    if (!item) {
        return;
    }
    int art = ICON_ART_PLATE;
    if (item->armor_family == ARMOR_FAMILY_ROBE) {
        art = ICON_ART_ROBE;
    } else if (item->visual_id == ITEM_VISUAL_RANGER_CLOAK || item->visual_id == ITEM_VISUAL_BRIARWEAVE_CLOAK) {
        art = ICON_ART_CLOAK;
    } else if (item->visual_id == ITEM_VISUAL_CHAIN_MAIL || item->visual_id == ITEM_VISUAL_SCALE_MAIL) {
        art = ICON_ART_MAIL;
    } else if (item->armor_family == ARMOR_FAMILY_LIGHT) {
        art = ICON_ART_LEATHER;
    }
    SDL_Color palette[128] = {{0}};
    icon_palette(palette, item);
    draw_item_art(r, px, py, art, palette, item);
}

void draw_icon_shield(Renderer *r, int px, int py, const Item *item) {
    if (!item) {
        return;
    }
    int art = item->visual_id == ITEM_VISUAL_LABYRINTH_AEGIS ? ICON_ART_LABYRINTH_AEGIS
        : item->visual_id == ITEM_VISUAL_BUCKLER ? ICON_ART_BUCKLER
        : item->visual_id == ITEM_VISUAL_TOWER_SHIELD || item->visual_id == ITEM_VISUAL_CASTELLAN_BULWARK ? ICON_ART_TOWER
        : item->visual_id == ITEM_VISUAL_MAGIC_SHIELD ? ICON_ART_MAGIC_SHIELD : ICON_ART_KITE;
    SDL_Color palette[128] = {{0}};
    icon_palette(palette, item);
    draw_item_art(r, px, py, art, palette, item);
}

void draw_icon_spell(Renderer *r, int px, int py, SpellID spell_id) {
    static const int art[] = {ICON_ART_MAGIC_ARROW, ICON_ART_FIREBALL, ICON_ART_HEAL, ICON_ART_RETURN, ICON_ART_FROST, ICON_ART_PORTAL};
    SDL_Color palette[128] = {{0}};
    icon_palette(palette, NULL);
    if (spell_id == SPELL_FIREBALL) {
        palette['c'] = (SDL_Color){199, 54, 27, 255};
        palette['p'] = (SDL_Color){255, 135, 31, 255};
        palette['P'] = (SDL_Color){255, 233, 111, 255};
    } else if (spell_id == SPELL_HEAL) {
        palette['p'] = (SDL_Color){64, 203, 114, 255};
        palette['P'] = (SDL_Color){196, 255, 213, 255};
    } else if (spell_id == SPELL_TELEPORT || spell_id == SPELL_RETURN_TO_TOWN) {
        palette['p'] = (SDL_Color){150, 92, 227, 255};
        palette['P'] = (SDL_Color){231, 203, 255, 255};
    }
    int index = spell_id >= SPELL_MAGIC_ARROW && spell_id <= SPELL_TELEPORT ? spell_id : SPELL_TELEPORT;
    draw_item_art(r, px, py, art[index], palette, NULL);
}

void draw_icon_item(Renderer *r, int px, int py, const Item *item) {
    if (!item || item->type == ITEM_NONE) {
        return;
    }
    if (item->type == ITEM_WEAPON) {
        draw_icon_weapon(r, px, py, item);
        return;
    } else if (item->type == ITEM_ARMOR) {
        draw_icon_armor(r, px, py, item);
        return;
    } else if (item->type == ITEM_SHIELD) {
        draw_icon_shield(r, px, py, item);
        return;
    }
    int art = ICON_ART_POTION;
    switch (item->type) {
        case ITEM_HELMET: art = ICON_ART_CAP; break;
        case ITEM_ACCESSORY:
            art = item->accessory_family == ACCESSORY_FAMILY_RING ? ICON_ART_RING
                : item->accessory_family == ACCESSORY_FAMILY_AMULET ? ICON_ART_AMULET
                : item->accessory_family == ACCESSORY_FAMILY_BRACERS ? ICON_ART_BRACERS : ICON_ART_GAUNTLETS;
            break;
        case ITEM_SCROLL: art = ICON_ART_SCROLL; break;
        case ITEM_SPELL_TOME: art = ICON_ART_BOOK; break;
        case ITEM_TREASURE_MAP: art = ICON_ART_MAP; break;
        case ITEM_ARROWS: art = ICON_ART_ARROWS; break;
        case ITEM_GOLD: art = ICON_ART_GOLD; break;
        default: break;
    }
    SDL_Color palette[128] = {{0}};
    icon_palette(palette, item);
    if (item->type == ITEM_SCROLL || item->type == ITEM_SPELL_TOME) {
        palette['p'] = item->spell_id == SPELL_FIREBALL ? (SDL_Color){241, 133, 39, 255}
            : item->spell_id == SPELL_HEAL ? (SDL_Color){69, 203, 120, 255}
            : item->spell_id == SPELL_TELEPORT || item->spell_id == SPELL_RETURN_TO_TOWN ? (SDL_Color){167, 105, 227, 255}
            : (SDL_Color){91, 217, 241, 255};
        palette['P'] = lighter(palette['p']);
        palette['C'] = (SDL_Color){65, 69, 130, 255};
    }
    draw_item_art(r, px, py, art, palette, item);
}

void draw_icon_empty_slot(Renderer *r, int px, int py) {
    fill_rect_px(r, px, py, ICON_SIZE, ICON_SIZE, (SDL_Color){16, 19, 32, 255});
    fill_rect_px(r, px, py, ICON_SIZE, 1, (SDL_Color){67, 72, 96, 255});
    fill_rect_px(r, px, py, 1, ICON_SIZE, (SDL_Color){48, 54, 77, 255});
    fill_rect_px(r, px, py + ICON_SIZE - 1, ICON_SIZE, 1, (SDL_Color){8, 11, 23, 255});
    fill_rect_px(r, px + ICON_SIZE - 1, py, 1, ICON_SIZE, (SDL_Color){8, 11, 23, 255});
}

// Status icons are drawn from rows of pixels. Each character in keys picks the
// matching colour; any other character leaves the pixel empty.
static void draw_icon_pattern(Renderer *r, int px, int py, const char *const rows[], int height, const char *keys, const SDL_Color colors[]) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; rows[y][x] != '\0'; x++) {
            for (int k = 0; keys[k] != '\0'; k++) {
                if (rows[y][x] == keys[k]) {
                    fill_rect_px(r, px + x, py + y, 1, 1, colors[k]);
                }
            }
        }
    }
}

// A 7 x 9 green drop marks a poisoned player.
void draw_icon_poison(Renderer *r, int px, int py) {
    static const char *const rows[9] = {
        "...o...",
        "..olo..",
        "..olo..",
        ".olggo.",
        "olggggo",
        "olggggo",
        "oggggdo",
        ".oggdo.",
        "..ooo..",
    };
    static const SDL_Color colors[4] = {
        {16, 40, 22, 255}, {176, 255, 170, 255}, {70, 200, 80, 255}, {36, 130, 50, 255}
    };
    draw_icon_pattern(r, px, py, rows, 9, "olgd", colors);
}

// A 7 x 7 snowflake with a dark shadow marks a frozen enemy.
void draw_icon_frozen(Renderer *r, int px, int py) {
    static const char *const rows[7] = {
        "w..w..w",
        ".w.w.w.",
        "..www..",
        "wwwwwww",
        "..www..",
        ".w.w.w.",
        "w..w..w",
    };
    static const SDL_Color shadow[1] = {{16, 30, 48, 255}};
    static const SDL_Color ice[1] = {{210, 245, 255, 255}};
    draw_icon_pattern(r, px + 1, py + 1, rows, 7, "w", shadow);
    draw_icon_pattern(r, px, py, rows, 7, "w", ice);
}
