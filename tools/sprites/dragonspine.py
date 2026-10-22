#!/usr/bin/env python3
"""Dragonspine entrance, a 7x9 tile landmark in the established 24px pixelkit pipeline.

A snow-dusted basalt peak rises in vertebra-like spikes to a tall summit. Red
dragons circle the peak and perch on a ledge. The south-facing cave mouth is a
horned dragon skull with a faint ember glow, ringed by scorch marks, claw
gouges, snowdrifts and a bone pile.
"""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, composite, line_points, poly_mask, save_sprite, wobble

SIZE = (168, 216)
OUT = (14, 16, 23)
ROCK = [(24, 28, 38), (36, 42, 55), (51, 59, 74), (70, 80, 96), (95, 106, 122), (126, 136, 150)]
SNOW = [(116, 134, 160), (156, 174, 198), (199, 213, 228), (234, 241, 247)]
BONE = [(70, 62, 56), (110, 100, 86), (152, 142, 122), (192, 183, 160), (226, 219, 198)]
DRAKE = [(60, 16, 20), (104, 26, 26), (152, 40, 32), (200, 70, 42), (238, 128, 66)]
WING = [(66, 18, 26), (98, 28, 32), (134, 44, 40)]
EMBER = [(34, 14, 15), (66, 22, 17), (112, 40, 20), (186, 82, 34), (246, 160, 70)]
SOOT = [(26, 24, 30), (38, 35, 41), (55, 51, 57)]
DOOR = (9, 9, 13)
SOCKET = (24, 15, 17)
GOLD = [(150, 104, 52), (214, 166, 82)]
EYE = (255, 214, 90)

# The skyline climbs from the lower left in a row of dorsal spikes to the summit.
SPINE = [(95, 2), (91, 15), (88, 26), (80, 13), (78, 46), (68, 34), (66, 70), (56, 58), (54, 94), (44, 83),
         (42, 118), (32, 108), (30, 140), (20, 132), (18, 160), (9, 156), (6, 172), (2, 188), (0, 215)]
CLIFF = [(167, 215), (166, 184), (162, 164), (156, 148), (148, 132), (142, 118), (136, 102), (132, 90),
         (127, 78), (124, 66), (119, 58), (116, 48), (110, 40), (108, 30), (104, 24), (102, 16), (99, 10)]
RIDGE = [(95, 2), (90, 40), (93, 70), (88, 100), (86, 126), (84, 215)]
VALLEYS = SPINE[2:16:2]
TIPS = SPINE[3:16:2]


def along(points, y):
    """x of a top-to-bottom polyline at row y."""
    for (x0, y0), (x1, y1) in zip(points, points[1:]):
        if y0 <= y <= y1:
            return x0 + (x1 - x0) * (y - y0) / max(1, y1 - y0)
    return points[-1][0]


def sunlit(x, y):
    return x < along(RIDGE, y)


def snow_tops(L, mask, lit, below=999):
    """Settle snow on every upward-facing edge, deeper where the slope is gentle."""
    tops = {}
    for x, y in mask:
        if (x, y - 1) not in mask and y < below:
            tops.setdefault(x, []).append(y)
    for x, ys in tops.items():
        for y in ys:
            near = [min(tops.get(x + d, [y]), key=lambda t: abs(t - y)) for d in (-1, 1)]
            slope = abs(near[1] - near[0]) / 2.0
            depth = 3 if slope < 0.8 else 2 if slope < 1.6 else 1 if slope < 3 else 0
            for k in range(depth):
                if (x, y + k) in mask:
                    if lit(x, y + k):
                        L.put(x, y + k, SNOW[3 if k == 0 else 2])
                    else:
                        L.put(x, y + k, SNOW[1 if k == 0 else 0])


