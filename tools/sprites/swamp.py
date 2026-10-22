#!/usr/bin/env python3
"""Blackwater Swamp entrance in the established 24px pixelkit pipeline."""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, flat_wall, line_points, poly_mask, save_sprite, stamp
from forest import woody, crown

SIZE = (168, 144)
OUT = (11, 22, 20)
STONE = [(27, 36, 31), (43, 52, 39), (62, 70, 48), (86, 91, 61), (119, 120, 79)]
WATER = [(12, 29, 29), (18, 43, 40), (29, 58, 49), (46, 72, 54)]
MOSS = [(38, 55, 29), (62, 78, 37), (92, 105, 49)]
WOOD = [(39, 37, 25), (64, 59, 34), (93, 82, 45), (125, 108, 61)]


def build():
    canvas = Image.new('RGBA', SIZE)
    rng = random.Random(71)
    basin = Layer(*SIZE)
    mask = poly_mask(SIZE, [(3, 124), (13, 108), (38, 101), (60, 108), (100, 107), (127, 102), (155, 113), (165, 135), (143, 141), (29, 142), (8, 137)])
    for x, y in sorted(mask):
        basin.put(x, y, WATER[1 if y < 130 else 0])
        if (x * 3 + y * 7) % 31 < 3:
            basin.put(x, y, WATER[2])
    for x, y in ((13, 127), (28, 134), (121, 124), (143, 132), (40, 121), (111, 136)):
        for dx in range(10):
            basin.put(x + dx, y, WATER[3])
    composite(canvas, basin, OUT)
    # Mangrove trunks, bare antlers and roots disappearing into blackwater.
    for points, seed in [([(10, 134), (22, 112), (25, 59), (15, 42), (19, 29), (28, 50), (36, 37), (40, 42), (33, 62), (36, 111), (47, 135), (31, 128)], 31),
                         ([(120, 136), (131, 110), (130, 53), (118, 36), (121, 27), (136, 44), (144, 24), (151, 24), (143, 51), (144, 115), (158, 136), (140, 130)], 32),
                         ([(42, 125), (49, 96), (47, 48), (39, 28), (44, 18), (55, 42), (65, 25), (69, 30), (57, 55), (59, 100), (65, 126)], 33),
                         ([(107, 125), (112, 99), (109, 56), (98, 38), (104, 31), (118, 53), (126, 40), (131, 43), (121, 62), (123, 109), (130, 129)], 34)]:
        woody(canvas, points, seed)
    # The old stone causeway gate is swallowed by roots and hanging moss.
    gate = Layer(*SIZE)
    for left in (61, 98):
        flat_wall(gate, left, left + 10, 83, 134, STONE, row_h=7, base=2, seed=left, bw=(6, 10))
    for y in range(70, 96):
        half = min(23, 5 + (y - 70))
        for x in range(84 - half, 85 + half):
            gate.put(x, y, STONE[3] if x < 84 - half + 2 else STONE[1])
            if (x + y * 2) % 19 == 0:
                gate.put(x, y, MOSS[1])
    composite(canvas, gate, OUT)
    opening = Layer(*SIZE)
    for y in range(83, 136):
        half = min(12, 2 + y - 83)
        for x in range(84 - half, 85 + half):
            opening.put(x, y, (8, 22, 22) if y < 119 else WATER[0])
    composite(canvas, opening, None)
    for args in [(19, 65, 17, 19, 21), (47, 26, 22, 20, 22), (145, 48, 20, 20, 23), (119, 28, 22, 18, 24)]:
        crown(canvas, *args)
    detail = Layer(*SIZE)
    for x, top, length in ((36, 42, 35), (48, 43, 27), (119, 44, 28), (143, 65, 30), (66, 81, 30), (101, 86, 20)):
        for n in range(length):
            detail.put(x + n // 11, top + n, MOSS[1 if n % 4 else 2])
            if n < length - 6:
                detail.put(x - 1, top + n, MOSS[0])
    # Weathered planks lead through the water into the dark marsh.
    for row in range(7):
        y = 115 + row * 4
        half = 6 + row
        for yy in range(y, y + 3):
            for x in range(84 - half, 85 + half):
                detail.put(x, yy, WOOD[3 if yy == y else 1 if (x + row) % 9 == 0 else 2])
        detail.put(85 - half, y + 1, WOOD[0])
        detail.put(83 + half, y + 1, WOOD[0])
    for x0 in (16, 39, 121, 148):
        for stalk in range(4):
            x = x0 + stalk * 3
            top = rng.randrange(102, 117)
            for y in range(top, 135):
                detail.put(x, y, MOSS[1])
                if y < top + 5:
                    detail.put(x + 1, y, (103, 91, 53))
            for x, y in line_points(x, top + 10, x - 4, top + 5):
                detail.put(x, y, MOSS[2])
    stamp(detail, 79, 73, ['..bbbbb..', '.bBBBBBb.', 'bBBooBBBb', '.BBooBB.', '..bB.Bb..', '...bbb...'],
          {'b': STONE[2], 'B': STONE[4], 'o': (22, 38, 29)})
    for x, y in ((58, 102), (110, 99)):
        stamp(detail, x, y, ['..w..', '.wWw.', 'wWWWw', '.wWw.', '..w..'], {'w': (40, 91, 74), 'W': (134, 191, 125)})
    composite(canvas, detail, OUT)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/swamp-entrance.bmp')
