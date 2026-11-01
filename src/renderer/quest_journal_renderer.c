#include "quest_journal_renderer.h"
#include "game_renderer.h"
#include "sprites.h"

static void draw_panel(Renderer *r, SDL_Rect rect) {
    SDL_SetRenderDrawColor(r->sdl, 55, 52, 77, 255);
    SDL_RenderFillRect(r->sdl, &rect);
    SDL_Rect inner = {rect.x + 2, rect.y + 2, rect.w - 4, rect.h - 4};
    SDL_SetRenderDrawColor(r->sdl, 14, 14, 29, 255);
    SDL_RenderFillRect(r->sdl, &inner);
}

static void draw_boss_progress(Renderer *r, const GameState *g) {
    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color green = {80, 170, 90, 255};
    SDL_Color dimmed = {105, 105, 120, 255};
    int top = r->screen_h < 320 ? 100 : 110;
    int gap = r->screen_h < 400 ? 18 : 42;
    int available_gap = (r->screen_h - top - 68) / JOURNAL_BOSS_COUNT;
    if (available_gap < gap) {
        gap = available_gap > 12 ? available_gap : 12;
    }
    int y = top + 8;
    int defeated = 0;
    draw_panel(r, (SDL_Rect){50, top, r->screen_w - 100,
        JOURNAL_BOSS_COUNT * gap + 42});
    for (int i = 0; i < JOURNAL_BOSS_COUNT; i++) {
        BossJournalEntry boss;
        quest_journal_get_boss(g, i, &boss);
        defeated += boss.defeated;
        SDL_Color color = boss.defeated ? green : white;
        char name[48];
        SDL_snprintf(name, sizeof(name), "[%c] %s", boss.defeated ? 'X' : ' ', boss.name);
        renderer_draw_text(r, name, 65, y, color, r->font_tiny);
        renderer_draw_text(r, boss.area, r->screen_w / 2 - 30, y, dimmed, r->font_tiny);
        renderer_draw_text(r, boss.defeated ? "DEFEATED" : "UNDEFEATED",
            r->screen_w - 145, y, color, r->font_tiny);
        y += gap;
    }
    char progress[48];
    SDL_snprintf(progress, sizeof(progress), "BOSSES DEFEATED: %d / %d", defeated, JOURNAL_BOSS_COUNT);
    renderer_draw_text(r, progress, 65, y + 8, gold, r->font_tiny);
    if (g->crown_of_unbound) {
        renderer_draw_text(r, "TROPHY: CROWN OF THE UNBOUND", 65, y + 24, gold, r->font_tiny);
    }
    renderer_draw_text(r, "TAB/LEFT/RIGHT CHANGE TAB   Q/ESC CLOSE",
        r->screen_w / 2 - 148, r->screen_h - 18, dimmed, r->font_tiny);
}

// Draws or measures text wrapped to the width; returns the height beyond its first line.
static int wrapped(Renderer *r, const char *text, int x, int y, int width, TTF_Font *font, SDL_Color color, int draw) {
    int char_w = 0;
    if (!font || TTF_SizeText(font, "M", &char_w, NULL) != 0 || char_w <= 0) {
        return 0;
    }
    int lines = draw_wrapped_text(r, text, x, y, width / char_w, font, color, draw);
    return lines > 1 ? (lines - 1) * (TTF_FontHeight(font) + 8) : 0;
}

// Lays out one quest and returns its bottom edge; compact spacing fits short windows.
static int draw_details(Renderer *r, const QuestJournalEntry *entry, int x, int y, int width, int compact, int draw) {
    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dimmed = {105, 105, 120, 255};
    SDL_Color green = {80, 170, 90, 255};
    char line[96];
    y += wrapped(r, entry->title, x, y, width, r->font_large, gold, draw) + (compact ? 28 : 42);
    SDL_snprintf(line, sizeof(line), "QUEST GIVER: %s", entry->giver);
    y += wrapped(r, line, x, y, width, r->font_tiny, dimmed, draw) + (compact ? 16 : 24);
    const char *status = entry->state == 3 ? "COMPLETED" :
        (entry->state == 2 ? "COMPLETED - REWARD PENDING" : "ACTIVE");
    SDL_snprintf(line, sizeof(line), "STATUS: %s", status);
    y += wrapped(r, line, x, y, width, r->font_tiny, entry->state == 1 ? white : green, draw) + (compact ? 22 : 35);
    y += wrapped(r, entry->summary_line_1, x, y, width, r->font_tiny, white, draw) + (compact ? 16 : 20);
    y += wrapped(r, entry->summary_line_2, x, y, width, r->font_tiny, white, draw) + (compact ? 22 : 38);
    y += wrapped(r, "OBJECTIVES", x, y, width, r->font_small, gold, draw) + (compact ? 20 : 30);
    int complete_count = 0;
    for (int objective = 0; objective < entry->objective_count; objective++) {
        int complete = entry->objective_complete[objective];
        complete_count += complete;
        SDL_snprintf(line, sizeof(line), "[%c] %s - %s %d", complete ? 'X' : ' ', entry->objectives[objective],
            entry->objective_areas[objective], entry->stages[objective]);
        y += wrapped(r, line, x, y, width, r->font_tiny, complete ? green : white, draw) + (compact ? 16 : 25);
    }
    y += compact ? 4 : 12;
    SDL_snprintf(line, sizeof(line), "PROGRESS: %d / %d", complete_count, entry->objective_count);
    y += wrapped(r, line, x, y, width, r->font_small, white, draw) + (compact ? 20 : 32);
    if (entry->reward_item && entry->reward_gold > 0) {
        SDL_snprintf(line, sizeof(line), "REWARD: %d GOLD + %s", entry->reward_gold, entry->reward_item);
    } else if (entry->reward_item) {
        SDL_snprintf(line, sizeof(line), "REWARD: %s  |  %d SCORE", entry->reward_item, entry->reward_score);
    } else {
        SDL_snprintf(line, sizeof(line), "REWARD: %d GOLD  |  %d SCORE", entry->reward_gold, entry->reward_score);
    }
    return y + wrapped(r, line, x, y, width, r->font_small, gold, draw) + 14;
}

