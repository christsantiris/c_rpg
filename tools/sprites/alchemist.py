#!/usr/bin/env python3
"""Town 1 alchemist sprite.

Draws 5 x 4 tiles (120 x 96) for the alchemist lot at TOWN_ALCHEMIST_X/Y in
Town 1, with the door on tile (31, 10). Run `make sprites` to rewrite
assets/images/alchemist.bmp.
"""
from pathlib import Path

from PIL import Image

from pixelkit import (CRATE, LANTERN, Layer, arched_door, beam, composite, flat_wall,
                      light_spill, lit_window, plaster, poly_mask, roof_tiles, save_sprite,
                      shadow_under, stamp, stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 120, 96
CX = 59
DOOR_CX = 60

OUT = (14, 14, 22)
TEAL = [(12, 34, 40), (18, 50, 58), (28, 86, 94), (40, 110, 118), (60, 138, 144), (92, 168, 168)]
MAUVE = [(70, 52, 82), (96, 72, 108), (118, 92, 130), (142, 116, 152)]
TIMBER = [(26, 15, 9), (44, 26, 15), (64, 39, 22), (86, 54, 31), (110, 71, 41)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
BREW = [(30, 84, 26), (60, 150, 40), (116, 214, 72), (190, 250, 140), (236, 255, 210)]
MAGIC = [(70, 28, 100), (116, 48, 168), (166, 90, 220), (208, 150, 244), (240, 212, 255)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
LEAF = [(24, 54, 28), (40, 88, 42), (66, 124, 56), (98, 156, 76)]
VIOLET = [(76, 40, 104), (122, 70, 164), (168, 116, 206)]
STRAW = [(110, 84, 40), (160, 126, 64), (200, 168, 96)]
SHADE = [(16, 14, 20), (26, 22, 30), (38, 32, 42)]
GLASS = [(70, 110, 96), (120, 170, 150), (196, 226, 210)]

FLASK = [
    "..c..",
    "..g..",
    ".gGg.",
    "gGWGg",
    "gGGGg",
    ".ggg.",
]

BOTTLE = [
    ".c.",
    "gGg",
    "gWg",
    "ggg",
]

KEG = [
    ".ttttt.",
    "tTWTTTt",
    "iIIIIIi",
    "tTWTTTt",
    "tTWTTTt",
    "iIIIIIi",
    "tTWTTTt",
    ".ttttt.",
]

BUNDLE = [
    ".t.",
    "aAa",
    "aAa",
    ".a.",
]


def display_window(L):
    """Shop window full of glowing green potions under a teal awning."""
    for y in range(69, 84):
        for x in range(18, 44):
            c = BREW[1] if (x + y) % 5 else BREW[0]
            if y > 78:
                c = BREW[0]
            L.put(x, y, c)
    for x in range(17, 45):
        L.put(x, 68, TIMBER[1])
        L.put(x, 84, TIMBER[3])
        L.put(x, 85, TIMBER[2])
        L.put(x, 86, TIMBER[1])
    for y in range(68, 87):
        for x in (17, 30, 44):
            L.put(x, y, TIMBER[1] if x != 17 else TIMBER[3])
    potion = {"c": TIMBER[3], "g": BREW[1], "G": BREW[3], "W": BREW[4]}
    stamp(L, 20, 76, FLASK, potion)
    stamp(L, 33, 77, FLASK, potion)
    stamp(L, 26, 79, BOTTLE, {"c": TIMBER[3], "g": BREW[1], "G": BREW[2], "W": BREW[4]})
    stamp(L, 39, 78, BOTTLE, {"c": TIMBER[3], "g": MAGIC[1], "G": MAGIC[2], "W": MAGIC[4]})
    awning = poly_mask((W, H), [(14, 63), (47, 63), (45, 68), (16, 68)])
    roof_tiles(L, awning, TEAL, 17, tile_w=4, row_h=3, rounded=False)


def herb_stall(L):
    """Open stall with hanging herb bundles and a shelf of bottles."""
    for y in range(64, 91):
        for x in range(80, 111):
            L.put(x, y, SHADE[1] if (x + y) % 7 else SHADE[2])
    for x0 in (79, 109):
        for y in range(64, 91):
            L.put(x0, y, TIMBER[3])
            L.put(x0 + 1, y, TIMBER[1])
    for x in range(80, 110):
        L.put(x, 65, TIMBER[2])
    for (hx, pal) in ((85, {"t": TIMBER[3], "a": LEAF[1], "A": LEAF[3]}),
                      (92, {"t": TIMBER[3], "a": VIOLET[0], "A": VIOLET[2]}),
                      (99, {"t": TIMBER[3], "a": STRAW[0], "A": STRAW[2]}),
                      (105, {"t": TIMBER[3], "a": LEAF[0], "A": LEAF[2]})):
        stamp(L, hx - 1, 66, BUNDLE, pal)
        L.put(hx, 70, pal["a"])
    for x in range(81, 108):
        L.put(x, 80, TIMBER[4])
        L.put(x, 81, TIMBER[1])
    for i, bx in enumerate((83, 88, 93)):
        pal = (MAGIC, BREW, GLASS)[i]
        stamp(L, bx, 76, BOTTLE, {"c": TIMBER[3], "g": pal[0], "G": pal[1], "W": pal[2]})


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # chimney behind the roof
    ch = Layer(W, H)
    flat_wall(ch, 27, 36, 14, 38, STONE, row_h=4, base=3, seed=4, bw=(4, 6),
              shade=lambda x, y: -1 if x > 33 else 0)
    for x in range(26, 38):
        ch.put(x, 11, STONE[4])
        ch.put(x, 12, STONE[3])
        ch.put(x, 13, STONE[1])
    for x in range(29, 35):
        ch.put(x, 11, OUT)
    composite(canvas, ch, OUT)

    # broad roof, its long right slope sheltering the herb stall
    rf = Layer(W, H)
    outer = poly_mask((W, H), [(10, 56), (CX, 10), (CX + 1, 10), (116, 62)])
    inner = poly_mask((W, H), [(18, 63), (18, 56), (CX, 25), (CX + 1, 25), (104, 60),
                               (104, 63)])
    roof_tiles(rf, outer - inner, TEAL, 9, shade=lambda x, y: -1 if x > CX + 20 else 0,
               tile_w=5, row_h=3, rounded=False)
    composite(canvas, rf, OUT)

    # mauve upper floor with timber framing
    gf = Layer(W, H)
    face = {(x, y) for (x, y) in inner if y <= 61}
    plaster(gf, face, MAUVE, 13)
    edge = {(x, y) for (x, y) in face if (x - 1, y) not in face or (x + 1, y) not in face
            or (x, y - 1) not in face}
    for (x, y) in edge:
        gf.put(x, y, TIMBER[2])
    for (x, y) in edge:
        if (x, y + 1) in face and (x, y + 1) not in edge:
            gf.put(x, y + 1, TIMBER[1])
    beam(gf, CX, 7, CX, 40, TIMBER)
    beam(gf, 22, 58, 100, 58, TIMBER)
    for px in (CX - 14, CX + 15, 32, 88):
        beam(gf, px, 44 if px in (CX - 14, CX + 15) else 50, px, 61, TIMBER)
    beam(gf, CX - 15, 48, CX - 24, 57, TIMBER)
    beam(gf, CX + 16, 48, CX + 25, 57, TIMBER)
    shadow_under(gf, face, MAUVE, TIMBER, MAUVE[1])
    lit_window(gf, CX - 5, 43, 11, 10, MAGIC, TIMBER)
    stamp(gf, CX - 2, 45, FLASK, {"c": TIMBER[3], "g": MAGIC[1], "G": MAGIC[3], "W": MAGIC[4]})
    composite(canvas, gf, OUT)

    # jetty beam, then the ground floor: display window, stone core, herb stall
    gr = Layer(W, H)
    for x in range(14, 113):
        gr.put(x, 62, TIMBER[3])
        gr.put(x, 63, TIMBER[2])
        gr.put(x, 64, TIMBER[1])
    flat_wall(gr, 45, 78, 65, 90, STONE, row_h=5, base=3, seed=17, bw=(6, 10),
              shade=lambda x, y: -1 if y < 68 else 0)
    for x in range(45, 79):
        gr.put(x, 90, STONE[1])
    composite(canvas, gr, OUT)
    dw = Layer(W, H)
    display_window(dw)
    composite(canvas, dw, OUT)
    hs = Layer(W, H)
    herb_stall(hs)
    composite(canvas, hs, OUT)

    # door, arch and steps
    dr = Layer(W, H)
    stone_arch(dr, DOOR_CX, 72, 90, 6, STONE)
    arched_door(dr, DOOR_CX, 72, 90, 6, TIMBER, IRON)
    for x in range(DOOR_CX - 8, DOOR_CX + 9):
        dr.put(x, 91, STONE[4])
        dr.put(x, 92, STONE[2])
    for x in range(DOOR_CX - 10, DOOR_CX + 11):
        dr.put(x, 93, STONE[5] if x < DOOR_CX + 5 else STONE[4])
        dr.put(x, 94, STONE[2])
    composite(canvas, dr, OUT)

    # hanging potion sign, lantern, barrels and a crate
    pr = Layer(W, H)
    for x in range(1, 16):
        pr.put(x, 53, TIMBER[3])
        pr.put(x, 54, TIMBER[1])
    pr.put(5, 55, IRON[2])
    pr.put(5, 56, IRON[2])
    stamp(pr, 3, 57, FLASK, {"c": TIMBER[3], "g": BREW[1], "G": BREW[3], "W": BREW[4]})
    pr.put(115, 58, IRON[2])
    stamp(pr, 113, 59, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                                "W": GLOW[4]})
    wood = {"t": TIMBER[1], "T": TIMBER[2], "W": TIMBER[3], "i": IRON[1], "I": IRON[2]}
    stamp(pr, 44, 86, KEG, wood)
    stamp(pr, 70, 86, KEG, wood)
    stamp(pr, 97, 84, CRATE, wood)
    composite(canvas, pr, OUT)
    light_spill(canvas, ((31, 77, 12),), set(STONE) | set(TIMBER) | set(SHADE), BREW[2])
    return canvas


def main():
    save_sprite(build(), ASSETS / "alchemist.bmp")


if __name__ == "__main__":
    main()
