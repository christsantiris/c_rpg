#!/usr/bin/env python3
"""Frostfell Wastes entrance: a frozen fortress beneath a towering peak, in the 24px pixelkit pipeline.

A dark, frost-rimed keep with gothic spires is built into icy cliffs below a
snow-swept mountain while a blizzard blows across the range. Its south-facing
gate anchors to the area's entrance tile (columns 72-95 of the 168 x 144 sprite).
"""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, composite, cyl_wall, flat_wall, gothic_half, merlons, poly_mask, save_sprite, spire, stamp

SIZE = (168, 144)
CX = 84
OUT = (12, 18, 32)
PEAK = [(58, 80, 116), (84, 108, 146), (114, 140, 176), (150, 174, 206), (188, 206, 228), (222, 233, 245), (246, 250, 253)]
CLIFF = [(16, 22, 36), (26, 36, 54), (40, 54, 76), (60, 76, 100), (88, 106, 132), (130, 150, 176), (196, 210, 228)]
STONE = [(18, 22, 34), (28, 34, 50), (40, 48, 68), (56, 66, 88), (76, 88, 112), (104, 118, 142)]
SLATE = [(20, 26, 40), (32, 42, 62), (48, 60, 84), (70, 84, 110), (98, 114, 140)]
SNOW = [(150, 176, 206), (190, 208, 228), (224, 234, 245), (246, 250, 253)]
LIGHT = [(96, 60, 34), (176, 112, 52), (236, 170, 84)]
BANNER = [(22, 40, 78), (36, 62, 112), (60, 94, 150)]
GATE = (8, 12, 22)

# Each mass: silhouette, creases into slabs, and slab tones from lit west to shaded east.
RANGE = [
    ([(6, 112), (2, 90), (0, 66), (8, 50), (16, 40), (24, 30), (30, 24), (34, 22), (38, 26), (44, 36), (52, 50), (60, 64),
      (64, 112)], [[(34, 22), (30, 112)]], [4, 2]),
    ([(100, 112), (108, 62), (116, 46), (124, 34), (130, 26), (134, 22), (138, 26), (146, 36), (156, 46),
      (167, 54), (167, 112)], [[(134, 22), (138, 112)]], [4, 2]),
    ([(14, 112), (24, 96), (34, 80), (44, 62), (52, 46), (60, 32), (66, 20), (72, 10), (77, 3), (80, 0),
      (83, 4), (88, 12), (94, 22), (102, 36), (110, 52), (118, 66), (128, 82), (140, 98), (152, 112)],
     [[(80, 1), (64, 112)], [(81, 2), (94, 112)]], [5, 6, 3]),
]
CRAGS = [
    ([(8, 144), (3, 128), (6, 112), (1, 94), (4, 76), (0, 58), (2, 40), (0, 26), (5, 20), (9, 26), (14, 18), (19, 24), (24, 34), (29, 44), (34, 60), (38, 80),
      (41, 100), (40, 124), (36, 144)], [[(14, 18), (10, 144)], [(24, 34), (24, 144)]], [3, 2, 1]),
    ([(131, 144), (129, 118), (132, 96), (137, 80), (142, 70), (147, 62), (151, 66), (156, 56), (161, 62),
      (167, 58), (164, 76), (167, 92), (163, 110), (166, 126), (161, 144)], [[(147, 62), (144, 144)], [(156, 56), (158, 144)]], [3, 2, 1]),
]


def crease_x(crease, y):
    (ax, ay), (bx, by) = crease
    if y <= ay:
        return ax
    return ax + (bx - ax) * (y - ay) / max(1, by - ay)


