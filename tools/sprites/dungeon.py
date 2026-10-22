#!/usr/bin/env python3
"""Lich-infested crypt entrance, at the labyrinth's native 3 x 3 tile scale."""
from pathlib import Path

from PIL import Image

from pixelkit import Layer, composite, flat_wall, line_points, save_sprite, stamp

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
SIZE = 72
OUT = (12, 13, 22)
STONE = [(23, 24, 35), (37, 36, 51), (54, 52, 69), (72, 68, 86), (91, 86, 105), (116, 109, 126)]
IRON = [(17, 22, 26), (37, 47, 49), (65, 77, 77), (102, 117, 109)]
BONE = [(99, 98, 84), (156, 156, 127), (204, 207, 171)]
SOUL = [(31, 65, 53), (51, 123, 82), (108, 207, 126), (201, 248, 172)]

SKULL = [
    "...bbbbb...",
    "..bBBBBBb..",
    ".bBBBBBBBb.",
    ".BBssBssBB.",
    ".BBssBssBB.",
    "..BB.b.BB..",
    "...B.b.B...",
    "...bBbBb...",
    "....bbb....",
]
FLAME = [
    "....s..",
    "..s.s..",
    "..ssS..",
    ".sSsS..",
    ".sSWSs.",
    "ssWWSs.",
    ".sWWSs.",
    "..sss..",
]


def build():
    canvas = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    body = Layer(SIZE, SIZE)
    flat_wall(body, 8, 63, 23, 65, STONE, row_h=6, base=3, seed=41, bw=(9, 14))
    # A broken, pointed pediment above the burial vault.
    for y in range(8, 25):
        half = min(27, 5 + (y - 8) * 2)
        for x in range(36 - half, 36 + half):
            body.put(x, y, STONE[4] if x == 36 - half else STONE[2])
    for x in range(7, 65):
        body.put(x, 24, STONE[5])
        body.put(x, 25, STONE[1])
    composite(canvas, body, OUT)

    # Deep stone archivolts, with a narrow doorway matching the town doors.
    arch = Layer(SIZE, SIZE)
    for y in range(28, 66):
        half = min(16, 3 + (y - 28))
        for x in range(36 - half, 37 + half):
            edge = x - (36 - half)
            arch.put(x, y, STONE[5] if edge == 0 else STONE[3])
            if y % 6 == 3 and abs(x - 36) > 11:
                arch.put(x, y, STONE[1])
    composite(canvas, arch, OUT)
    opening = Layer(SIZE, SIZE)
    for y in range(36, 66):
        half = min(9, 1 + y - 36)
        for x in range(36 - half, 37 + half):
            opening.put(x, y, (9, 15, 20) if y < 55 else (16, 25, 28))
    # Descending steps disappear into the black crypt; no closed door or bars.
    for row in range(4):
        y = 54 + row * 3
        half = 4 + row
        for x in range(36 - half, 37 + half):
            opening.put(x, y, STONE[3])
            opening.put(x, y + 1, STONE[1])
    for x in (28, 44):
        for y in range(46, 64):
            opening.put(x, y, SOUL[0])
    composite(canvas, opening, None)

    # Shaded buttresses, iron funeral spikes, and green soul-fire braziers.
    for side, x0 in enumerate((4, 57)):
        pillar = Layer(SIZE, SIZE)
        flat_wall(pillar, x0, x0 + 10, 28, 65, STONE, row_h=7, base=3, seed=21 + side, bw=(6, 10),
                  shade=lambda x, y, left=x0: -1 if x > left + 7 else 0)
        for x in range(x0 - 1, x0 + 12):
            pillar.put(x, 27, STONE[5])
            pillar.put(x, 28, STONE[2])
        for y in range(15, 27):
            pillar.put(x0 + 5, y, IRON[2] if y > 17 else IRON[3])
            if y > 19:
                pillar.put(x0 + 6, y, IRON[0])
        for y in range(48, 57):
            pillar.put(x0 + 5, y, IRON[2])
            pillar.put(x0 + 6, y, IRON[0])
        for x in range(x0 + 1, x0 + 10):
            pillar.put(x, 47, IRON[2])
            pillar.put(x, 48, IRON[0])
        stamp(pillar, x0 + 2, 39, FLAME, {"s": SOUL[1], "S": SOUL[2], "W": SOUL[3]})
        composite(canvas, pillar, OUT)

    detail = Layer(SIZE, SIZE)
    # Crowned skull, inset in a weathered shield above the entrance.
    for y in range(9, 28):
        half = 9 if y < 23 else 9 - (y - 23)
        for x in range(36 - half, 37 + half):
            detail.put(x, y, STONE[5] if x == 36 - half or y == 9 else STONE[1])
    stamp(detail, 31, 14, SKULL, {"b": BONE[0], "B": BONE[1], "s": SOUL[2]})
    for x, top in ((31, 5), (36, 2), (41, 5)):
        for y in range(top, 12):
            detail.put(x, y, BONE[1] if x < 37 else BONE[0])
        detail.put(x, top, BONE[2])
    # Fissures and a few necromantic runes cut into the arch jambs.
    for points in (((16, 27), (18, 31), (16, 35), (18, 38)), ((57, 30), (53, 34), (55, 39))):
        for start, end in zip(points, points[1:]):
            for x, y in line_points(*start, *end):
                detail.put(x, y, STONE[0])
                detail.put(x + 1, y, STONE[4])
    for x in (22, 50):
        for y in (43, 53):
            detail.put(x, y, SOUL[2])
            detail.put(x - 1, y + 1, SOUL[1])
            detail.put(x, y + 2, SOUL[1])
    composite(canvas, detail, OUT)

    steps = Layer(SIZE, SIZE)
    for y in range(66, 72):
        inset = 3 if y < 69 else 0
        for x in range(15 + inset, 57 - inset):
            steps.put(x, y, STONE[4] if y % 3 == 0 else STONE[2])
    for x in (24, 46):
        steps.put(x, 69, STONE[1])
        steps.put(x + 1, 70, STONE[1])
    composite(canvas, steps, OUT)
    return canvas


if __name__ == "__main__":
    save_sprite(build(), ASSETS / "dungeon-entrance.bmp")
