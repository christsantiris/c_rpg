#!/usr/bin/env python3
"""Saltwind shore creatures, drawn at the existing player/dark-elf scale."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, poly_mask, line_points, save_sprite

ROOT = Path(__file__).resolve().parents[2]
OUTLINE = (20, 29, 24)
SCALE = [(31, 57, 39), (48, 82, 47), (77, 111, 58), (116, 142, 75)]
HIDE = [(60, 42, 29), (91, 64, 39), (128, 95, 56), (162, 126, 73)]
METAL = [(49, 64, 58), (81, 100, 88), (130, 151, 124), (191, 203, 162)]


def shape(canvas, points, palette):
    layer = Layer(*canvas.size)
    for x, y in poly_mask(canvas.size, points):
        value = (x * 13 + y * 7) % 23
        layer.put(x, y, palette[min(len(palette) - 1, (x + y) % 3 + (value == 0))])
    composite(canvas, layer, OUTLINE)


def creature(index):
    canvas = Image.new('RGBA', (48, 48))
    if index == 3:
        # A broad shell, four distinct feet and an elongated beaked head.
        shape(canvas, [(8, 38), (9, 32), (14, 29), (30, 29), (35, 34), (35, 39),
                       (31, 43), (13, 43)], HIDE)
        shape(canvas, [(9, 39), (7, 44), (12, 44), (15, 40), (29, 40), (32, 44),
                       (36, 43), (34, 39), (35, 35), (40, 35), (42, 38), (38, 40)], SCALE)
        details = Layer(48, 48)
        for a, b in [((15, 30), (18, 35)), ((18, 35), (28, 35)), ((28, 35), (31, 30)),
                     ((18, 35), (14, 40)), ((28, 35), (32, 40)), ((14, 40), (32, 40))]:
            for x, y in line_points(*a, *b):
                details.put(x, y, HIDE[0])
        details.put(39, 36, (241, 202, 93))
        composite(canvas, details, None)
        return canvas
    boss = index == 5
    top = 9 if boss else 20
    # Scaled head with a projecting snout and a short dorsal crest.
    shape(canvas, [(19, top + 3), (22, top), (26, top + 1), (27, top + 5),
                   (32, top + 6), (32, top + 9), (26, top + 10), (21, top + 9)], SCALE)
    shape(canvas, [(20, top + 11), (27, top + 11), (30, 33), (28, 37),
                   (28, 43), (24, 44), (23, 37), (21, 44), (17, 44), (19, 36), (17, 32)], METAL if index == 4 or boss else HIDE)
    # Tail and weapon arm remain thin so humanoids fit one combat tile.
    shape(canvas, [(18, 35), (15, 39), (10, 38), (7, 35), (10, 41), (17, 42), (21, 37)], SCALE)
    details = Layer(48, 48)
    for x, y in line_points(29, 32, 34, 35):
        details.put(x, y, SCALE[2])
    details.put(26, top + 5, (245, 210, 95))
    details.put(27, top + 5, OUTLINE)
    if index in (1, 4, 5):
        for x, y in line_points(35, 21 if not boss else 5, 35, 44):
            details.put(x, y, HIDE[3])
        for y in range(17 if not boss else 1, 23 if not boss else 7):
            details.put(35, y, METAL[3])
            if y > (19 if not boss else 3):
                details.put(34, y, METAL[1])
                details.put(36, y, METAL[2])
    elif index == 0:
        for x, y in line_points(34, 33, 38, 28):
            details.put(x, y, METAL[3])
    else:
        # Spitter's contrasting ochre throat identifies the ranged role.
        for y in range(28, 32):
            for x in range(26, 30):
                details.put(x, y, (172, 155, 57))
    if index == 4:
        for y in range(30, 40):
            for x in range(12, 17):
                details.put(x, y, METAL[(x + y) % 3])
    if boss:
        for x in range(18, 30):
            details.put(x, 8 - x % 3, (208, 178, 83))
        for x, y in line_points(16, 23, 13, 34):
            details.put(x, y, (168, 110, 57))
    composite(canvas, details, None)
    return canvas


if __name__ == '__main__':
    atlas = Image.new('RGBA', (288, 48))
    for index in range(6):
        atlas.alpha_composite(creature(index), (48 * index, 0))
    save_sprite(atlas, ROOT / 'assets/images/mangrove-enemies.bmp')
