#ifndef HARBOR_HEADER_H
#define HARBOR_HEADER_H

typedef enum {
    HARBOR_NONE = 0,
    HARBOR_BOARD,
    HARBOR_MAP_REQUIRED,
    HARBOR_CLOSED
} HarborResult;

typedef struct {
    int selected;
} HarborScreen;

void harbor_init(HarborScreen *s);
HarborResult harbor_activate(const HarborScreen *s, int has_map);
HarborResult harbor_handle_key(HarborScreen *s, int scancode, int has_map);

#endif
