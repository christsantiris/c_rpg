#!/usr/bin/env python3
"""Ashen Hollow volcanic entrance in the established 24px pixelkit pipeline."""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, composite, line_points, save_sprite, stamp

SIZE = (168, 144)
OUT = (16, 11, 12)
ROCK = [(37, 32, 34), (52, 45, 46), (70, 61, 60), (92, 81, 76), (118, 105, 95)]
OBSIDIAN = [(13, 11, 17), (24, 21, 30), (37, 33, 45), (55, 50, 66), (84, 78, 98)]
LAVA = [(96, 20, 16), (152, 36, 18), (210, 72, 22), (244, 132, 34), (255, 200, 72), (255, 244, 180)]
SMOKE = [(30, 30, 38), (48, 47, 56), (70, 69, 78), (98, 96, 104), (132, 130, 136)]
IRON = [(26, 25, 28), (56, 54, 58), (92, 89, 92), (138, 132, 128)]
GLOW = (255, 110, 30)
HEAT = [(62, 31, 25), (90, 52, 40), (122, 66, 42), (156, 92, 58)]
EMBER_SMOKE = {-1: (86, 58, 58), -2: (124, 62, 40)}
MOUTH = [(20, 9, 11), (58, 17, 14)]
CX = 84


def cone_mask(cx, top, base, chw, bhw, p):
    mask = set()
    for y in range(top, base + 1):
        s = (y - top) / float(base - top)
        hw = chw + (bhw - chw) * s ** p
        for x in range(math.ceil(cx - hw), math.floor(cx + hw) + 1):
            if 0 <= x < SIZE[0]:
                mask.add((x, y))
    # The back lip of the crater rises a little above the flat summit.
    for y in range(top - 3, top + 1):
        for x in range(cx - chw, cx + chw + 1):
            if ((x - cx) / float(chw)) ** 2 + ((y - top) / 3.2) ** 2 <= 1.0:
                mask.add((x, y))
    return mask


def cone(canvas, cx, top, base, chw, bhw, seed, p=1.9):
    """A squat volcanic cone, lit from the upper left, scored by radial gullies."""
    rng = random.Random(seed)
    rock = Layer(*SIZE)
    mask = cone_mask(cx, top, base, chw, bhw, p)
    for x, y in sorted(mask):
        s = max(0.0, (y - top) / float(base - top))
        hw = chw + (bhw - chw) * s ** p
        t = (x - cx) / hw
        shade = 3 if t < -0.55 else 2 if t < -0.1 else 1 if t < 0.5 else 0
        if y <= top:
            shade = 2 if x < cx else 1
        if rng.randrange(29) == 0:
            shade = max(0, min(3, shade + rng.choice((-1, 1))))
        rock.put(x, y, ROCK[shade])
    # A few long gullies fan out from the summit to the foot.
    for k in (-0.8, -0.45, -0.15, 0.2, 0.5, 0.82):
        y = top + 2 + rng.randrange(0, 5)
        jog = 0
        while y < base:
            s = (y - top) / float(base - top)
            hw = chw + (bhw - chw) * s ** p
            if rng.randrange(7) == 0:
                jog = max(-1, min(1, jog + rng.choice((-1, 1))))
            nx = int(round(cx + k * hw)) + jog
            if (nx, y) in mask:
                rock.put(nx, y, ROCK[0] if k > 0 else ROCK[1])
                if (nx - 1, y) in mask:
                    rock.put(nx - 1, y, ROCK[3] if k < 0 else ROCK[2])
            y += 1
    composite(canvas, rock, OUT)
    return mask


def stroke(points, w0, w1):
    """Pixels within a widening ribbon along a polyline: (x, y) -> distance / half width."""
    segs = list(zip(points, points[1:]))
    total = sum(math.hypot(b[0] - a[0], b[1] - a[1]) for a, b in segs)
    out = {}
    run = 0.0
    for a, b in segs:
        seg = math.hypot(b[0] - a[0], b[1] - a[1])
        n = max(1, int(seg * 2))
        for i in range(n + 1):
            f = i / float(n)
            x = a[0] + (b[0] - a[0]) * f
            y = a[1] + (b[1] - a[1]) * f
            hw = (w0 + (w1 - w0) * (run + seg * f) / total) / 2.0
            for py in range(int(y - hw) - 1, int(y + hw) + 2):
                for px in range(int(x - hw) - 1, int(x + hw) + 2):
                    d = math.hypot(px + 0.5 - x, py + 0.5 - y) / hw
                    if d <= 1.0 and d < out.get((px, py), 9):
                        out[(px, py)] = d
        run += seg
    return out


