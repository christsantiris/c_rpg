#include "equipment_compare_renderer.h"

static void format_weapon_range(const Item *item, char *text, int size) {
    if (item->is_ranged) {
        SDL_snprintf(text, size, "%d TILES", item->range);
    } else {
        SDL_snprintf(text, size, "MELEE");
    }
}

static void format_weapon_trait(const Item *item, char *text, int size) {
    if (item->critical_chance_bonus > 0) {
        SDL_snprintf(text, size, "+%d%% MELEE CRIT",
            item->critical_chance_bonus);
    } else if (item->cleave_percent > 0) {
        SDL_snprintf(text, size, "%d%% CLEAVE", item->cleave_percent);
    } else if (item->pierces_targets) {
        SDL_snprintf(text, size, "PIERCES ALL TARGETS");
    } else if (item->spell_power_bonus > 0) {
        SDL_snprintf(text, size, "+%d SPELL POWER",
            item->spell_power_bonus);
    } else if (item->armor_penetration_percent > 0) {
        SDL_snprintf(text, size, "IGNORES %d%% DEFENSE",
            item->armor_penetration_percent);
    } else {
        SDL_snprintf(text, size, "NONE");
    }
}

static void draw_compare_column(Renderer *r, const GameState *g, const Item *item, const Item *equipped, int x, int y, const char *heading, int selected_column) {
    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dimmed = {105, 105, 120, 255};
    SDL_Color green = {80, 170, 90, 255};
    SDL_Color red = {205, 70, 70, 255};
    renderer_draw_text(r, heading, x + 10, y + 8, gold, r->font_tiny);

    if (!item) {
        renderer_draw_text(r, "NONE", x + 10, y + 25, dimmed,
            r->font_small);
        return;
    }

    int allowed = item_class_allowed(item, g->player.player_class);
    renderer_draw_text(r, item->name, x + 10, y + 25,
        allowed || !selected_column ? white : red, r->font_small);

    char line[80];
    int attack_difference = equipped
        ? item->attack_bonus - equipped->attack_bonus
        : item->attack_bonus;
    if (selected_column) {
        SDL_snprintf(line, sizeof(line), "ATK: +%d  (%+d)",
            item->attack_bonus, attack_difference);
    } else {
        SDL_snprintf(line, sizeof(line), "ATK: +%d", item->attack_bonus);
    }
    SDL_Color attack_color = white;
    if (selected_column && attack_difference > 0) {
        attack_color = green;
    } else if (selected_column && attack_difference < 0) {
        attack_color = red;
    }
    renderer_draw_text(r, line, x + 10, y + 47, attack_color,
        r->font_tiny);

    char range[32];
    format_weapon_range(item, range, sizeof(range));
    SDL_snprintf(line, sizeof(line), "RANGE: %s", range);
    renderer_draw_text(r, line, x + 10, y + 63, white, r->font_tiny);

    SDL_snprintf(line, sizeof(line), "HANDS: %s", item_hands_label(item));
    renderer_draw_text(r, line, x + 10, y + 79, white, r->font_tiny);

    char trait[48];
    format_weapon_trait(item, trait, sizeof(trait));
    SDL_snprintf(line, sizeof(line), "TRAIT: %s", trait);
    renderer_draw_text(r, line, x + 10, y + 95, white, r->font_tiny);

    SDL_snprintf(line, sizeof(line), "PRICE: %d GOLD", item->value);
    renderer_draw_text(r, line, x + 10, y + 111, white, r->font_tiny);

    if (selected_column && !allowed) {
        SDL_snprintf(line, sizeof(line), "%s | REQUIRES %s",
            item_rarity_label(item), item_class_label(item));
    } else {
        SDL_snprintf(line, sizeof(line), "%s | %s",
            item_rarity_label(item), item_class_label(item));
    }
    renderer_draw_text(r, line, x + 10, y + 127,
        allowed || !selected_column ? dimmed : red, r->font_tiny);
}

