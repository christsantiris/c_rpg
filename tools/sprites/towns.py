#!/usr/bin/env python3
"""Dense medieval town landmarks, drawn at native resolution with pixelkit.

The user's hill-town reference supplies the curved enclosure and roof density.
These are entrance landmarks, not miniature walkable town interiors.
"""
from pathlib import Path
import random
from PIL import Image, ImageDraw
from pixelkit import (Layer, arched_door, beam, composite, flat_wall, lit_window,
                      plaster, poly_mask, roof_tiles, save_sprite, stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / 'assets/images'
SIZE = (408, 360)
OUT = (27, 25, 23)
STONE = [(39, 37, 35), (61, 58, 51), (86, 82, 71), (111, 106, 91), (139, 133, 113), (165, 158, 135)]
WOOD = [(30, 22, 17), (49, 34, 23), (72, 49, 31), (103, 73, 45), (135, 101, 63)]
PLASTER = [(125, 101, 72), (159, 132, 94), (188, 162, 119), (213, 190, 148)]
IRON = [(24, 26, 27), (48, 50, 48), (77, 79, 71), (113, 115, 101)]
GLOW = [(129, 65, 18), (176, 99, 29), (225, 153, 53), (253, 201, 99), (255, 226, 148)]
ROOFS = [
    [(53, 28, 22), (83, 38, 25), (119, 51, 30), (153, 72, 41), (185, 99, 58), (212, 132, 83)],
    [(48, 31, 27), (76, 45, 33), (108, 63, 42), (140, 85, 56), (172, 113, 76), (201, 143, 103)],
    [(48, 28, 29), (77, 38, 37), (111, 50, 43), (143, 68, 54), (176, 92, 72), (204, 121, 95)],
]
SLATE = [(27, 31, 40), (40, 48, 60), (57, 68, 80), (76, 90, 102), (101, 116, 126), (133, 146, 152)]
PLUM = [(38, 26, 37), (57, 36, 54), (83, 49, 75), (111, 67, 96), (139, 91, 117), (167, 120, 141)]
TOWNS = [
    ('oakhaven', (157, 76, 43), 13, 0),
    ('stillbury', (73, 117, 78), 31, 1),
    ('rosemoor', (146, 62, 88), 47, 2),
    ('ridgeshire', (74, 105, 151), 67, 3),
    ('dawnmere', (196, 155, 65), 89, 4),
]
# A faceted oval: the projecting towers break up its silhouette.
RIM = [(20, 125), (55, 78), (116, 47), (204, 31), (292, 47),
       (353, 78), (388, 125), (394, 216), (357, 283), (289, 319),
       (204, 333), (119, 319), (51, 283), (14, 216)]


def rect(layer, box, color):
    x0, y0, x1, y1 = box
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            layer.put(x, y, color)


def curtain(canvas, a, b, seed, front=False):
    x0, y0 = a
    x1, y1 = b
    drop = 24 if front else 21
    mask = poly_mask(SIZE, [(x0, y0), (x1, y1), (x1, y1 + drop), (x0, y0 + drop)])
    wall = Layer(*SIZE)
    flat_wall(wall, min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1) + drop,
              STONE, row_h=5, base=3 if front else 2, seed=seed, bw=(7, 12))
    for y in range(wall.h):
        for x in range(wall.w):
            if (x, y) not in mask:
                wall.erase(x, y)
    beam(wall, x0, y0, x1, y1, STONE, width=3)
    # Crenellations follow the curved wall rather than an enclosing square.
    length = max(abs(x1 - x0), abs(y1 - y0))
    for step in range(0, length + 1, 9):
        x = round(x0 + (x1 - x0) * step / max(1, length))
        y = round(y0 + (y1 - y0) * step / max(1, length))
        rect(wall, (x - 2, y - 5, x + 3, y), STONE[3])
        rect(wall, (x - 2, y - 5, x + 3, y - 5), STONE[5])
    composite(canvas, wall, OUT)