def massif(canvas):
    rng = random.Random(41)
    rock = Layer(*SIZE)
    mask = poly_mask(SIZE, SPINE + CLIFF)
    cap = wobble(SIZE[0], 7, 40, 47)
    for x, y in sorted(mask):
        ridge = along(RIDGE, y)
        sun = x < ridge
        shade = 3 if sun else 1
        snowy = False
        if sun:
            # Rib-like ledges run down the flank from each dorsal spike.
            for i, (vx, vy) in enumerate(VALLEYS):
                run = x - vx
                if run < -1 or run > 14 + 3 * i:
                    continue
                d = y - round(vy + run * 1.25 + run * run * 0.015)
                if 0 <= d <= 3:
                    shade = 1 if d == 0 else 2
                elif d == -1 or (d == -2 and run < 8 + i):
                    shade = 4
                    snowy = y < 150
        else:
            for vx, vy in VALLEYS:
                py = vy + (along(RIDGE, vy) - vx) * 1.3
                d = y - (py + 16 + (x - ridge) * 0.6)
                if d == 0:
                    shade = 0
                elif d == -1:
                    shade = 2
                    snowy = y < 110
        for (tx, ty), (vx, vy) in zip(TIPS, VALLEYS):
            if ty <= y < vy and tx - 2 <= x <= vx and y < vy - (vx - x) * 1.25:
                shade = 4 if x <= tx + 1 else 3
                snowy = y < ty + 4 + (tx - x) // 2
        if rng.randrange(31) == 0:
            shade += rng.choice((-1, 1))
        c = ROCK[max(0, min(5, shade))]
        if snowy:
            c = SNOW[3 if sun and shade >= 3 else 2] if sun else SNOW[1]
        # The summit carries a thick cap that drips down its gullies.
        streak = 9 if x % 11 in (4, 5) else 5 if x % 7 == 2 else 0
        if y < cap[x] + streak and y < 58:
            c = SNOW[3 if shade >= 3 else 2] if sun else SNOW[1 if shade >= 1 else 0]
        rock.put(x, y, c)
    # Angular ledges break up the faces; the higher ones hold a line of snow.
    for n in range(16):
        x = rng.randrange(14, 156)
        y = rng.randrange(40, 150)
        for px, py in line_points(x, y, x + rng.randrange(5, 12), y + rng.randrange(-2, 3)):
            if (px, py) in mask and (px, py - 1) in mask:
                sun = sunlit(px, py)
                rock.put(px, py, ROCK[0])
                rock.put(px, py - 1, (SNOW[2] if sun else SNOW[0]) if py < 120 else ROCK[4 if sun else 2])
    snow_tops(rock, mask, sunlit, below=150)
    composite(canvas, rock, OUT)


def back_peak(canvas):
    peak = Layer(*SIZE)
    mask = poly_mask(SIZE, [(112, 150), (122, 96), (131, 82), (136, 86), (144, 62), (150, 74), (154, 84),
                            (158, 100), (162, 122), (166, 148), (167, 170)])
    for x, y in sorted(mask):
        sun = x < 144 - (y - 62) * 0.15
        shade = 3 if sun else 1
        if (y - (x - 144) * 0.6) % 14 < 1:
            shade -= 1
        c = ROCK[shade]
        if y < 74 + (x % 5):
            c = SNOW[3] if sun else SNOW[1]
        peak.put(x, y, c)
    snow_tops(peak, mask, lambda x, y: x < 144)
    composite(canvas, peak, OUT)


def ledge(canvas):
    shelf = Layer(*SIZE)
    mask = poly_mask(SIZE, [(124, 112), (134, 107), (150, 105), (163, 104), (166, 108), (158, 114),
                            (140, 117), (126, 118)])
    for x, y in sorted(mask):
        shelf.put(x, y, ROCK[2] if y > 110 else ROCK[3])
    snow_tops(shelf, mask, lambda x, y: x < 150)
    composite(canvas, shelf, OUT)


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


