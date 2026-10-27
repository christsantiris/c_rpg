#!/usr/bin/env python3
"""Approval-only town gate; no runtime assets or rendering are replaced."""
import math
from pathlib import Path
import sys

from PIL import Image

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools/sprites"))
from pixelkit import Layer, LANTERN, composite, cyl_wall, flat_wall, merlons, save_sprite, stamp
from local_buildings import IRON, GLOW

DEST = Path(__file__).resolve().parent
SIZE = (120, 96)
OUT = (22, 21, 30)
STONE = [(30, 28, 40), (48, 45, 59), (70, 67, 81), (94, 91, 105), (122, 119, 129), (157, 153, 158)]
ARCH = [(47, 44, 43), (77, 70, 62), (116, 106, 90), (145, 133, 110), (180, 167, 140), (211, 198, 169)]
WALL = [(26, 24, 39), (35, 30, 60), (50, 45, 80), (63, 58, 91), (82, 76, 108), (111, 104, 133)]
WOOD = [(34, 25, 22), (58, 39, 27), (86, 58, 36), (117, 83, 48), (151, 110, 62)]
GOLD = [(91, 66, 32), (157, 117, 51), (214, 177, 90), (241, 216, 148)]
BLUE = [(25, 45, 55), (36, 69, 81), (57, 102, 111)]


def rect(layer, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            layer.put(x, y, color)


def course(layer, x0, x1, y, pal):
    rect(layer, x0, y, x1, y, pal[5])
    rect(layer, x0, y + 1, x1, y + 2, pal[3])
    rect(layer, x0, y + 3, x1, y + 3, pal[1])


def tower(layer, left, seed):
    right = left + 25
    cyl_wall(layer, left, right, 31, 90, STONE, row_h=5, cols=6, seed=seed)
    rect(layer, left - 2, 28, right + 2, 32, STONE[2])
    course(layer, left - 2, right + 2, 30, STONE)
    merlons(layer, left - 2, right + 2, 20, 10, STONE, mw=5, gap=3, base=3, point=0)
    course(layer, left - 1, right + 1, 52, STONE)
    course(layer, left - 2, right + 2, 89, STONE)
    rect(layer, left - 1, 93, right + 1, 95, STONE[1])
    cx = left + 11
    rect(layer, cx - 1, 38, cx + 2, 46, STONE[1])
    rect(layer, cx, 39, cx, 45, OUT)
    rect(layer, cx - 2, 42, cx + 2, 42, OUT)
    rect(layer, cx - 1, 47, cx + 3, 47, STONE[4])


def build(closed=False):
    canvas = Image.new("RGBA", SIZE)
    body = Layer(*SIZE)
    # Low wings use the existing town wall's slate-purple tones.
    for left, right, seed in ((0, 21, 903), (98, 119, 904)):
        flat_wall(body, left, right, 72, 95, WALL, row_h=8, base=2, seed=seed, bw=(10, 15))
        course(body, left, right, 72, WALL)
    flat_wall(body, 43, 76, 40, 95, STONE, row_h=5, base=3, seed=905, bw=(6, 10))
    merlons(body, 43, 76, 32, 9, STONE, mw=5, gap=3, base=3, point=0)
    course(body, 43, 76, 40, STONE)
    # A 24-pixel opening aligns exactly with the single-tile road.
    for y in range(46, 96):
        half = math.sqrt(max(0, 13 ** 2 - (y - 62) ** 2)) if y < 62 else 13
        for x in range(44, 77):
            distance = abs(x + 0.5 - 60)
            outer = math.sqrt(max(0, 17 ** 2 - (y - 62) ** 2)) if y < 62 else 17
            if distance < half:
                body.erase(x, y)
            elif distance < outer:
                tone = 4 + (int(math.atan2(y - 62, x - 60) * 5) % 2)
                if y >= 62:
                    tone = 1 if (y - 62) % 6 == 5 else 3 + (x < 60)
                body.put(x, y, ARCH[tone])
    for left, seed in ((19, 910), (75, 911)):
        tower(body, left, seed)
    composite(canvas, body, OUT)

    detail = Layer(*SIZE)
    # Open oak leaves remain against the jambs, leaving the whole road clear.
    for left in (45, 73):
        rect(detail, left, 65, left + 1, 93, WOOD[2])
        rect(detail, left, 68, left + 1, 69, IRON[1])
        rect(detail, left, 86, left + 1, 87, IRON[1])
    for x in range(49, 72, 4):
        bottom = 94 if closed else 64
        top = 62 - int(math.sqrt(max(0, 12 ** 2 - (x + 0.5 - 60) ** 2)))
        rect(detail, x, top, x, bottom, IRON[1])
        rect(detail, x + 1, top, x + 1, bottom - 1, IRON[2])
        detail.put(x, bottom + 1, IRON[3])
    for y in ((65, 74, 83, 91) if closed else (57, 62)):
        for x in range(48, 72):
            if y >= 62 or (x + 0.5 - 60) ** 2 + (y - 62) ** 2 < 12 ** 2:
                detail.put(x, y, IRON[3])
                detail.put(x, y + 1, IRON[0])
    for left in (27, 83):
        rect(detail, left, 60, left + 8, 62, IRON[1])
        for x in range(left + 1, left + 8):
            bottom = 81 - abs(x - left - 4)
            rect(detail, x, 63, x, bottom, BLUE[0 if x > left + 5 else 1])
            detail.put(x, 64, BLUE[2])
        stamp(detail, left + 2, 67, ["..g..", ".gGg.", "gGGGg", ".gGg.", "..g.."], {"g": GOLD[1], "G": GOLD[3]})
    lantern = {"i": IRON[0], "I": IRON[2], "y": GLOW[1], "Y": GLOW[3], "W": GLOW[4]}
    for x in (41, 74):
        stamp(detail, x, 59, LANTERN, lantern)
    # A small blank civic plaque leaves destination names to the existing signs.
    rect(detail, 55, 44, 64, 46, ARCH[1])
    rect(detail, 56, 44, 63, 45, ARCH[4])
    composite(canvas, detail, None)
    return canvas


if __name__ == "__main__":
    for name, closed in (("gate-open", False), ("gate-closed", True)):
        sprite = build(closed)
        sprite.save(DEST / f"{name}.png")
        save_sprite(sprite, DEST / f"{name}.bmp")
    print("Wrote approval-only 120 x 96 town gate sprites.")
