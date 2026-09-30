#!/usr/bin/env python3
"""Crown Road gatehouse sprite.

Draws 3 x 3 tiles (72 x 72) centred on the Crown Road, from the tile left of
CROWNROAD_X: Town 2's north exit and Town 3's south exit. The archway is
transparent so the road shows through. Run `make sprites` to rewrite
assets/images/crownroad-gate.bmp.
"""
import math
import random
from pathlib import Path

from PIL import Image

from pixelkit import Layer, composite, flat_wall, gothic_half, merlons, save_sprite, stamp

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 72, 72
CX = 36

OUT = (16, 14, 16)
PALE = [(40, 38, 42), (56, 54, 58), (108, 104, 102), (132, 128, 124), (158, 154, 146), (186, 182, 170)]
GOLD = [(110, 80, 28), (170, 130, 48), (222, 182, 76), (252, 226, 146)]
RED = [(76, 16, 20), (126, 26, 30), (168, 44, 44)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
FIRE = [(141, 60, 28), (236, 140, 43), (255, 221, 120)]
IVY = [(28, 54, 28), (40, 74, 37), (69, 109, 45), (98, 140, 60)]
DARK = [(10, 10, 14), (20, 18, 24), (32, 28, 36)]

CROWN = [
    "y..y..y",
    "yY.Y.Yy",
    "yYYYYYy",
    "yYrYbYy",
    "yyyyyyy",
]

BANNER_CROWN = [
    "y.y.y",
    "yYYYy",
    "yyyyy",
]

TORCH = [
    ".y.",
    "yWy",
    "oyo",
    ".i.",
    ".I.",
    "iIi",
]


def banner(L, x0, y0, w, h, seed):
    """Torn crimson banner with gold edges and a small crown."""
    rnd = random.Random(seed)
    for x in range(x0 - 1, x0 + w + 1):
        L.put(x, y0 - 1, IRON[2])
    for i in range(w):
        length = h - rnd.choice((0, 0, 2, 4, 6)) - (3 if i in (w // 2 - 1, w // 2) else 0)
        for y in range(y0, y0 + length):
            c = RED[1]
            if i in (0, w - 1):
                c = GOLD[1]
            elif i == 1:
                c = RED[2]
            elif i == w - 2:
                c = RED[0]
            L.put(x0 + i, y, c)
    stamp(L, x0 + (w - 5) // 2, y0 + 5, BANNER_CROWN, {"y": GOLD[1], "Y": GOLD[3]})


def ivy(L, x, y0, y1, side, seed):
    """Ivy climbing a tower edge, leaning towards side (-1 left, 1 right)."""
    rnd = random.Random(seed)
    for y in range(y0, y1):
        L.put(x, y, IVY[1] if y % 3 else IVY[2])
        if rnd.random() < 0.5:
            L.put(x + side, y, IVY[2] if rnd.random() < 0.5 else IVY[3])
        if y % 4 == 0:
            L.put(x + side * 2, y, IVY[2])
            L.put(x + side * 2, y + 1, IVY[1])
        if rnd.random() < 0.15:
            x += rnd.choice((-1, 1))


def tower(L, x0, x1, seed):
    """Crenellated gate tower with a string course, an arrow slit and a plinth."""
    flat_wall(L, x0, x1, 10, 71, PALE, row_h=6, base=3, seed=seed, bw=(6, 10),
              shade=lambda x, y: (1 if x < x0 + 3 else (-1 if x > x1 - 3 else 0)))
    merlons(L, x0 - 1, x1 + 1, 2, 9, PALE, mw=4, gap=3, base=4, point=0, start=0)
    for x in range(x0 - 1, x1 + 2):
        L.put(x, 10, PALE[5])
        L.put(x, 11, PALE[2])
        L.put(x, 27, PALE[5])
        L.put(x, 28, PALE[1])
    cx = (x0 + x1) // 2
    for y in range(14, 23):
        L.put(cx, y, DARK[0])
        L.put(cx - 1 if y > 16 else cx, y, DARK[1])
    for x in range(x0 - 1, x1 + 2):
        for y in (64, 65):
            L.put(x, y, PALE[5] if y == 64 else PALE[2])


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # central wall with its arch and a cracked crown relief
    wall = Layer(W, H)
    flat_wall(wall, 20, 51, 14, 71, PALE, row_h=6, base=2, seed=5, bw=(7, 11))
    merlons(wall, 20, 51, 8, 7, PALE, mw=4, gap=3, base=3, point=0, start=1)
    for x in range(20, 52):
        wall.put(x, 14, PALE[4])
        wall.put(x, 15, PALE[1])
    ya, ys, hw = 30, 42, 11
    for y in range(ya - 4, H):
        half = gothic_half(y, ya, ys, hw)
        outer = gothic_half(y, ya - 4, ys, hw + 3)
        for x in range(CX - hw - 4, CX + hw + 4):
            dx = abs(x + 0.5 - CX)
            if half >= 0 and dx <= half:
                wall.erase(x, y)
            elif outer >= 0 and dx <= outer:
                seg = int(math.atan2(y - ys, x - CX + 0.5) * 6) % 2
                c = PALE[5] if seg else PALE[4]
                if y >= ys:
                    c = PALE[4] if (y - ys) % 6 < 5 else PALE[1]
                wall.put(x, y, c)
    stamp(wall, CX - 4, 17, CROWN, {"y": GOLD[1], "Y": GOLD[2], "r": RED[2], "b": (60, 90, 180)})
    for (x, y) in ((CX, 17), (CX, 18), (CX + 1, 19), (CX + 1, 20)):
        wall.put(x, y, DARK[1])
    composite(canvas, wall, OUT)

    # passage shadow and the raised portcullis in the arch
    ps = Layer(W, H)
    for y in range(ya, ys + 6):
        half = gothic_half(y, ya, ys, hw)
        if half < 0:
            continue
        for x in range(CX - hw, CX + hw + 1):
            if abs(x + 0.5 - CX) > half:
                continue
            c = DARK[0] if y < ys else DARK[1]
            if (x - CX) % 4 == 0 and y < ys + 3:
                c = IRON[2] if x < CX else IRON[1]
            elif y == ys - 2:
                c = IRON[1]
            ps.put(x, y, c)
    for x in range(CX - hw + 1, CX + hw):
        if (x - CX) % 4 == 0:
            ps.put(x, ys + 3, IRON[3])
            ps.put(x, ys + 4, IRON[2])
    composite(canvas, ps, None)

    # towers, banners, torches and ivy
    for (x0, x1, seed, side) in ((1, 20, 11, -1), (51, 70, 12, 1)):
        tw = Layer(W, H)
        tower(tw, x0, x1, seed)
        banner(tw, x0 + 5, 31, 10, 24, seed)
        composite(canvas, tw, OUT)
        iv = Layer(W, H)
        ivy(iv, x0 if side < 0 else x1, 30, 66, side, seed + 3)
        composite(canvas, iv, None)
    tc = Layer(W, H)
    for tx in (19, 51):
        stamp(tc, tx - 1, 44, TORCH, {"y": FIRE[1], "W": FIRE[2], "o": FIRE[0], "i": IRON[1],
                                      "I": IRON[2]})
    composite(canvas, tc, OUT)
    return canvas


def main():
    save_sprite(build(), ASSETS / "crownroad-gate.bmp")


if __name__ == "__main__":
    main()