def molten(L, pix, rng, crust=0):
    """Shade lava: white-hot core, orange body, red cooling edge, a few crust plates."""
    for (x, y), d in sorted(pix.items()):
        c = LAVA[4] if d < 0.3 else LAVA[3] if d < 0.62 else LAVA[2] if d < 0.86 else LAVA[1]
        L.put(x, y, c)
    keys = sorted(pix)
    for n in range(crust):
        x, y = rng.choice(keys)
        if not 0.35 < pix[(x, y)] < 0.8:
            continue
        for dx, dy in ((0, 0), (1, 0)):
            if (x + dx, y + dy) in pix:
                L.put(x + dx, y + dy, LAVA[0] if dx else LAVA[1])


def crater(L, cx, top, chw):
    for y in range(top - 2, top + 2):
        for x in range(cx - chw, cx + chw + 1):
            e = ((x - cx) / (chw - 2.0)) ** 2 + ((y - top + 0.5) / 2.0) ** 2
            if e <= 1.0:
                L.put(x, y, LAVA[5] if e < 0.3 else LAVA[4] if e < 0.7 else LAVA[3])


def band(v, cuts, x, y):
    """Index of v among descending cuts, checker-dithered within 0.12 of each cut."""
    odd = (x + y) & 1
    for i, cut in enumerate(cuts):
        if abs(v - cut) < 0.12:
            return len(cuts) - i - (0 if odd else 1)
        if v > cut:
            return len(cuts) - i
    return 0


def plume(canvas, lobes, seed, fire_y):
    """A billowing ash column: lumpy lobes merged into one soft mass, fire-lit at its root."""
    rng = random.Random(seed)
    cloud = Layer(*SIZE)
    for cx, cy, r in lobes:
        L = Layer(*SIZE)
        phase = rng.uniform(0, 6.28)
        for y in range(cy - r - 2, cy + r + 2):
            for x in range(cx - r - 4, cx + r + 5):
                dx = (x + 0.5 - cx) / (r + 1.5)
                dy = (y + 0.5 - cy) / float(r)
                a = math.atan2(dy, dx)
                rr = 1.0 + 0.07 * math.sin(3 * a + phase) + 0.04 * math.sin(5 * a + phase * 2)
                if dx * dx + dy * dy > rr * rr:
                    continue
                lit = -dx * 0.65 - dy * 0.75
                shade = 1 + band(lit, (0.5, -0.15), x, y)
                if dx > -0.3 and band(dy, (0.6,), x, y):
                    shade = 0
                if y > fire_y - 5 and dy > 0.0 and (y > fire_y - 4 or (x + y) & 1):
                    shade = -1
                if y > fire_y - 3 and dy > 0.15:
                    shade = -2
                L.put(x, y, SMOKE[shade] if shade >= 0 else EMBER_SMOKE[shade])
        # A soft crescent of highlight along the lit upper-left rim.
        for y in range(cy - r, cy):
            for x in range(cx - r - 2, cx + 1):
                if L.on(x, y) and L.get(x, y) == SMOKE[3] and not L.on(x - 2, y - 1):
                    L.put(x, y, SMOKE[4])
        composite(cloud.im, L, None)
    # The column stands in front of the crater's back lip; its root glows straight
    # out of the lava, so no outline is drawn over molten pixels.
    keep = canvas.copy()
    before = keep.load()
    composite(canvas, cloud, OUT)
    cp = canvas.load()
    molten = set(LAVA)
    for y in range(SIZE[1]):
        for x in range(SIZE[0]):
            if cp[x, y] == OUT + (255,) and before[x, y][3] and before[x, y][:3] in molten:
                cp[x, y] = before[x, y]


