#!/usr/bin/env python3
"""Town 1 tavern sprite.

Draws 7 x 5 tiles (168 x 120) for the tavern lot at TOWN_TAVERN_X/Y in Town 1,
with the door on TOWN_TAVERN_DOOR_X/Y. Run `make sprites` to rewrite
assets/images/tavern.bmp.
"""
from pathlib import Path

from PIL import Image

from pixelkit import (BARREL, CRATE, LANTERN, Layer, arched_door, beam, composite, flat_wall,
                      lit_window, plaster, poly_mask, roof_tiles, save_sprite, shadow_under, stamp,
                      stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 168, 120
DOOR_CX = 84

OUT = (22, 13, 10)
ROOF = [(40, 11, 9), (58, 16, 13), (100, 29, 22), (128, 40, 28), (156, 56, 38), (184, 82, 54)]
PLASTER = [(136, 110, 78), (170, 144, 106), (200, 176, 134), (222, 202, 162)]
TIMBER = [(26, 15, 9), (44, 26, 15), (64, 39, 22), (86, 54, 31), (110, 71, 41)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
GOLD = [(140, 90, 20), (204, 150, 40), (244, 196, 70), (255, 232, 144)]


# ---------------------------------------------------------------- parts
MUG = [
    ".wWWWw...",
    "wWWWWWw..",
    "gYYYYYg..",
    "gYyYYYggg",
    "gYyYYYg.g",
    "gYyYYYg.g",
    "gYyYYYggg",
    "gYYYYYg..",
    ".ggggg...",
]


def prop(L, x0, y0, rows):
    stamp(L, x0, y0, rows, {
        "t": TIMBER[1], "T": TIMBER[2], "W": TIMBER[3],
        "i": IRON[1], "I": IRON[2],
    })


# ---------------------------------------------------------------- build
def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # chimney behind the roof
    ch = Layer(W, H)
    flat_wall(ch, 39, 49, 20, 46, STONE, row_h=4, base=3, seed=3, bw=(4, 6),
              shade=lambda x, y: -1 if x > 46 else 0)
    for x in range(38, 51):
        ch.put(x, 16, STONE[4])
        ch.put(x, 17, STONE[3])
        ch.put(x, 18, STONE[1])
    for x in range(41, 48):
        ch.put(x, 16, OUT)
    composite(canvas, ch, OUT)

    # main roof
    roof = Layer(W, H)
    roof_tiles(roof, poly_mask((W, H), [(10, 66), (33, 28), (136, 28), (163, 66)]), ROOF, 11,
               shade=lambda x, y: -1 if x > 120 else 0)
    for x in range(34, 136):
        roof.put(x, 28, ROOF[4] if x % 4 else ROOF[2])
        roof.put(x, 29, ROOF[2])
    composite(canvas, roof, OUT)

    # upper floor, half-timbered, visible between the gables
    up = Layer(W, H)
    wall = {(x, y) for x in range(17, 162) for y in range(60, 79)}
    plaster(up, wall, PLASTER, 5)
    for x in (17, 29, 57, 112, 124, 150, 160):
        beam(up, x, 60, x, 78, TIMBER)
    beam(up, 17, 60, 161, 60, TIMBER)
    beam(up, 57, 64, 45, 77, TIMBER)
    beam(up, 112, 64, 124, 77, TIMBER)
    shadow_under(up, wall, PLASTER, TIMBER, PLASTER[1])
    composite(canvas, up, OUT)

    # jetty beam between the floors
    jt = Layer(W, H)
    for x in range(14, 164):
        jt.put(x, 79, TIMBER[3])
        jt.put(x, 80, TIMBER[2])
        jt.put(x, 81, TIMBER[2])
        jt.put(x, 82, TIMBER[1])
    for x in range(18, 162, 8):
        jt.put(x, 83, TIMBER[1])
        jt.put(x + 1, 83, TIMBER[0])
    composite(canvas, jt, OUT)

    # ground floor, stone with timber posts
    gf = Layer(W, H)
    flat_wall(gf, 19, 159, 84, 116, STONE, row_h=5, base=3, seed=21, bw=(6, 11),
              shade=lambda x, y: -1 if y < 87 else 0)
    for x0 in (19, 55, 112, 156):
        for y in range(84, 117):
            gf.put(x0, y, TIMBER[3])
            gf.put(x0 + 1, y, TIMBER[2])
            gf.put(x0 + 2, y, TIMBER[1])
    for x in range(19, 160):
        gf.put(x, 116, STONE[1])
    composite(canvas, gf, OUT)

    # gables: the two dormers and the big central cross gable
    for (cx, apex, base_y, half, face_top, seed) in ((36, 38, 68, 25, 47, 31),
                                                     (137, 38, 68, 25, 47, 32),
                                                     (DOOR_CX, 16, 68, 36, 30, 33)):
        gr = Layer(W, H)
        outer = poly_mask((W, H), [(cx - half, base_y), (cx, apex), (cx + 1, apex),
                                   (cx + half + 1, base_y)])
        inset = int(half * 0.34)
        inner = poly_mask((W, H), [(cx - half + inset, 79), (cx - half + inset, base_y + 2),
                                   (cx, face_top), (cx + 1, face_top),
                                   (cx + half + 1 - inset, base_y + 2),
                                   (cx + half + 1 - inset, 79)])
        roof_tiles(gr, outer - inner, ROOF, seed, shade=lambda x, y, c=cx: -1 if x > c else 0)
        composite(canvas, gr, OUT)
        gf2 = Layer(W, H)
        face = {(x, y) for (x, y) in inner if y <= 78}
        plaster(gf2, face, PLASTER, seed + 1)
        edge = {(x, y) for (x, y) in face if (x - 1, y) not in face or (x + 1, y) not in face
                or (x, y - 1) not in face}
        for (x, y) in edge:
            gf2.put(x, y, TIMBER[2])
        for (x, y) in edge:
            if (x, y + 1) in face and (x, y + 1) not in edge:
                gf2.put(x, y + 1, TIMBER[1])
        left = min(x for (x, y) in face if y == 78)
        right = max(x for (x, y) in face if y == 78)
        if cx == DOOR_CX:
            beam(gf2, cx, face_top + 2, cx, 56, TIMBER)
            beam(gf2, left + 1, 72, right - 1, 72, TIMBER)
            for px in (72, 96):
                beam(gf2, px, 56, px, 77, TIMBER)
            beam(gf2, 72, 60, 62, 71, TIMBER)
            beam(gf2, 97, 60, 107, 71, TIMBER)
            beam(gf2, 72, 56, 97, 56, TIMBER)
        else:
            beam(gf2, cx, face_top + 2, cx, 60, TIMBER)
            beam(gf2, left + 1, 74, right - 1, 74, TIMBER)
        shadow_under(gf2, face, PLASTER, TIMBER, PLASTER[1])
        composite(canvas, gf2, OUT)

    # windows
    win = Layer(W, H)
    lit_window(win, 32, 62, 9, 9, GLOW, TIMBER)
    lit_window(win, 133, 62, 9, 9, GLOW, TIMBER)
    lit_window(win, 79, 59, 11, 11, GLOW, TIMBER)
    lit_window(win, 34, 92, 11, 10, GLOW, TIMBER)
    lit_window(win, 127, 92, 11, 10, GLOW, TIMBER)
    composite(canvas, win, OUT)

    # door, its stone arch and the steps
    dr = Layer(W, H)
    stone_arch(dr, DOOR_CX, 89, 115, 7, STONE)
    arched_door(dr, DOOR_CX, 89, 115, 7, TIMBER, IRON)
    for x in range(DOOR_CX - 9, DOOR_CX + 10):
        dr.put(x, 116, STONE[4])
        dr.put(x, 117, STONE[2])
    for x in range(DOOR_CX - 11, DOOR_CX + 12):
        dr.put(x, 118, STONE[5] if x < DOOR_CX + 6 else STONE[4])
        dr.put(x, 119, STONE[2])
    composite(canvas, dr, OUT)

    # lanterns and the hanging mug sign
    lt = Layer(W, H)
    for lx in (65, 102):
        lt.put(lx + 2, 86, IRON[2])
        lt.put(lx + 2, 87, IRON[1])
        stamp(lt, lx, 88, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                                   "W": GLOW[4]})
    for x in range(2, 20):
        lt.put(x, 72, IRON[3] if x < 12 else IRON[2])
        lt.put(x, 73, IRON[1])
    for y in range(74, 79):
        lt.put(4, y, IRON[2])
        lt.put(13, y, IRON[2])
    for y in range(79, 94):
        for x in range(2, 16):
            c = TIMBER[2] if (y - 79) % 4 else TIMBER[1]
            if x in (2, 15) or y in (79, 93):
                c = TIMBER[1]
            lt.put(x, y, c)
    stamp(lt, 4, 81, MUG, {"g": GOLD[0], "Y": GOLD[2], "y": GOLD[3], "w": PLASTER[2],
                           "W": PLASTER[3]})
    composite(canvas, lt, OUT)

    # barrels, crate and bench
    pr = Layer(W, H)
    prop(pr, 8, 106, BARREL)
    prop(pr, 20, 108, CRATE)
    prop(pr, 99, 107, BARREL)
    prop(pr, 150, 106, BARREL)
    for x in range(118, 146):
        pr.put(x, 110, TIMBER[3])
        pr.put(x, 111, TIMBER[2])
        pr.put(x, 112, TIMBER[1])
    for lx in (120, 143):
        for y in range(113, 118):
            pr.put(lx, y, TIMBER[1])
            pr.put(lx + 1, y, TIMBER[0])
    composite(canvas, pr, OUT)
    return canvas


def main():
    save_sprite(build(), ASSETS / "tavern.bmp")


if __name__ == "__main__":
    main()
