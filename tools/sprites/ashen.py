#!/usr/bin/env python3
"""Ashen Hollow's towering fire mountain, using the established native pixelkit pipeline."""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, composite, line_points, poly_mask, save_sprite

ROOT = Path(__file__).resolve().parents[2]
SIZE = (168, 216)
OUT = (12, 8, 10)
ROCK = [(20, 16, 18), (31, 24, 25), (44, 34, 33), (60, 46, 42), (80, 61, 52), (104, 80, 64), (130, 101, 78)]
CRAG = [(16, 12, 14), (26, 20, 21), (38, 29, 28), (54, 40, 36), (74, 55, 46)]
LAVA = [(74, 14, 12), (122, 26, 16), (178, 46, 18), (226, 88, 26), (250, 146, 44), (255, 206, 96), (255, 240, 180)]
SMOKE = [(30, 25, 27), (44, 37, 38), (60, 51, 50), (79, 68, 64), (101, 88, 80), (126, 111, 99)]
EMBER = [(150, 58, 30), (196, 84, 36)]

SUMMIT = (86, 45)
CONE = [(6, 202), (16, 194), (28, 182), (38, 166), (46, 148), (53, 128), (59, 108), (64, 90), (68, 74), (72, 60),
        (74, 50), (76, 44), (79, 41), (82, 45), (86, 51), (90, 45), (93, 42), (95, 38), (98, 43), (102, 56), (107, 72), (112, 88),
        (117, 102), (123, 112), (130, 118), (136, 130), (142, 146), (149, 166), (155, 184), (162, 202)]


def puff(layer, cx, cy, r, glow, seed):
    rng = random.Random(seed)
    lobes = [(rng.uniform(0, 6.28), rng.uniform(0.3, 0.7) * min(1.0, r / 5)) for n in range(2)]
    for y in range(int(cy - r - 2), int(cy + r + 3)):
        for x in range(int(cx - r - 2), int(cx + r + 3)):
            angle = math.atan2(y - cy, x - cx)
            edge = r + sum(a * math.sin(angle * (n + 3) + phase) for n, (phase, a) in enumerate(lobes))
            if math.hypot(x - cx, y - cy) > edge or y < 1:
                continue
            # Soft, low-contrast volume: lit crown, darker underside, warm crater light beneath.
            light = (-(y - cy) * 0.7 - (x - cx) * 0.35) / max(1.0, r)
            tone = 2 + (1 if light > 0.25 else 0) + (1 if light > 0.65 else 0) - (1 if light < -0.45 else 0)
            color = SMOKE[tone]
            if glow and light < -0.2:
                color = EMBER[1] if glow > 1 and light < -0.55 else EMBER[0]
            layer.put(x, y, color)


def plume(canvas):
    # Heavy billows climb from the crater, then roll east on the wind and thin into wisps.
    tail = Layer(*SIZE)
    for n, (cx, cy, r) in enumerate([(162, 33, 2.0), (157, 29, 3.0), (151, 26, 4.0)]):
        puff(tail, cx, cy, r, 0, 40 + n)
    composite(canvas, tail, SMOKE[1])
    body = Layer(*SIZE)
    billows = [(143, 21, 6.0, 0), (134, 27, 5.0, 0), (133, 15, 7.5, 0), (122, 22, 7.0, 0), (121, 11, 8.0, 0),
               (109, 16, 8.5, 0), (108, 28, 5.5, 1), (98, 11, 7.5, 0), (97, 22, 7.0, 0), (91, 15, 5.5, 0),
               (92, 30, 6.0, 1), (88, 37, 4.8, 2), (87, 42, 3.4, 2)]
    for n, (cx, cy, r, glow) in enumerate(billows):
        puff(body, cx, cy, r, glow, 50 + n)
    composite(canvas, body, SMOKE[0])
    # A thinner second trail drifts from the vent on the eastern shoulder.
    vent = Layer(*SIZE)
    for n, (cx, cy, r) in enumerate([(125, 107, 2.0), (129, 103, 2.6), (135, 100, 3.0), (142, 99, 2.6), (148, 99, 1.8)]):
        puff(vent, cx, cy, r, 1 if n == 0 else 0, 60 + n)
    composite(canvas, vent, SMOKE[0])


