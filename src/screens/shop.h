#ifndef SHOP_HEADER_H
#define SHOP_HEADER_H

#include "../game/game.h"

#define MAX_SHOP_ITEMS 33
#define ARROW_BUNDLE_SIZE 20

typedef enum {
    SHOP_NONE = 0,
    SHOP_CLOSED,
    SHOP_BUY,
    SHOP_SELL
} ShopResult;

typedef enum {
    SHOP_TYPE_ALCHEMIST,
    SHOP_TYPE_BLACKSMITH,
    SHOP_TYPE_HEALER,
    SHOP_TYPE_WITCH,
    SHOP_TYPE_APOTHECARY
} ShopType;

typedef struct {
    int selected;
    ShopType type;
    Item items[MAX_SHOP_ITEMS];
    int item_count;
    int mode; // 0 = buy, 1 = sell
    int stock_tier;
} ShopScreen;

void shop_init(ShopScreen *s, ShopType type, int defeated_bosses);
ShopResult shop_handle_key(ShopScreen *s, int scancode);
int shop_buy_price(const Item *item);
int shop_purchase_price(const GameState *g, const Item *item);
int shop_purchase(GameState *g, const Item *item);
int shop_sell_price(const Item *item);
int shop_accepts_item(ShopType type, const Item *item);

#endif
