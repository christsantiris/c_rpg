#ifndef ITEM_ICONS_HEADER_H
#define ITEM_ICONS_HEADER_H

#include "renderer.h"
#include "../game/item.h"

#define ICON_SIZE 24

void draw_icon_weapon(Renderer *r, int px, int py);
void draw_icon_armor(Renderer *r, int px, int py);
void draw_icon_empty_slot(Renderer *r, int px, int py);

#endif