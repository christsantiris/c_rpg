#!/usr/bin/env python3
"""Moonveil Gardens moon gate entrance at the native 24px tile scale."""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, clamp, composite, flat_wall, line_points, save_sprite, stamp

SIZE = (168, 144)
OUT = (14, 12, 31)
BARK = [(54, 53, 80), (84, 85, 114), (118, 121, 149), (156, 160, 182), (196, 200, 216)]
LEAF = [(27, 25, 58), (39, 37, 85), (56, 52, 115), (78, 72, 147), (104, 97, 179), (140, 133, 211)]
BLOOM = [(88, 62, 134), (126, 96, 180), (168, 138, 216), (212, 196, 240)]
MOON = [(66, 71, 104), (99, 107, 142), (134, 143, 174), (172, 180, 205), (211, 217, 233), (241, 244, 251)]
HEDGE = [(12, 35, 42), (19, 53, 55), (29, 75, 69), (47, 101, 87), (78, 132, 108)]
VINE = [(15, 35, 34), (44, 94, 68), (86, 142, 94)]
ROSE = [(98, 27, 74), (164, 48, 116), (226, 106, 172), (250, 176, 218)]
GLOW = [(104, 190, 212), (172, 234, 240), (246, 250, 238)]
GOLD = [(122, 96, 58), (214, 184, 106), (252, 236, 172)]
DARK = [(9, 10, 26), (15, 19, 37), (23, 29, 47)]
CX, CY, RIM = 84, 108, 29
LAMP_Y = 83


def lobes(layer, blobs, pal, bias=0):
    """Scalloped lobes lit from the upper left; later lobes overlap and shade earlier ones."""
    owner = {}
    for i, (bx, by, br) in enumerate(blobs):
        for y in range(int(by - br) - 2, int(by + br) + 3):
            for x in range(int(bx - br) - 2, int(bx + br) + 3):
                a = math.atan2(y - by, x - bx)
                if math.hypot(x - bx, y - by) <= br * (1 + 0.12 * math.sin(6 * a + i * 2.1)):
                    owner[(x, y)] = i
    for (x, y), i in sorted(owner.items()):
        bx, by, br = blobs[i]
        lit = -((x - bx) * 0.6 + (y - by) * 0.8) / br
        idx = 2 + bias + (lit > 0.55) + (lit > 0.05) - (lit < -0.55)
        if owner.get((x, y - 1), i) > i or owner.get((x - 1, y), i) > i:
            idx -= 2
        layer.put(x, y, pal[clamp(idx, 0, len(pal) - 1)])
    return owner


def tufts(layer, owner, blobs, pal, rng, count, bias=0):
    """Small leaf strokes give each lobe texture without per-pixel noise."""
    for n in range(count):
        i = rng.randrange(len(blobs))
        bx, by, br = blobs[i]
        x, y = int(bx + rng.uniform(-0.6, 0.4) * br), int(by + rng.uniform(-0.6, 0.4) * br)
        for dx, dy in ((0, 0), (1, 0), (2, 1), (-1, 1)):
            if owner.get((x + dx, y + dy)) == i:
                layer.put(x + dx, y + dy, pal[clamp((len(pal) - 2 if y < by else 2) + bias, 0, len(pal) - 1)])


def raceme(layer, x, top, length, sway):
    """A drooping wisteria cluster, broad at the stem and lit on its left."""
    for n in range(length):
        width = 4 if n < length * 0.3 else 3 if n < length - 3 else 2 if n < length - 1 else 1
        x0 = x - width // 2 + ((n // 2) % 2 if width > 1 else 0) + (sway if n > length // 2 else 0)
        for k in range(width):
            shade = 3 if k == 0 and n % 2 == 0 else 1 if k == width - 1 and width > 1 else 2
            layer.put(x0 + k, top + n, BLOOM[shade])