def tower(canvas, x, base, flag, seed, tall=False):
    wall = Layer(*SIZE)
    top = base - (45 if tall else 32)
    flat_wall(wall, x - 10, x + 10, top, base, STONE, row_h=5, base=3, seed=seed, bw=(5, 8))
    rect(wall, (x + 7, top + 2, x + 10, base), STONE[2])
    rect(wall, (x - 11, top, x + 11, top + 5), STONE[4])
    rect(wall, (x - 11, top, x + 11, top), STONE[5])
    for bx in range(x - 11, x + 12, 8):
        rect(wall, (bx, top - 5, bx + 4, top), STONE[3])
        rect(wall, (bx, top - 5, bx + 4, top - 5), STONE[5])
    rect(wall, (x - 1, top + 12, x + 1, top + 20), STONE[0])
    rect(wall, (x - 2, top + 23, x + 2, top + 24), STONE[1])
    if tall:
        beam(wall, x, top - 19, x, top - 5, WOOD, width=1)
        for row in range(7):
            rect(wall, (x + 1, top - 19 + row, x + 12 - row // 2, top - 19 + row), flag)
    composite(canvas, wall, OUT)


def house(canvas, cx, base, w, h, roof, seed, timber=True):
    layer = Layer(*SIZE)
    left = cx - w // 2
    right = cx + w // 2
    eave = base - h
    apex = eave - w // 3
    body = poly_mask(SIZE, [(left, eave), (cx, apex + 2), (right, eave), (right, base), (left, base)])
    plaster(layer, body, PLASTER, seed)
    flat_wall(layer, left, right, base - 9, base, STONE, row_h=4, base=3, seed=seed, bw=(5, 8))
    if timber:
        for x in (left, cx, right - 1):
            beam(layer, x, eave + 3, x, base - 9, WOOD, width=2)
        beam(layer, left, eave + h // 2, right, eave + h // 2, WOOD, width=2)
    roof_mask = poly_mask(SIZE, [(left - 5, eave + 3), (cx - 4, apex - 6),
                                (right - 5, apex + 2), (right + 7, eave - 3), (cx + 3, eave + 6)])
    roof_tiles(layer, roof_mask, roof, seed, shade=lambda x, y: -1 if x > cx + 2 else 0)
    beam(layer, cx - 4, apex - 6, right - 5, apex + 2, roof, width=2)
    beam(layer, left - 5, eave + 3, cx + 3, eave + 6, WOOD, width=2)
    beam(layer, cx + 3, eave + 6, right + 7, eave - 3, WOOD, width=2)
    # Doorways retain individual pixels, stone surrounds and iron bands.
    door_x = cx - w // 5
    stone_arch(layer, door_x, base - 19, base, 5, STONE, thick=2)
    arched_door(layer, door_x, base - 19, base, 5, WOOD, IRON)
    lit_window(layer, cx + w // 5, base - 24, 6, 7, GLOW, WOOD, shutters=False)
    if h >= 42:
        lit_window(layer, cx - 5, eave + 10, 7, 7, GLOW, WOOD, shutters=False)
    # Masonry chimney, with its own lit cap and dark opening.
    chimney = right - 9
    flat_wall(layer, chimney - 3, chimney + 3, apex - 5, eave - 7, STONE,
              row_h=4, base=3, seed=seed, bw=(4, 5))
    rect(layer, (chimney - 4, apex - 7, chimney + 4, apex - 4), STONE[4])
    rect(layer, (chimney - 2, apex - 7, chimney + 2, apex - 7), STONE[0])
    composite(canvas, layer, OUT)


def build(flag, seed, variant):
    canvas = Image.new('RGBA', SIZE)
    ground = Image.new('RGBA', SIZE)
    drawing = ImageDraw.Draw(ground)
    drawing.polygon(RIM, fill=(82, 81, 59, 255))
    # Small courtyards and irregular pale streets between packed roof rows.
    drawing.line([(204, 55), (193, 126), (223, 184), (201, 244), (204, 330)], fill=(141, 132, 104, 255), width=17)
    drawing.line([(29, 187), (126, 182), (220, 199), (306, 179), (386, 183)], fill=(141, 132, 104, 255), width=12)
    rnd = random.Random(seed)
    pixels = ground.load()
    for _ in range(10000):
        x = rnd.randrange(SIZE[0])
        y = rnd.randrange(SIZE[1])
        r, g, b, a = pixels[x, y]
        if a:
            d = rnd.choice((-7, -4, 3, 5))
            pixels[x, y] = (r + d, g + d, b + d, a)
    canvas.alpha_composite(ground)
    for i in range(7):
        curtain(canvas, RIM[i], RIM[(i + 1) % len(RIM)], seed + i)
    # Rear towers and buildings are occluded naturally by the nearer rooftops.
    for i in range(7):
        x, y = RIM[i]
        tower(canvas, x, y + 23, flag, seed + i, tall=i in (1, 3, 5))
    lots = [
        (133, 104, 48, 29), (187, 101, 48, 35), (243, 108, 49, 31), (290, 121, 43, 30),
        (82, 152, 47, 32), (137, 163, 55, 38), (267, 165, 57, 40), (328, 166, 48, 34),
        (55, 211, 43, 30), (106, 219, 55, 38), (165, 216, 43, 32),
        (259, 221, 51, 38), (316, 223, 49, 35), (362, 221, 33, 28),
        (87, 271, 48, 35), (143, 286, 54, 42), (259, 287, 54, 40), (317, 268, 49, 36),
    ]
    # A tall communal building breaks the skyline, without a second castle.
    house(canvas, 208, 167, 61, 65, ROOFS[variant % 3], seed + 90)
    for i, (x, y, w, h) in enumerate(lots):
        roof = ROOFS[(i + variant) % 3]
        if variant == 3 and i % 3 != 1:
            roof = SLATE
        if variant == 1 and i % 4 == 1:
            roof = PLUM
        house(canvas, x + rnd.randrange(-3, 4), y + rnd.randrange(-3, 4), w, h, roof, seed + i * 3,
              timber=variant != 3 or i % 3 == 0)
    # Market awnings and a stone well in the central square.
    market = Layer(*SIZE)
    for x, y in ((190, 229), (203, 287)):
        rect(market, (x - 9, y, x + 9, y + 7), WOOD[2])
        for k in range(19):
            rect(market, (x - 9 + k, y - 5, x - 9 + k, y), flag if k % 6 < 3 else PLASTER[3])
        beam(market, x - 9, y, x - 9, y + 12, WOOD, width=1)
        beam(market, x + 9, y, x + 9, y + 12, WOOD, width=1)
    flat_wall(market, 206, 223, 202, 211, STONE, row_h=4, base=3, seed=seed, bw=(4, 6))
    rect(market, (209, 200, 220, 203), STONE[0])
    rect(market, (207, 199, 222, 200), STONE[5])
    composite(canvas, market, OUT)
    for i in range(7, len(RIM)):
        curtain(canvas, RIM[i], RIM[(i + 1) % len(RIM)], seed + i, front=True)
    for i in range(7, len(RIM)):
        x, y = RIM[i]
        tower(canvas, x, y + 24, flag, seed + i, tall=i in (9, 11))
    if variant == 3:
        snow = ImageDraw.Draw(canvas)
        for x, y in RIM:
            snow.line([(x - 9, y - 9), (x + 8, y - 9)], fill=(195, 205, 205, 255), width=2)
    return canvas


if __name__ == '__main__':
    for name, flag, seed, variant in TOWNS:
        save_sprite(build(flag, seed, variant), ASSETS / (name + '-town.bmp'))
