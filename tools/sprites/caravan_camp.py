#!/usr/bin/env python3
"""Sunscar roadside camp, at native 24-pixel tile scale, using pixelkit."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, beam, composite, poly_mask, save_sprite

W, H = 312, 144
OUT = (24, 20, 19)
WOOD = [(39, 27, 19), (64, 44, 29), (91, 65, 39), (124, 89, 50), (154, 117, 68)]
CANVAS = [(80, 61, 42), (121, 96, 65), (165, 139, 94), (199, 179, 129), (225, 210, 163)]
BLUE = [(29, 47, 59), (42, 65, 79), (64, 92, 104), (97, 124, 130), (144, 164, 156)]


def tent(image, x, pal):
    layer = Layer(W, H)
    roof = poly_mask((W, H), [(x + 7, 105), (x + 43, 19), (x + 85, 102)])
    for px, py in roof:
        seam = (px - x + (105 - py) // 4) % 17
        shade = 3 if px < x + 43 else 2
        if seam <= 1:
            shade -= 1
        layer.put(px, py, pal[max(0, shade + (1 if (px * 11 + py * 7) % 29 == 0 else 0))])
    beam(layer, x + 43, 18, x + 43, 110, WOOD)
    # Deep triangular opening with a rolled canvas flap, sized like existing doors.
    opening = poly_mask((W, H), [(x + 38, 61), (x + 23, 108), (x + 61, 108)])
    for px, py in opening:
        layer.put(px, py, (39, 30, 27) if px < x + 47 else pal[0])
    beam(layer, x + 38, 61, x + 61, 108, pal, width=3)
    beam(layer, x + 8, 104, x + 85, 104, WOOD)
    for ax, ay, bx in [(x + 12, 76, x + 1), (x + 76, 75, x + 93)]:
        beam(layer, ax, ay, bx, 119, CANVAS, width=1)
        beam(layer, bx, 116, bx + 1, 123, WOOD)
    composite(image, layer, OUT)


def wagon(image):
    layer = Layer(W, H)
    # Iron-shod spoked wheels, plank bed, and a ribbed canvas cover.
    for cx in (126, 181):
        for y in range(101, 128):
            for x in range(cx - 12, cx + 13):
                radius = (x - cx) ** 2 + (y - 114) ** 2
                if 82 <= radius <= 157:
                    layer.put(x, y, (57, 56, 50) if radius > 122 else WOOD[3])
                elif radius < 82 and (abs(x - cx) <= 1 or abs(y - 114) <= 1 or abs(abs(x - cx) - abs(y - 114)) <= 1):
                    layer.put(x, y, WOOD[2])
    for y in range(82, 110):
        for x in range(111, 196):
            layer.put(x, y, WOOD[1 if y % 6 == 0 else 3 if x % 7 else 2])
    cover = poly_mask((W, H), [(110, 82), (114, 56), (123, 45), (182, 45), (193, 58), (196, 82)])
    for x, y in cover:
        stripe = (x - 113) % 16
        layer.put(x, y, CANVAS[1 if stripe < 2 else 4 if y < 53 else 3])
    for x in (115, 139, 163, 189):
        beam(layer, x, 57, x, 81, WOOD, width=1)
    beam(layer, 113, 84, 193, 84, WOOD)
    beam(layer, 194, 104, 211, 115, WOOD)
    composite(image, layer, OUT)
    # Travel chests and tied sacks beside the wagon.
    goods = Layer(W, H)
    for x0, y0 in ((99, 116), (202, 119)):
        for y in range(y0, y0 + 17):
            for x in range(x0, x0 + 19):
                goods.put(x, y, WOOD[3 if y == y0 else 1 if x % 8 == 0 else 2])
        beam(goods, x0 + 4, y0, x0 + 4, y0 + 15, WOOD, width=1)
        beam(goods, x0 + 14, y0, x0 + 14, y0 + 15, WOOD, width=1)
    composite(image, goods, OUT)


def build():
    image = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    tent(image, 1, CANVAS)
    tent(image, 215, BLUE)
    wagon(image)
    return image


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/caravan-camp.bmp')
