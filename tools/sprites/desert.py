#!/usr/bin/env python3
"""Sunscar Wastes tomb gateway, drawn in the established 24px pixelkit pipeline."""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, clamp, composite, line_points, poly_mask, save_sprite, stamp

SIZE = (168, 144)
OUT = (31, 18, 11)
SAND = [(94, 57, 31), (132, 88, 45), (166, 120, 62), (192, 148, 81), (211, 173, 103), (228, 199, 136)]
STONE = [(61, 40, 30), (90, 62, 45), (121, 88, 63), (152, 116, 83), (179, 144, 106), (204, 173, 132)]
GOLD = [(118, 76, 27), (182, 134, 45), (226, 191, 92)]
TURQ = [(21, 86, 92), (44, 152, 146), (132, 220, 196)]
BONE = [(143, 129, 104), (196, 184, 156), (231, 223, 200)]
CACTUS = [(52, 66, 40), (78, 94, 52), (112, 124, 70), (150, 156, 96)]
SUN = [(140, 46, 28), (206, 88, 40), (242, 150, 70)]
VOID = [(18, 10, 8), (31, 19, 13)]
GLYPHS = [['.d.', 'd.d', '.d.', 'ddd', '.d.'], ['ddd', 'd.d', 'ddd'], ['d.d', 'd.d', 'ddd'],
          ['.dd', 'dd.', '.d.'], ['dd.', '.dd', 'dd.'], ['ddd', '.d.', '.d.'], ['.d.', 'ddd', '.d.']]


def glyphs(L, x, y0, y1, rng, ink):
    """A column of tiny carved hieroglyphs between y0 and y1."""
    y = y0
    while True:
        g = rng.choice(GLYPHS)
        if y + len(g) > y1:
            return
        stamp(L, x, y, g, {'d': ink})
        y += len(g) + 1


def pyramid(canvas, ax, ay, base_y, half_base, ridge_x, seed):
    """Block-coursed pyramid lit from the upper left, with a gold capstone."""
    rng = random.Random(seed)
    L = Layer(*SIZE)
    for y in range(ay, base_y + 1):
        t = (y - ay) / float(base_y - ay)
        half = t * half_base
        rx = ax + t * (ridge_x - ax)
        course = (y - ay) // 6
        yy = (y - ay) % 6
        x0 = math.ceil(ax - half - 0.5) + (1 if yy == 0 and course > 2 else 0)
        x1 = math.floor(ax + half + 0.5) - (1 if yy == 0 and course > 2 else 0)
        for x in range(x0, x1 + 1):
            lit = x < rx
            shade = 4 if lit else 2
            if y - ay < 7:
                L.put(x, y, GOLD[2] if lit else GOLD[1])
                continue
            if y - ay < 16:
                shade += 1 if lit else 0
            elif yy == 5:
                shade -= 1
            elif (x + course * 7) % 13 == 0:
                shade -= 1
            elif rng.randrange(31) == 0:
                shade += rng.choice((-1, 1))
            L.put(x, y, STONE[clamp(shade, 0, 5)])
    composite(canvas, L, OUT)


def smooth(points, rounds=3):
    """Chaikin corner cutting: rounds a crest polyline but keeps its end points."""
    for _ in range(rounds):
        out = [points[0]]
        for (x0, y0), (x1, y1) in zip(points, points[1:]):
            out += [(x0 * 0.75 + x1 * 0.25, y0 * 0.75 + y1 * 0.25), (x0 * 0.25 + x1 * 0.75, y0 * 0.25 + y1 * 0.75)]
        points = out + [points[-1]]
    return [(round(x), round(y)) for x, y in points]


def dune(canvas, crest, foot, lee, seed, base=4):
    """A sand heap: lit windward slope, shaded lee face and crest-following ripples."""
    mask = poly_mask(SIZE, smooth(crest) + foot)
    shadow = poly_mask(SIZE, smooth(lee) + [(170, 144), (170, 90)])
    top = {}
    for x, y in mask:
        top[x] = min(top.get(x, 999), y)
    L = Layer(*SIZE)
    for x, y in sorted(mask):
        d = y - top[x]
        dark = (x, y) in shadow
        shade = base - 2 if dark else base
        wave = round(2 * math.sin(x * 0.17 + seed))
        if d == 0 and not dark:
            shade += 1
        elif not dark and d > 3 and (d + wave) % 6 == 0 and (x * 3 + d * 5 + seed) % 17 < 14:
            shade -= 1
        elif dark and d > 5 and (d + wave) % 9 == 0 and (x + seed) % 13 < 8:
            shade -= 1
        L.put(x, y, SAND[clamp(shade, 0, 5)])
    composite(canvas, L, OUT)


