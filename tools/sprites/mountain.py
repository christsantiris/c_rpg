#!/usr/bin/env python3
"""Goblin mountain entrance among red sandstone buttes and needles, using the town sprites' 24px tile pipeline."""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, line_points, poly_mask, save_sprite, stamp

SIZE = (168, 216)
OUT = (34, 16, 26)
# Sunlit sandstone runs from gold ledges through red-orange cliffs into warm violet shadow.
ROCK = [(44, 24, 40), (70, 34, 48), (102, 46, 50), (146, 62, 46), (186, 86, 50), (216, 116, 62), (242, 160, 90),
        (255, 204, 132)]
HAZE = [(124, 80, 96), (150, 98, 104), (178, 118, 112), (204, 142, 124)]
WOOD = [(38, 24, 20), (67, 42, 28), (101, 64, 35), (143, 97, 52)]
IRON = [(30, 26, 30), (70, 66, 70), (124, 118, 112)]

# Each mass: silhouette, creases dividing it into slabs, and slab tones from sunlit west to shadowed east.
MASSIF = ([(36, 212), (40, 184), (46, 166), (52, 164), (56, 146), (62, 130), (68, 128), (72, 108), (76, 92), (80, 90),
           (84, 70), (87, 52), (89, 38), (91, 26), (94, 34), (97, 38), (100, 22), (103, 10), (106, 16), (108, 30),
           (112, 32), (114, 48), (118, 60), (124, 62), (128, 82), (134, 94), (138, 96), (142, 120), (148, 148),
           (154, 150), (158, 178), (162, 212)],
          [[(91, 28), (70, 212)], [(95, 36), (86, 212)], [(103, 12), (102, 212)], [(110, 32), (122, 212)]], [5, 4, 5, 3, 2])
NEEDLE = ([(14, 212), (18, 186), (24, 160), (30, 134), (36, 108), (42, 84), (47, 62), (51, 40), (54, 22), (56, 8),
           (58, 4), (60, 12), (59, 30), (57, 52), (54, 76), (50, 104), (46, 134), (44, 168), (46, 212)],
          [[(57, 6), (28, 212)], [(59, 10), (40, 212)]], [5, 4, 2])
BUTTE = ([(118, 212), (120, 186), (124, 170), (128, 166), (130, 146), (136, 136), (140, 134), (142, 118), (148, 114),
          (152, 116), (154, 132), (158, 136), (160, 160), (164, 176), (166, 212)],
         [[(145, 116), (134, 212)], [(150, 118), (150, 212)]], [5, 4, 2])
DISTANT = [([(20, 152), (22, 122), (30, 118), (34, 106), (46, 106), (50, 120), (58, 152)], [[(34, 108), (40, 152)]], [2, 1]),
           ([(108, 124), (112, 98), (118, 94), (122, 80), (130, 82), (134, 98), (146, 102), (150, 140)],
            [[(126, 82), (132, 140)]], [2, 1])]


def crease_x(crease, y):
    for (ax, ay), (bx, by) in zip(crease, crease[1:]):
        if ay <= y <= by:
            return ax + (bx - ax) * (y - ay) / max(1, by - ay)
    return crease[0][0] if y < crease[0][1] else crease[-1][0]


