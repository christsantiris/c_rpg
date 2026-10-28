#!/usr/bin/env python3
"""Dragonspine entrance, a 7x9 tile landmark in the established 24px pixelkit pipeline.

Pale granite needles rise in sheer, fractured aretes to a towering summit, after
high alpine granite spires. Red dragons circle the peaks and perch on a ledge.
The south-facing cave is a dark cleft at the foot of the cliffs, with a faint
ember glow and soot above it, behind a field of fallen talus.
"""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, line_points, poly_mask, save_sprite

SIZE = (168, 216)
OUT = (22, 26, 40)
GRANITE = [(46, 54, 78), (66, 76, 104), (92, 104, 132), (124, 136, 160), (158, 168, 188), (192, 200, 214), (224, 229, 236)]
WARM = [(176, 166, 158), (208, 198, 186)]
BONE = [(70, 62, 56), (110, 100, 86), (152, 142, 122), (192, 183, 160), (226, 219, 198)]
DRAKE = [(60, 16, 20), (104, 26, 26), (152, 40, 32), (200, 70, 42), (238, 128, 66)]
WING = [(66, 18, 26), (98, 28, 32), (134, 44, 40)]
GOLD = [(150, 104, 52), (214, 166, 82)]
EYE = (255, 214, 90)
DOOR = (10, 12, 18)

# Each peak: silhouette, the creases dividing it into slabs, and each slab's tone from sunlit west to shadowed east.
PEAKS = [
    ([(0, 206), (0, 132), (4, 118), (8, 110), (12, 98), (16, 102), (19, 90), (23, 94), (26, 84), (30, 88), (33, 80),
      (37, 90), (42, 104), (46, 122), (50, 150), (53, 206)], [[(26, 86), (22, 206)], [(31, 80), (32, 206)]], [4, 3, 1]),
    ([(22, 206), (28, 170), (34, 130), (40, 98), (46, 72), (51, 54), (55, 44), (58, 40), (60, 46), (62, 44), (65, 58),
      (70, 86), (78, 124), (88, 166), (96, 206)], [[(58, 42), (50, 206)], [(59, 43), (64, 206)]], [5, 4, 1]),
    ([(52, 206), (58, 160), (64, 118), (70, 82), (75, 56), (79, 38), (82, 28), (84, 24), (86, 30), (88, 28), (92, 46),
      (98, 78), (106, 120), (114, 166), (120, 206)], [[(84, 26), (76, 206)], [(85, 28), (88, 206)]], [5, 4, 1]),
    ([(88, 206), (94, 160), (99, 118), (103, 84), (107, 56), (111, 32), (115, 16), (118, 8), (120, 4), (122, 8),
      (125, 6), (128, 14), (132, 30), (137, 52), (143, 78), (149, 104), (155, 134), (160, 166), (164, 206)],
     [[(120, 6), (110, 206)], [(121, 8), (122, 206)], [(130, 26), (146, 206)]], [5, 4, 2, 1]),
]


