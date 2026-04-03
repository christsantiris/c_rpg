#ifndef SHOP_HEADER_H
#define SHOP_HEADER_H

#include "../game/item.h"

#define MAX_SHOP_ITEMS 8

typedef enum {
    SHOP_NONE = 0,
    SHOP_CLOSED,
    SHOP_BUY
} ShopResult;

typedef enum {
    SHOP_TYPE_ALCHEMIST,
    SHOP_TYPE_BLACKSMITH
} ShopType;

typedef struct {
    int      selected;
    ShopType type;
    Item     items[MAX_SHOP_ITEMS];
    int      item_count;
} ShopScreen;

void       shop_init(ShopScreen *s, ShopType type);
ShopResult shop_handle_key(ShopScreen *s, int scancode);

#endif