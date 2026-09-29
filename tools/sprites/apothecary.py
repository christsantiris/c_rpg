#!/usr/bin/env python3
"""Town 3 apothecary sprite.

Draws 5 x 4 tiles (120 x 96) for the apothecary lot at TOWN_APOTHECARY_X/Y in
Town 3, with the door on TOWN_APOTHECARY_DOOR_X/Y. Run `make sprites` to
rewrite assets/apothecary.png and assets/apothecary.bmp.
"""
from pathlib import Path

from PIL import Image

from pixelkit import (CRATE, LANTERN, Layer, arched_door, beam, composite, flat_wall,
                      light_spill, lit_window, plaster, poly_mask, roof_tiles, save_sprite,
                      shadow_under, stamp, stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets"
W, H = 120, 96
CX = 60

OUT = (16, 12, 16)
PLUM = [(32, 16, 28), (50, 24, 42), (78, 38, 64), (102, 52, 84), (128, 70, 106), (156, 96, 130)]
PLASTER = [(136, 110, 78), (170, 144, 106), (200, 176, 134), (222, 202, 162)]
TIMBER = [(26, 15, 9), (44, 26, 15), (64, 39, 22), (86, 54, 31), (110, 71, 41)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
LAMPLIT = [(84, 44, 16), (112, 60, 22), (164, 94, 34)]
BRASS = [(96, 66, 26), (150, 110, 44), (200, 156, 70), (240, 206, 118)]
SIGN = [(20, 44, 34), (30, 64, 48), (46, 86, 64)]
LEAF = [(24, 54, 28), (40, 88, 42), (66, 124, 56), (98, 156, 76)]
CLAY = [(96, 46, 28), (132, 68, 40), (164, 92, 56)]
# potion colours: healing, mana, strength, intelligence
POTIONS = [
    [(110, 20, 28), (200, 40, 50), (250, 120, 120)],
    [(24, 44, 120), (50, 90, 210), (140, 180, 250)],
    [(130, 66, 16), (230, 130, 40), (255, 200, 120)],
    [(74, 36, 120), (150, 80, 210), (210, 170, 250)],
]

BOTTLE = [
    ".c.",
    "gGg",
    "gWg",
    "ggg",
]

FLASK = [
    ".c.",
    ".g.",
    "gGg",
    "GWG",
    "ggg",
]

MORTAR = [
    "......pp",
    ".....pP.",
    "bbbbPbbb",
    "bBBBBBBb",
    ".bBBBBb.",
    "..bbbb..",
    ".bbbbbb.",
]

PLANTER = [
    "..L.lL.L.",
    ".lLlLlLl.",
    "lLlLlLlLl",
    "cCCCCCCCc",
    ".ccccccc.",
]


def display_window(L, x0, x1, y0, y1, seed):
    """Shop window with shelves of coloured potions in warm lamplight."""
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            c = LAMPLIT[1] if (x + y) % 6 else LAMPLIT[0]
            if y < y0 + 3:
                c = LAMPLIT[2]
            L.put(x, y, c)
    for x in range(x0 - 1, x1 + 2):
        L.put(x, y0 - 1, TIMBER[0])
        L.put(x, y1 + 1, TIMBER[3])
        L.put(x, y1 + 2, TIMBER[1])
    for y in range(y0 - 1, y1 + 2):
        L.put(x0 - 1, y, TIMBER[0])
        L.put(x1 + 1, y, TIMBER[0])
    shelves = (y0 + 7, y1 - 1)
    for sy in shelves:
        for x in range(x0, x1 + 1):
            L.put(x, sy, TIMBER[2])
    i = seed
    for sy in shelves:
        x = x0 + 1
        while x + 2 <= x1:
            pal = POTIONS[i % 4]
            shape = FLASK if i % 3 == 0 else BOTTLE
            stamp(L, x, sy - len(shape), shape, {"c": TIMBER[3], "g": pal[0], "G": pal[1],
                                                 "W": pal[2]})
            x += 4
            i += 1


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # chimney behind the roof
    ch = Layer(W, H)
    flat_wall(ch, 82, 90, 6, 30, STONE, row_h=4, base=3, seed=4, bw=(4, 6),
              shade=lambda x, y: -1 if x > 87 else 0)
    for x in range(81, 92):
        ch.put(x, 3, STONE[4])
        ch.put(x, 4, STONE[3])
        ch.put(x, 5, STONE[1])
    for x in range(84, 89):
        ch.put(x, 3, OUT)
    composite(canvas, ch, OUT)

    # steep plum roof and the plastered gable
    rf = Layer(W, H)
    outer = poly_mask((W, H), [(CX - 46, 50), (CX, 4), (CX + 1, 4), (CX + 47, 50)])
    inner = poly_mask((W, H), [(CX - 36, 60), (CX - 36, 50), (CX, 16), (CX + 1, 16),
                               (CX + 37, 50), (CX + 37, 60)])
    roof_tiles(rf, outer - inner, PLUM, 9, shade=lambda x, y: -1 if x > CX else 0,
               tile_w=5, row_h=3, rounded=False)
    composite(canvas, rf, OUT)
    gf = Layer(W, H)
    face = {(x, y) for (x, y) in inner if y <= 58}
    plaster(gf, face, PLASTER, 13)
    edge = {(x, y) for (x, y) in face if (x - 1, y) not in face or (x + 1, y) not in face
            or (x, y - 1) not in face}
    for (x, y) in edge:
        gf.put(x, y, TIMBER[2])
    for (x, y) in edge:
        if (x, y + 1) in face and (x, y + 1) not in edge:
            gf.put(x, y + 1, TIMBER[1])
    beam(gf, CX, 1, CX, 26, TIMBER)
    beam(gf, CX - 30, 50, CX + 31, 50, TIMBER)
    for px in (CX - 20, CX - 6, CX + 7, CX + 21):
        beam(gf, px, 32, px, 58, TIMBER)
    beam(gf, CX - 21, 38, CX - 30, 49, TIMBER)
    beam(gf, CX + 22, 38, CX + 31, 49, TIMBER)
    shadow_under(gf, face, PLASTER, TIMBER, PLASTER[1])
    lit_window(gf, CX - 3, 20, 7, 6, GLOW, TIMBER, shutters=False)
    lit_window(gf, CX - 17, 37, 9, 9, GLOW, TIMBER, shutters=False)
    lit_window(gf, CX + 9, 37, 9, 9, GLOW, TIMBER, shutters=False)
    for wx in (CX - 19, CX + 7):
        for x in range(wx, wx + 13):
            gf.put(x, 48, LEAF[2] if x % 2 else POTIONS[0][1])
    composite(canvas, gf, OUT)

    # jetty beam and stone shop front with two display windows
    gr = Layer(W, H)
    for x in range(20, 102):
        gr.put(x, 59, TIMBER[3])
        gr.put(x, 60, TIMBER[2])
        gr.put(x, 61, TIMBER[1])
    flat_wall(gr, 22, 99, 62, 91, STONE, row_h=5, base=3, seed=17, bw=(6, 10),
              shade=lambda x, y: -1 if y < 65 else 0)
    for x0 in (21, 97):
        for y in range(62, 92):
            gr.put(x0, y, TIMBER[3])
            gr.put(x0 + 1, y, TIMBER[2])
            gr.put(x0 + 2, y, TIMBER[1])
    for x in range(22, 100):
        gr.put(x, 91, STONE[1])
    display_window(gr, 26, 49, 66, 84, 0)
    display_window(gr, 71, 94, 66, 84, 2)
    composite(canvas, gr, OUT)

    # door, arch and steps
    dr = Layer(W, H)
    stone_arch(dr, CX, 71, 91, 6, STONE)
    arched_door(dr, CX, 71, 91, 6, TIMBER, IRON)
    for x in range(CX - 8, CX + 9):
        dr.put(x, 92, STONE[4])
        dr.put(x, 93, STONE[2])
    for x in range(CX - 10, CX + 11):
        dr.put(x, 94, STONE[5] if x < CX + 5 else STONE[4])
        dr.put(x, 95, STONE[2])
    composite(canvas, dr, OUT)

    # mortar-and-pestle sign, lantern, planters and a crate of bottles
    pr = Layer(W, H)
    for x in range(98, 119):
        pr.put(x, 52, BRASS[2] if x < 108 else BRASS[1])
        pr.put(x, 53, IRON[1])
    for y in range(54, 57):
        pr.put(104, y, IRON[2])
        pr.put(114, y, IRON[2])
    for y in range(57, 69):
        for x in range(101, 118):
            c = SIGN[1]
            if x in (101, 117) or y in (57, 68):
                c = BRASS[1]
            elif y == 58:
                c = SIGN[2]
            pr.put(x, y, c)
    stamp(pr, 105, 59, MORTAR, {"b": BRASS[1], "B": BRASS[3], "p": TIMBER[3], "P": TIMBER[4]})
    pr.put(19, 55, IRON[2])
    stamp(pr, 17, 56, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                               "W": GLOW[4]})
    plant = {"l": LEAF[1], "L": LEAF[2], "c": CLAY[0], "C": CLAY[1]}
    stamp(pr, 10, 90, PLANTER, plant)
    stamp(pr, 100, 90, PLANTER, plant)
    stamp(pr, 6, 80, CRATE, {"t": TIMBER[1], "T": TIMBER[2], "W": TIMBER[3]})
    for i, bx in enumerate((7, 11)):
        pal = POTIONS[i]
        stamp(pr, bx, 76, BOTTLE, {"c": TIMBER[3], "g": pal[0], "G": pal[1], "W": pal[2]})
    composite(canvas, pr, OUT)
    light_spill(canvas, ((38, 76, 14), (82, 76, 14)), set(STONE) | set(TIMBER), GLOW[2])
    return canvas


def main():
    save_sprite(build(), ASSETS / "apothecary.png", ASSETS / "apothecary.bmp")


if __name__ == "__main__":
    main()
