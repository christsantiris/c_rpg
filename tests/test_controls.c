#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/controls.h"
#include "../src/screens/inventory.h"
#include "../src/screens/quest_journal.h"
#include "../src/screens/controls_screen.h"
#include "../src/screens/landing.h"
#include <string.h>

static void test_controls_screen(void) {
    int bindings[CONTROL_COUNT];
    controls_reset(bindings);
    ControlsScreen s;
    controls_screen_init(&s);
    controls_screen_handle_key(&s, SDL_SCANCODE_UP, bindings);
    int top = s.selected;
    for (int i = 0; i < CONTROL_COUNT + 2; i++) {
        controls_screen_handle_key(&s, SDL_SCANCODE_DOWN, bindings);
    }
    ASSERT("the controls list stops at its first and last commands",
        top == 0 && s.selected == CONTROL_COUNT - 1);

    s.selected = CONTROL_INVENTORY;
    ControlsResult result = controls_screen_handle_key(&s, SDL_SCANCODE_K, bindings);
    ASSERT("a letter does nothing until Enter starts a rebind",
        result == CONTROLS_NONE && bindings[CONTROL_INVENTORY] == SDL_SCANCODE_I);

    controls_screen_handle_key(&s, SDL_SCANCODE_RETURN, bindings);
    int waiting = s.waiting_for_key;
    result = controls_screen_handle_key(&s, SDL_SCANCODE_K, bindings);
    ASSERT("Enter then a key rebinds the selected command",
        waiting && result == CONTROLS_REBOUND && !s.waiting_for_key &&
        bindings[CONTROL_INVENTORY] == SDL_SCANCODE_K && s.swapped_action == -1);

    controls_screen_handle_key(&s, SDL_SCANCODE_RETURN, bindings);
    result = controls_screen_handle_key(&s, SDL_SCANCODE_T, bindings);
    ASSERT("taking a used key reports which command swapped",
        result == CONTROLS_REBOUND && bindings[CONTROL_INVENTORY] == SDL_SCANCODE_T &&
        bindings[CONTROL_TALK] == SDL_SCANCODE_K && s.swapped_action == CONTROL_TALK);

    controls_screen_handle_key(&s, SDL_SCANCODE_RETURN, bindings);
    ControlsResult arrow = controls_screen_handle_key(&s, SDL_SCANCODE_UP, bindings);
    int still_waiting = s.waiting_for_key;
    ControlsResult cancel = controls_screen_handle_key(&s, SDL_SCANCODE_ESCAPE, bindings);
    ASSERT("a fixed key is refused and Escape cancels the rebind",
        arrow == CONTROLS_KEY_RESERVED && still_waiting && cancel == CONTROLS_NONE &&
        !s.waiting_for_key && s.selected == CONTROL_INVENTORY &&
        bindings[CONTROL_INVENTORY] == SDL_SCANCODE_T);

    controls_screen_handle_key(&s, SDL_SCANCODE_RETURN, bindings);
    controls_screen_handle_key(&s, SDL_SCANCODE_LEFT, bindings);
    int refused_shown = s.key_refused;
    controls_screen_handle_key(&s, SDL_SCANCODE_J, bindings);
    ASSERT("the refusal note shows until a usable key is pressed",
        refused_shown && !s.key_refused && bindings[CONTROL_INVENTORY] == SDL_SCANCODE_J);

    int screen_w = 1280;
    int row_y = CONTROLS_LIST_TOP + CONTROL_HELP * CONTROLS_ROW_H + CONTROLS_ROW_H / 2;
    controls_screen_handle_click(&s, screen_w / 2, row_y, screen_w);
    int clicked_row = s.selected == CONTROL_HELP && s.waiting_for_key;
    controls_screen_handle_click(&s, screen_w / 2, row_y, screen_w);
    int click_cancelled = !s.waiting_for_key;
    controls_screen_handle_click(&s, 10, row_y, screen_w);
    ASSERT("clicking a row starts rebinding it and another click cancels",
        clicked_row && click_cancelled && !s.waiting_for_key);

    ASSERT("Escape closes the controls screen when not rebinding",
        controls_screen_handle_key(&s, SDL_SCANCODE_ESCAPE, bindings) == CONTROLS_CLOSED);
    ASSERT("every command has a label for the controls screen",
        strcmp(controls_label(CONTROL_MOVE_LEFT), "Move left / interact") == 0 &&
        strcmp(controls_label(CONTROL_HELP), "Help") == 0 &&
        strcmp(controls_label(CONTROL_COUNT), "") == 0);

    LandingScreen menu;
    landing_init(&menu);
    int title_items = landing_item_count(&menu);
    menu.has_active_game = 1;
    menu.selected = 6;
    LandingResult controls_choice = landing_handle_key(&menu, SDL_SCANCODE_RETURN);
    menu.selected = 7;
    LandingResult quit_choice = landing_handle_key(&menu, SDL_SCANCODE_RETURN);
    ASSERT("the in-game menu lists Controls before Quit; the title menu does not",
        title_items == 5 && landing_item_count(&menu) == 8 &&
        controls_choice == LANDING_CONTROLS && quit_choice == LANDING_QUIT);
    int item_y = 720 / 2 - 20 + 6 * 36 + 10;
    ASSERT("clicking Controls in the in-game menu opens the controls screen",
        landing_handle_click(&menu, 1280 / 2, item_y, 1280, 720) == LANDING_CONTROLS);
}