def horn(L, p0, p1, p2, r0):
    """A ridged, tapering horn along a quadratic curve, lit from the upper left."""
    for i in range(81):
        t = i / 80.0
        cx = (1 - t) ** 2 * p0[0] + 2 * (1 - t) * t * p1[0] + t * t * p2[0]
        cy = (1 - t) ** 2 * p0[1] + 2 * (1 - t) * t * p1[1] + t * t * p2[1]
        r = r0 * (1 - t) ** 0.8 + 0.5
        band = 2 if int(t * 11) % 2 else 3
        for y in range(int(cy - r - 1), int(cy + r + 2)):
            for x in range(int(cx - r - 1), int(cx + r + 2)):
                dx = x + 0.5 - cx
                dy = y + 0.5 - cy
                if dx * dx + dy * dy > r * r:
                    continue
                k = (dx + dy) / max(r, 0.1)
                L.put(x, y, BONE[band + 1] if k < -0.5 else BONE[band - 1] if k > 0.6 else BONE[band])


def mirror(points):
    return [(168 - x, y) for x, y in points]


def maw(canvas):
    """The open jaws are the doorway: black, with embers smouldering deep inside."""
    hole = Layer(*SIZE)
    for y in range(164, 216):
        for x in range(70, 99):
            dx = abs(x - 84)
            c = DOOR
            if 187 <= y <= 194:
                reach = 11 - (194 - y) * 1.5
                if dx < reach:
                    c = EMBER[2] if dx < reach - 6 else EMBER[1] if dx < reach - 3 else EMBER[0]
            elif y >= 195:
                c = EMBER[0] if y < 197 and dx < 9 else SOOT[1] if y > 201 else SOOT[0]
            hole.put(x, y, c)
    # Scorched steps lead up into the dark, their lips catching the ember light.
    for y in (199, 204, 209):
        for x in range(70, 99):
            hole.put(x, y, EMBER[1] if abs(x - 84) < 4 and y < 205 else SOOT[2])
            hole.put(x, y + 1, SOOT[0])
    for x, y in ((80, 193), (81, 193), (87, 192), (88, 192), (84, 193)):
        hole.put(x, y, EMBER[3])
    composite(canvas, hole, None)