def heat(canvas, hot, rocks, reach=3):
    """Warm the rock that borders molten pixels, strongest right at the edge."""
    cp = canvas.load()
    w, h = SIZE
    tinted = {}
    for (x, y) in sorted(hot):
        for dy in range(-reach, reach + 1):
            for dx in range(-reach, reach + 1):
                px, py = x + dx, y + dy
                if not (0 <= px < w and 0 <= py < h):
                    continue
                d = math.hypot(dx, dy)
                if d <= reach + 0.5 and d < tinted.get((px, py), 9):
                    tinted[(px, py)] = d
    for (x, y), d in sorted(tinted.items()):
        c = cp[x, y][:3]
        if cp[x, y][3] == 0 or c not in rocks:
            continue
        k = 0.4 if d < 1.5 else 0.2
        warm = tuple(c[i] + (GLOW[i] - c[i]) * k for i in range(3))
        cp[x, y] = min(HEAT, key=lambda h: sum((h[i] - warm[i]) ** 2 for i in range(3))) + (255,)


def lump(L, cx, cy, rx, ry, pal, seed):
    """A rounded boulder of cooled lava, lit on its upper left."""
    phase = random.Random(seed).uniform(0, 6.28)
    for y in range(cy - ry - 1, cy + ry + 2):
        for x in range(cx - rx - 1, cx + rx + 2):
            dx = (x + 0.5 - cx) / float(rx)
            dy = (y + 0.5 - cy) / float(ry)
            rr = 1.0 + 0.1 * math.sin(5 * math.atan2(dy, dx) + phase)
            if dx * dx + dy * dy > rr * rr:
                continue
            lit = -dx * 0.6 - dy * 0.8
            L.put(x, y, pal[4] if lit > 0.9 else pal[3] if lit > 0.5 else pal[2] if lit > -0.15 else pal[1])