def serrate(points, rng):
    # Break each edge into small granite teeth, sharper near the summit.
    top = min(y for x, y in points)
    out = []
    for (ax, ay), (bx, by) in zip(points, points[1:]):
        out.append((ax, ay))
        steps = max(1, int(max(abs(bx - ax), abs(by - ay)) // 4))
        for n in range(1, steps):
            t = n / steps
            x = ax + (bx - ax) * t
            y = ay + (by - ay) * t
            amp = 2 if y < top + 24 else 1 if y < top + 70 else 0
            if amp:
                x += rng.randint(-amp, amp)
            out.append((round(x), round(y)))
    out.append(points[-1])
    return out


def crease_x(crease, y):
    for (ax, ay), (bx, by) in zip(crease, crease[1:]):
        if ay <= y <= by:
            return ax + (bx - ax) * (y - ay) / max(1, by - ay)
    return crease[0][0] if y < crease[0][1] else crease[-1][0]


def granite(canvas, points, creases, tones, haze, seed):
    rng = random.Random(seed)
    outline = serrate(points, rng)
    layer = Layer(*SIZE)
    mask = poly_mask(SIZE, outline)
    cracks = set()
    for n in range(len(mask) // 520):
        x, y = rng.choice(sorted(mask))
        for k in range(rng.randrange(3, 9)):
            cracks.add((x + k // 2, y - k))
    # Long sheared seams climb the faces like the slabs of high granite walls.
    for n in range(len(mask) // 900):
        x, y = rng.choice(sorted(mask))
        for k in range(rng.randrange(12, 30)):
            if (x + k // 3, y - k) in mask:
                cracks.add((x + k // 3, y - k))
    for x, y in sorted(mask):
        slab = sum(1 for crease in creases if crease_x(crease, y) < x)
        tone = tones[slab]
        on_crease = any(abs(crease_x(crease, y) - x) < 0.6 for crease in creases)
        if on_crease:
            tone = max(0, tones[slab] - 2)
        elif any(0.6 <= x - crease_x(crease, y) < 1.6 for crease in creases) and slab < len(tones) - 1:
            tone += 1
        elif (x, y) in cracks:
            tone -= 2
        elif (x - 1, y + 1) in cracks and tone >= 3:
            tone += 1
        if (x - 1, y) not in mask and slab == 0:
            tone = 6
        elif (x, y - 1) not in mask:
            tone = 5 if slab < len(tones) - 1 else 3
        tone = max(0, min(6, tone - haze))
        color = GRANITE[tone]
        if tone >= 4 and rng.randrange(23) == 0:
            color = WARM[1 if tone > 4 else 0]
        layer.put(x, y, color)
    composite(canvas, layer, OUT)


def ledge(canvas):
    shelf = Layer(*SIZE)
    mask = poly_mask(SIZE, [(128, 108), (138, 104), (152, 103), (163, 105), (160, 110), (146, 113), (132, 113)])
    for x, y in sorted(mask):
        shelf.put(x, y, GRANITE[5] if (x, y - 1) not in mask else GRANITE[3] if y < 110 else GRANITE[2])
    composite(canvas, shelf, OUT)


def cleft(canvas):
    # A narrow, ragged cave mouth with a faint ember glow on its threshold.
    rng = random.Random(29)
    mouth = Layer(*SIZE)
    for y in range(162, 216):
        half = min(11.0, 1.5 + (y - 162) * 0.42) + rng.choice((-1, 0, 0, 0))
        for x in range(int(84 - half), int(85 + half)):
            glow = y > 206 and abs(x - 84.5) < 7 - (215 - y) * 0.6
            mouth.put(x, y, (84, 30, 20) if glow and abs(x - 84.5) < 3 else (44, 20, 22) if glow else DOOR)
    composite(canvas, mouth, OUT)


def talus(canvas):
    # Fallen granite blocks: lit tops, mid fronts, and dark eastern sides.
    rng = random.Random(37)
    rocks = Layer(*SIZE)
    blocks = [(rng.randrange(0, 66), rng.randrange(192, 212), rng.randrange(5, 12), rng.randrange(3, 6)) for n in range(18)]
    blocks += [(rng.randrange(100, 160), rng.randrange(192, 212), rng.randrange(5, 12), rng.randrange(3, 6)) for n in range(18)]
    blocks += [(1, 202, 16, 8), (148, 200, 18, 9), (58, 206, 10, 5), (100, 207, 10, 5)]
    for x, y, w, h in sorted(blocks, key=lambda b: b[1]):
        shape = poly_mask(SIZE, [(x, y), (x + 2, y - 2), (x + w - 1, y - 2), (x + w + 1, y), (x + w, y + h), (x, y + h)])
        for px, py in shape:
            if py <= y - 1:
                tone = 6 if px < x + w // 2 else 5
            elif px >= x + w - 1:
                tone = 2
            else:
                tone = 4 if py < y + h - 1 else 3
            rocks.put(px, py, GRANITE[tone])
    composite(canvas, rocks, OUT)


def blob(L, mask, ramp, base):
    """Fill a shape with its base tone, lighter on upper-left edges and darker on lower-right ones."""
    for x, y in mask:
        i = base
        if (x - 1, y) not in mask or (x, y - 1) not in mask:
            i += 1
        elif (x + 1, y) not in mask or (x, y + 1) not in mask:
            i -= 1
        L.put(x, y, ramp[max(0, min(len(ramp) - 1, i))])


def place(points, ox, oy, s=1.0, flip=False):
    return [(ox + (-px if flip else px) * s, oy + py * s) for px, py in points]


def bones_of(L, segs, ox, oy, c, s=1.0, flip=False):
    for a, b in segs:
        (ax, ay), (bx, by) = place([a, b], ox, oy, s, flip)
        for x, y in line_points(round(ax), round(ay), round(bx), round(by)):
            L.put(x, y, c)


def perched_dragon(L, ox, oy):
    """A red dragon sitting on the ledge, facing west, wings raised and spread."""
    far = [(-1, -10), (-5, -22), (-15, -29), (-14, -24), (-19, -21), (-15, -17), (-17, -11), (-9, -10), (-4, -6)]
    for x, y in poly_mask(SIZE, place(far, ox, oy)):
        L.put(x, y, WING[0])
    bones_of(L, [((-5, -22), (-15, -29)), ((-5, -22), (-19, -21)), ((-5, -22), (-17, -11)), ((-1, -10), (-5, -22))],
             ox, oy, DRAKE[1])
    near = [(2, -10), (8, -23), (19, -29), (17, -23), (22, -18), (17, -14), (19, -7), (11, -7), (6, -4)]
    for x, y in poly_mask(SIZE, place(near, ox, oy)):
        L.put(x, y, WING[2] if y < oy - 18 else WING[1])
    bones_of(L, [((8, -23), (19, -29)), ((8, -23), (22, -18)), ((8, -23), (19, -7)), ((2, -10), (8, -23))],
             ox, oy, DRAKE[3])
    tail = [(5, -2), (10, -1), (13, 2), (12, 5), (15, 7)]
    for (ax, ay), (bx, by) in zip(tail, tail[1:]):
        for x, y in line_points(ox + ax, oy + ay, ox + bx, oy + by):
            L.put(x, y, DRAKE[2])
            L.put(x, y - 1, DRAKE[1])
    body = poly_mask(SIZE, place([(-5, -1), (-6, -7), (-3, -11), (3, -11), (6, -7), (6, -1), (2, 0), (-3, 0)], ox, oy))
    neck = poly_mask(SIZE, place([(-4, -9), (-7, -14), (-8, -18), (-5, -19), (-3, -15), (0, -10)], ox, oy))
    head = poly_mask(SIZE, place([(-9, -21), (-6, -22), (-4, -20), (-5, -17), (-9, -16), (-14, -17), (-15, -19),
                                  (-12, -20)], ox, oy))
    blob(L, body | neck | head, DRAKE, 2)
    for y in range(-8, -1):
        L.put(ox - 3, oy + y, GOLD[1])
        L.put(ox - 2, oy + y, GOLD[0])
    for dx, dy, c in ((-6, -23, 3), (-5, -24, 3), (-4, -25, 4), (-8, -22, 2), (-8, -23, 2), (-7, -24, 3)):
        L.put(ox + dx, oy + dy, BONE[c])
    L.put(ox - 9, oy - 19, EYE)
    for x in (-4, -3, 3, 4):
        L.put(ox + x, oy, DRAKE[1])


def flying_dragon(L, ox, oy, s=1.0, flip=False):
    """A dragon in banking flight, side on, near wing raised."""
    def pts(p):
        return place(p, ox, oy, s, flip)

    for x, y in poly_mask(SIZE, pts([(0, 1), (-4, 8), (0, 6), (3, 9), (5, 4), (4, 1)])):
        L.put(x, y, WING[0])
    tail = pts([(6, 0), (11, 2), (15, 1), (19, -2)])
    for (ax, ay), (bx, by) in zip(tail, tail[1:]):
        for x, y in line_points(round(ax), round(ay), round(bx), round(by)):
            L.put(x, y, DRAKE[2])
    tx, ty = tail[-1]
    for dx, dy in ((0, 0), (1, 0), (0, -1), (1, -1), (2, -1), (1, -2)):
        L.put(round(tx) + (-dx if flip else dx), round(ty) + dy, DRAKE[1])
    body = poly_mask(SIZE, pts([(-7, -2), (-2, -3), (4, -2), (7, 0), (4, 2), (-2, 2), (-7, 1)]))
    head = poly_mask(SIZE, pts([(-15, -2), (-12, -5), (-9, -4), (-7, -2), (-9, 0), (-13, 0)]))
    blob(L, body | head, DRAKE, 2)
    near = [(-2, -3), (3, -10), (-4, -15), (0, -13), (3, -16), (6, -12), (10, -14), (10, -8), (13, -6), (5, -3)]
    for x, y in poly_mask(SIZE, pts(near)):
        L.put(x, y, WING[2] if y < oy - 9 * s else WING[1])
    bones_of(L, [((-2, -3), (3, -10)), ((3, -10), (-4, -15)), ((3, -10), (3, -16)), ((3, -10), (10, -14)),
                 ((3, -10), (13, -6))], ox, oy, DRAKE[3], s, flip)
    (hx, hy), (ex, ey) = pts([(-11, -6), (-11, -3)])
    L.put(round(hx), round(hy), BONE[3])
    L.put(round(ex), round(ey), EYE)


def build():
    canvas = Image.new('RGBA', SIZE)
    # The great summit stands behind; the needles' shadowed faces fall against its sunlit wall.
    for n in (3, 0, 1, 2):
        points, creases, tones = PEAKS[n]
        granite(canvas, points, creases, tones, 1 if n == 0 else 0, 80 + n)
    ledge(canvas)
    cleft(canvas)
    talus(canvas)
    drakes = Layer(*SIZE)
    perched_dragon(drakes, 145, 104)
    flying_dragon(drakes, 145, 34, 1.0)
    flying_dragon(drakes, 26, 34, 0.9, True)
    composite(canvas, drakes, OUT)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/dragonspine-entrance.bmp')
