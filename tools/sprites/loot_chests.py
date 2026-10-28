#!/usr/bin/env python3
"""Native open/closed loot containers in the game's existing material palettes."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, save_sprite
from local_buildings import WOOD, IRON, STONE

OUT = (20, 17, 23)
PALETTES = (WOOD, [(25, 40, 35), (42, 64, 49), (75, 97, 65), (114, 132, 84), (159, 164, 109)], STONE, IRON)


def rectangle(layer, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            layer.put(x, y, color)


def chest(palette, opened):
    image = Image.new('RGBA', (24, 24))
    layer = Layer(24, 24)
    rectangle(layer, 3, 13, 20, 21, palette[1])
    rectangle(layer, 4, 14, 19, 19, palette[2])
    rectangle(layer, 4, 20, 19, 20, palette[0])
    if opened:
        rectangle(layer, 3, 4, 20, 11, palette[2])
        rectangle(layer, 4, 5, 19, 9, palette[1])
        rectangle(layer, 4, 6, 19, 6, palette[3])
        rectangle(layer, 4, 12, 19, 14, OUT)
        rectangle(layer, 5, 14, 18, 14, palette[0])
    else:
        rectangle(layer, 3, 9, 20, 14, palette[2])
        rectangle(layer, 4, 8, 19, 10, palette[3])
        rectangle(layer, 4, 13, 19, 13, OUT)
    for x in (5, 17):
        rectangle(layer, x, 5 if opened else 9, x + 1, 10 if opened else 12, IRON[2])
        rectangle(layer, x, 15, x + 1, 20, IRON[2])
    rectangle(layer, 10, 15, 12, 17, (211, 172, 85))
    if not opened:
        layer.put(11, 16, OUT)
    composite(image, layer, OUT)
    return image


if __name__ == '__main__':
    atlas = Image.new('RGBA', (48, 96))
    for row, palette in enumerate(PALETTES):
        for opened in range(2):
            atlas.alpha_composite(chest(palette, opened), (opened * 24, row * 24))
    save_sprite(atlas, Path(__file__).resolve().parents[2] / 'assets/images/loot-chests.bmp')