def cone(canvas, rng):
    rock = Layer(*SIZE)
    top = SUMMIT[1]
    for x, y in sorted(poly_mask(SIZE, CONE)):
        half = max(4, (y - top) * 0.5)
        t = (x - SUMMIT[0]) / half
        # Lit western face, deep eastern shadow, and gullies running down from the crater.
        tone = 4.4 - 2.3 * (t + 0.55) if t > -0.55 else 4.4 + 0.8 * (t + 0.55)
        angle = math.atan2(y - top + 4, x - SUMMIT[0])
        groove = math.sin(angle * 44 + math.sin(y * 0.07) * 1.6)
        if groove > 0.72:
            tone -= 1.2
        elif groove < -0.86:
            tone += 0.7
        if rng.randrange(23) == 0:
            tone += rng.choice((-1, 1))
        rock.put(x, y, ROCK[max(0, min(6, int(round(tone))))])
    composite(canvas, rock, OUT)
    # A notched crater lip glows where the lava spills over.
    rim = Layer(*SIZE)
    for x, y in line_points(77, 44, 82, 47) + line_points(90, 47, 94, 42):
        rim.put(x, y, LAVA[2])
    for x in range(82, 91):
        rim.put(x, 47 + (1 if 84 < x < 89 else 0), LAVA[3])
    composite(canvas, rim, None)


def flow(layer, points, widths, hot=True):
    cells = set()
    for (a, b), width in zip(zip(points, points[1:]), widths):
        for x, y in line_points(*a, *b):
            for dx in range(-width, width + 1):
                for dy in range(-1, 2):
                    distance = abs(dx) + abs(dy) * 0.6
                    if distance > width:
                        continue
                    level = width - distance
                    if hot:
                        index = 6 if level >= 1.9 else 5 if level >= 1.2 else 4 if level >= 0.6 else 2
                    else:
                        index = 3 if level >= 1.4 else 2 if level >= 0.6 else 1
                    if not layer.on(x + dx, y + dy) or LAVA.index(layer.get(x + dx, y + dy)) < index:
                        layer.put(x + dx, y + dy, LAVA[index])
                    cells.add((x + dx, y + dy))
    return cells


def lava(canvas, rng):
    # The crater pours one river down the face, splitting around the gate below.
    layer = Layer(*SIZE)
    hot = set()
    hot |= flow(layer, [(86, 49), (84, 64), (87, 80), (83, 96), (86, 112), (84, 128), (85, 142)], [1, 2, 2, 2, 3, 3])
    hot |= flow(layer, [(85, 142), (79, 150), (70, 158), (62, 170), (57, 184), (55, 200)], [3, 2, 2, 2, 2])
    hot |= flow(layer, [(85, 142), (91, 150), (100, 158), (108, 170), (112, 184), (113, 200)], [3, 2, 2, 2, 2])
    # Cooler secondary runs keep the glow from being perfectly symmetrical.
    hot |= flow(layer, [(78, 47), (73, 62), (68, 78), (64, 94), (61, 104)], [1, 1, 1, 0], hot=False)
    hot |= flow(layer, [(122, 110), (127, 122), (132, 134), (136, 144)], [1, 1, 0], hot=False)
    for x, y in sorted(hot):
        if rng.randrange(17) == 0 and layer.on(x, y) and LAVA.index(layer.get(x, y)) < 5:
            layer.put(x, y, ROCK[1])
    composite(canvas, layer, None)
    glow(canvas, hot | crater_cells(), ROCK + CRAG)


def crater_cells():
    return {(x, y) for x in range(80, 93) for y in range(45, 50)}


def glow(canvas, cells, surfaces):
    # Warm rim light hugs the lava in two bands instead of spreading in round halos.
    cp = canvas.load()
    w, h = canvas.size
    near = {}
    for cx, cy in cells:
        for dy in range(-4, 5):
            for dx in range(-4, 5):
                d = math.hypot(dx, dy)
                key = (cx + dx, cy + dy)
                if d <= 4 and d < near.get(key, 9):
                    near[key] = d
    for (x, y), d in near.items():
        if not (0 <= x < w and 0 <= y < h) or cp[x, y][3] == 0:
            continue
        c = cp[x, y][:3]
        if c not in surfaces:
            continue
        k = 0.42 if d <= 1.5 else 0.2 if d <= 3 else 0.1
        cp[x, y] = tuple(int(c[i] + ((214, 86, 34)[i] - c[i]) * k) for i in range(3)) + (255,)


