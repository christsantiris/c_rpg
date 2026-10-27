#!/usr/bin/env python3
"""Native Saltwind quest props, using the marsh's existing pixelkit palettes."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, line_points, poly_mask, save_sprite
import forest
import mangrove

ROOT = Path(__file__).resolve().parents[2]


def wreck():
    canvas = mangrove.skiff().crop((36, 0, 72, 36))
    pixels = canvas.load()
    # Split the existing hull with a ragged break and a snapped prow.
    for y in range(11, 27):
        for x in range(23 + y % 3, 36):
            if x > 27 or y % 4 < 2:
                pixels[x, y] = (0, 0, 0, 0)
    wood = Layer(36, 36)
    for x, y in line_points(4, 29, 28, 22):
        wood.put(x, y, (57, 44, 28))
        wood.put(x, y - 1, (156, 111, 60))
    for y in range(9, 17):
        for x in range(11, 19):
            wood.put(x, y, (215, 196, 143) if x < 18 else (148, 119, 73))
    for y in (11, 13, 15):
        for x in range(13, 17):
            wood.put(x, y, (102, 74, 46))
    composite(canvas, wood, None)
    return canvas


def casket(empty=False):
    canvas = Image.new('RGBA', (24, 24))
    wood = Layer(24, 24)
    for x, y in poly_mask(canvas.size, [(4, 10), (19, 10), (21, 13), (20, 21), (4, 21), (3, 13)]):
        wood.put(x, y, (120, 77, 36) if y % 4 else (71, 48, 28))
        if x < 6:
            wood.put(x, y, (175, 121, 57))
    lid = [(4, 9), (7, 5), (18, 5), (21, 9), (20, 12), (4, 12)]
    if empty:
        lid = [(4, 3), (7, 1), (18, 1), (21, 3), (19, 7), (5, 7)]
    for x, y in poly_mask(canvas.size, lid):
        wood.put(x, y, (184, 129, 64) if y % 3 else (130, 86, 39))
    if empty:
        for y in range(9, 12):
            for x in range(5, 20):
                wood.put(x, y, (40, 35, 24))
    for x in (7, 17):
        for y in range(6 if not empty else 2, 21):
            if wood.on(x, y):
                wood.put(x, y, (77, 83, 72))
                wood.put(x + 1, y, (135, 139, 111))
        wood.put(x, 15, (193, 188, 144))
    if not empty:
        for x, y in poly_mask(canvas.size, [(11, 9), (14, 9), (15, 12), (13, 14), (10, 12)]):
            wood.put(x, y, (151, 50, 39) if x % 2 else (212, 100, 57))
    composite(canvas, wood, (26, 31, 25))
    return canvas


def root_arch():
    forest.BARK = mangrove.BARK
    forest.LEAF = mangrove.LEAF
    canvas = Image.new('RGBA', (72, 72))
    forest.woody(canvas, [(3, 69), (11, 48), (17, 29), (25, 16), (39, 11), (54, 19),
                          (61, 37), (66, 59), (70, 69), (56, 65), (51, 40), (44, 29),
                          (33, 26), (26, 36), (22, 58), (15, 68), (17, 47), (10, 65)], 1211)
    for args in ((14, 18, 13, 10, 1212), (55, 19, 13, 10, 1213), (35, 9, 18, 8, 1214)):
        forest.crown(canvas, *args)
    vines = Layer(72, 72)
    for x, height in ((22, 12), (49, 17), (54, 10)):
        for y in range(25, 25 + height):
            vines.put(x + y % 3 // 2, y, (83, 113, 45))
            if y % 5 == 0:
                vines.put(x + 1, y, (133, 151, 65))
    composite(canvas, vines, None)
    return canvas


def build():
    atlas = Image.new('RGBA', (156, 72))
    for sprite, xy in ((wreck(), (0, 0)), (casket(), (36, 0)), (casket(True), (60, 0)), (root_arch(), (84, 0))):
        atlas.alpha_composite(sprite, xy)
    return atlas


if __name__ == '__main__':
    save_sprite(build(), ROOT / 'assets/images/spice-skiff-props.bmp')