def skull(canvas):
    horns = Layer(*SIZE)
    horn(horns, (62, 134), (42, 130), (34, 104), 5.5)
    horn(horns, (106, 134), (126, 130), (134, 104), 5.5)
    composite(canvas, horns, OUT)
    bone = Layer(*SIZE)
    head = [(56, 140), (60, 132), (70, 127), (84, 124), (98, 127), (108, 132), (112, 140), (108, 148), (102, 154),
            (100, 162), (99, 170), (69, 170), (68, 162), (66, 154), (60, 148)]
    crest = [(70, 129), (73, 117), (78, 127), (84, 112), (90, 127), (95, 117), (98, 129)]
    cheek = [(62, 148), (48, 158), (45, 168), (54, 163), (64, 160)]
    shape = poly_mask(SIZE, head) | poly_mask(SIZE, crest)
    sides = set()
    for pts in (cheek, mirror(cheek)):
        sides |= poly_mask(SIZE, pts)
    shape |= sides
    for x, y in shape:
        i = 3 if x < 86 else 2
        if (x - 1, y) not in shape or (x, y - 1) not in shape:
            i += 1
        elif (x + 1, y) not in shape or (x, y + 1) not in shape:
            i -= 1
        bone.put(x, y, BONE[max(0, min(4, i))])
    # Hollow cheeks and an old crack across the brow.
    hollow = [(64, 151), (73, 153), (71, 158), (65, 156)]
    for pts, c in ((hollow, BONE[2]), (mirror(hollow), BONE[1])):
        for x, y in poly_mask(SIZE, pts):
            bone.put(x, y, c)
    for x, y in line_points(92, 125, 95, 131) + line_points(95, 131, 93, 136):
        bone.put(x, y, BONE[1])
    # Heavy brows over slanted sockets, a nasal ridge and two nostril slits.
    eye = [(63, 141), (76, 145), (76, 150), (70, 151), (64, 147)]
    for pts in (eye, mirror(eye)):
        for x, y in poly_mask(SIZE, pts):
            bone.put(x, y, SOCKET)
    for x, y in ((70, 148), (71, 148), (97, 148), (98, 148)):
        bone.put(x, y, EMBER[3])
    for k in range(2):
        for x, y in line_points(59, 138 + k, 77, 142 + k):
            bone.put(x, y, BONE[4 - k])
        for x, y in line_points(109, 138 + k, 91, 142 + k):
            bone.put(x, y, BONE[3 - k])
    for y in range(148, 160):
        bone.put(83, y, BONE[4])
        bone.put(85, y, BONE[2])
    for x, y in ((79, 158), (79, 159), (78, 160), (89, 158), (89, 159), (90, 160)):
        bone.put(x, y, SOCKET)
    # Upper fangs hang over the doorway; lower fangs rise beside it.
    for tx, h, hw in ((73, 10, 2.2), (77, 4, 1.4), (81, 5, 1.4), (87, 5, 1.4), (91, 4, 1.4), (95, 10, 2.2)):
        for k in range(h):
            w = (h - k) / float(h) * hw
            for x in range(round(tx - w), round(tx + w) + 1):
                bone.put(x, 170 + k, BONE[4] if x < tx else BONE[2] if x > tx else BONE[3])
    composite(canvas, bone, OUT)
    # Two great lower fangs rise from the ground on either side of the door.
    tusks = Layer(*SIZE)
    for side in (-1, 1):
        for y in range(173, 216):
            t = (215 - y) / 42.0
            cx = 84 + side * (19 - 6 * t * t)
            hw = 6.5 * (1 - t) ** 0.7 + 0.4
            for x in range(math.floor(cx - hw), math.ceil(cx + hw)):
                u = (x + 0.5 - cx) / hw
                i = 4 if u < -0.55 else 3 if u < 0.1 else 2 if u < 0.6 else 1
                if side < 0 and u > 0.6 and y > 196:
                    i = 1
                tusks.put(x, y, BONE[max(0, i)])
    composite(canvas, tusks, OUT)


def scorch(canvas, tongues):
    """Soot licks up the rock and snow in flame-shaped streaks."""
    cp = canvas.load()
    rock = {c: i for i, c in enumerate(ROCK)}
    snow = set(SNOW)
    for bx, by, h, w, lean in tongues:
        for y in range(by - h, by + 1):
            t = (by - y) / float(h)
            half = w * (1 - t) ** 0.6 * (0.8 + 0.2 * math.sin(y * 0.9 + bx))
            cx = bx + lean * t * t
            for x in range(int(cx - half), int(cx + half) + 1):
                if not (0 <= x < SIZE[0] and 0 <= y < SIZE[1]):
                    continue
                c = cp[x, y][:3]
                core = abs(x - cx) < half * 0.55
                if c in rock:
                    cp[x, y] = SOOT[0 if rock[c] < 2 or core else 1 if rock[c] < 4 else 2] + (255,)
                elif c in snow:
                    cp[x, y] = (SOOT[2] if core else ROCK[2]) + (255,)


def gouges(canvas, x0, y0, sun):
    """Three parallel claw rakes torn into the rock face."""
    cp = canvas.load()
    for n in range(3):
        pts = line_points(x0 + n * 5, y0 + n * 2, x0 + n * 5 - 8, y0 + n * 2 + 17)
        for k, (x, y) in enumerate(pts):
            if 1 <= k <= len(pts) - 2:
                cp[x, y] = ROCK[0] + (255,)
                cp[x + 1, y] = ROCK[0] + (255,)
                cp[x + 2, y] = (ROCK[5] if sun else ROCK[3]) + (255,)


