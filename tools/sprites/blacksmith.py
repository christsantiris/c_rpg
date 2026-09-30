#!/usr/bin/env python3
"""Town 1 blacksmith sprite.

Draws 5 x 4 tiles (120 x 96) for the blacksmith lot at TOWN_BLACKSMITH_X/Y in
Town 1, with the door on tile (13, 10). Run `make sprites` to rewrite
assets/images/blacksmith.bmp.
"""
import math
from pathlib import Path

from PIL import Image

from pixelkit import (BARREL, CRATE, LANTERN, Layer, arched_door, beam, composite, flat_wall,
                      light_spill, lit_window, plaster, poly_mask, roof_tiles, save_sprite,
                      shadow_under, stamp, stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 120, 96
CX = 70
DOOR_CX = 63

OUT = (20, 14, 12)
SLATE = [(22, 24, 30), (32, 34, 42), (56, 58, 68), (72, 74, 86), (90, 92, 104), (112, 114, 126)]
PLASTER = [(136, 110, 78), (170, 144, 106), (200, 176, 134), (222, 202, 162)]
TIMBER = [(26, 15, 9), (44, 26, 15), (64, 39, 22), (86, 54, 31), (110, 71, 41)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
FIRE = [(110, 26, 10), (190, 62, 14), (240, 122, 28), (255, 190, 64), (255, 238, 164)]
SOOT = [(18, 12, 10), (30, 20, 16), (46, 30, 22)]
LOG = [(70, 44, 24), (112, 76, 44), (156, 116, 72), (190, 150, 100)]

ANVIL = [
    ".IIIIIIIIIIWW.",
    "iIIIIIIIIIIIIW",
    ".iiIIIIIIIii..",
    "...iIIIIIi....",
    "...iIIIIIi....",
    "..iiIIIIIii...",
    ".iIIIIIIIIIi..",
]

STUMP = [
    ".tTTTTTTTTt.",
    "tTWWTTTTTTTt",
    "tTTTTTTTTTTt",
    "tTTWTTTTTTTt",
    ".tTTTTTTTTt.",
]

LOG_END = [
    ".oo.",
    "oRWo",
    "oRRo",
    ".oo.",
]

SIGN_ANVIL = [
    "IIIIIIWW",
    ".IIIII..",
    "..III...",
    ".IIIII..",
]


def forge(L):
    """Stone hearth under the lean-to, with a glowing fire and a hood."""
    flat_wall(L, 8, 29, 70, 91, STONE, row_h=4, base=3, seed=12, bw=(4, 7))
    for y in range(75, 88):
        for x in range(11, 27):
            d = math.hypot((x - 18.5) / 7.5, (y - 84) / 9.0)
            if y < 79 and abs(x - 18.5) > 4 + (y - 75):
                continue
            c = SOOT[1]
            if d < 0.95:
                c = FIRE[1]
            if d < 0.7:
                c = FIRE[2]
            if d < 0.45:
                c = FIRE[3]
            if d < 0.22:
                c = FIRE[4]
            if y > 85:
                c = FIRE[2] if (x + y) % 2 else FIRE[1]
            L.put(x, y, c)
    for x in range(7, 31):
        L.put(x, 69, STONE[4])
        L.put(x, 70, STONE[2])


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    # chimney behind the roof
    ch = Layer(W, H)
    flat_wall(ch, 86, 95, 14, 46, STONE, row_h=4, base=3, seed=3, bw=(4, 6),
              shade=lambda x, y: -1 if x > 92 else 0)
    for x in range(85, 97):
        ch.put(x, 11, STONE[4])
        ch.put(x, 12, STONE[3])
        ch.put(x, 13, STONE[1])
    for x in range(88, 94):
        ch.put(x, 11, OUT)
    composite(canvas, ch, OUT)

    # lean-to over the forge
    lr = Layer(W, H)
    roof_tiles(lr, poly_mask((W, H), [(44, 44), (44, 53), (0, 70), (0, 62)]), SLATE, 5,
               tile_w=5, row_h=3, rounded=False)
    composite(canvas, lr, OUT)
    sh = Layer(W, H)
    for y in range(64, 93):
        for x in range(4, 42):
            if y < 64 + (42 - x) * 0.14:
                continue
            c = SOOT[1] if (x + y) % 7 else SOOT[2]
            if 14 <= x <= 24 and y < 72:
                c = SOOT[2]
            sh.put(x, y, c)
    for x0 in (3, 38):
        for y in range(64, 93):
            sh.put(x0, y, TIMBER[3])
            sh.put(x0 + 1, y, TIMBER[2])
            sh.put(x0 + 2, y, TIMBER[1])
    beam(sh, 3, 64, 41, 57, TIMBER)
    forge(sh)
    # tools on the back wall: tongs and hammers
    for (tx, h) in ((31, 9), (34, 7)):
        for y in range(72, 72 + h):
            sh.put(tx, y, IRON[2])
        sh.put(tx - 1, 72 + h, IRON[3])
        sh.put(tx + 1, 72 + h, IRON[3])
    for y in range(71, 79):
        sh.put(36, y, TIMBER[3])
    sh.put(35, 71, IRON[3])
    sh.put(37, 71, IRON[3])
    sh.put(36, 70, IRON[3])
    composite(canvas, sh, OUT)

    # main roof and gable face
    rf = Layer(W, H)
    outer = poly_mask((W, H), [(CX - 40, 64), (CX, 16), (CX + 1, 16), (CX + 41, 64)])
    inner = poly_mask((W, H), [(CX - 28, 67), (CX - 28, 64), (CX, 29), (CX + 1, 29),
                               (CX + 29, 64), (CX + 29, 67)])
    roof_tiles(rf, outer - inner, SLATE, 9, shade=lambda x, y: -1 if x > CX else 0,
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
    for x in range(41, 100):
        gr.put(x, 67, TIMBER[3])
        gr.put(x, 68, TIMBER[2])
        gr.put(x, 69, TIMBER[1])
    flat_wall(gr, 43, 97, 70, 91, STONE, row_h=5, base=3, seed=17, bw=(6, 10),
              shade=lambda x, y: -1 if y < 73 else 0)
    for x0 in (42, 95):
        for y in range(70, 92):
            gr.put(x0, y, TIMBER[3])
            gr.put(x0 + 1, y, TIMBER[2])
            gr.put(x0 + 2, y, TIMBER[1])
    for x in range(43, 98):
        gr.put(x, 91, STONE[1])
    composite(canvas, gr, OUT)

    # door, arch and steps
    dr = Layer(W, H)
    stone_arch(dr, DOOR_CX, 75, 91, 6, STONE)
    arched_door(dr, DOOR_CX, 75, 91, 6, TIMBER, IRON)
    for x in range(DOOR_CX - 8, DOOR_CX + 9):
        dr.put(x, 92, STONE[4])
        dr.put(x, 93, STONE[2])
    for x in range(DOOR_CX - 10, DOOR_CX + 11):
        dr.put(x, 94, STONE[5] if x < DOOR_CX + 5 else STONE[4])
        dr.put(x, 95, STONE[2])
    composite(canvas, dr, OUT)

    # sign, lantern and the yard props
    pr = Layer(W, H)
    for x in range(98, 119):
        pr.put(x, 56, IRON[3] if x < 108 else IRON[2])
        pr.put(x, 57, IRON[1])
    for y in range(58, 61):
        pr.put(104, y, IRON[2])
        pr.put(115, y, IRON[2])
    for y in range(61, 71):
        for x in range(102, 118):
            c = TIMBER[2] if (y - 61) % 3 else TIMBER[1]
            if x in (102, 117) or y in (61, 70):
                c = TIMBER[1]
            pr.put(x, y, c)
    stamp(pr, 106, 63, SIGN_ANVIL, {"I": IRON[3], "W": IRON[3]})
    pr.put(99, 71, IRON[2])
    stamp(pr, 97, 72, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3],
                               "W": GLOW[4]})
    wood = {"t": TIMBER[1], "T": TIMBER[2], "W": TIMBER[3], "i": IRON[1], "I": IRON[2]}
    stamp(pr, 25, 89, STUMP, wood)
    stamp(pr, 24, 83, ANVIL, {"i": IRON[1], "I": IRON[2], "W": IRON[3]})
    stamp(pr, 44, 83, BARREL, wood)
    stamp(pr, 81, 85, CRATE, wood)
    logs = {"o": LOG[0], "R": LOG[2], "W": LOG[3]}
    for (y, xs) in ((91, (96, 100, 104, 108, 112)), (88, (98, 102, 106, 110)),
                    (85, (100, 104, 108))):
        for x in xs:
            stamp(pr, x, y, LOG_END, logs)
    composite(canvas, pr, OUT)
    light_spill(canvas, ((18, 82, 16),), set(SOOT) | set(STONE) | set(TIMBER), (255, 120, 40))
    return canvas


def main():
    save_sprite(build(), ASSETS / "blacksmith.bmp")


if __name__ == "__main__":
    main()
