#!/usr/bin/env python3
"""Town 2 witch's hut sprite.

Draws 5 x 4 tiles (120 x 96) for the witch's lot at TOWN_WITCH_X/Y in Town 2,
with the door on TOWN_WITCH_DOOR_X/Y. Run `make sprites` to rewrite
assets/witch-hut.png and assets/witch-hut.bmp.
"""
import random
from pathlib import Path

from PIL import Image

from pixelkit import (LANTERN, Layer, arched_door, beam, composite, flat_wall, light_spill,
                      lit_window, plaster, poly_mask, roof_tiles, save_sprite, shadow_under,
                      stamp, stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets"
W, H = 120, 96
CX = 54
DOOR_CX = 57

OUT = (18, 12, 20)
PURPLE = [(32, 14, 38), (48, 22, 56), (78, 38, 90), (102, 52, 116), (128, 72, 142), (156, 98, 168)]
MOSS = [(38, 66, 30), (62, 100, 40), (94, 136, 56)]
PLASTER = [(136, 110, 78), (170, 144, 106), (200, 176, 134), (222, 202, 162)]
TIMBER = [(26, 15, 9), (44, 26, 15), (64, 39, 22), (86, 54, 31), (110, 71, 41)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
MAGIC = [(70, 28, 100), (116, 48, 168), (166, 90, 220), (208, 150, 244), (240, 212, 255)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
GOLD = [(140, 90, 20), (204, 150, 40), (244, 196, 70), (255, 232, 144)]
LEAF = [(24, 54, 28), (40, 88, 42), (66, 124, 56), (98, 156, 76)]
SHROOM = [(120, 24, 28), (186, 44, 44), (228, 96, 84), (236, 226, 214)]
VIOLET = [(76, 40, 104), (122, 70, 164), (168, 116, 206)]
SHADE = [(20, 12, 18), (32, 20, 28), (46, 30, 40)]
GLASS = [(70, 110, 96), (120, 170, 150), (196, 226, 210)]

MOON = [
    "..yy.",
    ".yY..",
    "yY...",
    "yY...",
    ".yY..",
    "..yy.",
]

CAULDRON = [
    "...bMbMb...",
    ".iMMWMMMMi.",
    "iIMMMMMMMIi",
    "iIIIIIIIIIi",
    "iIIIIIIIIIi",
    ".iIIIIIIIi.",
    "..iIIIIIi..",
    ".ii.....ii.",
]

RED_SHROOM = [
    ".rRRr.",
    "rRwRRr",
    "RRRRwR",
    "..ss..",
    "..ss..",
]

VIOLET_SHROOM = [
    ".vVv.",
    "vVwVv",
    "..s..",
    "..s..",
]

BOTTLE = [
    ".c.",
    "gGg",
    "gWg",
    "ggg",
]


def moss(L, mask, seed, density=0.12):
    """Scatter clumps of moss over the roof pixels in mask."""
    rnd = random.Random(seed)
    pts = sorted(mask)
    for _ in range(int(len(pts) * density / 5)):
        x, y = rnd.choice(pts)
        for dx, dy in ((0, 0), (1, 0), (-1, 1), (0, 1), (1, 1), (2, 1), (0, 2)):
            if (x + dx, y + dy) in mask and rnd.random() < 0.85:
                L.put(x + dx, y + dy, MOSS[1] if dy else MOSS[2])


def vines(L, x, y0, length, seed):
    """A wandering strand of ivy hanging down from (x, y0)."""
    rnd = random.Random(seed)
    for k in range(length):
        L.put(x, y0 + k, LEAF[1] if k % 3 else LEAF[2])
        if rnd.random() < 0.4:
            L.put(x + rnd.choice((-1, 1)), y0 + k, LEAF[0])
        if rnd.random() < 0.3:
            x += rnd.choice((-1, 1))


def potion_stall(L):
    """Lean-to interior: hanging herbs and crystals, a cauldron and a potion table."""
    for y in range(58, 92):
        for x in range(80, 114):
            if y < 58 + (x - 80) * 0.26:
                continue
            L.put(x, y, SHADE[1] if (x + y) % 7 else SHADE[2])
    for x0 in (79, 112):
        for y in range(60, 92):
            L.put(x0, y, TIMBER[3])
            L.put(x0 + 1, y, TIMBER[2])
            L.put(x0 + 2, y, TIMBER[1])
    for (hx, pal, h) in ((88, LEAF, 5), (94, VIOLET, 6), (100, LEAF, 4)):
        L.put(hx, 65, TIMBER[3])
        for y in range(66, 66 + h):
            L.put(hx, y, pal[1 + (y % 2)])
            if y > 67:
                L.put(hx - 1, y, pal[0])
                L.put(hx + 1, y, pal[0])
    for x in range(96, 111):
        L.put(x, 80, TIMBER[4])
        L.put(x, 81, TIMBER[2])
        L.put(x, 82, TIMBER[1])
    for lx in (97, 108):
        for y in range(83, 91):
            L.put(lx, y, TIMBER[2])
            L.put(lx + 1, y, TIMBER[1])
    for i, bx in enumerate((98, 102, 106)):
        pal = (GLASS, VIOLET, MAGIC)[i]
        stamp(L, bx, 76, BOTTLE, {"c": TIMBER[3], "g": pal[0], "G": pal[1], "W": pal[2]})


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # chimney behind the roof
    ch = Layer(W, H)
    flat_wall(ch, 22, 31, 21, 42, STONE, row_h=4, base=3, seed=6, bw=(4, 6),
              shade=lambda x, y: -1 if x > 28 else 0)
    for x in range(21, 33):
        ch.put(x, 18, STONE[4])
        ch.put(x, 19, STONE[3])
        ch.put(x, 20, STONE[1])
    for (x, y) in ((22, 22), (23, 22), (23, 23), (30, 27), (30, 28), (29, 28), (24, 33)):
        ch.put(x, y, MOSS[1])
    composite(canvas, ch, OUT)

    # lean-to over the potion stall
    lr = Layer(W, H)
    lean = poly_mask((W, H), [(74, 44), (74, 52), (119, 66), (119, 58)])
    roof_tiles(lr, lean, PURPLE, 4, tile_w=5, row_h=3, rounded=False)
    moss(lr, lean, 14)
    composite(canvas, lr, OUT)
    st = Layer(W, H)
    potion_stall(st)
    composite(canvas, st, OUT)

    # left wing under the long roof slope
    lw = Layer(W, H)
    wing = {(x, y) for x in range(15, 34) for y in range(58, 89)}
    plaster(lw, wing, PLASTER, 21)
    for x0 in (15, 32):
        for y in range(58, 89):
            lw.put(x0, y, TIMBER[3])
            lw.put(x0 + 1, y, TIMBER[1])
    beam(lw, 15, 58, 33, 58, TIMBER)
    beam(lw, 16, 72, 32, 72, TIMBER)
    beam(lw, 17, 60, 31, 71, TIMBER)
    for x in range(18, 31):
        lw.put(x, 81, TIMBER[4])
        lw.put(x, 82, TIMBER[1])
    for i, bx in enumerate((19, 23, 27)):
        pal = (MAGIC, GLASS, VIOLET)[i]
        stamp(lw, bx, 77, BOTTLE, {"c": TIMBER[3], "g": pal[0], "G": pal[1], "W": pal[2]})
    shadow_under(lw, wing, PLASTER, TIMBER, PLASTER[1])
    composite(canvas, lw, OUT)

    # main roof with moss, and the gable face
    rf = Layer(W, H)
    outer = poly_mask((W, H), [(CX - 42, 60), (CX, 14), (CX + 1, 14), (CX + 43, 60)])
    inner = poly_mask((W, H), [(CX - 22, 63), (CX - 22, 60), (CX, 28), (CX + 1, 28),
                               (CX + 23, 60), (CX + 23, 63)])
    band = outer - inner
    roof_tiles(rf, band, PURPLE, 9, shade=lambda x, y: -1 if x > CX else 0,
               tile_w=5, row_h=3, rounded=False)
    moss(rf, band, 23)
    composite(canvas, rf, OUT)
    gf = Layer(W, H)
    face = {(x, y) for (x, y) in inner if y <= 62}
    plaster(gf, face, PLASTER, 13)
    edge = {(x, y) for (x, y) in face if (x - 1, y) not in face or (x + 1, y) not in face
            or (x, y - 1) not in face}
    for (x, y) in edge:
        gf.put(x, y, TIMBER[2])
    for (x, y) in edge:
        if (x, y + 1) in face and (x, y + 1) not in edge:
            gf.put(x, y + 1, TIMBER[1])
    beam(gf, CX, 10, CX, 38, TIMBER)
    beam(gf, CX - 16, 58, CX + 17, 58, TIMBER)
    for px in (CX - 11, CX + 12):
        beam(gf, px, 44, px, 62, TIMBER)
    beam(gf, CX - 12, 50, CX - 19, 57, TIMBER)
    beam(gf, CX + 13, 50, CX + 20, 57, TIMBER)
    shadow_under(gf, face, PLASTER, TIMBER, PLASTER[1])
    lit_window(gf, CX - 4, 43, 9, 8, MAGIC, TIMBER)
    for x in range(CX - 6, CX + 7):
        gf.put(x, 54, TIMBER[3] if x < CX + 6 else TIMBER[1])
        gf.put(x, 55, TIMBER[1])
        gf.put(x, 53, VIOLET[2] if x % 2 else LEAF[2])
    vines(gf, CX - 5, 56, 7, 3)
    vines(gf, CX + 5, 56, 5, 8)
    composite(canvas, gf, OUT)

    # jetty beam and stone ground floor
    gr = Layer(W, H)
    for x in range(30, 81):
        gr.put(x, 63, TIMBER[3])
        gr.put(x, 64, TIMBER[2])
        gr.put(x, 65, TIMBER[1])
    flat_wall(gr, 35, 77, 66, 88, STONE, row_h=5, base=3, seed=19, bw=(6, 10),
              shade=lambda x, y: -1 if y < 69 else 0)
    for x0 in (34, 76):
        for y in range(66, 89):
            gr.put(x0, y, TIMBER[3])
            gr.put(x0 + 1, y, TIMBER[2])
            gr.put(x0 + 2, y, TIMBER[1])
    for x in range(35, 78):
        gr.put(x, 88, STONE[1])
    composite(canvas, gr, OUT)

    # door, arch, steps and lanterns
    dr = Layer(W, H)
    stone_arch(dr, DOOR_CX, 71, 88, 6, STONE)
    arched_door(dr, DOOR_CX, 71, 88, 6, TIMBER, IRON)
    for x in range(DOOR_CX - 8, DOOR_CX + 9):
        dr.put(x, 89, STONE[4])
        dr.put(x, 90, STONE[2])
    for x in range(DOOR_CX - 10, DOOR_CX + 11):
        dr.put(x, 91, STONE[5] if x < DOOR_CX + 5 else STONE[4])
        dr.put(x, 92, STONE[2])
    for lx in (DOOR_CX - 15, DOOR_CX + 11):
        dr.put(lx + 2, 67, IRON[2])
        stamp(dr, lx, 68, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                                   "W": GLOW[4]})
    composite(canvas, dr, OUT)

    # hanging moons, sign, cauldron, mushrooms
    pr = Layer(W, H)
    gold = {"y": GOLD[1], "Y": GOLD[3]}
    stamp(pr, CX - 20, 46, MOON, gold)
    pr.put(CX - 18, 45, IRON[2])
    for x in range(2, 16):
        pr.put(x, 56, TIMBER[3])
        pr.put(x, 57, TIMBER[1])
    for y in range(58, 61):
        pr.put(6, y, IRON[2])
    stamp(pr, 3, 61, MOON, gold)
    pr.put(9, 64, MAGIC[3])
    pr.put(9, 66, MAGIC[2])
    vines(pr, 17, 60, 12, 5)
    vines(pr, 33, 66, 9, 11)
    stamp(pr, 82, 78, CAULDRON, {"i": IRON[0], "I": IRON[1], "M": MAGIC[2], "W": MAGIC[4],
                                 "b": MAGIC[3]})
    for (bx, by) in ((85, 75), (89, 73), (87, 71)):
        pr.put(bx, by, MAGIC[3])
    for x in range(84, 92):
        if x % 3:
            pr.put(x, 86, (220, 110, 30))
            pr.put(x, 87, (140, 50, 20))
    shroom = {"r": SHROOM[1], "R": SHROOM[2], "w": SHROOM[3], "s": SHROOM[3]}
    stamp(pr, 2, 85, RED_SHROOM, shroom)
    stamp(pr, 8, 88, RED_SHROOM, shroom)
    stamp(pr, 14, 88, VIOLET_SHROOM, {"v": VIOLET[1], "V": VIOLET[2], "w": SHROOM[3],
                                      "s": SHROOM[3]})
    stamp(pr, 110, 88, VIOLET_SHROOM, {"v": VIOLET[1], "V": VIOLET[2], "w": SHROOM[3],
                                       "s": SHROOM[3]})
    pr.put(110, 60, IRON[2])
    stamp(pr, 108, 61, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                                "W": GLOW[4]})
    composite(canvas, pr, OUT)
    light_spill(canvas, ((88, 80, 12),), set(SHADE) | set(TIMBER), MAGIC[2])
    return canvas


def main():
    save_sprite(build(), ASSETS / "witch-hut.png", ASSETS / "witch-hut.bmp")


if __name__ == "__main__":
    main()
