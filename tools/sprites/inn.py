#!/usr/bin/env python3
"""Town 2 inn sprite.

Draws 7 x 5 tiles (168 x 120) for the inn lot at (5, 16) in Town 2, with the
double door centred on tile (8, 20). Run `make sprites` to rewrite
assets/inn.png and assets/inn.bmp.
"""
from pathlib import Path

from PIL import Image

from pixelkit import (LANTERN, Layer, beam, composite, flat_wall, lit_window, plaster, poly_mask,
                      roof_tiles, save_sprite, shadow_under, stamp)

ASSETS = Path(__file__).resolve().parents[2] / "assets"
W, H = 168, 120

OUT = (18, 14, 16)
SLATE = [(20, 24, 36), (30, 36, 54), (48, 58, 82), (62, 76, 104), (80, 96, 126), (104, 122, 150)]
PLASTER = [(150, 146, 136), (184, 180, 168), (212, 208, 194), (234, 230, 218)]
TIMBER = [(24, 16, 12), (40, 27, 19), (58, 40, 28), (80, 56, 38), (104, 74, 50)]
SAND = [(38, 30, 24), (48, 38, 30), (116, 96, 72), (138, 116, 88), (162, 140, 108), (186, 164, 130)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
GOLD = [(140, 90, 20), (204, 150, 40), (244, 196, 70), (255, 232, 144)]
NAVY = [(18, 22, 46), (28, 36, 70), (42, 52, 94)]
LEAF = [(26, 58, 30), (44, 94, 44), (70, 128, 58)]
FLOWERS = [(196, 46, 58), (230, 124, 152), (246, 206, 80), (240, 236, 224)]
WATER = [(26, 58, 84), (42, 88, 118), (84, 142, 162)]
HAY = [(112, 84, 28), (164, 126, 48), (206, 166, 78), (234, 204, 118)]


def flower_box(L, x0, y, w, seed):
    """Window box of leaves and red, pink and yellow flowers."""
    for x in range(x0, x0 + w):
        L.put(x, y + 2, TIMBER[3] if x < x0 + w - 1 else TIMBER[1])
        L.put(x, y + 3, TIMBER[2])
        L.put(x, y + 4, TIMBER[1])
    for i, x in enumerate(range(x0, x0 + w)):
        L.put(x, y + 1, LEAF[1] if (x + seed) % 3 else LEAF[2])
        if (i + seed) % 3 == 0:
            L.put(x, y, FLOWERS[(i // 3 + seed) % 3])
        elif (i + seed) % 3 == 1:
            L.put(x, y, LEAF[0])


def railing(L, x0, x1, y0, y1):
    """Balcony rails with balusters every third pixel."""
    for x in range(x0, x1 + 1):
        L.put(x, y0, TIMBER[3])
        L.put(x, y0 + 1, TIMBER[1])
        L.put(x, y1, TIMBER[2])
    for x in range(x0 + 1, x1, 3):
        for y in range(y0 + 2, y1):
            L.put(x, y, TIMBER[3] if y < y0 + 4 else TIMBER[2])


def double_door(L, x0, y0, x1, y1):
    """Two plank door leaves with iron bands and handles."""
    mid = (x0 + x1) // 2
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if x == mid:
                c = TIMBER[0]
            else:
                k = (x - x0) % 3
                c = TIMBER[3] if k == 1 else (TIMBER[1] if k == 0 else TIMBER[2])
            if y in (y0 + 4, y1 - 5) and x != mid:
                c = IRON[1]
            L.put(x, y, c)
    for (hx, hy) in ((mid - 2, y0 + 12), (mid + 2, y0 + 12)):
        L.put(hx, hy, IRON[3])
        L.put(hx, hy + 1, IRON[2])


MOON = [
    "...yyy.....",
    ".yYYy...s..",
    ".yYy...sSs.",
    "yYY.....s..",
    "yYY........",
    ".yYy.......",
    ".yYYy......",
    "...yyy.....",
]

TROUGH = [
    "ttttttttttttttttttt",
    "tbbbbbbbbbbbbbbbbbt",
    "tWwwwwwwWwwwwwwwwwt",
    "tTTTTTTTTTTTTTTTTTt",
    "tTiTTTTTTTTTTTTTiTt",
    "tTTTTTTTTTTTTTTTTTt",
    "tTiTTTTTTTTTTTTTiTt",
    "ttttttttttttttttttt",
    ".tt.............tt.",
]

POT = [
    ".rfr.",
    "flflf",
    ".lLl.",
    "cCCCc",
    ".cCc.",
    ".ccc.",
]

HAYBALE = [
    ".hhhhhhhhh.",
    "hHHYHHYHHHh",
    "hHYHHHHYHHh",
    "hsssssssssh",
    "hHHHYHHHYHh",
    "hHYHHHYHHHh",
    "hsssssssssh",
    "hHHHHYHHHHh",
    ".hhhhhhhhh.",
]


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # chimney behind the middle roof
    ch = Layer(W, H)
    flat_wall(ch, 118, 127, 14, 44, SAND, row_h=4, base=3, seed=4, bw=(4, 6),
              shade=lambda x, y: -1 if x > 124 else 0)
    for x in range(117, 129):
        ch.put(x, 11, SAND[4])
        ch.put(x, 12, SAND[3])
        ch.put(x, 13, SAND[1])
    for x in range(120, 126):
        ch.put(x, 11, OUT)
    composite(canvas, ch, OUT)

    # middle roof between the wings
    mr = Layer(W, H)
    roof_tiles(mr, poly_mask((W, H), [(44, 60), (62, 34), (106, 34), (124, 60)]), SLATE, 7,
               shade=lambda x, y: -1 if x > 100 else 0, tile_w=6, row_h=3, rounded=False)
    for x in range(62, 107):
        mr.put(x, 34, SLATE[4] if x % 3 else SLATE[2])
    composite(canvas, mr, OUT)

    # middle upper floor behind the balcony
    up = Layer(W, H)
    wall = {(x, y) for x in range(52, 117) for y in range(59, 80)}
    plaster(up, wall, PLASTER, 5)
    for x in (52, 74, 93, 115):
        beam(up, x, 59, x, 79, TIMBER)
    beam(up, 52, 59, 116, 59, TIMBER)
    shadow_under(up, wall, PLASTER, TIMBER, PLASTER[1])
    lit_window(up, 60, 62, 9, 9, GLOW, TIMBER, shutters=False)
    lit_window(up, 79, 61, 11, 12, GLOW, TIMBER, shutters=False)
    lit_window(up, 100, 62, 9, 9, GLOW, TIMBER, shutters=False)
    composite(canvas, up, OUT)

    # wing gables, left and right
    for (cx, seed) in ((30, 21), (138, 22)):
        gr = Layer(W, H)
        outer = poly_mask((W, H), [(cx - 30, 61), (cx, 12), (cx + 1, 12), (cx + 31, 61)])
        inner = poly_mask((W, H), [(cx - 22, 80), (cx - 22, 63), (cx, 25), (cx + 1, 25),
                                   (cx + 23, 63), (cx + 23, 80)])
        roof_tiles(gr, outer - inner, SLATE, seed, shade=lambda x, y, c=cx: -1 if x > c else 0,
                   tile_w=6, row_h=3, rounded=False)
        composite(canvas, gr, OUT)
        fc = Layer(W, H)
        face = {(x, y) for (x, y) in inner if y <= 79}
        plaster(fc, face, PLASTER, seed + 1)
        edge = {(x, y) for (x, y) in face if (x - 1, y) not in face or (x + 1, y) not in face
                or (x, y - 1) not in face}
        for (x, y) in edge:
            fc.put(x, y, TIMBER[2])
        for (x, y) in edge:
            if (x, y + 1) in face and (x, y + 1) not in edge:
                fc.put(x, y + 1, TIMBER[1])
        beam(fc, cx, 27, cx, 34, TIMBER)
        beam(fc, cx - 12, 46, cx + 13, 46, TIMBER)
        beam(fc, cx - 21, 72, cx + 22, 72, TIMBER)
        for px in (cx - 11, cx + 12):
            beam(fc, px, 47, px, 71, TIMBER)
        beam(fc, cx - 12, 58, cx - 20, 70, TIMBER)
        beam(fc, cx + 13, 58, cx + 21, 70, TIMBER)
        shadow_under(fc, face, PLASTER, TIMBER, PLASTER[1])
        lit_window(fc, cx - 2, 36, 5, 6, GLOW, TIMBER, shutters=False)
        lit_window(fc, cx - 5, 53, 11, 11, GLOW, TIMBER)
        flower_box(fc, cx - 6, 64, 13, seed)
        composite(canvas, fc, OUT)

    # balcony across the middle
    bl = Layer(W, H)
    railing(bl, 53, 115, 70, 78)
    for x in (53, 115):
        for y in range(62, 79):
            bl.put(x, y, TIMBER[3] if x == 53 else TIMBER[2])
    composite(canvas, bl, OUT)

    # jetty beam
    jt = Layer(W, H)
    for x in range(4, 165):
        jt.put(x, 80, TIMBER[3])
        jt.put(x, 81, TIMBER[2])
        jt.put(x, 82, TIMBER[2])
        jt.put(x, 83, TIMBER[1])
    for x in range(8, 162, 8):
        jt.put(x, 84, TIMBER[1])
        jt.put(x + 1, 84, TIMBER[0])
    composite(canvas, jt, OUT)

    # sandstone ground floor with timber posts
    gf = Layer(W, H)
    flat_wall(gf, 8, 160, 85, 116, SAND, row_h=5, base=3, seed=31, bw=(7, 12),
              shade=lambda x, y: -1 if y < 88 else 0)
    for x0 in (8, 53, 113, 158):
        for y in range(85, 117):
            gf.put(x0, y, TIMBER[3])
            gf.put(x0 + 1, y, TIMBER[2])
            gf.put(x0 + 2, y, TIMBER[1])
    for x in range(8, 161):
        gf.put(x, 116, SAND[1])
    lit_window(gf, 25, 92, 11, 10, GLOW, TIMBER)
    lit_window(gf, 133, 92, 11, 10, GLOW, TIMBER)
    flower_box(gf, 23, 103, 15, 3)
    flower_box(gf, 131, 103, 15, 4)
    composite(canvas, gf, OUT)

    # entrance: fanlight, double door, steps and lanterns
    dr = Layer(W, H)
    for x in range(72, 97):
        dr.put(x, 88, TIMBER[3])
        dr.put(x, 89, TIMBER[1])
    for y in range(90, 116):
        for x in (73, 95):
            dr.put(x, y, TIMBER[2])
    lit_window(dr, 78, 85, 13, 2, GLOW, TIMBER, shutters=False)
    double_door(dr, 75, 90, 93, 115)
    for x in range(72, 97):
        dr.put(x, 116, SAND[4])
        dr.put(x, 117, SAND[2])
    for x in range(70, 99):
        dr.put(x, 118, SAND[5] if x < 92 else SAND[4])
        dr.put(x, 119, SAND[2])
    for lx in (63, 101):
        dr.put(lx + 2, 85, IRON[2])
        dr.put(lx + 2, 86, IRON[1])
        stamp(dr, lx, 87, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                                   "W": GLOW[4]})
    composite(canvas, dr, OUT)

    # hanging moon-and-star sign on the right
    sg = Layer(W, H)
    for x in range(150, 168):
        sg.put(x, 85, IRON[3] if x < 160 else IRON[2])
        sg.put(x, 86, IRON[1])
    for y in range(87, 89):
        sg.put(153, y, IRON[2])
        sg.put(164, y, IRON[2])
    for y in range(89, 101):
        for x in range(151, 167):
            c = NAVY[1]
            if x in (151, 166) or y in (89, 100):
                c = TIMBER[2]
            elif y == 90:
                c = NAVY[2]
            sg.put(x, y, c)
    stamp(sg, 153, 91, MOON, {"y": GOLD[1], "Y": GOLD[2], "s": GOLD[2], "S": GOLD[3]})
    composite(canvas, sg, OUT)

    # trough and hitching post, flower pots, hay bales
    pr = Layer(W, H)
    stamp(pr, 6, 110, TROUGH, {"t": TIMBER[1], "T": TIMBER[2], "W": WATER[2], "w": WATER[1],
                               "b": WATER[0], "i": IRON[2]})
    for y in range(100, 119):
        pr.put(2, y, TIMBER[3])
        pr.put(3, y, TIMBER[1])
    for x in range(1, 6):
        pr.put(x, 100, TIMBER[3])
    pr.put(4, 103, IRON[3])
    pr.put(5, 104, IRON[2])
    pr.put(4, 105, IRON[2])
    for px in (66, 98):
        stamp(pr, px, 112, POT, {"r": FLOWERS[0], "f": FLOWERS[1], "l": LEAF[1], "L": LEAF[2],
                                 "c": (120, 58, 36), "C": (156, 80, 48)})
    stamp(pr, 145, 110, HAYBALE, {"h": HAY[0], "H": HAY[2], "Y": HAY[3], "s": HAY[1]})
    stamp(pr, 156, 110, HAYBALE, {"h": HAY[0], "H": HAY[2], "Y": HAY[3], "s": HAY[1]})
    stamp(pr, 150, 102, HAYBALE, {"h": HAY[0], "H": HAY[2], "Y": HAY[3], "s": HAY[1]})
    composite(canvas, pr, OUT)
    return canvas


def main():
    save_sprite(build(), ASSETS / "inn.png", ASSETS / "inn.bmp")


if __name__ == "__main__":
    main()