def column(canvas, x0, x1, tops, y1):
    """A broken drum column, rounded and lit from the left, with a jagged break."""
    L = Layer(*SIZE)
    cx = (x0 + x1 + 1) / 2.0
    for x in range(x0, x1 + 1):
        top = tops[x - x0]
        for y in range(top, y1 + 1):
            t = (x + 0.5 - cx) / ((x1 - x0 + 1) / 2.0)
            shade = 4 if t < -0.55 else 3 if t < 0.15 else 2 if t < 0.7 else 1
            if y == top:
                shade = 5 if t < 0.15 else 3
            elif (y - 70) % 10 == 9:
                shade -= 1
            elif (y - 70) % 10 == 0:
                shade += 1
            if y > top and (x - x0) % 2 == 1 and 0 < x - x0 < x1 - x0:
                shade -= 1
            L.put(x, y, STONE[clamp(shade, 0, 5)])
    composite(canvas, L, OUT)


def cactus(canvas, x, y):
    """A two-armed saguaro with lit and shaded ribs."""
    L = Layer(*SIZE)
    for x0, x1, y0, y1 in ((x, x + 5, y, y + 50), (x - 7, x - 3, y + 10, y + 23), (x + 9, x + 13, y + 17, y + 31)):
        for yy in range(y0, y1 + 1):
            for xx in range(x0, x1 + 1):
                k = xx - x0
                if yy == y0 and k in (0, x1 - x0):
                    continue
                c = CACTUS[3] if k == 0 else CACTUS[0] if xx == x1 else CACTUS[1] if k % 2 else CACTUS[2]
                L.put(xx, yy, c)
    for yy in range(y + 20, y + 24):
        for xx in range(x - 3, x):
            L.put(xx, yy, CACTUS[2] if yy < y + 22 else CACTUS[1])
    for yy in range(y + 28, y + 32):
        for xx in range(x + 6, x + 9):
            L.put(xx, yy, CACTUS[2] if yy < y + 30 else CACTUS[1])
    composite(canvas, L, OUT)


def cornice(L, x0, x1, y0):
    """A flared cavetto cornice: lit lip, reeded cove and a shadowed torus."""
    for y in range(y0, y0 + 7):
        inset = 1 if y > y0 + 3 else 0
        for x in range(x0 + inset, x1 + 1 - inset):
            if y == y0:
                c = STONE[5]
            elif y == y0 + 6:
                c = STONE[1]
            elif y == y0 + 5:
                c = STONE[2]
            else:
                c = (STONE[4], STONE[3], STONE[2])[x % 3]
            L.put(x, y, c)