def pool(L, cx, cy, rx, ry, seed):
    """A lava pool in a cooled crust basin; returns the molten pixels."""
    rng = random.Random(seed)
    phase = rng.uniform(0, 6.28)
    hot = set()
    for y in range(cy - ry - 4, cy + ry + 5):
        for x in range(cx - rx - 5, cx + rx + 6):
            dx = (x + 0.5 - cx) / float(rx)
            dy = (y + 0.5 - cy) / float(ry)
            a = math.atan2(dy, dx)
            wob = 1.0 + 0.08 * math.sin(3 * a + phase) + 0.05 * math.sin(7 * a)
            e_in = math.hypot(dx, dy * 1.05) / wob
            e_out = math.hypot((x + 0.5 - cx) / (rx + 3.5), (y + 0.5 - cy - 0.8) / (ry + 3.2)) / wob
            if e_in <= 1.0:
                c = LAVA[5] if e_in < 0.18 else LAVA[4] if e_in < 0.4 else LAVA[3] if e_in < 0.72 else LAVA[2]
                L.put(x, y, c)
                hot.add((x, y))
            elif e_out <= 1.0:
                L.put(x, y, OBSIDIAN[3] if dy < -0.2 and dx < 0.3 else OBSIDIAN[2] if dy < 0.3 else OBSIDIAN[1])
    keys = sorted(hot)
    for n in range(len(keys) // 30):
        x, y = rng.choice(keys)
        if (x + 1, y) in hot and (x - 1, y) in hot:
            L.put(x, y, LAVA[1])
            L.put(x + 1, y, LAVA[1])
    return hot


def build():
    canvas = Image.new('RGBA', SIZE)
    rng = random.Random(97)
    # Two lesser cones stand behind the great central volcano.
    left = cone(canvas, 30, 84, 136, 6, 29, 11, p=1.25)
    right = cone(canvas, 138, 80, 136, 6, 28, 12, p=1.25)
    centre = cone(canvas, CX, 50, 140, 17, 80, 13, p=2.1)
    # Lava fills each crater and pours down the cracked flanks.
    lava = Layer(*SIZE)
    for cx, top, chw in ((30, 84, 6), (138, 80, 6), (CX, 50, 17)):
        crater(lava, cx, top, chw)
    flows = [([(31, 85), (34, 95), (30, 107), (33, 119), (31, 130)], 3, 6, left),
             ([(137, 81), (134, 92), (138, 104), (135, 117), (136, 130)], 3, 6, right),
             ([(73, 51), (69, 62), (64, 72), (61, 88), (55, 104), (49, 118), (42, 132)], 4, 9, centre),
             ([(95, 51), (99, 62), (104, 72), (107, 88), (113, 104), (119, 118), (126, 132)], 4, 9, centre),
             ([(80, 52), (78, 62), (82, 72), (79, 84)], 3, 5, centre),
             ([(89, 52), (92, 60), (90, 72)], 2, 4, centre)]
    hot = set()
    fissures = set()
    for points, w0, w1, mask in flows:
        pix = {p: d for p, d in stroke(points, w0, w1).items() if p in mask}
        molten(lava, pix, rng, crust=len(pix) // 45)
        hot |= set(pix)
    composite(canvas, lava, OUT)
    # Glowing fissures split the cooled crust between the flows.
    cracks = Layer(*SIZE)
    for x, y, steps in ((64, 98, 6), (106, 96, 6), (10, 102, 6), (158, 98, 6), (70, 74, 5), (99, 74, 5), (47, 78, 4), (122, 82, 4)):
        for n in range(steps):
            nx = x + rng.choice((-2, -1, 1, 2))
            ny = y + rng.randrange(2, 4)
            for px, py in line_points(x, y, nx, ny):
                if (px, py) in centre or (px, py) in left or (px, py) in right:
                    cracks.put(px, py, LAVA[2] if n % 3 else LAVA[3])
                    fissures.add((px, py))
            x, y = nx, ny
    composite(canvas, cracks, None)
    heat(canvas, hot, set(ROCK), reach=2)
    heat(canvas, fissures, set(ROCK), reach=1)
    # Lava pools catch the flows; cooled boulders fill the outer corners.
    ground = Layer(*SIZE)
    pool_hot = pool(ground, 33, 133, 17, 5, 5) | pool(ground, 135, 133, 17, 5, 6)
    composite(canvas, ground, OUT)
    rubble = Layer(*SIZE)
    for cx, cy, rx, ry, seed in ((9, 131, 7, 9, 1), (16, 139, 7, 4, 2), (158, 130, 7, 10, 3), (152, 139, 7, 4, 4),
                                 (55, 139, 6, 4, 5), (113, 139, 6, 4, 6)):
        lump(rubble, cx, cy, rx, ry, ROCK, seed)
    composite(canvas, rubble, OUT)
    # Stepped basalt columns crowd around the cave mouth.
    basalt = Layer(*SIZE)
    for i, top in enumerate((117, 108, 96, 91, 86, 87, 89, 98, 106, 119)):
        x0 = 55 + i * 6
        top += rng.randrange(-1, 2)
        joints = {top + rng.randrange(8, 14), top + rng.randrange(19, 27)}
        for y in range(top, 144):
            for k in range(6):
                c = OBSIDIAN[4] if k == 0 else OBSIDIAN[3] if k < 3 else OBSIDIAN[2] if k < 5 else OBSIDIAN[0]
                if y == top and k in (0, 5):
                    continue
                if y < top + 2:
                    c = OBSIDIAN[4] if k < 4 else OBSIDIAN[3]
                elif y == top + 2:
                    c = OBSIDIAN[1]
                elif y in joints:
                    c = OBSIDIAN[0]
                elif y - 1 in joints and k < 5:
                    c = OBSIDIAN[3]
                basalt.put(x0 + k, y, c)
    composite(canvas, basalt, OUT)
    # The cave mouth: dark at the lip, molten light rising from deep inside.
    mouth = Layer(*SIZE)
    edge = set()
    for y in range(98, 144):
        half = 12.5 if y >= 110 else math.sqrt(max(0.0, 12.5 ** 2 - (110 - y) ** 2))
        for x in range(CX - 12, CX + 13):
            if abs(x - CX) > half:
                continue
            c = MOUTH[0]
            if y >= 118:
                g = math.hypot((x - CX) / 9.0, (y - 129) / 7.0)
                c = LAVA[3] if g < 0.45 else LAVA[2] if g < 0.7 else LAVA[1] if g < 0.9 else MOUTH[1] if g < 1.1 else c
                if 1.1 <= g < 1.3 and (x + y) & 1:
                    c = MOUTH[1]
            mouth.put(x, y, c)
            if abs(x - CX) > half - 1.5:
                edge.add((x, y))
    for n, y in enumerate((128, 132, 136, 140)):
        half = 7 + n * 2
        for x in range(CX - half, CX + half + 1):
            mouth.put(x, y, HEAT[3] if n == 0 else OBSIDIAN[3] if n < 3 else OBSIDIAN[2])
            mouth.put(x, y + 1, OBSIDIAN[2])
            mouth.put(x, y + 2, OBSIDIAN[1])
            mouth.put(x, y + 3, OBSIDIAN[0])
    composite(canvas, mouth, None)
    # An iron forge plate with an anvil sigil is bolted above the mouth.
    iron = Layer(*SIZE)
    for y in range(83, 96):
        for x in range(CX - 9, CX + 10):
            c = IRON[2] if x == CX - 9 or y == 83 else IRON[0] if x == CX + 9 or y == 95 else IRON[1]
            iron.put(x, y, c)
    for x, y in ((CX - 7, 85), (CX + 7, 85), (CX - 7, 93), (CX + 7, 93)):
        iron.put(x, y, IRON[3])
    stamp(iron, CX - 6, 87, ['aaaaaaaaaaaa.',
                             '.AAAAAAAAAa..',
                             '...aAAAa.....',
                             '....AAa......',
                             '..aAAAAAa....',
                             '..aaaaaaa....'],
          {'a': LAVA[2], 'A': LAVA[4]})
    composite(canvas, iron, OUT)
    heat(canvas, {(x, y) for x, y in edge if y >= 112} | pool_hot, set(OBSIDIAN) | set(ROCK), reach=2)
    # Ash columns rise out of the three crater pools, drifting east.
    plume(canvas, [(107, 11, 7), (94, 11, 8), (81, 15, 6), (101, 19, 8), (88, 23, 9), (78, 31, 7),
                   (91, 35, 7), (84, 44, 5)], 1, 50)
    plume(canvas, [(54, 47, 2), (48, 51, 4), (43, 55, 5), (40, 61, 4), (36, 66, 4), (33, 72, 3), (31, 77, 3), (30, 81, 2)], 2, 84)
    plume(canvas, [(162, 41, 2), (159, 45, 4), (154, 49, 5), (150, 55, 5), (146, 60, 4), (143, 66, 4), (140, 71, 3),
                   (139, 75, 3), (138, 78, 2)], 3, 80)
    # Embers drift up through the ash.
    embers = Layer(*SIZE)
    shapes = {'s': [(0, 0, 4)], 'v': [(0, 0, 4), (0, 1, 3)], 'h': [(0, 0, 4), (1, 0, 3)], 'd': [(0, 0, 3)]}
    for x, y, kind in ((70, 38, 'v'), (101, 29, 's'), (86, 14, 'h'), (112, 12, 'v'), (67, 24, 's'),
                       (52, 66, 'd'), (131, 60, 'v'), (118, 40, 'd'), (96, 4, 'd'), (26, 64, 's')):
        for dx, dy, i in shapes[kind]:
            embers.put(x + dx, y + dy, LAVA[i])
    composite(canvas, embers, None)
    # Trim one-pixel outline spurs left where shapes meet at a single point.
    cp = canvas.load()
    spurs = []
    for y in range(SIZE[1]):
        for x in range(SIZE[0]):
            if cp[x, y] != OUT + (255,):
                continue
            n = sum(1 for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))
                    if 0 <= x + dx < SIZE[0] and 0 <= y + dy < SIZE[1] and cp[x + dx, y + dy][3])
            if n <= 1:
                spurs.append((x, y))
    for x, y in spurs:
        cp[x, y] = (0, 0, 0, 0)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/ashen-entrance.bmp')
