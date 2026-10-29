#ifndef INVENTORY_HEADER_H
#define INVENTORY_HEADER_H

typedef enum {
    INVENTORY_NONE = 0,
    INVENTORY_CLOSED,
    INVENTORY_USE,
    INVENTORY_EQUIP,
    INVENTORY_EQUIP_OFF_HAND,
    INVENTORY_DROP,
    INVENTORY_UNEQUIP
} InventoryResult;

typedef struct {
    int selected;
    int equipment_tab;
    int accessory_slot;
} InventoryScreen;

int inventory_selected_index(const InventoryScreen *s);
int inventory_visible_rows(int tiles_y);
int inventory_list_start(int selected, int count, int visible_rows);
void inventory_clamp_selection(InventoryScreen *s, int bag_count);
void inventory_init(InventoryScreen *s);
InventoryResult inventory_handle_key(InventoryScreen *s, int scancode, int item_count, int close_key);

#endif