def wisteria(canvas, cx, cy, rx, ry, seed, tassels, bias=-1, scale=1.0):
    """A clumped blue-violet canopy with lavender racemes along its lower edge."""
    rng = random.Random(seed)
    blobs = []
    for n in range(10):
        a = (n + rng.uniform(-0.3, 0.3)) * 2 * math.pi / 10
        blobs.append((cx + math.cos(a) * rx * 0.72, cy + math.sin(a) * ry * 0.72, rng.uniform(6.5, 9.5) * scale))
    for n in range(6):
        a = rng.uniform(0, 2 * math.pi)
        k = math.sqrt(rng.random()) * 0.5
        blobs.append((cx + math.cos(a) * rx * k, cy + math.sin(a) * ry * k, rng.uniform(7, 10) * scale))
    blobs.sort(key=lambda b: b[1])
    crown = Layer(*SIZE)
    owner = lobes(crown, blobs, LEAF, bias)
    tufts(crown, owner, blobs, LEAF, rng, 30, bias)
    for tx, length in tassels:
        column = [y for (x, y) in owner if x == tx]
        if column:
            raceme(crown, tx, max(column) - 3, length, 1 if tx % 2 else -1)
    for n in range(2):
        bx, by, br = blobs[rng.randrange(4, len(blobs))]
        x, y = int(bx - br * 0.35), int(by - br * 0.35)
        if all((x + dx, y + dy) in owner for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1))):
            stamp(crown, x - 1, y - 1, ['.g.', 'gGg', '.g.'], {'g': LEAF[5], 'G': GLOW[2]})
    composite(canvas, crown, OUT)


def limb_mask(limbs):
    """Union of tapered polylines: [(points, start_radius, end_radius), ...]."""
    mask = set()
    for points, r0, r1 in limbs:
        path = []
        for (x0, y0), (x1, y1) in zip(points, points[1:]):
            path += line_points(x0, y0, x1, y1)[:-1]
        path.append(points[-1])
        for n, (px, py) in enumerate(path):
            r = r0 + (r1 - r0) * n / max(1, len(path) - 1)
            ir = int(r) + 1
            for y in range(py - ir, py + ir + 1):
                for x in range(px - ir, px + ir + 1):
                    if (x - px) ** 2 + (y - py) ** 2 <= r * r + 0.5:
                        mask.add((x, y))
    return {(x, y) for x, y in mask if 0 <= x < SIZE[0] and 0 <= y < SIZE[1]}


def silver_trunk(canvas, limbs, seed):
    """Pale, birch-marked bark lit from the upper left across its narrow axis."""
    rng = random.Random(seed)
    tree = Layer(*SIZE)
    mask = limb_mask(limbs)

    def reach(x, y, dx, dy):
        n = 0
        while (x + dx * (n + 1), y + dy * (n + 1)) in mask:
            n += 1
        return n

    for x, y in sorted(mask):
        left, right = reach(x, y, -1, 0), reach(x, y, 1, 0)
        up, down = reach(x, y, 0, -1), reach(x, y, 0, 1)
        if left + right <= up + down:
            t = (left + 0.5) / (left + right + 1)
        else:
            t = (up + 0.5) / (up + down + 1)
        idx = 4 if t < 0.15 else 3 if t < 0.35 else 2 if t < 0.72 else 1
        tree.put(x, y, BARK[idx])
    for n in range(250):
        x, y = rng.randrange(0, SIZE[0]), rng.randrange(20, 138)
        run = [(x + dx, y) for dx in range(rng.randrange(2, 5))]
        if all(p in mask for p in run) and (x - 1, y) in mask and (run[-1][0] + 1, y) in mask:
            for px, py in run:
                tree.put(px, py, BARK[0])
                if (px, py - 1) in mask:
                    tree.put(px, py - 1, BARK[4] if px < x + 2 else BARK[3])
    composite(canvas, tree, OUT)


def in_door(x, y):
    return math.hypot(x - CX, y - CY) <= 12.5 or (y >= CY and abs(x - CX) <= 12)


