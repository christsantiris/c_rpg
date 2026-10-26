#!/usr/bin/env python3
"""Haunted Forest gateway at the existing buildings' native 24px tile scale."""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, line_points, poly_mask, save_sprite, stamp

SIZE = (168, 144)
OUT = (10, 18, 20)
BARK = [(26, 29, 29), (42, 43, 39), (61, 59, 48), (82, 75, 60), (109, 97, 75)]
LEAF = [(13, 25, 26), (19, 34, 33), (27, 44, 39), (38, 56, 46), (50, 69, 55), (65, 82, 66)]
MOSS = [(35, 47, 39), (51, 65, 46), (71, 83, 57)]


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
    # Skeletal upper limbs break the sparse canopy instead of forming a lush arch.
    woody(canvas, [(39, 69), (42, 35), (34, 22), (24, 19), (23, 11), (28, 17),
                   (37, 18), (44, 30), (47, 16), (57, 7), (60, 7), (51, 20),
                   (50, 37), (67, 28), (75, 14), (80, 11), (72, 31), (51, 48), (49, 73)], 71)
    woody(canvas, [(121, 74), (119, 44), (102, 31), (99, 18), (95, 9), (99, 10),
                   (105, 25), (119, 34), (121, 18), (133, 7), (136, 8), (126, 23),
                   (128, 40), (144, 27), (149, 15), (153, 14), (150, 33), (130, 52), (133, 74)], 72)
    for args in [(19, 69, 16, 25, 1), (149, 68, 15, 24, 2), (36, 48, 20, 16, 3),
                 (133, 50, 21, 16, 4), (78, 49, 19, 14, 5),
                 (23, 98, 17, 17, 8), (147, 100, 16, 18, 9)]:
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
