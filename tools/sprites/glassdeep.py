#!/usr/bin/env python3
"""Glassdeep Caverns entrance: a crystal-studded outcrop at the 24px tile scale."""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, composite, poly_mask, save_sprite, stamp, LANTERN

SIZE = (168, 144)
OUT = (15, 16, 22)
ROCK = [(33, 33, 42), (48, 47, 57), (67, 64, 72), (89, 84, 88), (114, 106, 104), (143, 134, 124)]
CYAN = [(18, 58, 80), (30, 112, 140), (62, 176, 200), (156, 236, 244)]
AQUA = [(16, 70, 68), (30, 128, 116), (70, 200, 172), (174, 248, 222)]
VIOLET = [(50, 32, 92), (94, 62, 156), (148, 110, 214), (216, 186, 250)]
MAGENTA = [(82, 26, 74), (150, 52, 132), (218, 104, 192), (252, 194, 236)]
GLINT = (240, 255, 255)
WOOD = [(40, 29, 24), (70, 48, 32), (104, 72, 42), (142, 104, 60)]
IRON = [(30, 32, 36), (72, 76, 80), (128, 132, 130)]
DARK = [(8, 9, 15), (16, 17, 24), (26, 26, 33), (37, 36, 42)]
GLOW = [(12, 30, 40), (16, 46, 58)]
LIGHT = (-0.55, -0.62, 0.56)
BAYER = ((0.125, 0.625), (0.875, 0.375))


def blob(cx, cy, rx, ry, seed, flat=143):
    """A lumpy rounded outline, cut flat at the ground line."""
    rng = random.Random(seed)
    waves = [(k, rng.uniform(0.02, 0.05), rng.uniform(0, 6.28)) for k in (3, 5, 7)]
    pts = []
    for i in range(48):
        a = i / 48.0 * 2 * math.pi
        r = 1 + sum(amp * math.sin(k * a + ph) for k, amp, ph in waves)
        pts.append((cx + math.cos(a) * rx * r, min(flat, cy + math.sin(a) * ry * r)))
    return pts


def brightness(nx, ny):
    """Lambert term for a surface normal given by its screen-plane part."""
    if ny > 0:
        ny *= 0.35
    d = nx * nx + ny * ny
    if d > 0.95:
        k = math.sqrt(d / 0.95)
        nx, ny, d = nx / k, ny / k, 0.95
    n = math.sqrt(sum(c * c for c in LIGHT))
    return (nx * LIGHT[0] + ny * LIGHT[1] + math.sqrt(1 - d) * LIGHT[2]) / n


