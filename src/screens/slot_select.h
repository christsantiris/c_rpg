#ifndef SLOT_SELECT_HEADER_H
#define SLOT_SELECT_HEADER_H

typedef enum {
    SLOT_NONE = 0,
    SLOT_SELECTED,
    SLOT_CANCELLED
} SlotResult;

typedef struct {
    int selected;
} SlotSelect;

void       slot_select_init(SlotSelect *s);
SlotResult slot_select_handle_key(SlotSelect *s, int scancode);

#endif