def drift(canvas, pts):
    snow = Layer(*SIZE)
    mask = poly_mask(SIZE, pts)
    for x, y in mask:
        c = SNOW[3] if (x, y - 2) not in mask else SNOW[2]
        if (x, y - 1) in mask and ((x + 1, y) not in mask or (x + 2, y) not in mask or (x, y + 3) not in mask):
            c = SNOW[1]
        snow.put(x, y, c)
    composite(canvas, snow, OUT)


def long_bone(L, x0, y0, x1, y1):
    """A thigh bone with knobbed ends, lit along its upper edge."""
    for x, y in line_points(x0, y0, x1, y1):
        L.put(x, y, BONE[4])
        L.put(x, y + 1, BONE[2])
    for x, y in ((x0, y0), (x1, y1)):
        for dx, dy, c in ((-1, -1, 3), (1, -1, 4), (-1, 1, 2), (1, 1, 2), (0, 2, 1)):
            L.put(x + dx, y + dy, BONE[c])


def remains(canvas):
    """A heap of picked-clean bones: a rib, crossed thigh bones and two skulls."""
    rib = Layer(*SIZE)
    for i in range(41):
        t = i / 40.0
        cx = (1 - t) ** 2 * 12 + 2 * (1 - t) * t * 14 + t * t * 34
        cy = (1 - t) ** 2 * 205 + 2 * (1 - t) * t * 178 + t * t * 196
        for x, y in ((int(cx), int(cy)), (int(cx) + 1, int(cy))):
            rib.put(x, y, BONE[3])
        rib.put(int(cx), int(cy) - 1, BONE[4])
    composite(canvas, rib, OUT)
    pile = Layer(*SIZE)
    long_bone(pile, 7, 211, 23, 206)
    long_bone(pile, 20, 202, 36, 205)
    skull = ['.bBBb.', 'bBBBBb', 'BoBBoB', 'bBBBBb', '.b.b..']
    for ox, oy in ((36, 199), (4, 200)):
        for dy, row in enumerate(skull):
            for dx, ch in enumerate(row):
                c = {'b': BONE[2], 'B': BONE[4], 'o': SOCKET}.get(ch)
                if c:
                    pile.put(ox + dx, oy + dy, c)
    composite(canvas, pile, OUT)


def foreground(canvas):
    drift(canvas, [(0, 188), (5, 181), (14, 178), (28, 183), (40, 190), (50, 198), (56, 206), (59, 215), (0, 215)])
    drift(canvas, [(109, 215), (112, 206), (120, 198), (132, 191), (146, 185), (158, 181), (164, 182), (167, 186), (167, 215)])
    drift(canvas, [(0, 202), (8, 197), (20, 199), (32, 205), (36, 215), (0, 215)])
    drift(canvas, [(130, 215), (136, 205), (148, 200), (160, 199), (167, 201), (167, 215)])
    stones = Layer(*SIZE)
    for pts in ([(44, 215), (46, 207), (52, 203), (57, 207), (57, 215)],
                [(110, 215), (110, 207), (116, 203), (123, 207), (124, 215)]):
        blob(stones, poly_mask(SIZE, pts), ROCK, 2)
    composite(canvas, stones, OUT)
    remains(canvas)


def build():
    canvas = Image.new('RGBA', SIZE)
    back_peak(canvas)
    massif(canvas)
    gouges(canvas, 28, 150, True)
    gouges(canvas, 134, 134, False)
    ledge(canvas)
    foreground(canvas)
    scorch(canvas, [(66, 132, 26, 6, -4), (84, 120, 26, 5, 2), (102, 132, 24, 6, 5), (62, 212, 18, 8, -3),
                    (106, 212, 18, 8, 3), (128, 168, 14, 5, 2), (50, 178, 28, 6, -7)])
    maw(canvas)
    skull(canvas)
    drakes = Layer(*SIZE)
    perched_dragon(drakes, 146, 104)
    flying_dragon(drakes, 132, 30, 1.15)
    flying_dragon(drakes, 30, 34, 0.85, True)
    composite(canvas, drakes, OUT)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/dragonspine-entrance.bmp')
