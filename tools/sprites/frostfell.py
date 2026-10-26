#!/usr/bin/env python3
"""Frostfell Wastes entrance: a glacier ice cave in the 24px pixelkit pipeline."""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, composite, line_points, poly_mask, save_sprite, stamp

SIZE = (168, 144)
CX = 84  # the door is centred between columns 83 and 84 (pixel-centre coordinates x + 0.5)
OUT = (14, 22, 36)
ICE = [(36, 60, 96), (52, 88, 132), (76, 124, 168), (112, 162, 200), (160, 204, 228), (214, 238, 248)]
SNOW = [(120, 146, 182), (156, 184, 212), (194, 214, 234), (226, 237, 247), (248, 251, 253)]
PINE = [(12, 30, 32), (20, 46, 44), (30, 64, 56), (46, 86, 70)]
BARK = [(40, 32, 32), (66, 52, 44)]
ROCK = [(42, 48, 64), (66, 74, 92), (96, 104, 122), (132, 140, 156)]
CAVE = [(6, 12, 24), (12, 24, 42)]
INK = [(46, 54, 104), (92, 96, 160)]
GLOW = (150, 238, 240)


def shard(canvas, bx, by, length, hw, angle, chisel, seed):
    """A faceted ice spire rising from (bx, by), tilted by angle degrees, with a chisel tip.

    Coverage is tested at four sub-pixel points so the tilted edges step evenly.
    """
    rng = random.Random(seed)
    layer = Layer(*SIZE)
    a = math.radians(angle)
    ux, uy = math.sin(a), -math.cos(a)
    vx, vy = math.cos(a), math.sin(a)
    tip = hw * 1.8

    def sample(px, py):
        dx = px - bx
        dy = py - by
        u = dx * ux + dy * uy
        v = dx * vx + dy * vy
        if u < -6 or u > length:
            return None
        f = min(1.0, (length - u) / tip)
        half = hw * f
        vc = chisel * hw * (1 - f)
        if abs(v - vc) > half:
            return None
        return u, f, (v - vc) / max(half, 0.5)

    for y in range(SIZE[1]):
        for x in range(SIZE[0]):
            hits = [s for s in (sample(x + sx, y + sy) for sx in (0.25, 0.75) for sy in (0.25, 0.75)) if s]
            if len(hits) < 2:
                continue
            u = sum(h[0] for h in hits) / len(hits)
            f = sum(h[1] for h in hits) / len(hits)
            t = sum(h[2] for h in hits) / len(hits)
            idx = 4 if t < -0.4 else 3 if t < 0.25 else 1
            if f < 1 and idx == 4:
                idx = 5
            elif idx == 3 and -0.15 < t < 0.0 and u > length * 0.35:
                idx = 4
            if u < length * 0.3 and idx > 1:
                idx -= 1
            layer.put(x, y, ICE[idx])
    for n in range(2):
        u = rng.uniform(length * 0.35, length - tip - 4)
        v = rng.uniform(-hw * 0.5, 0)
        x0 = round(bx + u * ux + v * vx)
        y0 = round(by + u * uy + v * vy)
        for px, py in line_points(x0, y0, x0 + hw // 2, y0 - rng.randrange(2, 5)):
            if layer.on(px, py) and layer.get(px, py) != ICE[1]:
                layer.put(px, py, ICE[5])
    composite(canvas, layer, OUT)


def clumps(rng, choices):
    """Per-column snow depths that change in runs of 2-4 columns, so a snow edge hangs in lumps."""
    depths = []
    while len(depths) < SIZE[0]:
        depths += [rng.choice(choices)] * rng.randrange(2, 5)
    return depths[:SIZE[0]]


def pine(canvas, cx, top, bottom, hw, seed):
    """A dark spruce whose drooping tiers each carry a shelf of snow just under the tier above."""
    rng = random.Random(seed)
    trunk = Layer(*SIZE)
    for y in range(bottom - 12, bottom + 1):
        for x in range(cx - 2, cx + 2):
            trunk.put(x, y, BARK[1] if x == cx - 2 else BARK[0])
    composite(canvas, trunk, OUT)
    n = max(3, (bottom - top) // 15)
    step = (bottom - 6 - top) / (n + 0.8)
    th = int(step * 1.8)
    tiers = []
    for k in range(n):
        a = int(top + k * step) + (rng.randrange(-1, 2) if k else 0)
        tiers.append((a, a + th, 3 + (hw - 3) * (k + 1) / n))
    for k in range(n - 1, -1, -1):
        a, b, w = tiers[k]
        # The tier above and its outline hide rows down to its bottom + 1, so the shelf starts below that.
        shelf = tiers[k - 1][1] + 2 if k else a
        lumps = clumps(rng, (4, 5) if k == 0 else (2, 3, 3, 4))
        strokes = set()
        for y in range(shelf + 4, b - 1):
            if rng.random() < 0.5:
                half = w * (y - a + 1) / (b - a + 1)
                x0 = round(cx - half) + 1 + rng.randrange(0, max(1, round(half * 0.5)))
                for x in range(x0, x0 + rng.choice((2, 3))):
                    if x < cx - half / 3:
                        strokes.add((x, y))
        tier = Layer(*SIZE)
        for y in range(a, b + 1):
            half = w * (y - a + 1) / (b - a + 1) + (1 if (y + k) % 3 == 0 and y > a + 2 else 0)
            for x in range(round(cx - half), round(cx + half) + 1):
                if y == b and (x - cx + k) % 3:
                    continue
                t = (x - cx) / max(half, 1)
                depth = y - shelf - lumps[x]
                if depth < 0:
                    c = SNOW[3] if t < -0.3 else SNOW[2] if t < 0.35 else SNOW[1]
                    if depth == -1:
                        c = SNOW[1] if t < 0.35 else SNOW[0]
                else:
                    c = PINE[2] if t < -0.15 else PINE[1]
                    if (x, y) in strokes:
                        c = PINE[3]
                    elif y >= b - 1 and t > 0:
                        c = PINE[0]
                tier.put(x, y, c)
        composite(canvas, tier, OUT)


def snowbank(canvas, bumps):
    """Overlapping wind-carved drifts on one layer; each pixel is shaded by its frontmost mound.

    A mound is (cx, cy, left, right, top, bottom) radii, so one side can be steeper. Where a
    nearer mound overlaps one behind it, a one-row blue crease separates them.
    """
    layer = Layer(*SIZE)
    owner = {}
    for y in range(SIZE[1]):
        for x in range(SIZE[0]):
            best = None
            for i, (cx, cy, rl, rr, rt, rb) in enumerate(bumps):
                nx = (x + 0.5 - cx) / (rl if x + 0.5 < cx else rr)
                ny = (y + 0.5 - cy) / (rt if y + 0.5 < cy else rb)
                if nx * nx + ny * ny <= 1 and (best is None or cy >= best[0]):
                    best = (cy, nx, ny, i)
            if best is None:
                continue
            cy, nx, ny, owner[(x, y)] = best
            lit = (-0.55 * nx - 0.85 * ny) * 0.8 + 0.4 * math.sqrt(max(0.0, 1 - nx * nx - ny * ny))
            idx = 4 if lit > 0.85 else 3 if lit > 0.58 else 2 if lit > 0.15 else 1 if lit > -0.25 else 0
            if nx < 0 and idx < 2:
                idx = 2
            layer.put(x, y, SNOW[idx])
    for (x, y), i in owner.items():
        j = owner.get((x, y + 1))
        if j is not None and j != i and bumps[j][1] > bumps[i][1]:
            layer.put(x, y, SNOW[0])
    composite(canvas, layer, OUT)


def frost_rock(layer, x, y, w, h):
    """A blue-grey boulder with a cap of snow on its upper faces."""
    mask = poly_mask(SIZE, [(x, y + h), (x + 1, y + 3), (x + w // 3, y), (x + w * 3 // 4, y + 1), (x + w, y + 4), (x + w, y + h)])
    for px, py in sorted(mask):
        top = min(yy for (xx, yy) in mask if xx == px)
        c = ROCK[2] if px < x + w * 0.45 else ROCK[1]
        if py == y + h:
            c = ROCK[0]
        elif py - top == 2 and px < x + w * 0.45:
            c = ROCK[3]
        if py - top < 2:
            c = SNOW[3] if px < x + w * 0.45 else SNOW[1]
        layer.put(px, py, c)


KRAKEN = [
    '....kkkk....',
    '..kkkkkkkk..',
    '.kkkkkkkkkk.',
    '.kkkkkkkkkk.',
    '.kkekkkkekk.',
    '..mmmmmmmm..',
    '.m.mmmmmm.m.',
    'm.mm.mm.mm.m',
    'm.m..mm..m.m',
    '.m..m..m..m.',
]


def build():
    canvas = Image.new('RGBA', SIZE)
    rng = random.Random(97)
    # Jagged ice crags splay out of the glacier, the tallest above the door.
    for args in [(40, 104, 56, 7, -20, -0.4, 1), (130, 104, 60, 7, 18, 0.5, 2), (62, 100, 80, 9, -9, -0.5, 3),
                 (108, 100, 84, 10, 9, 0.4, 4), (96, 96, 62, 6, 22, 0.6, 6), (84, 100, 97, 13, 2, -0.35, 5)]:
        shard(canvas, *args)
    # A glacier front of frozen columns holds the cave mouth.
    wall = Layer(*SIZE)
    mask = poly_mask(SIZE, [(44, 143), (44, 108), (47, 94), (53, 84), (62, 78), (73, 74), (84, 73), (95, 74),
                            (106, 78), (115, 84), (121, 94), (124, 108), (124, 143)])
    col_edges = [44]
    while col_edges[-1] < 124:
        col_edges.append(col_edges[-1] + rng.randrange(6, 10))
    for x, y in sorted(mask):
        k = max(i for i, e in enumerate(col_edges) if e <= x)
        idx = 2 if x < CX else 1
        if x == col_edges[k]:
            idx += 1
        elif x == col_edges[k + 1] - 1:
            idx -= 1
        if (y + k * 5) % 13 == 0:
            idx -= 1
        if y > 126:
            idx -= 1
        wall.put(x, y, ICE[max(0, min(5, idx))])
    composite(canvas, wall, OUT)
    # A snow cap along the cliff top drapes unevenly; icicles hang from its lower edge.
    cap = Layer(*SIZE)
    tops = {x: min(y for y in range(SIZE[1]) if (x, y) in mask) for x in range(45, 124)}
    hang = clumps(rng, (3, 4, 4, 5, 6))
    for x in range(45, 124):
        for y in range(tops[x] - 2 - (x * 7) % 3 // 2, tops[x] + hang[x]):
            idx = 3 if x < 92 else 2
            if y >= tops[x] + hang[x] - 1:
                idx = 1
            elif y >= tops[x] + 1:
                idx -= 1
            cap.put(x, y, SNOW[idx])
    composite(canvas, cap, OUT)
    drip = Layer(*SIZE)
    x = 48 + rng.randrange(0, 3)
    while x < 121:
        if not 70 < x < 97:
            top = tops[x] + hang[x]
            length = rng.randrange(3, 8)
            for n in range(length):
                drip.put(x, top + n, ICE[5] if n < length - 1 else ICE[4])
                if n < length - 2:
                    drip.put(x + 1, top + n, ICE[3])
        x += rng.randrange(4, 8)
    composite(canvas, drip, None)
    # A thick lip of clear ice rings the cave mouth.
    rim = Layer(*SIZE)
    for y in range(92, 144):
        for x in range(64, 105):
            dx = x + 0.5 - CX
            if y < 111:
                d = math.hypot(dx, y - 111)
                if not 12.5 < d <= 18.5:
                    continue
                seg = int((math.atan2(y - 111, dx) + math.pi) * 2.6)
            elif 12 < abs(dx) <= 18:
                seg = 10 + (y - 111) // 8
            else:
                continue
            idx = 4 if dx < 0 else 2
            if seg % 2:
                idx += 1
            rim.put(x, y, ICE[min(5, idx)])
    composite(canvas, rim, OUT)
    # The keystone is carved with the Frost Kraken, its eyes faintly lit.
    key = Layer(*SIZE)
    for x, y in sorted(poly_mask(SIZE, [(75, 83), (92, 83), (90, 97), (77, 97)])):
        key.put(x, y, ICE[4] if y == 83 else ICE[3] if x < 89 else ICE[2])
    composite(canvas, key, OUT)
    carving = Layer(*SIZE)
    stamp(carving, 78, 85, KRAKEN, {'k': INK[1], 'm': INK[0], 'e': GLOW})
    for dy, row in enumerate(KRAKEN[:5]):
        for dx, ch in enumerate(row):
            if ch != '.' and not carving.on(79 + dx, 86 + dy):
                carving.put(79 + dx, 86 + dy, ICE[4])
    composite(canvas, carving, None)
    cave = Layer(*SIZE)
    for y in range(99, 144):
        for x in range(72, 96):
            if y < 111 and math.hypot(x + 0.5 - CX, y - 111) > 12.5:
                continue
            cave.put(x, y, CAVE[0] if y < 124 else CAVE[1])
    for y in (130, 135, 140):
        for x in range(73, 95):
            cave.put(x, y, ICE[2])
            cave.put(x, y + 1, ICE[0])
    for x, length in ((75, 5), (79, 8), (83, 6), (88, 9), (92, 4)):
        top = 111 - int(math.sqrt(max(0, 156 - (x + 0.5 - CX) ** 2)))
        for n in range(length):
            cave.put(x, top + n, ICE[5] if n < length - 2 else ICE[3])
            if n < length - 3:
                cave.put(x + 1, top + n, ICE[3])
    composite(canvas, cave, None)
    # Snow-laden spruces frame the glacier.
    for args in [(19, 16, 132, 17, 11), (148, 22, 132, 17, 12), (45, 58, 128, 12, 13), (124, 52, 128, 12, 14)]:
        pine(canvas, *args)
    # Frosted rocks half buried in deep drifts that bank against both sides of the door.
    rocks = Layer(*SIZE)
    for args in ((20, 112, 12, 10), (50, 118, 13, 10), (104, 119, 12, 9), (136, 113, 11, 10)):
        frost_rock(rocks, *args)
    composite(canvas, rocks, OUT)
    snowbank(canvas, [(18, 131, 17, 18, 10, 10), (40, 133, 12, 14, 8, 8), (150, 131, 18, 17, 10, 10),
                      (128, 133, 14, 12, 8, 8), (11, 138, 9, 9, 5, 3), (34, 137, 16, 9, 6, 3),
                      (63, 137, 14, 8, 8, 4), (105, 137, 8, 14, 8, 4), (134, 138, 9, 16, 6, 3),
                      (157, 138, 9, 9, 5, 3)])
    sparkle = Layer(*SIZE)
    for x, y in ((80, 22), (58, 50), (113, 42), (101, 88), (66, 104)):
        stamp(sparkle, x - 1, y - 1, ['.w.', 'wWw', '.w.'], {'w': ICE[5], 'W': (255, 255, 255)})
    composite(canvas, sparkle, None)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/frostfell-entrance.bmp')
