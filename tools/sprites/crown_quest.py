#!/usr/bin/env python3
"""Royal relics and a warded vault door at Giant's Peaks' native pixel scale."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, poly_mask, line_points, flat_wall, save_sprite
from giants_crown import STONE, OUT

ROOT = Path(__file__).resolve().parents[2]
GOLD = [(99, 76, 34), (169, 126, 48), (226, 192, 101), (249, 229, 155)]


def polygon(layer, points, color):
    for x, y in poly_mask((layer.w, layer.h), points):
        layer.put(x, y, color)


def pedestal(kind):
    image = Image.new('RGBA', (24, 48))
    layer = Layer(24, 48)
    for y in range(34, 46):
        for x in range(5, 20):
            layer.put(x, y, STONE[4] if x < 8 else STONE[2] if x < 16 else STONE[1])
    for y in (33, 44):
        for x in range(3, 22):
            layer.put(x, y, STONE[5])
            layer.put(x, y + 1, STONE[2])
    if kind == 'crown':
        polygon(layer, [(5, 22), (9, 25), (12, 20), (16, 25), (20, 22), (18, 32), (7, 32)], GOLD[1])
        for x in range(7, 19):
            layer.put(x, 30, GOLD[3])
            layer.put(x, 32, GOLD[0])
        for x, y in ((6, 22), (12, 20), (19, 22)):
            layer.put(x, y, GOLD[3])
            layer.put(x, y + 1, GOLD[2])
        layer.put(12, 28, (105, 176, 141))
        layer.put(13, 28, (62, 106, 84))
    elif kind == 'seal':
        polygon(layer, [(8, 22), (17, 22), (20, 25), (18, 31), (12, 33), (6, 29), (6, 25)], GOLD[1])
        polygon(layer, [(9, 23), (16, 23), (18, 26), (16, 30), (11, 31), (8, 28)], GOLD[2])
        for x, y in line_points(10, 26, 15, 26):
            layer.put(x, y, STONE[0])
        for x, y in ((11, 24), (14, 24), (12, 27), (13, 27), (12, 29), (13, 29)):
            layer.put(x, y, STONE[0])
    composite(image, layer, OUT)
    return image


def inscription():
    image = Image.new('RGBA', (24, 48))
    layer = Layer(24, 48)
    polygon(layer, [(5, 19), (8, 16), (18, 17), (20, 21), (20, 45), (4, 45)], STONE[2])
    for x, y in line_points(7, 19, 7, 42):
        layer.put(x, y, STONE[4])
    for y in (23, 28, 33, 38):
        for x in range(10, 18):
            if (x + y) % 3:
                layer.put(x, y, GOLD[2])
                layer.put(x, y + 1, GOLD[0])
    for x in range(3, 22):
        layer.put(x, 45, STONE[4])
    composite(image, layer, OUT)
    return image


def door(opened):
    image = Image.new('RGBA', (24, 48))
    layer = Layer(24, 48)
    flat_wall(layer, 1, 5, 8, 46, STONE, row_h=7, base=3, seed=401)
    flat_wall(layer, 19, 22, 8, 46, STONE, row_h=7, base=2, seed=402)
    flat_wall(layer, 1, 22, 6, 13, STONE, row_h=7, base=3, seed=403)
    if not opened:
        flat_wall(layer, 6, 18, 14, 46, STONE, row_h=8, base=2, seed=404)
        for y in range(16, 44):
            layer.put(12, y, STONE[0])
        polygon(layer, [(8, 24), (16, 24), (18, 28), (16, 33), (8, 33), (6, 28)], GOLD[1])
        polygon(layer, [(10, 26), (14, 26), (16, 28), (14, 31), (10, 31), (8, 28)], GOLD[2])
        layer.put(12, 28, (68, 108, 88))
    else:
        for x in (3, 20):
            for y in (21, 30, 39):
                layer.put(x, y, GOLD[2])
    composite(image, layer, OUT)
    return image


def build():
    atlas = Image.new('RGBA', (144, 48))
    sprites = (inscription(), pedestal('seal'), pedestal('crown'), pedestal('empty'), door(False), door(True))
    for i, sprite in enumerate(sprites):
        atlas.alpha_composite(sprite, (i * 24, 0))
    return atlas


if __name__ == '__main__':
    save_sprite(build(), ROOT / 'assets/images/crown-quest-props.bmp')
