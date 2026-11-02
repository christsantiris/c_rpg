#!/usr/bin/env python3
"""Haunted Forest gateway at the existing buildings' native 24px tile scale."""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, line_points, poly_mask, save_sprite, stamp

SIZE = (168, 144)
OUT = (10, 20, 18)
BARK = [(26, 31, 28), (40, 43, 32), (57, 56, 38), (80, 74, 47), (111, 98, 63)]
LEAF = [(12, 32, 25), (18, 46, 31), (27, 62, 37), (39, 78, 44), (58, 95, 54), (82, 113, 65)]
MOSS = [(41, 59, 36), (64, 82, 43), (93, 108, 53)]


def woody(canvas, points, seed):
    rng = random.Random(seed)
    tree = Layer(*SIZE)
    mask = poly_mask(SIZE, points)
    for x, y in sorted(mask):
        shade = 2 + ((x + y // 18) % 13 < 3) - ((x + y // 11) % 11 > 7)
        tree.put(x, y, BARK[shade])
    for n in range(25):
        x, y = rng.randrange(10, 157), rng.randrange(55, 138)
        for px, py in line_points(x, y, x + rng.randrange(-4, 5), y + rng.randrange(7, 19)):
            if (px, py) in mask:
                tree.put(px, py, BARK[0])
                if (px - 1, py) in mask:
                    tree.put(px - 1, py, BARK[4])
    composite(canvas, tree, OUT)


def crown(canvas, cx, cy, rx, ry, seed):
    rng = random.Random(seed)
    points = [(cx - rx, cy), (cx - rx + 4, cy - ry // 2), (cx - rx // 2, cy - ry + 3),
              (cx - 3, cy - ry), (cx + rx // 2, cy - ry + 5), (cx + rx - 3, cy - ry // 3),
              (cx + rx, cy + 6), (cx + rx - 7, cy + ry // 2), (cx + rx // 2, cy + ry - 3),
              (cx + 2, cy + ry), (cx - rx // 2, cy + ry - 6), (cx - rx + 2, cy + ry // 3)]
    mask = poly_mask(SIZE, points)
    foliage = Layer(*SIZE)
    for x, y in sorted(mask):
        patch = ((x // 7) * 17 + (y // 5) * 13 + seed) % 7
        shade = 2 + (y < cy - ry // 3) - (y > cy + ry // 2)
        if patch < 2:
            shade += 1
        if (x + y * 3) % 17 == 0:
            shade += 1
        foliage.put(x, y, LEAF[max(0, min(5, shade))])
    for n in range(45):
        x, y = rng.randrange(cx - rx, cx + rx + 1), rng.randrange(cy - ry, cy + ry + 1)
        for dx, dy in ((0, 0), (1, 0), (2, 1), (-1, 1)):
            if (x + dx, y + dy) in mask:
                foliage.put(x + dx, y + dy, LEAF[4 if y < cy else 2])
    composite(canvas, foliage, OUT)


def build():
    canvas = Image.new('RGBA', SIZE)
    # Distant trees and roots frame a deep, narrow route into the woods.
    for x, seed in ((17, 91), (147, 92)):
        woody(canvas, [(x - 5, 135), (x - 2, 64), (x + 4, 41), (x + 8, 66), (x + 6, 120), (x + 13, 138)], seed)
    path = Layer(*SIZE)
    for y in range(79, 144):
        half = min(15, 3 + (y - 79) // 3)
        for x in range(84 - half, 85 + half):
            color = (8, 19, 18) if y < 112 else (24, 35, 27) if y < 130 else (44, 48, 34)
            path.put(x, y, color)
            if y > 129 and (x * 13 + y * 7) % 19 == 0:
                path.put(x, y, (73, 72, 46))
    composite(canvas, path, None)
    # Two ancient trunks lean inward and interlace above the entrance.
    woody(canvas, [(20, 139), (32, 119), (34, 78), (26, 55), (36, 43), (49, 67), (56, 82),
                   (67, 83), (79, 72), (101, 68), (95, 78), (79, 85), (66, 99), (54, 102),
                   (53, 124), (67, 140), (46, 136), (37, 142)], 23)
    woody(canvas, [(104, 140), (116, 119), (114, 99), (100, 89), (87, 89), (73, 82), (68, 71),
                   (88, 79), (103, 76), (116, 82), (125, 65), (131, 42), (140, 54), (136, 81),
                   (139, 119), (151, 140), (132, 136), (122, 142)], 24)
    # Staggered crowns use the established clustered shading and dark outlines.
    for args in [(20, 65, 18, 30, 1), (148, 66, 18, 31, 2), (40, 35, 30, 29, 3),
                 (126, 34, 31, 30, 4), (80, 27, 34, 23, 5), (60, 55, 30, 24, 6),
                 (110, 57, 30, 25, 7), (23, 93, 18, 18, 8), (147, 95, 17, 20, 9)]:
        crown(canvas, *args)
    details = Layer(*SIZE)
    # Hanging moss, twisted root tips and restrained necromantic glow.
    for x, y, length in ((59, 77, 17), (70, 75, 11), (99, 77, 17), (110, 78, 13), (34, 107, 11), (134, 107, 13)):
        for n in range(length):
            details.put(x + (n // 5) % 2, y + n, MOSS[1 if n % 3 else 2])
            if n < length - 5:
                details.put(x - 1, y + n, MOSS[0])
    for x, y in ((36, 99), (127, 100)):
        stamp(details, x, y, ['.pp.', 'pPPp', '.pp.'], {'p': (70, 56, 92), 'P': (161, 142, 183)})
    # A tiny weathered warning charm hangs from the natural arch.
    for y in range(84, 96):
        details.put(84, y, (102, 92, 61))
    stamp(details, 80, 96, ['..bbb..', '.bBBBb.', 'bBoBoBb', '.BBbBB.', '..bBb..'],
          {'b': (96, 94, 66), 'B': (159, 155, 111), 'o': (21, 31, 25)})
    for x, y in ((13, 134), (30, 138), (57, 134), (113, 138), (145, 135)):
        stamp(details, x, y, ['..m..', 'm.m.m', '.mmm.', '..m..'], {'m': MOSS[1]})
    composite(canvas, details, OUT)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/forest-entrance.bmp')