def moon_gate(canvas):
    """A full-moon disc of moonstone with a carved rim and a round-topped doorway."""
    gate = Layer(*SIZE)
    for y in range(CY - RIM - 1, 144):
        for x in range(CX - RIM - 1, CX + RIM + 2):
            dx, dy = x - CX, y - CY
            d = math.hypot(dx, dy)
            if in_door(x, y) or d > RIM + 0.3:
                continue
            lit = -(dx * 0.6 + dy * 0.8) / max(d, 1)
            jamb = dy >= 0 and abs(dx) <= 17
            if d > RIM - 3.5:
                idx = 3 + round(lit * 1.5)
            elif d > RIM - 4.5:
                idx = 1
            elif d > 17.5 and not jamb:
                idx = 3 if lit * d / RIM > 0.2 else 2
            elif d > 16.5 and not (jamb and abs(dx) <= 16):
                idx = 1
            elif dy < 0:
                seg = (math.atan2(dy, dx) + math.pi) / (2 * math.pi) * 16
                idx = 4 if lit < -0.1 else 3
                if seg - int(seg) < 0.3:
                    idx = 1
            else:
                idx = 4 if dx < 0 else 3
                if dy % 7 == 6:
                    idx = 1
            gate.put(x, y, MOON[clamp(idx, 0, 5)])
    composite(canvas, gate, OUT)
    base = Layer(*SIZE)
    flat_wall(base, 48, 120, 133, 143, MOON, row_h=6, base=3, seed=5, bw=(8, 13))
    for x in range(48, 121):
        base.put(x, 133, MOON[4])
    # A narrower footing flanks the steps; the outer plinth stops short of the ground line.
    for y in range(142, 144):
        for x in range(48, 121):
            if abs(x - CX) > 16:
                base.erase(x, y)
    for x in range(CX - 16, CX + 17):
        base.put(x, 141, MOON[2] if abs(x - CX) > 12 else MOON[1])
    for y in range(133, 144):
        for x in range(CX - 12, CX + 13):
            base.erase(x, y)
    composite(canvas, base, OUT)
    # A crescent crest cups a guiding star above the gate.
    crest = Layer(*SIZE)
    for y in range(54, 84):
        for x in range(CX - 12, CX + 13):
            if math.hypot(x - CX, y - 70) <= 11 and math.hypot(x - CX, y - 65) > 10.5:
                crest.put(x, y, MOON[5] if x < CX - 4 else MOON[4] if x < CX + 4 else MOON[3])
    stamp(crest, CX - 3, 63, ['...w...', '..wWw..', '.wWWWw.', 'wWWWWWw', '.wWWWw.', '..wWw..', '...w...'],
          {'w': GLOW[0], 'W': GLOW[1]})
    crest.put(CX, 66, GLOW[2])
    composite(canvas, crest, OUT)


def opening(canvas):
    """The dark walk-in passage, with moonstone steps rising to the bottom edge."""
    door = Layer(*SIZE)
    for y in range(CY - 13, 144):
        for x in range(CX - 13, CX + 14):
            if in_door(x, y):
                band = 0 if y < 116 else 1 if y < 127 else 2
                if y in (115, 116):
                    band = 1 if (x + y) % 2 == 0 else 0
                elif y in (126, 127):
                    band = 2 if (x + y) % 2 == 0 else 1
                door.put(x, y, DARK[band])
    for y0, half, shade in ((130, 8, 1), (135, 10, 2), (140, 12, 3)):
        for y in range(y0, min(144, y0 + 4 if y0 == 140 else y0 + 3)):
            for x in range(CX - half, CX + half + 1):
                door.put(x, y, MOON[shade] if y == y0 else MOON[shade - 1])
    composite(canvas, door, None)


def vines(canvas):
    """Thorny climbing roses wind up both sides of the gate toward the crest."""
    vine = Layer(*SIZE)
    blooms = Layer(*SIZE)
    for side in (-1, 1):
        pts = []
        for n in range(84):
            a = math.radians(90 - side * (40 + n * 1.5))
            rad = 22 + 2.8 * math.sin(n * 0.2)
            pts.append((round(CX + math.cos(a) * rad), round(CY + math.sin(a) * rad)))
        for n, ((x0, y0), (x1, y1)) in enumerate(zip(pts, pts[1:])):
            for x, y in line_points(x0, y0, x1, y1):
                vine.put(x, y + 1, VINE[0])
                vine.put(x, y, VINE[1])
            if n % 8 == 4:
                vine.put(x0 - side, y0 - 1, VINE[0])
            if n % 12 == 7:
                stamp(vine, x0 - (1 if side < 0 else 0), y0 - 2, ['.l', 'll'] if side < 0 else ['l.', 'll'], {'l': VINE[2]})
        for n in (4, 24, 44, 64, 80):
            x, y = pts[n]
            stamp(blooms, x - 2, y - 2, ['.rr.', 'rRRr', 'rRrr', '.rr.'], {'r': ROSE[1], 'R': ROSE[2]})
            blooms.put(x - 1, y - 1, ROSE[3])
    composite(canvas, vine, None)
    composite(canvas, blooms, OUT)