def serrate(points, rng):
    # Break long edges into small ledges and teeth, so cliffs never read as ruled lines.
    out = []
    for (ax, ay), (bx, by) in zip(points, points[1:]):
        out.append((ax, ay))
        steps = max(1, int(max(abs(bx - ax), abs(by - ay)) // 5))
        for n in range(1, steps):
            t = n / steps
            x = round(ax + (bx - ax) * t)
            y = round(ay + (by - ay) * t)
            if y < 210:
                if rng.random() < 0.35:
                    out.append((x, y))
                    x += rng.choice((-2, 2))
                else:
                    x += rng.randint(-1, 1)
            out.append((x, y))
    out.append(points[-1])
    return out


def sandstone(canvas, mass, palette, outline, seed):
    points, creases, tones = mass
    rng = random.Random(seed)
    layer = Layer(*SIZE)
    mask = poly_mask(SIZE, serrate(points, rng))
    # Broken horizontal strata: a dark seam with a sunlit ledge lip above it.
    strata = {}
    y = min(y for x, y in points) + rng.randrange(6, 12)
    while y < 212:
        start = rng.randrange(0, 168)
        strata[y] = (start, start + rng.randrange(10, 40))
        y += rng.randrange(6, 13)
    for x, y in sorted(mask):
        slab = sum(1 for crease in creases if crease_x(crease, y) < x)
        tone = tones[slab]
        sunny = slab < len(tones) - 1
        seam = y in strata and strata[y][0] <= x <= strata[y][1]
        lip = y + 1 in strata and strata[y + 1][0] <= x <= strata[y + 1][1]
        if any(abs(crease_x(crease, y) - x) < 0.6 for crease in creases):
            tone = tones[-1] - 1
        elif seam:
            tone -= 1
        elif lip and sunny:
            tone += 1
        if (x, y - 1) not in mask:
            # Ledge tops and summits glow in the low sun.
            tone = tones[0] + 2 if sunny else tones[-1] + 2
        elif (x - 1, y) not in mask and sunny:
            tone = tones[0] + 1
        if y > 198:
            tone -= 1
        if rng.randrange(47) == 0:
            tone += rng.choice((-1, 1))
        layer.put(x, y, palette[max(0, min(len(palette) - 1, tone))])
    composite(canvas, layer, outline)


def talus(canvas):
    # Fallen sandstone blocks: lit tops, red fronts and shadowed eastern sides.
    rng = random.Random(41)
    rocks = Layer(*SIZE)
    blocks = [(rng.randrange(2, 62), rng.randrange(196, 212), rng.randrange(5, 11), rng.randrange(3, 5)) for n in range(12)]
    blocks += [(rng.randrange(104, 158), rng.randrange(196, 212), rng.randrange(5, 11), rng.randrange(3, 5)) for n in range(12)]
    for x, y, w, h in sorted(blocks, key=lambda b: b[1]):
        shape = poly_mask(SIZE, [(x, y), (x + 2, y - 2), (x + w - 1, y - 2), (x + w + 1, y), (x + w, y + h), (x, y + h)])
        for px, py in shape:
            tone = 6 if py <= y - 1 else 2 if px >= x + w - 1 else 4
            rocks.put(px, py, ROCK[tone])
    composite(canvas, rocks, OUT)


def gate(canvas):
    # Rough-cut jambs and goblin timber frame a cave driven into the massif.
    jambs = Layer(*SIZE)
    for x0, x1, lit in ((58, 68, True), (100, 110, False)):
        for y in range(170, 213):
            for x in range(x0, x1):
                edge = x == x0 or y == 170
                jambs.put(x, y, ROCK[5 if edge and lit else 4 if lit else 3 if edge else 2])
        for y in range(176, 212, 9):
            for x in range(x0 + 1, x1 - 1):
                jambs.put(x, y, ROCK[3 if lit else 1])
    composite(canvas, jambs, OUT)
    cave = Layer(*SIZE)
    for y in range(172, 212):
        half = min(12, 2 + (y - 172))
        for x in range(84 - half, 85 + half):
            cave.put(x, y, (16, 9, 14) if y < 202 else (38, 22, 28))
    for y in (204, 208, 212):
        for x in range(73, 96):
            cave.put(x, y, ROCK[3])
            cave.put(x, y + 1, ROCK[1])
    composite(canvas, cave, None)
    timber = Layer(*SIZE)
    for x in (66, 98):
        for y in range(167, 212):
            for w in range(5):
                timber.put(x + w, y, WOOD[3 if w == 0 else 1 if w > 2 else 2])
        for y in (175, 195, 207):
            for w in range(5):
                timber.put(x + w, y, IRON[1])
            timber.put(x + 1, y, IRON[2])
    for x, y in line_points(64, 166, 104, 164):
        for h in range(5):
            timber.put(x, y + h, WOOD[3 if h == 0 else 1 if h > 2 else 2])
    # Iron spikes, the goblin face sigil and two ragged war banners.
    for x in (66, 75, 93, 102):
        for y in range(158, 166):
            timber.put(x, y, IRON[2] if y > 160 else IRON[1])
    stamp(timber, 80, 155, ['..GGGGG..', '.GGGGGGG.', 'G.GGGGG.G', 'GGooGooGG', '.GGGGGGG.', '..GgGgG..', '...GGG...'],
          {'G': (114, 131, 77), 'g': (188, 177, 124), 'o': (24, 30, 23)})
    for x0 in (49, 111):
        for y in range(173, 199):
            for x in range(x0, x0 + 9):
                if y < 194 or (x + y) % 6 < 4:
                    timber.put(x, y, (52, 40, 38) if x < x0 + 5 else (34, 26, 28))
        for x, y in line_points(x0 - 2, 172, x0 + 11, 172):
            timber.put(x, y, WOOD[3])
        stamp(timber, x0 + 2, 178, ['..B..', '.BBB.', 'B.B.B', '.BBB.', '..B..'], {'B': (183, 155, 89)})
    composite(canvas, timber, OUT)


def build():
    canvas = Image.new('RGBA', SIZE)
    # Distant hazy mesas give the range the depth of canyon country at dusk.
    for n, mass in enumerate(DISTANT):
        sandstone(canvas, mass, HAZE, HAZE[0], 60 + n)
    sandstone(canvas, MASSIF, ROCK, OUT, 71)
    sandstone(canvas, BUTTE, ROCK, OUT, 72)
    # The tall needle leans in front of the massif, over the western road.
    sandstone(canvas, NEEDLE, ROCK, OUT, 73)
    talus(canvas)
    gate(canvas)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/mountain-entrance.bmp')
