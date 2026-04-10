#ifndef CLASS_SELECT_HEADER_H
#define CLASS_SELECT_HEADER_H

typedef enum {
    CLASS_SELECT_NONE = 0,
    CLASS_SELECT_CONFIRMED,
    CLASS_SELECT_CANCELLED
} ClassSelectResult;

typedef struct {
    int selected;
} ClassSelectScreen;

void class_select_init(ClassSelectScreen *s);
ClassSelectResult class_select_handle_key(ClassSelectScreen *s, int scancode);

#endif