def crags(canvas, rng):
    # Jagged foothill spires darken the foreground, as in the volcanic plains below the peak.
    for points, ridge in [
            ([(2, 215), (4, 200), (8, 190), (12, 176), (16, 166), (19, 178), (24, 170), (29, 152), (33, 168), (38, 178),
              (43, 172), (47, 184), (51, 196), (52, 215)], 29),
            ([(116, 215), (117, 198), (121, 188), (126, 181), (130, 168), (134, 153), (138, 168), (142, 176), (147, 164),
              (151, 178), (156, 186), (161, 196), (165, 215)], 134),
            ([(18, 215), (22, 202), (27, 195), (32, 199), (37, 189), (42, 197), (46, 215)], 37),
            ([(124, 215), (128, 204), (134, 197), (139, 202), (144, 192), (150, 203), (154, 215)], 144)]:
        layer = Layer(*SIZE)
        mask = poly_mask(SIZE, points)
        for x, y in sorted(mask):
            tone = 3 if x < ridge else 1
            if (x - ridge) % 9 == 0 and x > ridge:
                tone = 2
            if rng.randrange(31) == 0:
                tone += rng.choice((-1, 1))
            layer.put(x, y, CRAG[max(0, min(4, tone))])
        for x, y in sorted(mask):
            if (x - 1, y) not in mask and x < ridge:
                layer.put(x, y, CRAG[4])
        composite(canvas, layer, OUT)


def pools(canvas):
    layer = Layer(*SIZE)
    for cx, cy, rx, ry in [(55, 207, 6, 4), (113, 207, 6, 4)]:
        for y in range(cy - ry, cy + ry + 1):
            for x in range(cx - rx, cx + rx + 1):
                d = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
                if d <= 1:
                    layer.put(x, y, LAVA[6] if d < 0.15 else LAVA[5] if d < 0.4 else LAVA[4] if d < 0.7 else LAVA[2])
    composite(canvas, layer, OUT)


def gate(canvas, rng):
    # Low, jagged basalt buttresses frame a cleft lit by the fires deep in the mountain.
    frame = Layer(*SIZE)
    for points, ridge in [([(57, 215), (56, 200), (59, 192), (62, 188), (61, 183), (65, 179), (68, 182), (71, 177), (75, 183), (75, 215)], 66),
                          ([(93, 215), (93, 183), (97, 177), (100, 182), (103, 179), (107, 183), (106, 188), (109, 192), (112, 200), (111, 215)], 101)]:
        mask = poly_mask(SIZE, points)
        for x, y in sorted(mask):
            tone = 3 if x < ridge - (y - 180) // 9 else 1
            if rng.randrange(29) == 0:
                tone += rng.choice((-1, 1))
            frame.put(x, y, CRAG[max(0, min(4, tone))])
        # A few fractures break the slabs without forming a masonry pattern.
        for n in range(4):
            x0 = rng.randrange(points[0][0] + 2, points[-1][0] - 2)
            y0 = rng.randrange(188, 210)
            for x, y in line_points(x0, y0, x0 + rng.choice((-3, -2, 2, 3)), y0 + rng.randrange(3, 7)):
                if (x, y) in mask:
                    frame.put(x, y, CRAG[0])
        for x, y in sorted(mask):
            if (x - 1, y) not in mask or (x, y - 1) not in mask:
                frame.put(x, y, CRAG[4] if x < ridge else CRAG[2])
    composite(canvas, frame, OUT)
    mouth = Layer(*SIZE)
    for y in range(170, 216):
        depth = y - 170
        half = min(11.0, 2 + depth * 0.55) - (1 if (y // 3) % 2 and depth < 18 else 0)
        for x in range(int(84 - half), int(85 + half)):
            # Firelight rises from below the threshold in a soft, dithered dome.
            heat = 36 - math.hypot((x - 84.5) * 1.5, 218 - y) + (x + y) % 2 * 2
            color = (10, 6, 7)
            if heat > 6:
                color = (40, 12, 10)
            if heat > 13:
                color = (84, 24, 14)
            if heat > 20:
                color = (138, 44, 18)
            mouth.put(x, y, color)
    composite(canvas, mouth, OUT)
    steps = Layer(*SIZE)
    for y, inset in ((207, 0), (211, 1)):
        for x in range(74 - inset, 95 + inset):
            steps.put(x, y, LAVA[3] if abs(x - 84) < 5 else LAVA[2] if abs(x - 84) < 8 else CRAG[3])
            steps.put(x, y + 1, CRAG[1])
            steps.put(x, y + 2, CRAG[0])
    composite(canvas, steps, None)


def embers(canvas, rng):
    layer = Layer(*SIZE)
    for n in range(18):
        layer.put(rng.randrange(72, 122), rng.randrange(3, 52), LAVA[4] if n % 3 else LAVA[5])
    for x, y in [(66, 166), (103, 165), (52, 188), (117, 188), (79, 120), (91, 104), (80, 76)]:
        layer.put(x, y, LAVA[5])
    composite(canvas, layer, None)


def build():
    canvas = Image.new('RGBA', SIZE)
    rng = random.Random(97)
    plume(canvas)
    cone(canvas, rng)
    lava(canvas, rng)
    crags(canvas, rng)
    pools(canvas)
    gate(canvas, rng)
    embers(canvas, rng)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), ROOT / 'assets/images/ashen-entrance.bmp')