def serrate(points, rng, top):
    # Wind-carved teeth along the ridge, sharper near the summit.
    out = []
    for (ax, ay), (bx, by) in zip(points, points[1:]):
        out.append((ax, ay))
        steps = max(1, int(max(abs(bx - ax), abs(by - ay)) // 4))
        for n in range(1, steps):
            t = n / steps
            x = ax + (bx - ax) * t
            y = ay + (by - ay) * t
            if y < top + 60:
                y += rng.randint(-1, 1)
            out.append((round(x), round(y)))
    out.append(points[-1])
    return out


def facet(canvas, mass, palette, outline, seed, snowline, ledge_rate):
    """A faceted mass: slab shading, dark seams, snow on top faces and on lit ledges above snowline."""
    rng = random.Random(seed)
    points, creases, tones = mass
    mask = poly_mask(SIZE, serrate(points, rng, min(y for x, y in points)))
    seams = set()
    for n in range(len(mask) // 140):
        x, y = rng.choice(sorted(mask))
        for k in range(rng.randrange(4, 14)):
            seams.add((x + k // 3, y + k))
    ledges = set()
    for n in range(len(mask) // ledge_rate):
        x, y = rng.choice(sorted(mask))
        for k in range(rng.randrange(3, 9)):
            ledges.add((x + k, y + k // 3))
    layer = Layer(*SIZE)
    for x, y in sorted(mask):
        slab = sum(1 for crease in creases if crease_x(crease, y) < x)
        tone = tones[slab]
        lit = slab < len(tones) - 1
        if any(abs(crease_x(crease, y) - x) < 0.6 for crease in creases):
            tone = max(0, tones[-1] - 1)
        elif (x, y) in seams:
            tone -= 2
        elif (x, y) in ledges and (lit or y < snowline):
            tone = 6 if lit else 5
        if (x, y - 1) not in mask or (x, y - 2) not in mask:
            tone = 6 if lit else 5
        elif y < snowline and (x, y - 3) not in mask:
            tone = 5
        layer.put(x, y, palette[max(0, min(6, tone))])
    composite(canvas, layer, outline)


def tower(canvas, x0, x1, top, roof_tip, flag, seed):
    # A slender round tower with a needle spire, frost on its rim and lit arrow slits.
    layer = Layer(*SIZE)
    cyl_wall(layer, x0, x1, top, 143, STONE, row_h=5, cols=5, seed=seed)
    cx = (x0 + x1 + 1) / 2.0
    for x in range(x0 - 1, x1 + 2):
        layer.put(x, top, SNOW[2] if x < cx else SNOW[1])
        layer.put(x, top + 1, STONE[1])
    for y in range(top + 10, 120, 16):
        for dy in range(4):
            layer.put(int(cx) - 1, y + dy, LIGHT[1] if dy else LIGHT[2])
            layer.put(int(cx), y + dy, LIGHT[0])
    composite(canvas, layer, OUT)
    roof = Layer(*SIZE)
    spire(roof, cx, roof_tip, top - 1, (x1 - x0) / 2.0 + 1, SLATE, p=1.6, row_h=3, cols=7, seed=seed + 1, hooks=False)
    for y in range(roof_tip, top):
        # Snow clings to the windward side of each spire.
        for x in range(x0 - 2, int(cx) + 1):
            if roof.on(x, y) and not roof.on(x - 1, y) and (y + x) % 2 == 0:
                roof.put(x, y, SNOW[3])
                roof.put(x + 1, y, SNOW[2])
    composite(canvas, roof, OUT)
    if flag:
        pennant = Layer(*SIZE)
        for y in range(roof_tip - 9, roof_tip):
            pennant.put(int(cx), y, SLATE[4])
        stamp(pennant, int(cx) + 1, roof_tip - 9, ["bbbbbb.", "BBBBBBb", "bbbbb..", "bbb...."],
              {'b': BANNER[1], 'B': BANNER[2]})
        composite(canvas, pennant, OUT)


def keep(canvas):
    # The curtain wall and gatehouse: dark masonry, crenellations heavy with snow, icicles below.
    walls = Layer(*SIZE)
    flat_wall(walls, 40, 127, 92, 143, STONE, row_h=5, base=3, seed=31, bw=(6, 10),
              shade=lambda x, y: 1 if x < 60 else -1 if x > 110 else 0)
    merlons(walls, 40, 127, 85, 8, STONE, mw=5, gap=3, base=3, point=0, start=1)
    flat_wall(walls, 62, 105, 62, 143, STONE, row_h=5, base=4, seed=37, bw=(6, 9),
              shade=lambda x, y: 1 if x < 66 else -1 if x > 101 else 0)
    merlons(walls, 61, 106, 55, 8, STONE, mw=5, gap=3, base=4, point=0, start=1)
    composite(canvas, walls, OUT)
    snow = Layer(*SIZE)
    rng = random.Random(41)
    for x0, x1, y in ((40, 127, 85), (61, 106, 55)):
        for x in range(x0, x1 + 1):
            if walls.on(x, y + 1) or walls.on(x, y):
                top = y - (1 if (x * 7) % 5 == 0 else 0)
                for yy in range(top, top + 2):
                    snow.put(x, yy, SNOW[3] if yy == top else SNOW[2])
    for y0, x0, x1 in ((93, 41, 61), (93, 106, 127), (63, 62, 105)):
        x = x0 + rng.randrange(0, 3)
        while x < x1:
            for n in range(rng.randrange(2, 6)):
                snow.put(x, y0 + n, SNOW[2] if n < 2 else SNOW[0])
            x += rng.randrange(3, 6)
    composite(canvas, snow, None)
    # Lit windows high on the gatehouse and along the curtain.
    lights = Layer(*SIZE)
    for x, y in ((70, 70), (95, 70), (48, 100), (117, 100)):
        for dy in range(6):
            for dx in range(3):
                lights.put(x + dx, y + dy, LIGHT[2] if dy > 2 and dx == 1 else LIGHT[1] if dy > 0 else GATE)
    composite(canvas, lights, OUT)


def gate(canvas):
    # A pointed arch with a frosted voussoir rim; firelight spills from deep inside.
    rim = Layer(*SIZE)
    apex, spring, half = 100, 112, 12
    for y in range(apex - 4, 144):
        outer = gothic_half(y, apex - 4, spring, half + 4)
        inner = gothic_half(y, apex, spring, half)
        if outer < 0:
            continue
        for x in range(CX - math.ceil(outer), CX + math.ceil(outer)):
            dx = x + 0.5 - CX
            if inner >= 0 and abs(dx) <= inner:
                continue
            block = (y - apex) // 5
            rim.put(x, y, STONE[5] if (block + (dx < 0)) % 2 else STONE[4])
    for x in range(CX - 3, CX + 3):
        rim.put(x, apex - 5, SNOW[3])
        rim.put(x, apex - 4, SNOW[2])
    composite(canvas, rim, OUT)
    hole = Layer(*SIZE)
    for y in range(apex, 144):
        inner = gothic_half(y, apex, spring, half)
        if inner < 0:
            continue
        for x in range(CX - int(inner), CX + int(inner)):
            depth = abs(x + 0.5 - CX) / max(1.0, inner)
            glow = y > 128 and depth < 0.55
            hole.put(x, y, LIGHT[0] if glow and y > 135 else (38, 28, 30) if glow else GATE)
    for y in (136, 140):
        for x in range(CX - 11, CX + 11):
            hole.put(x, y, STONE[3])
            hole.put(x, y + 1, STONE[1])
    for x, length in ((74, 5), (78, 7), (83, 4), (88, 8), (92, 5)):
        top = apex + 1 + abs(x - CX) // 3
        for n in range(length):
            hole.put(x, top + n, SNOW[3] if n < length - 2 else SNOW[1])
    composite(canvas, hole, None)
    # A Frostfell banner bearing the kraken's mark hangs above the gate.
    banner = Layer(*SIZE)
    for y in range(76, 94):
        for x in range(78, 90):
            if y > 90 and abs(x - 83.5) < y - 89:
                continue
            banner.put(x, y, BANNER[2] if x < 80 else BANNER[0] if x > 87 else BANNER[1])
    stamp(banner, 80, 80, ["..ww..", ".wWWw.", "wWWWWw", "w.ww.w", "w.ww.w", ".w..w."],
          {'w': SNOW[1], 'W': SNOW[3]})
    composite(canvas, banner, OUT)


def drifts(canvas):
    # Deep snow banks against the walls and cliffs, brighter on top.
    layer = Layer(*SIZE)
    for cx, cy, rx, ry in ((14, 140, 18, 8), (44, 141, 14, 6), (124, 141, 14, 6), (154, 139, 18, 9),
                           (62, 142, 9, 4), (106, 142, 9, 4)):
        for y in range(cy - ry, 144):
            for x in range(cx - rx, cx + rx + 1):
                if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1 and 0 <= x < SIZE[0]:
                    layer.put(x, y, SNOW[3] if y < cy - ry + 2 else SNOW[2] if x < cx else SNOW[1])
    composite(canvas, layer, SNOW[0])


def blizzard(canvas):
    # Wind-driven snow streaks across the scene from the north-west.
    rng = random.Random(53)
    layer = Layer(*SIZE)
    for n in range(70):
        x = rng.randrange(0, SIZE[0])
        y = rng.randrange(0, 132)
        length = rng.choice((2, 3, 3, 4))
        for k in range(length):
            layer.put(x + k, y + k // 2, SNOW[3] if k == length - 1 else SNOW[1])
    composite(canvas, layer, None)


def build():
    canvas = Image.new('RGBA', SIZE)
    for n, mass in enumerate(RANGE):
        facet(canvas, mass, PEAK, PEAK[1], 11 + n, 60, 110)
    for n, mass in enumerate(CRAGS):
        facet(canvas, mass, CLIFF, OUT, 61 + n, 70, 260)
    keep(canvas)
    tower(canvas, 44, 57, 40, 6, True, 71)
    tower(canvas, 110, 122, 52, 24, False, 73)
    gate(canvas)
    drifts(canvas)
    blizzard(canvas)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/frostfell-entrance.bmp')