void quest_journal_draw(Renderer *r, const GameState *g, const QuestJournalScreen *screen) {
    int full_tiles_x = r->screen_w / TILE_SIZE;
    for (int y = 0; y < r->tiles_y; y++) {
        for (int x = 0; x < full_tiles_x; x++) {
            draw_floor(r, x, y);
        }
    }

    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dimmed = {105, 105, 120, 255};
    int margin = 50;
    int top = 110;
    int height = r->screen_h - 180;
    int list_width = 330;
    // Narrow windows give the details the full width; UP/DOWN still steps through the quests.
    int narrow = r->screen_w - 2 * margin - list_width - 16 < 400;
    if (narrow) {
        margin = 16;
        list_width = 0;
    }
    int detail_x = narrow ? margin : margin + list_width + 16;
    int detail_width = r->screen_w - detail_x - margin;

    renderer_draw_text(r, "QUEST JOURNAL", r->screen_w / 2 - 110, 35,
        gold, r->font_large);
    int active = game_active_quest_count(g);
    char active_label[32];
    SDL_snprintf(active_label, sizeof(active_label), "ACTIVE %d/%d", active, MAX_ACTIVE_QUESTS);
    const char *tabs[QUEST_TAB_COUNT] = {active_label, "COMPLETED", "BOSSES"};
    int tab_x = margin;
    for (int tab = 0; tab < QUEST_TAB_COUNT; tab++) {
        renderer_draw_text(r, tabs[tab], tab_x, 82, screen->tab == tab ? gold : dimmed, r->font_small);
        int width = 0;
        TTF_SizeText(r->font_small, tabs[tab], &width, NULL);
        tab_x += width + 24;
    }
    if (screen->tab == QUEST_TAB_BOSSES) {
        draw_boss_progress(r, g);
        return;
    }

    if (!narrow) {
        draw_panel(r, (SDL_Rect){margin, top, list_width, height});
    }
    draw_panel(r, (SDL_Rect){detail_x, top, detail_width, height});

    int count = quest_journal_count(g, screen->tab);
    if (narrow && count > 0) {
        char position[32];
        int width = 0;
        SDL_snprintf(position, sizeof(position), "QUEST %d OF %d", screen->selected + 1, count);
        TTF_SizeText(r->font_tiny, position, &width, NULL);
        renderer_draw_text(r, position, r->screen_w - margin - width, 84, dimmed, r->font_tiny);
    }
    if (count == 0) {
        const char *empty = screen->tab == QUEST_TAB_ACTIVE ? "NO ACTIVE QUESTS" : "NO COMPLETED QUESTS";
        renderer_draw_text(r, empty, margin + 20, top + 25, dimmed,
            r->font_small);
    }

    int visible = (height - 35) / 42;
    if (visible < 1) {
        visible = 1;
    }
    int first = screen->selected >= visible ? screen->selected - visible + 1 : 0;
    for (int index = first; !narrow && index < count && index < first + visible; index++) {
        QuestJournalEntry entry;
        quest_journal_get_entry(g, screen->tab, index, &entry);
        int y = top + 25 + (index - first) * 42;
        if (index == screen->selected) {
            renderer_draw_text(r, ">", margin + 12, y, gold,
                r->font_small);
        }
        renderer_draw_text(r, entry.title, margin + 35, y,
            index == screen->selected ? gold : white, r->font_tiny);
    }

    QuestJournalEntry entry;
    if (quest_journal_get_entry(g, screen->tab, screen->selected, &entry)) {
        int x = detail_x + 20;
        int width = detail_width - 40;
        // Short windows tighten the spacing; the clip keeps tiny windows inside the panel.
        int compact = draw_details(r, &entry, x, top + 22, width, 0, 0) > top + height - 12;
        SDL_Rect inside = {detail_x + 2, top + 2, detail_width - 4, height - 4};
        SDL_RenderSetClipRect(r->sdl, &inside);
        draw_details(r, &entry, x, compact ? top + 14 : top + 22, width, compact, 1);
        SDL_RenderSetClipRect(r->sdl, NULL);
    }

    renderer_draw_text(r, active > MAX_ACTIVE_QUESTS ?
        "Existing quests kept. Finish active quests to free a slot." :
        "Completed quests free a slot. Escape and recovery use no slots.",
        margin, 62, dimmed, r->font_tiny);
    renderer_draw_text(r,
        "UP/DOWN SELECT   TAB/LEFT/RIGHT CHANGE TAB   Q/ESC CLOSE",
        r->screen_w / 2 - 285, r->screen_h - 45, dimmed, r->font_tiny);
}