def build():
    canvas = Image.new('RGBA', SIZE)
    # A block-coursed pyramid rises behind the half-buried tomb gateway.
    pyramid(canvas, 84, 4, 116, 66, 102, 11)
    # A gold-tipped obelisk on the left, half sunk in sand.
    ob = Layer(*SIZE)
    for y in range(30, 118):
        if y < 38:
            half = (y - 30) * 0.55
            for x in range(math.ceil(28 - half), math.floor(28 + half) + 1):
                ob.put(x, y, GOLD[2] if x < 28 else GOLD[1])
            continue
        for x in range(24, 33):
            ob.put(x, y, STONE[5] if x == 24 else STONE[4] if x < 27 else STONE[3] if x < 31 else STONE[2])
    glyphs(ob, 27, 42, 108, random.Random(5), STONE[1])
    composite(canvas, ob, OUT)
    # A broken column and a saguaro on the right.
    column(canvas, 129, 139, [80, 78, 76, 75, 77, 80, 82, 79, 76, 74, 77], 112)
    cactus(canvas, 153, 64)
    # Two battered pylons with fluted cornices and inscribed panels.
    pylons = Layer(*SIZE)
    for pts, lit_left in (([(42, 136), (48, 63), (72, 63), (72, 136)], True), ([(96, 136), (96, 63), (120, 63), (126, 136)], False)):
        mask = poly_mask(SIZE, pts)
        for x, y in sorted(mask):
            pylons.put(x, y, STONE[2] if (y - 63) % 8 == 7 else STONE[3])
        for y in range(63, 137):
            row_x = sorted(x for x in range(40, 130) if (x, y) in mask)
            if not row_x:
                continue
            if lit_left:
                pylons.put(row_x[0], y, STONE[5])
                pylons.put(row_x[1], y, STONE[4])
            else:
                pylons.put(row_x[-1], y, STONE[1])
                pylons.put(row_x[-2], y, STONE[2])
        top = sorted(x for x, y in mask if y == 63)
        cornice(pylons, top[0] - 2, top[-1] + 2, 56)
        px0 = 50 if lit_left else 107
        for y in range(70, 116):
            for x in range(px0, px0 + 12):
                c = STONE[3]
                if y == 70 or x == px0:
                    c = STONE[1]
                elif y == 115 or x == px0 + 11:
                    c = STONE[4]
                pylons.put(x, y, c)
        prng = random.Random(px0)
        for gx in (px0 + 2, px0 + 7):
            glyphs(pylons, gx, 72, 114, prng, STONE[1])
    # Wind-blown sand has drifted onto the cornice tops.
    for x0, x1 in ((47, 61), (107, 121)):
        for x in range(x0, x1 + 1):
            h = min(x - x0, x1 - x, 3)
            for y in range(56 - h, 56):
                pylons.put(x, y, SAND[4] if y == 56 - h else SAND[3])
    composite(canvas, pylons, OUT)
    # The portal: cornice, winged scarab lintel and inscribed jambs.
    portal = Layer(*SIZE)
    for y in range(81, 144):
        for x in range(65, 104):
            c = STONE[4]
            if x == 65:
                c = STONE[5]
            elif x == 103:
                c = STONE[1]
            elif x == 102:
                c = STONE[2]
            portal.put(x, y, c)
    cornice(portal, 63, 105, 74)
    jrng = random.Random(41)
    glyphs(portal, 68, 101, 136, jrng, STONE[1])
    glyphs(portal, 98, 101, 136, jrng, STONE[1])
    # Winged sun disc and turquoise scarab above the door.
    for r in range(7):
        reach = 14 - r * 2
        for side in (-1, 1):
            for k in range(reach):
                if r < 1:
                    c = GOLD[2]
                elif r < 3:
                    c = TURQ[1] if k % 2 else TURQ[2]
                elif r < 5:
                    c = GOLD[1] if k % 2 else GOLD[2]
                else:
                    c = TURQ[0] if k % 2 else TURQ[1]
                portal.put(84 + side * (6 + k), 86 + r, c)
    stamp(portal, 81, 81, ['..sss..', '.sSSSs.', 'sSHSSSs', 'sSSSSSs', '.sSSSs.'],
          {'s': SUN[0], 'S': SUN[1], 'H': SUN[2]})
    stamp(portal, 80, 86,['g..ggg..g', '.gtTTTtg.', 'g.tTTTt.g', '.ggGgGgg.', 'gtTTgTtTg', '.tTTgTtt.', 'gtTTgTttg', '..ttgtt..'],
          {'g': GOLD[1], 'G': GOLD[2], 't': TURQ[0], 'T': TURQ[2]})
    composite(canvas, portal, OUT)
    # A dark stair descends into the tomb; the lit east reveal shows the wall depth.
    door = Layer(*SIZE)
    for y in range(97, 144):
        for x in range(73, 96):
            door.put(x, y, VOID[0] if y < 122 else VOID[1])
            if x > 93 and y < 124:
                door.put(x, y, STONE[2] if x == 94 else STONE[1])
    for n, y in enumerate((124, 129, 134, 139)):
        for x in range(73, 96):
            door.put(x, y, STONE[1 + n // 2] if x < 94 else STONE[1])
            door.put(x, y + 1, STONE[0])
            if (x < 76 or x > 91) and n > 1:
                door.put(x, y, SAND[2])
    composite(canvas, door, None)
    # Front dunes heap against the pylons, half burying them; their feet taper to the ground inside the canvas.
    dune(canvas, [(3, 144), (7, 134), (12, 121), (26, 106), (42, 99), (50, 103), (60, 114), (71, 128)],
         [(71, 144), (3, 144)], [(43, 98), (44, 112), (49, 128), (56, 144)], 1, base=3)
    dune(canvas, [(97, 130), (104, 121), (116, 109), (128, 101), (136, 99), (146, 103), (158, 114), (163, 128), (165, 144)],
         [(165, 144), (97, 144)], [(137, 98), (139, 114), (145, 130), (152, 144)], 2, base=3)
    dune(canvas, [(4, 144), (9, 134), (18, 126), (31, 130), (41, 137), (49, 144)],
         [(4, 144)], [(19, 124), (21, 132), (26, 144)], 4, base=3)
    dune(canvas, [(117, 145), (128, 136), (141, 130), (152, 128), (159, 133), (163, 144)],
         [(163, 144)], [(153, 127), (155, 135), (160, 144)], 5, base=3)
    # A sun-bleached longhorn skull on the left dune crest, ringed in umber against the sky.
    skull = Layer(*SIZE)
    stamp(skull, 11, 106, ['B.........B', 'Bb.......bB', '.BbbWWWbbB.', '...WoWoW...', '...WWWWW...', '....WWW....', '....WoW....', '.....W.....'],
          {'b': BONE[0], 'B': BONE[1], 'W': BONE[2], 'o': OUT})
    composite(canvas, skull, OUT)
    # A half-buried ribcage: a spine along the front dune with ribs arching out of the sand.
    ribs = Layer(*SIZE)
    for k, (x, y) in enumerate(line_points(124, 139, 150, 131)):
        ribs.put(x, y, BONE[1])
        ribs.put(x, y + 1, BONE[0] if k % 3 == 0 else BONE[1])
    for bx, h in ((128, 12), (134, 10), (140, 8), (146, 6)):
        by = 139 - round((bx - 124) * 8 / 26.0)
        for i in range(h + 1):
            t = i / float(h)
            x = bx + round(h * 0.45 * (1 - math.cos(t * math.pi / 2)))
            y = by - i
            ribs.put(x, y, BONE[2])
            if i < h - 2:
                ribs.put(x + 1, y, BONE[0])
    composite(canvas, ribs, SAND[1])
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/desert-entrance.bmp')
