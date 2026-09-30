#!/usr/bin/env python3
"""Town 2 healer sprite.

Draws 5 x 4 tiles (120 x 96) for the healer lot at TOWN_HEALER_X/Y in Town 2,
with the door on TOWN_HEALER_DOOR_X/Y. Run `make sprites` to rewrite
assets/images/healer.bmp.
"""
from pathlib import Path

from PIL import Image

from pixelkit import (LANTERN, Layer, arched_door, beam, composite, flat_wall, lit_window,
                      plaster, poly_mask, roof_tiles, save_sprite, shadow_under, stamp,
                      stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 120, 96
CX = 66
DOOR_CX = 62

OUT = (18, 16, 12)
SAGE = [(20, 34, 22), (30, 48, 32), (50, 76, 50), (66, 96, 62), (86, 118, 78), (110, 142, 98)]
PLASTER = [(136, 110, 78), (170, 144, 106), (200, 176, 134), (222, 202, 162)]
TIMBER = [(26, 15, 9), (44, 26, 15), (64, 39, 22), (86, 54, 31), (110, 71, 41)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
LEAF = [(24, 54, 28), (40, 88, 42), (66, 124, 56), (98, 156, 76)]
LAVENDER = [(84, 52, 120), (128, 86, 170), (170, 128, 206)]
BLOOM = [(200, 196, 184), (236, 234, 224)]
CLAY = [(96, 46, 28), (132, 68, 40), (164, 92, 56)]
GLASS = [(70, 110, 96), (120, 170, 150), (196, 226, 210)]
SIGN = [(26, 60, 38), (40, 88, 56), (58, 112, 72)]
IVORY = [(206, 198, 170), (238, 232, 210)]
SHADE = [(22, 14, 10), (34, 22, 16), (48, 32, 22)]

CROSS = [
    "..II..",
    "..Ii..",
    "IIIIII",
    "Iiiiii",
    "..Ii..",
    "..Ii..",
]

PLANTER = [
    "..L.lL.Ll..",
    ".lLblLlbLl.",
    "lLlLblLlLLl",
    ".lLlLlLlbl.",
    "cCCCCCCCCCc",
    "cCcCCCCCcCc",
    ".ccccccccc.",
]

BOTTLE = [
    ".c.",
    "gGg",
    "gWg",
    "ggg",
]


def planter(L, x0, y0, flower, seed):
    """Clay planter of leafy herbs with a few blooms on top."""
    pal = {"l": LEAF[1], "L": LEAF[2], "b": flower, "c": CLAY[0], "C": CLAY[1]}
    stamp(L, x0, y0, PLANTER, pal)
    for i in range(3):
        fx = x0 + 1 + (i * 4 + seed) % 9
        L.put(fx, y0 - 1, LEAF[3])
        L.put(fx, y0 - 2, flower)


def herb_stall(L):
    """Timber stall under the lean-to: lit window, hanging herbs, shelf and table."""
    for y in range(64, 93):
        for x in range(4, 42):
            if y < 64 + (42 - x) * 0.14:
                continue
            L.put(x, y, SHADE[1] if (x + y) % 7 else SHADE[2])
    for x0 in (3, 38):
        for y in range(64, 93):
            L.put(x0, y, TIMBER[3])
            L.put(x0 + 1, y, TIMBER[2])
            L.put(x0 + 2, y, TIMBER[1])
    beam(L, 3, 64, 41, 57, TIMBER)
    lit_window(L, 26, 68, 7, 8, GLOW, TIMBER, shutters=False)
    for (hx, pal) in ((9, LAVENDER), (13, BLOOM), (17, LAVENDER)):
        L.put(hx, 67, TIMBER[3])
        for y in range(68, 73):
            L.put(hx, y, pal[min(len(pal) - 1, 1 + (y % 2))])
            if y > 69:
                L.put(hx - 1, y, pal[0])
                L.put(hx + 1, y, pal[0])
    for x in range(7, 36):
        L.put(x, 77, TIMBER[3])
        L.put(x, 78, TIMBER[1])
    for i, bx in enumerate((9, 14, 19)):
        stamp(L, bx, 73, BOTTLE, {"c": TIMBER[3], "g": GLASS[0], "G": GLASS[1],
                                  "W": GLASS[2] if i % 2 else LAVENDER[2]})
    for x in range(6, 36):
        L.put(x, 83, TIMBER[4])
        L.put(x, 84, TIMBER[2])
        L.put(x, 85, TIMBER[1])
    for lx in (8, 33):
        for y in range(86, 93):
            L.put(lx, y, TIMBER[2])
            L.put(lx + 1, y, TIMBER[1])
    for (x, y, c) in ((21, 81, STONE[4]), (22, 81, STONE[4]), (23, 81, STONE[3]),
                      (20, 82, STONE[3]), (21, 82, STONE[4]), (22, 82, STONE[3]),
                      (23, 82, STONE[2]), (24, 82, STONE[2]), (24, 80, TIMBER[3]),
                      (25, 79, TIMBER[3])):
        L.put(x, y, c)
    stamp(L, 12, 79, BOTTLE, {"c": TIMBER[3], "g": GLASS[0], "G": GLASS[1], "W": GLASS[2]})
    stamp(L, 28, 79, BOTTLE, {"c": TIMBER[3], "g": LAVENDER[0], "G": LAVENDER[1],
                              "W": LAVENDER[2]})


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # chimney behind the roof
    ch = Layer(W, H)
    flat_wall(ch, 82, 91, 14, 46, STONE, row_h=4, base=3, seed=3, bw=(4, 6),
              shade=lambda x, y: -1 if x > 88 else 0)
    for x in range(81, 93):
        ch.put(x, 11, STONE[4])
        ch.put(x, 12, STONE[3])
        ch.put(x, 13, STONE[1])
    for x in range(84, 90):
        ch.put(x, 11, OUT)
    composite(canvas, ch, OUT)

    # lean-to over the herb stall
    lr = Layer(W, H)
    roof_tiles(lr, poly_mask((W, H), [(44, 44), (44, 53), (0, 70), (0, 62)]), SAGE, 5,
               tile_w=5, row_h=3, rounded=False)
    composite(canvas, lr, OUT)
    st = Layer(W, H)
    herb_stall(st)
    composite(canvas, st, OUT)

    # main roof and gable face
    rf = Layer(W, H)
    outer = poly_mask((W, H), [(CX - 40, 64), (CX, 16), (CX + 1, 16), (CX + 41, 64)])
    inner = poly_mask((W, H), [(CX - 28, 67), (CX - 28, 64), (CX, 29), (CX + 1, 29),
                               (CX + 29, 64), (CX + 29, 67)])
    roof_tiles(rf, outer - inner, SAGE, 9, shade=lambda x, y: -1 if x > CX else 0,
               tile_w=5, row_h=3, rounded=False)
    composite(canvas, rf, OUT)
    gf = Layer(W, H)
    face = {(x, y) for (x, y) in inner if y <= 66}
    plaster(gf, face, PLASTER, 13)
    edge = {(x, y) for (x, y) in face if (x - 1, y) not in face or (x + 1, y) not in face
            or (x, y - 1) not in face}
    for (x, y) in edge:
        gf.put(x, y, TIMBER[2])
    for (x, y) in edge:
        if (x, y + 1) in face and (x, y + 1) not in edge:
            gf.put(x, y + 1, TIMBER[1])
    beam(gf, CX, 12, CX, 40, TIMBER)
    beam(gf, CX - 19, 58, CX + 20, 58, TIMBER)
    for px in (CX - 14, CX + 15):
        beam(gf, px, 47, px, 66, TIMBER)
    beam(gf, CX - 15, 52, CX - 24, 63, TIMBER)
    beam(gf, CX + 16, 52, CX + 25, 63, TIMBER)
    shadow_under(gf, face, PLASTER, TIMBER, PLASTER[1])
    lit_window(gf, CX - 5, 44, 11, 10, GLOW, TIMBER)
    composite(canvas, gf, OUT)

    # jetty beam and stone ground floor
    gr = Layer(W, H)
    for x in range(37, 96):
        gr.put(x, 67, TIMBER[3])
        gr.put(x, 68, TIMBER[2])
        gr.put(x, 69, TIMBER[1])
    flat_wall(gr, 39, 93, 70, 91, STONE, row_h=5, base=3, seed=17, bw=(6, 10),
              shade=lambda x, y: -1 if y < 73 else 0)
    for x0 in (38, 91):
        for y in range(70, 92):
            gr.put(x0, y, TIMBER[3])
            gr.put(x0 + 1, y, TIMBER[2])
            gr.put(x0 + 2, y, TIMBER[1])
    for x in range(39, 94):
        gr.put(x, 91, STONE[1])
    composite(canvas, gr, OUT)

    # door, arch, steps and the lanterns beside the door
    dr = Layer(W, H)
    stone_arch(dr, DOOR_CX, 75, 91, 6, STONE)
    arched_door(dr, DOOR_CX, 75, 91, 6, TIMBER, IRON)
    for x in range(DOOR_CX - 8, DOOR_CX + 9):
        dr.put(x, 92, STONE[4])
        dr.put(x, 93, STONE[2])
    for x in range(DOOR_CX - 10, DOOR_CX + 11):
        dr.put(x, 94, STONE[5] if x < DOOR_CX + 5 else STONE[4])
        dr.put(x, 95, STONE[2])
    for lx in (DOOR_CX - 14, DOOR_CX + 10):
        dr.put(lx + 2, 71, IRON[2])
        stamp(dr, lx, 72, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                                   "W": GLOW[4]})
    composite(canvas, dr, OUT)

    # cross sign, lantern and planters
    pr = Layer(W, H)
    for x in range(94, 117):
        pr.put(x, 56, IRON[3] if x < 105 else IRON[2])
        pr.put(x, 57, IRON[1])
    for y in range(58, 61):
        pr.put(101, y, IRON[2])
        pr.put(112, y, IRON[2])
    for y in range(61, 73):
        for x in range(99, 115):
            c = SIGN[1]
            if x in (99, 114) or y in (61, 72):
                c = TIMBER[2]
            elif y == 62:
                c = SIGN[2]
            pr.put(x, y, c)
    stamp(pr, 104, 64, CROSS, {"I": IVORY[1], "i": IVORY[0]})
    pr.put(96, 73, IRON[2])
    stamp(pr, 94, 74, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                               "W": GLOW[4]})
    planter(pr, 40, 88, BLOOM[1], 2)
    planter(pr, 77, 88, LAVENDER[2], 5)
    planter(pr, 100, 88, BLOOM[1], 1)
    composite(canvas, pr, OUT)
    return canvas


def main():
    save_sprite(build(), ASSETS / "healer.bmp")


if __name__ == "__main__":
    main()