void draw_weapon_comparison(Renderer *r, const GameState *g, const Item *selected, const Item *equipped, int y) {
    int cx = r->screen_w / 2;
    int column_width = 300;
    int panel_x = cx - column_width - 8;
    int panel_width = column_width * 2 + 16;
    SDL_Rect border = {panel_x, y, panel_width, 148};
    SDL_Rect panel = {panel_x + 2, y + 2, panel_width - 4, 144};
    SDL_SetRenderDrawColor(r->sdl, 55, 52, 77, 255);
    SDL_RenderFillRect(r->sdl, &border);
    SDL_SetRenderDrawColor(r->sdl, 14, 14, 29, 255);
    SDL_RenderFillRect(r->sdl, &panel);
    SDL_SetRenderDrawColor(r->sdl, 55, 52, 77, 255);
    SDL_RenderDrawLine(r->sdl, cx, y + 2, cx, y + 145);

    draw_compare_column(r, g, selected, equipped, panel_x + 2, y,
        "SELECTED", 1);
    draw_compare_column(r, g, equipped, equipped, cx + 2, y,
        "CURRENTLY EQUIPPED", 0);
}

static void draw_armor_column(Renderer *r, const GameState *g, const Item *item, const Item *equipped, int x, int y, const char *heading, int selected_column) {
    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dimmed = {105, 105, 120, 255};
    SDL_Color green = {80, 170, 90, 255};
    SDL_Color red = {205, 70, 70, 255};
    renderer_draw_text(r, heading, x + 10, y + 8, gold, r->font_tiny);
    if (!item) {
        renderer_draw_text(r, "NONE", x + 10, y + 25, dimmed,
            r->font_small);
        return;
    }

    int allowed = item_class_allowed(item, g->player.player_class);
    renderer_draw_text(r, item->name, x + 10, y + 25,
        allowed || !selected_column ? white : red, r->font_small);
    int defense_difference = equipped
        ? item->defense_bonus - equipped->defense_bonus
        : item->defense_bonus;
    char line[80];
    if (selected_column) {
        SDL_snprintf(line, sizeof(line), "DEF: +%d  (%+d)",
            item->defense_bonus, defense_difference);
    } else {
        SDL_snprintf(line, sizeof(line), "DEF: +%d", item->defense_bonus);
    }
    SDL_Color defense_color = white;
    if (selected_column && defense_difference > 0) {
        defense_color = green;
    } else if (selected_column && defense_difference < 0) {
        defense_color = red;
    }
    renderer_draw_text(r, line, x + 10, y + 47, defense_color,
        r->font_tiny);
    SDL_snprintf(line, sizeof(line), "MAX HP: +%d  MAX MP: +%d",
        item->max_hp_bonus, item->max_mp_bonus);
    renderer_draw_text(r, line, x + 10, y + 63, white, r->font_tiny);
    SDL_snprintf(line, sizeof(line), "EVASION: %d%%", item->evasion_chance);
    renderer_draw_text(r, line, x + 10, y + 79, white, r->font_tiny);
    SDL_snprintf(line, sizeof(line), "SPELL COST: -%d%%",
        item->spell_cost_reduction_percent);
    renderer_draw_text(r, line, x + 10, y + 95, white, r->font_tiny);
    SDL_snprintf(line, sizeof(line), "PRICE: %d GOLD", item->value);
    renderer_draw_text(r, line, x + 10, y + 111, white, r->font_tiny);
    if (selected_column && !allowed) {
        SDL_snprintf(line, sizeof(line), "%s | REQUIRES %s",
            item_rarity_label(item), item_class_label(item));
    } else {
        SDL_snprintf(line, sizeof(line), "%s | %s",
            item_rarity_label(item), item_class_label(item));
    }
    renderer_draw_text(r, line, x + 10, y + 127,
        allowed || !selected_column ? dimmed : red, r->font_tiny);
}

void draw_armor_comparison(Renderer *r, const GameState *g, const Item *selected, const Item *equipped, int y) {
    int cx = r->screen_w / 2;
    int column_width = 300;
    int panel_x = cx - column_width - 8;
    int panel_width = column_width * 2 + 16;
    SDL_Rect border = {panel_x, y, panel_width, 148};
    SDL_Rect panel = {panel_x + 2, y + 2, panel_width - 4, 144};
    SDL_SetRenderDrawColor(r->sdl, 55, 52, 77, 255);
    SDL_RenderFillRect(r->sdl, &border);
    SDL_SetRenderDrawColor(r->sdl, 14, 14, 29, 255);
    SDL_RenderFillRect(r->sdl, &panel);
    SDL_SetRenderDrawColor(r->sdl, 55, 52, 77, 255);
    SDL_RenderDrawLine(r->sdl, cx, y + 2, cx, y + 145);
    draw_armor_column(r, g, selected, equipped, panel_x + 2, y,
        "SELECTED", 1);
    draw_armor_column(r, g, equipped, equipped, cx + 2, y,
        "CURRENTLY EQUIPPED", 0);
}
