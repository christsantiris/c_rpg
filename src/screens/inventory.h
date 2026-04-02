#ifndef INVENTORY_HEADER_H
#define INVENTORY_HEADER_H

typedef enum {
    INVENTORY_NONE = 0,
    INVENTORY_CLOSED,
    INVENTORY_USE,
    INVENTORY_EQUIP
} InventoryResult;

typedef struct {
    int selected;
} InventoryScreen;

void            inventory_init(InventoryScreen *s);
InventoryResult inventory_handle_key(InventoryScreen *s, int scancode, int item_count);

#endif