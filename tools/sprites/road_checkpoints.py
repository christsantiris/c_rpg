#!/usr/bin/env python3
"""Native 24px road checkpoints, using the established building pixel pipeline."""
from pathlib import Path
import random

from PIL import Image
from pixelkit import Layer, composite, cyl_wall, stamp, line_points, save_sprite

ROOT = Path(__file__).resolve().parents[2]
SIZE = (72, 96)
OUT = (24, 25, 25)
WOOD = [(40, 30, 23), (67, 46, 29), (102, 72, 41), (139, 103, 59), (178, 139, 82)]
IRON = [(26, 30, 33), (52, 61, 64), (92, 104, 102), (145, 151, 137)]
STONE = [(37, 41, 39), (56, 63, 58), (82, 91, 79), (115, 123, 103), (152, 155, 129), (185, 182, 151)]
ACCENTS = [(103, 191, 157), (127, 171, 106), (181, 74, 43), (119, 153, 83), (193, 164, 81), (113, 159, 183)]


def rect(layer, left, top, right, bottom, color):
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            layer.put(x, y, color)


def post(layer, x, foot, masonry, seed):
    if masonry:
        cyl_wall(layer, x - 3, x + 4, foot - 27, foot - 2, STONE, row_h=5, cols=3, seed=seed)
        rect(layer, x - 4, foot - 29, x + 5, foot - 27, STONE[3])
        rect(layer, x - 3, foot - 29, x + 4, foot - 29, STONE[5])
    else:
        rect(layer, x - 2, foot - 26, x + 3, foot - 2, WOOD[2])
        rect(layer, x - 2, foot - 25, x - 1, foot - 3, WOOD[3])
        rect(layer, x + 2, foot - 25, x + 3, foot - 3, WOOD[1])
        rect(layer, x - 1, foot - 27, x + 2, foot - 26, WOOD[4])
        for y in (foot - 21, foot - 7):
            rect(layer, x - 3, y, x + 4, y + 2, IRON[1])
            layer.put(x - 2, y, IRON[3])
        layer.put(x, foot - 16, WOOD[0])
        layer.put(x, foot - 15, WOOD[0])
    rect(layer, x - 5, foot - 2, x + 6, foot, STONE[1])
    rect(layer, x - 4, foot - 2, x + 5, foot - 2, STONE[3])


def beam(layer, start, end, metal=False):
    points = line_points(*start, *end)
    horizontal = abs(end[0] - start[0]) >= abs(end[1] - start[1])
    for i, (x, y) in enumerate(points):
        for offset in range(-2, 3):
            tone = (IRON if metal else WOOD)[1 + (offset < 0) + (offset == -2)]
            layer.put(x if horizontal else x + offset, y + offset if horizontal else y, tone)
        if i % 10 == 3:
            for offset in range(-2, 3):
                layer.put(x if horizontal else x + offset, y + offset if horizontal else y, IRON[1])
            layer.put(x, y - 1, IRON[3])


def build(theme, vertical, opened):
    canvas = Image.new("RGBA", SIZE)
    body = Layer(*SIZE)
    masonry = theme in (0, 5)
    if vertical:
        post(body, 19, 69, masonry, 201)
        post(body, 52, 69, masonry, 202)
        beam(body, (22, 50), (22, 21) if opened else (49, 50), masonry)
    else:
        post(body, 35, 47, masonry, 203)
        # The near post is shorter on screen, preserving the one-tile opening.
        beam(body, (35, 35), (8, 35) if opened else (35, 66), masonry)
        post(body, 35, 90, masonry, 204)
    composite(canvas, body, OUT)
    detail = Layer(*SIZE)
    accent = ACCENTS[theme]
    x, y = (16, 47) if vertical else (32, 24)
    rect(detail, x, y, x + 6, y + 9, tuple(int(v * .6) for v in accent))
    rect(detail, x + 1, y, x + 5, y + 7, accent)
    # A modest area emblem replaces town heraldry.
    if theme in (0, 5):
        stamp(detail, x + 1, y + 1, [".g.g.", "..g..", ".g.g.", "g...g"], {"g": (213, 220, 180)})
    elif theme == 2:
        stamp(detail, x + 1, y + 1, ["g...g", ".g.g.", "..g..", ".g.g."], {"g": (225, 193, 123)})
    else:
        stamp(detail, x + 1, y + 1, ["..g..", ".ggg.", "g.g.g", "..g.."], {"g": (217, 219, 163)})
    # Rivets, weathered grain and vines sit on the posts, never on the ground.
    if theme in (1, 3, 4):
        for dx, dy in line_points(x - 1, y - 8, x + 3, y + 15):
            detail.put(dx, dy, (57, 85, 44))
            if dy % 4 == 0:
                detail.put(dx + 1, dy, (112, 135, 65))
    composite(canvas, detail, None)
    return canvas


