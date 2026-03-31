#ifndef NAME_ENTRY_HEADER_H
#define NAME_ENTRY_HEADER_H

#include <SDL2/SDL.h>

#define MAX_NAME_LEN 20

typedef enum {
    NAME_ENTRY_NONE = 0,
    NAME_ENTRY_CONFIRMED,
    NAME_ENTRY_CANCELLED
} NameEntryResult;

typedef struct {
    char     name[MAX_NAME_LEN + 1];
    int      length;
    Uint32   cursor_last_blink;
    int      cursor_visible;
} NameEntry;

void            name_entry_init(NameEntry *n);
void            name_entry_update(NameEntry *n);
NameEntryResult name_entry_handle_key(NameEntry *n, int scancode, const char *keyname);

#endif