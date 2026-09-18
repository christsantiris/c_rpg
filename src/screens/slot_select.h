#ifndef SLOT_SELECT_HEADER_H
#define SLOT_SELECT_HEADER_H

typedef enum {
    SLOT_NONE = 0,
    SLOT_SELECTED,
    SLOT_CANCELLED,
    SLOT_SAVE_CONFIRMED
} SlotResult;

typedef struct {
    int selected;
    int confirming_save;
    int overwriting_save;
} SlotSelect;

void       slot_select_init(SlotSelect *s);
SlotResult slot_select_handle_key(SlotSelect *s, int scancode);
void slot_select_begin_save(SlotSelect *s, int occupied);

#endif