def bank_tile(theme, neighbors, variant):
    layer = Layer(24, 24)
    rnd = random.Random(419 + theme * 100 + variant * 9)
    forest = theme in (1, 3, 4)
    pal = [(12, 30, 22), (20, 43, 27), (29, 56, 30), (40, 69, 35), (58, 84, 42)] if forest else STONE[:5]
    for y in range(24):
        for x in range(24):
            layer.put(x, y, pal[1 if y > 18 else 2])
    for cluster in range(14):
        cx, cy = rnd.randrange(-3, 26), rnd.randrange(-3, 26)
        rx, ry = rnd.randrange(3, 8), rnd.randrange(3, 7)
        for y in range(max(0, cy - ry), min(24, cy + ry + 1)):
            for x in range(max(0, cx - rx), min(24, cx + rx + 1)):
                if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1:
                    tone = 3 if y < cy and x < cx else 2 if x < cx + 2 else 1
                    layer.put(x, y, pal[tone])
                    if rnd.randrange(22) == 0:
                        layer.put(x, y, pal[min(4, tone + 1)])
    for y in range(24):
        for x in range(24):
            north = not neighbors & 1
            east = not neighbors & 2
            south = not neighbors & 4
            west = not neighbors & 8
            # Rounded outside corners and shaded faces form continuous outcrops.
            margin = 2 + (x // 5 + variant) % 2
            if (north and y < margin) or (south and y >= 24 - margin):
                layer.erase(x, y)
            elif (west and x < 2 + (y // 7 + variant) % 2) or (east and x > 21 - (y // 6 + variant) % 2):
                layer.erase(x, y)
            elif (north and west and x + y < 8) or (north and east and 23 - x + y < 8):
                layer.erase(x, y)
            elif (south and west and x + 23 - y < 8) or (south and east and 46 - x - y < 8):
                layer.erase(x, y)
            elif south and y > 17:
                layer.put(x, y, pal[0 if y > 20 else 1])
            elif north and y < margin + 2:
                layer.put(x, y, pal[3])
            elif east and x > 19:
                layer.put(x, y, pal[0])
    return layer.im


def banks():
    atlas = Image.new("RGBA", (16 * 4 * 24, len(ACCENTS) * 24))
    for theme in range(len(ACCENTS)):
        for neighbors in range(16):
            for variant in range(4):
                atlas.alpha_composite(bank_tile(theme, neighbors, variant), ((neighbors * 4 + variant) * 24, theme * 24))
    return atlas


if __name__ == "__main__":
    atlas = Image.new("RGBA", (SIZE[0] * 4, SIZE[1] * len(ACCENTS)))
    for theme in range(len(ACCENTS)):
        for vertical in (False, True):
            for opened in (False, True):
                column = int(vertical) * 2 + int(opened)
                atlas.alpha_composite(build(theme, vertical, opened), (column * SIZE[0], theme * SIZE[1]))
    save_sprite(atlas, ROOT / "assets/images/road-checkpoints.bmp")
    save_sprite(banks(), ROOT / "assets/images/road-banks.bmp")
