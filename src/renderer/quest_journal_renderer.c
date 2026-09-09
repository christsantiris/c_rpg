#include "quest_journal_renderer.h"
#include "sprites.h"

static void draw_panel(Renderer *r, SDL_Rect rect) {
    SDL_SetRenderDrawColor(r->sdl, 55, 52, 77, 255);
    SDL_RenderFillRect(r->sdl, &rect);
    SDL_Rect inner = {rect.x + 2, rect.y + 2, rect.w - 4, rect.h - 4};
    SDL_SetRenderDrawColor(r->sdl, 14, 14, 29, 255);
    SDL_RenderFillRect(r->sdl, &inner);
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
    SDL_Color green = {80, 170, 90, 255};
    int margin = 50;
    int top = 110;
    int height = r->screen_h - 180;
    int list_width = 330;
    int detail_x = margin + list_width + 16;
    int detail_width = r->screen_w - detail_x - margin;

    renderer_draw_text(r, "QUEST JOURNAL", r->screen_w / 2 - 110, 35,
        gold, r->font_large);
    SDL_Color active_tab = screen->tab == QUEST_TAB_ACTIVE ? gold : dimmed;
    SDL_Color completed_tab = screen->tab == QUEST_TAB_COMPLETED
        ? gold : dimmed;
    renderer_draw_text(r, "ACTIVE", margin, 82, active_tab, r->font_small);
    renderer_draw_text(r, "COMPLETED", margin + 120, 82, completed_tab,
        r->font_small);

    draw_panel(r, (SDL_Rect){margin, top, list_width, height});
    draw_panel(r, (SDL_Rect){detail_x, top, detail_width, height});

    int count = quest_journal_count(g, screen->tab);
    if (count == 0) {
        const char *empty = screen->tab == QUEST_TAB_ACTIVE
            ? "NO ACTIVE QUESTS" : "NO COMPLETED QUESTS";
        renderer_draw_text(r, empty, margin + 20, top + 25, dimmed,
            r->font_small);
    }

    for (int index = 0; index < count; index++) {
        QuestJournalEntry entry;
        quest_journal_get_entry(g, screen->tab, index, &entry);
        int y = top + 25 + index * 42;
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
        int y = top + 22;
        renderer_draw_text(r, entry.title, x, y, gold, r->font_large);
        y += 42;
        char line[96];
        SDL_snprintf(line, sizeof(line), "QUEST GIVER: %s", entry.giver);
        renderer_draw_text(r, line, x, y, dimmed, r->font_tiny);
        y += 24;
        const char *status = entry.state == 3 ? "COMPLETED" :
            (entry.state == 2 ? "RETURN TO NPC" : "ACTIVE");
        SDL_snprintf(line, sizeof(line), "STATUS: %s", status);
        renderer_draw_text(r, line, x, y,
            entry.state == 1 ? white : green, r->font_tiny);
        y += 35;
        renderer_draw_text(r, entry.summary_line_1, x, y, white,
            r->font_tiny);
        y += 20;
        renderer_draw_text(r, entry.summary_line_2, x, y, white,
            r->font_tiny);
        y += 38;
        renderer_draw_text(r, "OBJECTIVES", x, y, gold, r->font_small);
        y += 30;
        int complete_count = 0;
        for (int objective = 0; objective < entry.objective_count;
            objective++) {
            int complete = entry.objective_complete[objective];
            complete_count += complete;
            SDL_snprintf(line, sizeof(line), "[%c] %s - %s %d",
                complete ? 'X' : ' ', entry.objectives[objective],
                entry.area, entry.stages[objective]);
            renderer_draw_text(r, line, x, y, complete ? green : white,
                r->font_tiny);
            y += 25;
        }
        y += 12;
        SDL_snprintf(line, sizeof(line), "PROGRESS: %d / %d",
            complete_count, entry.objective_count);
        renderer_draw_text(r, line, x, y, white, r->font_small);
        y += 32;
        SDL_snprintf(line, sizeof(line), "REWARD: %d GOLD  |  %d SCORE",
            entry.reward_gold, entry.reward_score);
        renderer_draw_text(r, line, x, y, gold, r->font_small);
    }

    renderer_draw_text(r,
        "UP/DOWN SELECT   TAB/LEFT/RIGHT CHANGE TAB   Q/ESC CLOSE",
        r->screen_w / 2 - 285, r->screen_h - 45, dimmed, r->font_tiny);
}