def garden_beds(canvas):
    """Rose hedges and glowing moonflowers close the footprint beside the door."""
    rng = random.Random(41)
    hedge = Layer(*SIZE)
    for blobs in ([(40, 128, 7), (52, 126, 7), (30, 133, 7), (44, 134, 7), (58, 134, 6)],
                  [(128, 128, 7), (116, 126, 7), (138, 133, 7), (124, 134, 7), (110, 134, 6)],
                  [(10, 135, 6), (17, 136, 5)], [(158, 135, 6), (151, 136, 5)]):
        owner = lobes(hedge, blobs, HEDGE)
        tufts(hedge, owner, blobs, HEDGE, rng, 10)
    composite(canvas, hedge, OUT)
    flowers = Layer(*SIZE)
    for x, y in ((38, 124), (54, 122), (122, 126), (132, 131), (10, 131)):
        stamp(flowers, x - 2, y - 2, ['.rr.', 'rRRr', 'rRrr', '.rr.'], {'r': ROSE[1], 'R': ROSE[2]})
        flowers.put(x - 1, y - 1, ROSE[3])
    for x, y, p, c in ((46, 130, GLOW[0], GLOW[2]), (60, 131, GLOW[1], GLOW[2]), (30, 132, (226, 226, 240), GOLD[2]),
                       (112, 123, GLOW[0], GLOW[2]), (108, 131, GLOW[1], GLOW[2]), (126, 134, (226, 226, 240), GOLD[2]),
                       (157, 132, GLOW[0], GLOW[2]), (52, 137, ROSE[2], ROSE[3]), (118, 137, ROSE[2], ROSE[3])):
        stamp(flowers, x - 1, y - 1, ['.p.', 'pcp', '.p.'], {'p': p, 'c': c})
    composite(canvas, flowers, OUT)


def back_hedge(canvas):
    """Dark clipped hedges behind the walls fill the gaps between the trees and the gate."""
    rng = random.Random(43)
    hedge = Layer(*SIZE)
    for blobs in ([(36, 104, 7), (47, 100, 8), (57, 103, 6)], [(132, 104, 7), (121, 100, 8), (111, 103, 6)]):
        owner = lobes(hedge, blobs, HEDGE, bias=-1)
        tufts(hedge, owner, blobs, HEDGE, rng, 8, bias=-1)
    composite(canvas, hedge, OUT)