void test_controls(void) {
    printf("Controls tests:\n");
    int bindings[CONTROL_COUNT];
    controls_reset(bindings);
    ASSERT("default controls match the original keys",
        bindings[CONTROL_MOVE_UP] == SDL_SCANCODE_W &&
        bindings[CONTROL_MOVE_LEFT] == SDL_SCANCODE_A &&
        bindings[CONTROL_DESCEND] == SDL_SCANCODE_PERIOD &&
        bindings[CONTROL_INVENTORY] == SDL_SCANCODE_I &&
        bindings[CONTROL_HELP] == SDL_SCANCODE_H &&
        controls_valid(bindings));
    ASSERT("a bound key finds its command and an unbound key finds none",
        controls_action_for_key(bindings, SDL_SCANCODE_F) == CONTROL_RANGED_ATTACK &&
        controls_action_for_key(bindings, SDL_SCANCODE_K) == -1);

    int assigned = controls_assign(bindings, CONTROL_INVENTORY, SDL_SCANCODE_K);
    ASSERT("a free key can be bound to a command",
        assigned && bindings[CONTROL_INVENTORY] == SDL_SCANCODE_K &&
        controls_action_for_key(bindings, SDL_SCANCODE_I) == -1);

    assigned = controls_assign(bindings, CONTROL_INVENTORY, SDL_SCANCODE_W);
    ASSERT("taking another command's key swaps the two keys",
        assigned && bindings[CONTROL_INVENTORY] == SDL_SCANCODE_W &&
        bindings[CONTROL_MOVE_UP] == SDL_SCANCODE_K && controls_valid(bindings));

    int reserved_blocked = 1;
    int reserved[5] = {SDL_SCANCODE_ESCAPE, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN,
        SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT};
    for (int i = 0; i < 5; i++) {
        reserved_blocked &= !controls_assign(bindings, CONTROL_TALK, reserved[i]);
    }
    ASSERT("Escape and the arrow keys cannot be rebound",
        reserved_blocked && bindings[CONTROL_TALK] == SDL_SCANCODE_T);

    controls_reset(bindings);
    bindings[CONTROL_TALK] = SDL_SCANCODE_P;
    int duplicate_valid = controls_valid(bindings);
    controls_reset(bindings);
    bindings[CONTROL_TALK] = SDL_SCANCODE_ESCAPE;
    int reserved_valid = controls_valid(bindings);
    bindings[CONTROL_TALK] = 0;
    ASSERT("duplicate, reserved and empty keys make a set invalid",
        !duplicate_valid && !reserved_valid && !controls_valid(bindings));

    static GameState g;
    g.key_bindings[CONTROL_TALK] = SDL_SCANCODE_K;
    game_init(&g);
    ASSERT("a new game starts with the default controls",
        g.key_bindings[CONTROL_TALK] == SDL_SCANCODE_T && controls_valid(g.key_bindings));

    InventoryScreen inventory;
    inventory_init(&inventory);
    ASSERT("a remapped inventory key closes the inventory instead of I",
        inventory_handle_key(&inventory, SDL_SCANCODE_K, 3, SDL_SCANCODE_K) ==
            INVENTORY_CLOSED &&
        inventory_handle_key(&inventory, SDL_SCANCODE_I, 3, SDL_SCANCODE_K) ==
            INVENTORY_NONE &&
        inventory_handle_key(&inventory, SDL_SCANCODE_ESCAPE, 3, SDL_SCANCODE_K) ==
            INVENTORY_CLOSED);
    ASSERT("inventory commands win over an inventory key bound to the same letter",
        inventory_handle_key(&inventory, SDL_SCANCODE_E, 3, SDL_SCANCODE_E) ==
            INVENTORY_EQUIP &&
        inventory_handle_key(&inventory, SDL_SCANCODE_D, 3, SDL_SCANCODE_D) ==
            INVENTORY_DROP);

    QuestJournalScreen journal;
    quest_journal_init(&journal);
    int closed_by_k = quest_journal_handle_key(&journal, SDL_SCANCODE_K, 1,
        SDL_SCANCODE_K) == QUEST_JOURNAL_CLOSED;
    int closed_by_q = quest_journal_handle_key(&journal, SDL_SCANCODE_Q, 1,
        SDL_SCANCODE_K) == QUEST_JOURNAL_CLOSED;
    int tab_result = quest_journal_handle_key(&journal, SDL_SCANCODE_TAB, 1,
        SDL_SCANCODE_TAB);
    ASSERT("a remapped journal key closes the journal while Tab still changes tabs",
        closed_by_k && !closed_by_q && tab_result == QUEST_JOURNAL_NONE &&
        journal.tab == QUEST_TAB_COMPLETED);

    test_controls_screen();
}
