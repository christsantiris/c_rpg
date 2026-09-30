#!/usr/bin/env python3
"""Town 1 harbor sprite.

Draws 5 x 4 tiles (120 x 96) for the harbor lot at TOWN_HARBOR_X/Y in the
south-east corner of Town 1. The dock crosses row TOWN_HARBOR_ENTRANCE_Y,
where the harbor road ends. The harbor screen shows the same art enlarged. Run
`make sprites` to rewrite assets/images/harbor.bmp.
"""
import random
from pathlib import Path

from PIL import Image

from pixelkit import (BARREL, CRATE, LANTERN, Layer, arched_door, beam, composite, flat_wall,
                      line_points, lit_window, plaster, poly_mask, roof_tiles, save_sprite,
                      shadow_under, stamp, stone_arch, wobble)

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 120, 96
CX = 37

OUT = (18, 14, 14)
ROOF = [(40, 11, 9), (58, 16, 13), (100, 29, 22), (128, 40, 28), (156, 56, 38), (184, 82, 54)]
PLASTER = [(136, 110, 78), (170, 144, 106), (200, 176, 134), (222, 202, 162)]
TIMBER = [(26, 15, 9), (44, 26, 15), (64, 39, 22), (86, 54, 31), (110, 71, 41)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
DECK = [(34, 21, 13), (56, 36, 22), (80, 53, 32), (104, 71, 43), (128, 90, 56)]
HULL = [(36, 20, 12), (60, 34, 20), (88, 52, 30), (114, 72, 42), (140, 94, 58)]
SAIL = [(128, 116, 94), (170, 158, 128), (206, 196, 166), (232, 224, 200), (246, 242, 226)]
ROPE = [(96, 76, 46), (150, 124, 82), (196, 170, 120)]
SEA = [(6, 38, 60), (12, 52, 76), (19, 75, 90), (29, 96, 122), (51, 120, 138), (104, 173, 166)]

ROPE_COIL = [
    "..rRRr..",
    ".rRrrRr.",
    "rRr..rRr",
    "rRr..rRr",
    ".rRrrRr.",
    "..rrrr..",
]


def piling(L, x, top, bottom, wrap):
    """Dock post with two rope wraps near its top."""
    for y in range(top, bottom + 1):
        L.put(x, y, DECK[3])
        L.put(x + 1, y, DECK[2])
        L.put(x + 2, y, DECK[1])
    L.put(x + 1, top, DECK[4])
    for y in (wrap, wrap + 2):
        for dx in range(-1, 4):
            L.put(x + dx, y, ROPE[2] if dx < 2 else ROPE[1])
        L.put(x - 1, y + 1, ROPE[0])
        L.put(x + 3, y + 1, ROPE[0])


def sea(L):
    """Harbour water with an irregular edge and ripples."""
    rnd = random.Random(4)
    top = wobble(W, 3, 62, 66)
    left = wobble(H, 5, 1, 5)
    right = wobble(H, 6, 1, 4)
    water = set()
    for y in range(H):
        for x in range(W):
            if y >= top[x] and x >= left[y] and x <= W - 1 - right[y] and y < 94:
                water.add((x, y))
    for (x, y) in water:
        c = SEA[2] if (x * 3 + y * 7) % 13 else SEA[1]
        if (x, y - 1) not in water:
            c = SEA[4]
        elif y > 86:
            c = SEA[1]
        L.put(x, y, c)
    keys = sorted(water)
    for _ in range(40):
        x, y = rnd.choice(keys)
        ln = rnd.randint(3, 6)
        if all((x + i, y) in water and (x + i, y - 1) in water for i in range(ln)):
            for i in range(ln):
                L.put(x + i, y, SEA[3])
            L.put(x + ln // 2, y - 1, SEA[4])


def boat(L):
    """Small sailboat moored at the end of the dock."""
    hull = poly_mask((W, H), [(78, 60), (116, 58), (112, 72), (104, 82), (86, 82), (80, 72)])
    for (x, y) in sorted(hull):
        c = HULL[2]
        if (y - 60) % 4 == 3:
            c = HULL[1]
        if x < 85:
            c = HULL[3] if c == HULL[2] else c
        if x > 108:
            c = HULL[1] if c == HULL[2] else HULL[0]
        L.put(x, y, c)
    for x in range(79, 116):
        L.put(x, 60, HULL[4])
        L.put(x, 61, HULL[0])
        L.put(x, 62, HULL[0] if x % 5 else HULL[1])
    for x in range(88, 95):
        for y in range(56, 61):
            L.put(x, y, DECK[2] if y > 56 else DECK[3])
    for y in range(6, 61):
        L.put(97, y, TIMBER[3])
        L.put(98, y, TIMBER[1])
    for (x, y) in line_points(82, 18, 114, 24):
        L.put(x, y, TIMBER[3])
        L.put(x, y + 1, TIMBER[1])
    sail = poly_mask((W, H), [(84, 20), (112, 25), (114, 46), (108, 52), (86, 50), (85, 34)])
    for (x, y) in sorted(sail):
        if x in (97, 98):
            continue
        t = (x - 84) / 30.0
        c = SAIL[3] if t < 0.35 else (SAIL[2] if t < 0.7 else SAIL[1])
        if (x + y) % 11 == 0 and t > 0.3:
            c = SAIL[1]
        if y > 48:
            c = SAIL[1]
        L.put(x, y, c)
    for (x, y) in line_points(97, 7, 80, 59) + line_points(98, 7, 115, 57):
        if not L.on(x, y):
            L.put(x, y, ROPE[1])


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    sw = Layer(W, H)
    sea(sw)
    composite(canvas, sw, None)

    bt = Layer(W, H)
    boat(bt)
    composite(canvas, bt, OUT)

    # chimney, roof and gable face
    ch = Layer(W, H)
    flat_wall(ch, 18, 25, 2, 22, STONE, row_h=4, base=3, seed=5, bw=(3, 5),
              shade=lambda x, y: -1 if x > 23 else 0)
    for x in range(17, 27):
        ch.put(x, 1, STONE[4])
        ch.put(x, 2, STONE[1])
    composite(canvas, ch, OUT)
    rf = Layer(W, H)
    outer = poly_mask((W, H), [(CX - 26, 33), (CX, 2), (CX + 1, 2), (CX + 27, 33)])
    inner = poly_mask((W, H), [(CX - 18, 37), (CX - 18, 33), (CX, 11), (CX + 1, 11),
                               (CX + 19, 33), (CX + 19, 37)])
    roof_tiles(rf, outer - inner, ROOF, 9, shade=lambda x, y: -1 if x > CX else 0)
    composite(canvas, rf, OUT)
    gf = Layer(W, H)
    face = {(x, y) for (x, y) in inner if y <= 35}
    plaster(gf, face, PLASTER, 3)
    edge = {(x, y) for (x, y) in face if (x - 1, y) not in face or (x + 1, y) not in face
            or (x, y - 1) not in face}
    for (x, y) in edge:
        gf.put(x, y, TIMBER[2])
    beam(gf, CX, 13, CX, 18, TIMBER)
    beam(gf, CX - 14, 30, CX + 15, 30, TIMBER)
    for px in (CX - 9, CX + 10):
        beam(gf, px, 21, px, 35, TIMBER)
    beam(gf, CX - 10, 25, CX - 15, 31, TIMBER)
    beam(gf, CX + 11, 25, CX + 16, 31, TIMBER)
    shadow_under(gf, face, PLASTER, TIMBER, PLASTER[1])
    lit_window(gf, CX - 4, 19, 9, 8, GLOW, TIMBER)
    composite(canvas, gf, OUT)

    # jetty beam and stone ground floor
    gr = Layer(W, H)
    for x in range(16, 60):
        gr.put(x, 36, TIMBER[3])
        gr.put(x, 37, TIMBER[1])
    flat_wall(gr, 18, 57, 38, 56, STONE, row_h=4, base=3, seed=8, bw=(5, 8),
              shade=lambda x, y: -1 if y < 40 else 0)
    for x0 in (17, 56):
        for y in range(38, 57):
            gr.put(x0, y, TIMBER[3])
            gr.put(x0 + 1, y, TIMBER[1])
    composite(canvas, gr, OUT)
    dr = Layer(W, H)
    stone_arch(dr, CX, 43, 55, 5, STONE, thick=2)
    arched_door(dr, CX, 43, 55, 5, TIMBER, IRON)
    for x in range(CX - 7, CX + 8):
        dr.put(x, 56, STONE[5] if x < CX + 4 else STONE[4])
        dr.put(x, 57, STONE[2])
    composite(canvas, dr, OUT)

    # dock deck with its front beam and pilings
    dk = Layer(W, H)
    for y in range(56, 64):
        for x in range(2, 78):
            c = DECK[3] if x % 5 else DECK[1]
            if y == 56:
                c = DECK[4] if x % 5 else DECK[2]
            dk.put(x, y, c)
    for x in range(2, 78):
        dk.put(x, 64, DECK[2])
        dk.put(x, 65, DECK[1])
    composite(canvas, dk, OUT)
    ps = Layer(W, H)
    for (px, top, bottom) in ((3, 52, 76), (26, 55, 80), (50, 55, 79), (72, 52, 82), (116, 50, 80)):
        piling(ps, px, top, bottom, top + 3)
    composite(canvas, ps, OUT)

    # crane post with a hanging hook, lantern, barrel, crate, rope coil
    pr = Layer(W, H)
    for y in range(24, 57):
        pr.put(66, y, TIMBER[3])
        pr.put(67, y, TIMBER[1])
    for x in range(64, 86):
        pr.put(x, 24, TIMBER[3])
        pr.put(x, 25, TIMBER[1])
    for (x, y) in line_points(67, 33, 74, 26):
        pr.put(x, y, TIMBER[2])
    for y in range(26, 42):
        pr.put(84, y, ROPE[1])
    pr.put(83, 42, IRON[3])
    pr.put(85, 42, IRON[3])
    pr.put(84, 43, IRON[2])
    for y in range(26, 31):
        pr.put(12, y, TIMBER[2])
    for x in range(10, 18):
        pr.put(x, 25, TIMBER[3])
    stamp(pr, 8, 30, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                              "W": GLOW[4]})
    wood = {"t": TIMBER[1], "T": TIMBER[2], "W": TIMBER[3], "i": IRON[1], "I": IRON[2]}
    stamp(pr, 2, 43, BARREL, wood)
    stamp(pr, 17, 46, CRATE, wood)
    stamp(pr, 57, 55, ROPE_COIL, {"r": ROPE[0], "R": ROPE[2]})
    composite(canvas, pr, OUT)
    return canvas


def main():
    save_sprite(build(), ASSETS / "harbor.bmp")


if __name__ == "__main__":
    main()
