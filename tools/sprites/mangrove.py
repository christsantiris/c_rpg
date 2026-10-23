#!/usr/bin/env python3
"""Saltwind's native pixelkit entrance, mangrove roots and four skiff directions."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, poly_mask, save_sprite
import forest

ROOT = Path(__file__).resolve().parents[2]
BARK = [(30, 35, 24), (54, 56, 34), (80, 77, 43), (107, 98, 56), (139, 125, 74)]
LEAF = [(15, 40, 30), (25, 59, 37), (37, 77, 39), (57, 99, 45), (80, 119, 53), (115, 144, 64)]


def tree(canvas, x, y, scale, seed):
    forest.BARK = BARK
    forest.LEAF = LEAF
    points = [(-21, 0), (-9, -14), (-9, -42), (-18, -51), (-11, -57),
              (0, -46), (10, -58), (16, -52), (6, -39), (7, -16),
              (22, 0), (11, -5), (1, -18), (-2, -5), (-9, 0), (-4, -20), (-15, -2)]
    forest.woody(canvas, [(x + int(px * scale), y + int(py * scale)) for px, py in points], seed)
    for dx, dy, rx, ry in [(-15, -48, 16, 14), (13, -49, 17, 14), (0, -62, 24, 15)]:
        forest.crown(canvas, x + int(dx * scale), y + int(dy * scale),
                     max(3, int(rx * scale)), max(3, int(ry * scale)), seed + dx)


def entrance():
    canvas = Image.new('RGBA', (168, 144))
    water = Layer(168, 144)
    for x, y in poly_mask(canvas.size, [(5, 120), (28, 103), (142, 102), (163, 126), (145, 141), (24, 143)]):
        water.put(x, y, (14, 57, 63) if (x * 7 + y * 11) % 29 else (41, 100, 105))
    composite(canvas, water, (12, 31, 30))
    for x, y, scale, seed in [(32, 127, 1.0, 802), (129, 127, 1.0, 803), (82, 102, .85, 804)]:
        tree(canvas, x, y, scale, seed)
    pier = Layer(168, 144)
    for y in range(112, 144):
        for x in range(73, 95):
            pier.put(x, y, (114, 85, 48) if y % 5 else (57, 49, 34))
    for x in (71, 96):
        for y in range(113, 142):
            if y % 14 < 8:
                pier.put(x, y, (146, 117, 67))
    composite(canvas, pier, (21, 32, 25))
    boat = skiff().crop((72, 0, 108, 36))
    canvas.alpha_composite(boat, (105, 106))
    return canvas


def skiff():
    hull = Image.new('RGBA', (36, 36))
    wood = Layer(36, 36)
    for x, y in poly_mask(hull.size, [(18, 1), (25, 8), (27, 23), (24, 31), (12, 31), (9, 23), (11, 8)]):
        edge = x in (10, 11, 24, 25) or y in (8, 30)
        wood.put(x, y, (152, 108, 56) if edge else (91, 62, 35) if x % 5 else (61, 47, 30))
    for y in (12, 23):
        for x in range(12, 25):
            wood.put(x, y, (167, 121, 65))
            wood.put(x, y + 1, (120, 82, 42))
    composite(hull, wood, (23, 28, 24))
    atlas = Image.new('RGBA', (144, 36))
    for i, angle in enumerate((0, -90, 180, 90)):
        atlas.alpha_composite(hull.rotate(angle), (i * 36, 0))
    return atlas


if __name__ == '__main__':
    root = Image.new('RGBA', (48, 72))
    tree(root, 24, 69, .78, 810)
    for name, sprite in [('mangrove-entrance', entrance()), ('mangrove-root', root), ('skiff', skiff())]:
        save_sprite(sprite, ROOT / 'assets' / 'images' / f'{name}.bmp')
