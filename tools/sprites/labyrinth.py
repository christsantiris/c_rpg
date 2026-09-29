#!/usr/bin/env python3
"""Town 2 labyrinth entrance sprite sheet.

Two 3 x 3 tile frames (72 x 72) side by side: sealed, then open. The game draws
a frame from one tile left of and two tiles above the entrance tile
TOWN_LABYRINTH_X/Y. Run `make sprites` to rewrite
assets/labyrinth-entrance.png and assets/labyrinth-entrance.bmp.
"""
import random
from pathlib import Path

from PIL import Image

from pixelkit import Layer, composite, flat_wall, save_sprite, stamp

ASSETS = Path(__file__).resolve().parents[2] / "assets"
FW, FH = 72, 72
CX = 36

OUT = (14, 18, 14)
MOSSY = [(25, 29, 25), (48, 49, 40), (91, 98, 81), (105, 109, 88), (123, 124, 100), (150, 150, 120)]
EDGE = (166, 161, 126)
DARK = [(8, 13, 12), (14, 21, 19), (24, 32, 28)]
IRON = [(30, 36, 33), (50, 58, 53), (84, 94, 86), (116, 126, 105)]
BRASS = [(96, 66, 26), (140, 100, 44), (176, 132, 62), (214, 176, 96)]
IVORY = [(170, 162, 126), (206, 198, 158), (232, 226, 190)]
IVY = [(28, 54, 28), (40, 74, 37), (69, 109, 45), (98, 140, 60)]
FIRE = [(141, 60, 28), (236, 140, 43), (255, 221, 120), (255, 246, 196)]

ROOK = [
    "c.c.c",
    "ccccc",
    ".CcC.",
    ".CcC.",
    ".CcC.",
    "CcccC",
    "ccccc",
]

LOCK = [
    ".bBb.",
    "bB.Bb",
    "bBBBb",
    "bBkBb",
    "bBkBb",
    "bbbbb",
]

FLAME = [
    "..y..",
    "..yy.",
    ".yWy.",
    ".yWyo",
    "oyWWo",
    "oyWyo",
    ".ooo.",
]


def ivy(L, x, y0, y1, side, seed):
    """Ivy climbing a buttress edge, leaning towards side (-1 left, 1 right)."""
    rnd = random.Random(seed)
    for y in range(y0, y1):
        L.put(x, y, IVY[1] if y % 3 else IVY[2])
        if rnd.random() < 0.55:
            L.put(x + side, y, IVY[2] if rnd.random() < 0.5 else IVY[3])
        if rnd.random() < 0.25:
            L.put(x - side, y, IVY[0])
        if y % 4 == 0:
            leaf_x = x + side * 2
            L.put(leaf_x, y, IVY[2])
            L.put(leaf_x, y + 1, IVY[1])
            L.put(leaf_x + side, y, IVY[3])
            L.put(x - side * 2, y + 2, IVY[1])
        if rnd.random() < 0.15:
            x += rnd.choice((-1, 1))