def rock_mass(canvas, mask, normal, seed, cell=16):
    """Rounded rock lit from the upper left, broken into broad layered slabs."""
    rng = random.Random(seed)
    xs = [x for x, _ in mask]
    ys = [y for _, y in mask]
    seeds = []
    for gy in range(min(ys) - cell, max(ys) + cell, cell // 2):
        for gx in range(min(xs) - cell, max(xs) + cell, cell):
            seeds.append((gx + rng.randrange(cell), gy + rng.randrange(cell // 2), rng.choice((-1, 0, 0, 0, 0, 1)), rng.random()))
    shade = {}
    owner = {}
    for x, y in sorted(mask):
        best = min(range(len(seeds)), key=lambda i: (seeds[i][0] - x) ** 2 + 3 * (seeds[i][1] - y) ** 2)
        owner[(x, y)] = best
        if best not in shade:
            sx, sy, jitter, _ = seeds[best]
            b = brightness(*normal(sx, sy))
            idx = 1 if b < 0.25 else 2 if b < 0.55 else 3 if b < 0.8 else 4
            shade[best] = max(1, min(4, idx + jitter))
    layer = Layer(*SIZE)
    for x, y in sorted(mask):
        o = owner[(x, y)]
        idx = shade[o]
        below = owner.get((x, y + 1), o)
        above = owner.get((x, y - 1), o)
        if below != o and seeds[o][3] < 0.25:
            idx = max(1, shade[o] - 2)
        elif above != o and shade[above] < idx and idx < 4:
            idx += 1
        layer.put(x, y, ROCK[max(0, min(5, idx))])
    composite(canvas, layer, OUT)


def ellipse_normal(cx, cy, rx, ry):
    return lambda x, y: ((x - cx) / rx, (y - cy) / ry)


def outcrop(canvas, cx, cy, rx, ry, seed, cell):
    rock_mass(canvas, poly_mask(SIZE, blob(cx, cy, rx, ry, seed)), ellipse_normal(cx, cy, rx, ry), seed, cell)


def boulders(canvas, specs):
    """Small smooth boulders lit from the upper left."""
    layer = Layer(*SIZE)
    for cx, cy, rx, ry in specs:
        for x, y in sorted(poly_mask(SIZE, blob(cx, cy, rx, ry, cx * 7 + cy))):
            b = brightness((x + 0.5 - cx) / rx, (y + 0.5 - cy) / ry)
            layer.put(x, y, ROCK[1 if b < 0.3 else 2 if b < 0.6 else 3 if b < 0.85 else 4])
    composite(canvas, layer, OUT)


def mouth_half(y):
    """Half width of the dark cave opening at row y (centre column 84)."""
    if y < 100:
        return 15 * math.sqrt(max(0.0, 1 - ((100 - y) / 20.0) ** 2))
    return 15 - (y - 100) * 3 / 43.0


def crystal(L, bx, by, angle, length, width, ramp):
    """A pointed hexagonal prism growing from (bx, by), lit from the left."""
    a = math.radians(angle)
    dx, dy = math.sin(a), -math.cos(a)
    px, py = math.cos(a), math.sin(a)
    hw = width / 2.0
    tip = max(2.0, width * 0.8)
    for y in range(int(by - length - 2), int(by + 3)):
        for x in range(int(bx - length - 2), int(bx + length + 3)):
            rx, ry = x + 0.5 - bx, y + 0.5 - by
            u = rx * dx + ry * dy
            v = rx * px + ry * py
            if u < -0.5 or u > length:
                continue
            lim = hw if u <= length - tip else hw * (length - u) / tip
            if abs(v) > lim:
                continue
            t = v / hw
            if u > length - tip:
                c = ramp[3] if t < 0 else ramp[2]
            elif t < -0.34:
                c = ramp[3]
            elif t < 0.3:
                c = ramp[2]
            else:
                c = ramp[1]
            if u < 2.5:
                c = ramp[0] if t > -0.34 else ramp[1]
            L.put(x, y, c)
    gx = bx + dx * (length - tip - 1) - px * (hw - 1.5)
    gy = by + dy * (length - tip - 1) - py * (hw - 1.5)
    L.put(gx, gy, GLINT)


def crystal_glow(canvas, lights, tint, k=0.2):
    """Tint rock near crystals towards tint: one tinted tone per rock shade, ordered-dithered falloff."""
    ramp = {c: tuple(int(c[i] + (tint[i] - c[i]) * k) for i in range(3)) for c in ROCK}
    cp = canvas.load()
    w, h = canvas.size
    for lx, ly, r in lights:
        for y in range(ly - r, ly + r + 1):
            for x in range(lx - r, lx + r + 1):
                if not (0 <= x < w and 0 <= y < h):
                    continue
                d = math.hypot(x - lx, y - ly)
                if d > r:
                    continue
                if min(1.0, (1 - d / r) / 0.6) <= BAYER[y % 2][x % 2]:
                    continue
                c = cp[x, y][:3]
                if c in ramp:
                    cp[x, y] = ramp[c] + (255,)


def cluster(canvas, specs):
    layer = Layer(*SIZE)
    for spec in specs:
        crystal(layer, *spec)
    composite(canvas, layer, OUT)


def build():
    canvas = Image.new('RGBA', SIZE)
    rng = random.Random(97)
    # A broad rounded hill with two lower shoulders, all in layered rock slabs.
    outcrop(canvas, 84, 132, 60, 104, 11, 26)
    outcrop(canvas, 31, 150, 27, 78, 12, 16)
    outcrop(canvas, 138, 150, 26, 86, 13, 16)
    # An irregular rocky brow and jambs ring the cave mouth.
    lip = set()
    edge = random.Random(5)
    thick = 6
    for y in range(66, 144):
        if edge.random() < 0.35:
            thick = max(4, min(9, thick + edge.choice((-1, 1))))
        half = mouth_half(max(y, 80)) if y >= 80 else 0
        reach = half + thick + (6 if y < 100 else 4)
        top = 80 - thick - 4
        for x in range(50, 119):
            dx = x - 84
            if y >= 80:
                inside = abs(dx) <= reach
            else:
                inside = (dx / 22.0) ** 2 + ((y - 84) / (84 - top)) ** 2 <= 1
            if inside:
                lip.add((x, y))
    rock_mass(canvas, lip, lambda x, y: ((x - 84) / 26.0, (y - 100) / 30.0), 21, 10)
    # The dark cave mouth, with a gravel floor rising into the deep.
    cave = Layer(*SIZE)
    # The opening is centred on the column line x = 84, so its width is even and ends at 72-95.
    for y in range(80, 144):
        half = mouth_half(y)
        for x in range(60, 108):
            if abs(x + 0.5 - 84) > half:
                continue
            k = 0 if y < 110 else 1 if y < 128 else 2
            if y in (110, 128) and (x + y) % 2:
                k -= 1
            c = DARK[k]
            g = math.hypot(x + 0.5 - 84, (y - 106) * 2.2)
            if g < 12 and y < 112 and (g < 10 or (x + y) % 2 == 0):
                c = GLOW[1] if g < 5 or (g < 7 and (x + y) % 2 == 0) else GLOW[0]
            cave.put(x, y, c)
    composite(canvas, cave, None)
    # Rails run out of the dark onto the approach.
    rail = Layer(*SIZE)
    for y in range(108, 144):
        f = (y - 106) / 37.0
        if (y - 106) % max(2, int(2 + f * 4)) == 0:
            for x in range(80 - int(7 * f), 88 + int(7 * f)):
                rail.put(x, y, WOOD[2] if y > 120 else WOOD[1])
    for y in range(108, 144):
        f = (y - 106) / 37.0
        for x in (81 - int(5 * f), 86 + int(5 * f)):
            rail.put(x, y, IRON[2] if y > 118 else IRON[1])
    composite(canvas, rail, None)
    # Stalactite teeth hang from the upper rim.
    teeth = Layer(*SIZE)
    for x, top, length in ((72, 89, 8), (76, 84, 6), (80, 81, 11), (85, 80, 7), (89, 81, 12),
                           (93, 84, 6), (95, 89, 8)):
        for k in range(length):
            w = 3 if k < length // 3 else 2 if k < length * 2 // 3 else 1
            for i in range(w):
                teeth.put(x - 1 + i, top + k, ROCK[4] if i == 0 else ROCK[2])
    composite(canvas, teeth, OUT)
    # Rubble, dressed quarry blocks and boulders fill the foot of the hill.
    boulders(canvas, [(10, 137, 8, 6), (24, 140, 8, 5), (160, 139, 5, 4),
                      (63, 139, 7, 5), (106, 139, 7, 5), (38, 141, 6, 4)])
    scree = Layer(*SIZE)
    for i in range(16):
        x, y = rng.randrange(4, 160), rng.randrange(131, 141)
        if 64 <= x <= 99:
            continue
        for px, py in poly_mask(SIZE, [(x, y), (x + 3, y - 2), (x + 6, y + 1), (x + 4, y + 3), (x, y + 2)]):
            scree.put(px, py, ROCK[4] if py <= y else ROCK[2])
    composite(canvas, scree, OUT)
    blocks = Layer(*SIZE)
    for x0, y0, w, h in ((134, 133, 13, 10), (147, 135, 11, 8), (140, 123, 12, 10)):
        for y in range(y0, y0 + h):
            for x in range(x0, x0 + w):
                k = 4 if y < y0 + 3 else 2 if x >= x0 + w - 3 else 3
                if y == y0 + 3:
                    k = 2
                blocks.put(x, y, ROCK[k])
        blocks.put(x0 + 3, y0 + 6, ROCK[3])
        blocks.put(x0 + 4, y0 + 7, ROCK[3])
    composite(canvas, blocks, OUT)
    # Quarry post with a hanging lantern beside the mouth.
    timber = Layer(*SIZE)
    for y in range(94, 141):
        for w in range(4):
            timber.put(60 + w, y, WOOD[3 if w == 0 else 1 if w == 3 else 2])
    for x in range(58, 68):
        for h in range(3):
            timber.put(x, 94 + h, WOOD[3 if h == 0 else 1 if h == 2 else 2])
    timber.put(67, 97, IRON[1])
    timber.put(67, 98, IRON[2])
    stamp(timber, 65, 99, LANTERN, {'i': IRON[0], 'I': IRON[1], 'y': (214, 150, 66), 'Y': (250, 206, 110), 'W': (255, 246, 196)})
    composite(canvas, timber, OUT)
    # Crystal light tints the stone around each cluster before the crystals grow.
    crystal_glow(canvas, ((67, 104, 9),), (232, 168, 92))
    crystal_glow(canvas, ((118, 118, 16), (122, 88, 5)), (170, 120, 230))
    crystal_glow(canvas, ((46, 118, 16), (97, 40, 9), (22, 84, 5), (60, 74, 5)), (90, 210, 230))
    cluster(canvas, [(42, 132, -18, 28, 7, CYAN), (47, 132, 4, 36, 8, CYAN), (52, 133, 30, 22, 6, AQUA),
                     (38, 134, -48, 16, 5, VIOLET), (55, 136, 58, 12, 5, CYAN)])
    cluster(canvas, [(118, 132, 8, 32, 8, VIOLET), (113, 133, -24, 22, 6, MAGENTA), (123, 133, 34, 24, 6, VIOLET),
                     (109, 136, -55, 12, 5, CYAN)])
    cluster(canvas, [(96, 37, 12, 18, 6, CYAN), (101, 38, 40, 13, 5, VIOLET), (91, 38, -24, 11, 5, AQUA)])
    cluster(canvas, [(20, 80, -10, 12, 5, AQUA), (25, 80, 28, 9, 4, CYAN)])
    cluster(canvas, [(58, 70, -30, 9, 4, CYAN), (62, 70, 8, 7, 3, VIOLET)])
    cluster(canvas, [(123, 84, 26, 9, 4, VIOLET), (120, 84, -12, 6, 3, MAGENTA)])
    cluster(canvas, [(80, 108, -10, 6, 3, CYAN), (84, 108, 6, 8, 4, AQUA), (88, 109, 30, 5, 3, VIOLET)])
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/glassdeep-entrance.bmp')