def garden_wall(canvas):
    """Low moonstone walls run from each tree to the gate, closing the garden."""
    wall = Layer(*SIZE)
    for x0, x1 in ((18, 60), (108, 150)):
        flat_wall(wall, x0, x1, 113, 136, MOON, row_h=6, base=1, seed=x0, bw=(7, 12))
        for x in range(x0 - 1, x1 + 2):
            wall.put(x, 110, MOON[3])
            wall.put(x, 111, MOON[2])
            wall.put(x, 112, MOON[0])
    composite(canvas, wall, OUT)
    ivy = Layer(*SIZE)
    rose = Layer(*SIZE)
    for x0, side in ((38, 1), (130, -1)):
        pts = [(x0 + round(math.sin(n * 0.35) * 2) + side * n // 6, 130 - n) for n in range(19)]
        for x, y in pts:
            ivy.put(x + 1, y, VINE[0])
            ivy.put(x, y, VINE[1])
        for n in (4, 10, 16):
            x, y = pts[n]
            stamp(ivy, x - 2 if n % 4 else x + 1, y, ['ll.', '.ll'] if n % 4 else ['.ll', 'll.'], {'l': VINE[2]})
        x, y = pts[-1]
        stamp(rose, x - 2, y - 3, ['.rr.', 'rRRr', 'rRrr', '.rr.'], {'r': ROSE[1], 'R': ROSE[2]})
        rose.put(x - 1, y - 2, ROSE[3])
    composite(canvas, ivy, None)
    composite(canvas, rose, OUT)


def brighten(canvas, cx, cy, r, ramps):
    """Lift ramp colours one palette step inside a radius, so a glow stays on-palette."""
    step = {a: b for pal in ramps for a, b in zip(pal, pal[1:])}
    cp = canvas.load()
    for y in range(max(0, cy - r), min(SIZE[1], cy + r + 1)):
        for x in range(max(0, cx - r), min(SIZE[0], cx + r + 1)):
            c = cp[x, y]
            if c[3] and c[:3] in step and math.hypot(x - cx, y - cy) <= r:
                cp[x, y] = step[c[:3]] + (255,)


def lanterns(canvas):
    """Two moon lanterns hang on silver chains from the inner branches, lighting the hedges."""
    cp = canvas.load()
    lamps = Layer(*SIZE)
    for x in (52, 116):
        brighten(canvas, x, LAMP_Y + 5, 11, [HEDGE])
        top = LAMP_Y
        while top > 0 and not cp[x, top - 1][3]:
            top -= 1
        for y in range(top, LAMP_Y):
            lamps.put(x, y, MOON[3] if y % 2 else MOON[1])
        stamp(lamps, x - 3, LAMP_Y, ['..iIi..', '.iIIIi.', 'iyYYYyi', 'iYYWYYi', 'iyYWYyi', 'iyYYYyi', '.iiIii.', '...i...'],
              {'i': MOON[1], 'I': MOON[3], 'y': GOLD[1], 'Y': GOLD[2], 'W': (255, 252, 230)})
    composite(canvas, lamps, OUT)


def check_edges(canvas):
    """Every shape keeps clear of the side edges and every fill pixel is ringed by outline."""
    cp = canvas.load()
    w, h = canvas.size
    assert all(cp[x, y][3] == 0 for x in (0, 1, w - 2, w - 1) for y in range(h)), 'shape clipped at a side edge'
    assert all(cp[x, 0][3] == 0 for x in range(w)), 'shape clipped at the top edge'
    for y in range(h):
        for x in range(w):
            if cp[x, y][3] and cp[x, y][:3] != OUT:
                for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                    assert not (0 <= nx < w and 0 <= ny < h) or cp[nx, ny][3], 'unoutlined pixel at %d,%d' % (x, y)


def build():
    canvas = Image.new('RGBA', SIZE)
    back_hedge(canvas)
    garden_wall(canvas)
    silver_trunk(canvas, [([(6, 140), (12, 137), (20, 130)], 1, 4),
                          ([(25, 133), (25, 124), (21, 104), (24, 88), (21, 70), (14, 54), (12, 40)], 7, 3),
                          ([(23, 82), (33, 66), (46, 56), (60, 46)], 4, 2),
                          ([(21, 70), (29, 52), (32, 38)], 4, 2),
                          ([(28, 134), (38, 139), (44, 140)], 4, 1)], 21)
    silver_trunk(canvas, [([(162, 140), (156, 137), (148, 130)], 1, 4),
                          ([(143, 133), (143, 124), (147, 104), (144, 88), (147, 70), (154, 54), (156, 40)], 7, 3),
                          ([(145, 82), (135, 66), (122, 56), (108, 46)], 4, 2),
                          ([(147, 70), (139, 52), (136, 38)], 4, 2),
                          ([(140, 134), (130, 139), (124, 140)], 4, 1)], 22)
    wisteria(canvas, 84, 42, 24, 13, 11, [(64, 14), (70, 10), (77, 7), (91, 7), (98, 10), (104, 14)], bias=-2)
    wisteria(canvas, 32, 32, 26, 21, 12, [(12, 12), (18, 16), (25, 11), (37, 13), (43, 18), (50, 14), (56, 10)])
    wisteria(canvas, 136, 32, 26, 21, 13, [(112, 10), (118, 14), (125, 18), (131, 13), (143, 11), (150, 16), (156, 12)])
    wisteria(canvas, 18, 78, 10, 9, 14, [(11, 10), (17, 14), (23, 9)], scale=0.78)
    wisteria(canvas, 150, 78, 10, 9, 15, [(145, 9), (151, 14), (157, 10)], scale=0.78)
    moon_gate(canvas)
    opening(canvas)
    vines(canvas)
    garden_beds(canvas)
    lanterns(canvas)
    brighten(canvas, CX, 66, 22, [LEAF, BLOOM])
    brighten(canvas, CX, 64, 12, [LEAF, BLOOM])
    # Fireflies drift over the darker parts of the garden.
    sparks = Layer(*SIZE)
    cp = canvas.load()
    for x, y in ((79, 103), (91, 111), (81, 122), (14, 86), (150, 88), (122, 44), (144, 134)):
        if cp[x, y][3]:
            sparks.put(x, y, GLOW[2] if x < 72 or x > 96 else GLOW[1])
    composite(canvas, sparks, None)
    check_edges(canvas)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/moonveil-entrance.bmp')