def frame(opened):
    """One 72 x 72 frame: portcullis down when sealed, steps and lit torches when open."""
    canvas = Image.new("RGBA", (FW, FH), (0, 0, 0, 0))

    # main wall and stepped coping
    wall = Layer(FW, FH)
    flat_wall(wall, 8, 63, 17, 65, MOSSY, row_h=8, base=3, seed=3, bw=(10, 15))
    for (x0, x1, y0, y1, c) in ((12, 59, 10, 16, MOSSY[2]), (16, 55, 6, 9, MOSSY[3])):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                wall.put(x, y, EDGE if y == y0 else (c if y < y1 else MOSSY[1]))
    composite(canvas, wall, OUT)

    # stepped arch around a deep doorway
    ar = Layer(FW, FH)
    for row, half in enumerate((7, 11, 15, 18, 20)):
        top = 19 + row * 4
        for y in range(top, top + 4):
            for x in range(CX - half, CX + half):
                c = EDGE if y == top else MOSSY[4]
                if abs(x - CX) >= half - 1:
                    c = MOSSY[2]
                ar.put(x, y, c)
    for y in range(39, 66):
        for x in range(16, 56):
            c = MOSSY[4]
            if x in (16, 17):
                c = EDGE
            if x >= 53:
                c = MOSSY[2]
            if (y - 39) % 8 == 5 and (x < 20 or x > 51):
                c = MOSSY[1]
            ar.put(x, y, c)
    composite(canvas, ar, OUT)

    door = Layer(FW, FH)
    for row, half in enumerate((3, 7, 11, 14, 16)):
        top = 22 + row * 4
        for y in range(top, top + 4):
            for x in range(CX - half, CX + half):
                door.put(x, y, DARK[0])
    for y in range(39, 66):
        for x in range(20, 52):
            door.put(x, y, DARK[0] if y < 50 else DARK[1])
    if opened:
        for y in range(40, 64):
            for x in (21, 22, 23):
                door.put(x, y, DARK[2] if x == 23 else DARK[1])
            door.put(49, y, DARK[1])
            door.put(50, y, DARK[2])
        for step in range(4):
            inset = 6 - step * 2
            y = 52 + step * 4
            for x in range(24 + inset, 48 - inset):
                door.put(x, y, EDGE)
                door.put(x, y + 1, MOSSY[3])
                door.put(x, y + 2, MOSSY[2])
    composite(canvas, door, None)

    if not opened:
        pc = Layer(FW, FH)
        for bar in range(5):
            top = 36 if bar in (0, 4) else 29
            bx = 23 + bar * 6
            for y in range(top, 66):
                pc.put(bx, y, IRON[3])
                pc.put(bx + 1, y, IRON[1])
                pc.put(bx + 2, y, IRON[0])
            pc.put(bx + 1, 57, BRASS[2])
        for by in (43, 56):
            for x in range(21, 51):
                pc.put(x, by, IRON[3])
                pc.put(x, by + 1, IRON[2])
                pc.put(x, by + 2, IRON[1])
            for rx in range(23, 51, 6):
                pc.put(rx + 1, by + 1, BRASS[3])
        stamp(pc, 34, 46, LOCK, {"b": BRASS[0], "B": BRASS[2], "k": DARK[0]})
        composite(canvas, pc, OUT)

    # buttresses with sconces and climbing ivy
    for side in (0, 1):
        bt = Layer(FW, FH)
        px = 3 + side * 54
        flat_wall(bt, px, px + 11, 27, 66, MOSSY, row_h=8, base=3, seed=11 + side, bw=(6, 12),
                  shade=lambda x, y, s=side: -1 if (x > px + 8) == bool(1 - s) and s == 0 else 0)
        for x in range(px - 1, px + 13):
            bt.put(x, 24, EDGE)
            bt.put(x, 25, MOSSY[3])
            bt.put(x, 26, MOSSY[1])
        for y in range(44, 56):
            for x in range(px + 4, px + 8):
                bt.put(x, y, IRON[1] if x > px + 5 else IRON[2])
        for x in range(px + 3, px + 9):
            bt.put(x, 47, BRASS[2])
            bt.put(x, 48, BRASS[1])
        if opened:
            stamp(bt, px + 3, 37, FLAME, {"y": FIRE[1], "W": FIRE[2], "o": FIRE[0]})
        composite(canvas, bt, OUT)
        iv = Layer(FW, FH)
        ivy(iv, px + (1 if side == 0 else 10), 16 + side * 2, 64, -1 if side == 0 else 1, 7 + side)
        composite(canvas, iv, None)

    # rook crest over the arch
    cr = Layer(FW, FH)
    for y in range(1, 18):
        for x in range(28, 44):
            c = BRASS[2] if x in (28, 29) or y in (1, 2) else BRASS[1]
            if 30 <= x <= 41 and 3 <= y <= 15:
                c = MOSSY[1]
            cr.put(x, y, c)
    stamp(cr, 34, 5, ROOK, {"c": IVORY[2], "C": IVORY[1]})
    for x in range(32, 41):
        cr.put(x, 12, IVORY[1])
        cr.put(x, 13, IVORY[0])
    composite(canvas, cr, OUT)

    # worn steps at the foot of the arch
    st = Layer(FW, FH)
    for x in range(14, 58):
        st.put(x, 66, EDGE if 16 < x < 55 else MOSSY[1])
        st.put(x, 67, MOSSY[1])
        st.put(x, 68, MOSSY[1])
    for x in range(12, 60):
        st.put(x, 69, MOSSY[4] if x < 58 else MOSSY[2])
        st.put(x, 70, MOSSY[2])
        st.put(x, 71, MOSSY[2])
    for x in (29, 46):
        st.put(x, 69, MOSSY[1])
        st.put(x, 70, MOSSY[1])
    composite(canvas, st, OUT)
    return canvas


def build():
    sheet = Image.new("RGBA", (FW * 2, FH), (0, 0, 0, 0))
    sheet.alpha_composite(frame(False), (0, 0))
    sheet.alpha_composite(frame(True), (FW, 0))
    return sheet


def main():
    save_sprite(build(), ASSETS / "labyrinth-entrance.png", ASSETS / "labyrinth-entrance.bmp")


if __name__ == "__main__":
    main()
