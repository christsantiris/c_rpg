#!/usr/bin/env python3
"""Ruined Temple landmark: a complete stepped pyramid at native 24px scale."""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, beam, composite, flat_wall, line_points, poly_mask, save_sprite

SIZE = (168, 144)
OUT = (24, 30, 27)
STONE = [(48, 50, 43), (70, 73, 59), (97, 101, 80), (125, 130, 103), (156, 161, 131), (186, 189, 156)]
MOSS = [(23, 44, 28), (38, 68, 36), (61, 96, 44), (99, 132, 61)]
GOLD = [(83, 67, 29), (134, 107, 43), (190, 157, 67), (233, 204, 112)]


def masonry(canvas, points, seed, shade=3):
    layer = Layer(*SIZE)
    mask = poly_mask(SIZE, points)
    x0 = min(x for x, y in mask)
    x1 = max(x for x, y in mask)
    y0 = min(y for x, y in mask)
    y1 = max(y for x, y in mask)
    flat_wall(layer, x0, x1, y0, y1, STONE, row_h=5, base=shade, seed=seed, bw=(7, 13))
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if (x, y) not in mask:
                layer.erase(x, y)
    # A few chipped, unaligned joints make the terraces read as ruins.
    rng = random.Random(seed)
    for _ in range(max(1, len(mask) // 180)):
        x, y = rng.choice(sorted(mask))
        for px, py in line_points(x, y, x + 2, y + 4):
            if (px, py) in mask:
                layer.put(px, py, STONE[1])
    composite(canvas, layer, OUT)


def build():
    canvas = Image.new('RGBA', SIZE)
    # Five receding terraces communicate the whole temple, not a doorway facade.
    tiers = [(50, 118, 41), (40, 128, 58), (30, 138, 76), (20, 148, 94), (10, 158, 113)]
    for i, (left, right, top) in enumerate(tiers):
        masonry(canvas, [(left, top + 3), (right - 10, top + 3), (right - 10, top + 18), (left, top + 18)], 50 + i)
        masonry(canvas, [(right - 10, top + 3), (right, top - 4), (right, top + 11), (right - 10, top + 18)], 60 + i, shade=1)
        coping = Layer(*SIZE)
        mask = poly_mask(SIZE, [(left - 2, top + 2), (left + 10, top - 5), (right + 1, top - 5), (right - 10, top + 3)])
        for x, y in sorted(mask):
            coping.put(x, y, STONE[4 if (x + y) % 13 else 2])
        beam(coping, left - 2, top + 2, right - 10, top + 2, STONE, width=2)
        composite(canvas, coping, OUT)
    masonry(canvas, [(8, 131), (148, 131), (160, 124), (160, 136), (148, 142), (8, 142)], 70)
    # A continuous, narrow stairway climbs to a roofless sun sanctuary.
    stairs = Layer(*SIZE)
    for y in range(47, 142):
        half = 7 + (y - 47) // 21
        for x in range(80 - half, 81 + half):
            stairs.put(x, y, STONE[5] if y % 4 == 0 else STONE[1] if y % 4 == 3 else STONE[3])
        stairs.put(80 - half - 1, y, STONE[4])
        stairs.put(81 + half, y, STONE[1])
    composite(canvas, stairs, OUT)
    # Broken freestanding columns and a sun-carved rear wall, open to the sky.
    masonry(canvas, [(59, 19), (100, 19), (105, 25), (105, 42), (59, 42)], 72)
    masonry(canvas, [(52, 16), (59, 13), (63, 17), (63, 46), (52, 46)], 73)
    masonry(canvas, [(102, 24), (110, 21), (114, 26), (114, 46), (102, 46)], 74, shade=2)
    details = Layer(*SIZE)
    for x, top in ((55, 14), (106, 23)):
        for y in range(top + 3, 44):
            details.put(x, y, STONE[5])
            details.put(x + 5, y, STONE[1])
        beam(details, x - 4, 44, x + 9, 44, STONE, width=3)
    # Small sun and moon motifs tie this monument to the temple's existing puzzle.
    for y in range(23, 38):
        for x in range(71, 88):
            radius = math.hypot(x - 79, y - 30)
            if 4 < radius < 6:
                details.put(x, y, GOLD[3 if x < 80 else 1])
    for dx, dy in ((0, -9), (0, 9), (-10, 0), (10, 0), (-7, -7), (7, -7), (-7, 7), (7, 7)):
        for x, y in line_points(79 + dx * 2 // 3, 30 + dy * 2 // 3, 79 + dx, 30 + dy):
            details.put(x, y, GOLD[2])
    for y in range(102, 110):
        for x in range(30, 38):
            if (x - 34) ** 2 + (y - 106) ** 2 <= 15 and (x - 36) ** 2 + (y - 104) ** 2 > 13:
                details.put(x, y, GOLD[2])
    # Moss settles along ledges; hanging vines follow the masonry down the sides.
    for i, (left, right, top) in enumerate(tiers):
        for x in range(left + 1, right - 10):
            if 68 < x < 95:
                continue
            if (x * 7 + i * 11) % 9 < 5:
                details.put(x, top + 1, MOSS[2 if x % 3 else 3])
                details.put(x, top + 2, MOSS[1])
    for x0, y0, length in ((57, 18, 42), (111, 27, 48), (34, 82, 35), (132, 80, 44), (24, 116, 20)):
        for n in range(length):
            x = x0 + round(math.sin(n / 5) * 2)
            details.put(x, y0 + n, MOSS[1])
            details.put(x - 1, y0 + n, MOSS[2])
            if n % 5 == 0:
                for px, py in ((x - 3, y0 + n - 1), (x - 2, y0 + n), (x + 2, y0 + n + 1)):
                    details.put(px, py, MOSS[3])
    composite(canvas, details, OUT)
    # Fallen blocks and low broad leaves soften the base without hiding the steps.
    for left, top, w in ((2, 130, 12), (149, 135, 14), (22, 136, 10), (132, 138, 8)):
        masonry(canvas, [(left, top), (left + w, top - 2), (left + w, min(143, top + 5)), (left, min(143, top + 6))], left + top, shade=2)
    leaves = Layer(*SIZE)
    for cx, cy in ((9, 126), (154, 126), (31, 135), (133, 137)):
        for dx, dy in ((-9, -3), (-6, -9), (0, -12), (8, -7), (10, -1)):
            leaf = poly_mask(SIZE, [(cx, cy), (cx + dx - 2, cy + dy + 2), (cx + dx, cy + dy), (cx + dx + 3, cy + dy + 1)])
            for x, y in sorted(leaf):
                leaves.put(x, y, MOSS[2 if x < cx else 1])
            for x, y in line_points(cx, cy, cx + dx, cy + dy):
                leaves.put(x, y, MOSS[3])
    composite(canvas, leaves, OUT)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/temple-entrance.bmp')
