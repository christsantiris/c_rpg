#!/usr/bin/env python3
"""Sunken Coast entrance: a drowned shrine grotto in a sea cliff, at the 24px tile scale."""
from pathlib import Path
import math
import random
from PIL import Image
from pixelkit import Layer, composite, flat_wall, line_points, poly_mask, save_sprite, stamp

SIZE = (168, 144)
OUT = (10, 19, 28)
ROCK = [(26, 34, 47), (38, 50, 65), (54, 68, 83), (74, 90, 103), (100, 116, 127), (132, 146, 151)]
STONE = [(46, 54, 60), (66, 76, 80), (90, 100, 100), (118, 126, 120), (148, 154, 142), (182, 186, 168)]
ALGAE = [(30, 46, 44), (42, 62, 54), (58, 82, 64), (80, 104, 74)]
SEA = [(13, 43, 56), (19, 64, 75), (30, 90, 97), (50, 124, 122), (96, 170, 158)]
FOAM = [(170, 210, 200), (228, 240, 230)]
SAND = [(112, 104, 80), (150, 139, 104), (184, 172, 132), (212, 201, 160)]
KELP = [(28, 42, 27), (50, 66, 33), (79, 94, 42), (114, 124, 58)]
CORAL = [(110, 42, 50), (170, 74, 70), (222, 124, 100)]
GOLD = [(92, 70, 34), (150, 118, 54), (204, 170, 86), (240, 218, 140)]
VOID = (7, 16, 22)
TIDE = 93


def cliff(canvas, points, ridge, seed, holes=(), bias=0, guano=3):
    """A weathered sea stack: lit west faces, cold east faces, broken strata, a turf cap and a wet foot."""
    rng = random.Random(seed)
    rock = Layer(*SIZE)
    mask = poly_mask(SIZE, points)
    for hole in holes:
        mask -= poly_mask(SIZE, hole)
    # Each band of rock steps its plane edges a little, like stacked cliff blocks.
    jog = [(rng.randrange(-3, 4), rng.randrange(-3, 4)) for _ in range(SIZE[1] // 12 + 1)]
    left = min(x for x, y in mask)
    # Eroded pits: short 2 px chips that only ever darken the stone.
    pts = sorted(mask)
    chips = set()
    for n in range(len(pts) // 120):
        x, y = pts[rng.randrange(len(pts))]
        chips |= {(x, y), (x + 1, y)}
    for x, y in pts:
        j1, j2 = jog[y // 12]
        lit = x < ridge + j2
        shade = 4 if x < left + (ridge - left) * 2 // 5 + j1 else 3 if lit else 1
        if (x, y - 3) not in mask:
            shade = 4 if lit else 2
        if (x, y - 1) not in mask and lit:
            shade = 5
        if (y - x // 6) % 14 == 0 and (x // 9 + y) % 5:
            shade -= 1
        if (x, y) in chips:
            shade -= 1
        if y >= TIDE:
            shade -= 1
        rock.put(x, y, ROCK[max(0, min(5, shade + bias))])
    # Vertical joints split the faces into blocky cliff columns.
    for n in range(9):
        x = rng.randrange(6, 162)
        y = rng.randrange(24, 70)
        for px, py in line_points(x, y, x + rng.randrange(-2, 3), y + rng.randrange(12, 26)):
            if (px, py) in mask and (px + 1, py) in mask and (px - 1, py) in mask and py < TIDE:
                rock.put(px, py, ROCK[0] if px >= ridge else ROCK[1])
                rock.put(px + 1, py, ROCK[4] if px < ridge else ROCK[2])
    # Short ledges: a dark crack under a lit lip, as on the mountain faces.
    for n in range(8):
        x = rng.randrange(6, 156)
        y = rng.randrange(22, TIDE - 4)
        for px, py in line_points(x, y, x + rng.randrange(5, 12), y + rng.choice((-1, 0, 0, 1))):
            if (px, py) in mask and (px, py - 1) in mask:
                rock.put(px, py, ROCK[0])
                rock.put(px, py - 1, ROCK[4] if px < ridge else ROCK[2])
    # Eroded pockets: a dark hollow with a lit lower lip.
    for n in range(7):
        x = rng.randrange(6, 160)
        y = rng.randrange(30, TIDE - 6)
        w = rng.randrange(3, 6)
        if all((px, py) in mask for px in range(x - 1, x + w + 1) for py in range(y - 1, y + 3)):
            for px in range(x, x + w):
                rock.put(px, y, ROCK[0])
                rock.put(px, y + 1, ROCK[0] if 0 < px - x < w - 1 else ROCK[1])
                rock.put(px, y + 2, ROCK[4] if px < ridge else ROCK[2])
    # Seabird ledges: a whitewashed lip over a dark crack, dripping down the face.
    top = {}
    for x, y in pts:
        top[x] = min(top.get(x, SIZE[1]), y)
    for n in range(guano):
        for tries in range(40):
            x = rng.randrange(left + 4, max(top) - 10)
            y = rng.randrange(top[x] + 8, TIDE - 22)
            if all((px, py) in mask for px in range(x - 2, x + 8) for py in range(y - 2, y + 9)):
                break
        else:
            continue
        w = rng.randrange(4, 7)
        for px in range(x, x + w):
            rock.put(px, y - 1, STONE[5] if px < ridge else ROCK[5])
            rock.put(px, y, ROCK[0])
        for px in rng.sample(range(x, x + w), 2):
            ln = rng.randrange(3, 7)
            for k in range(ln):
                rock.put(px, y + 1 + k, STONE[5] if k < ln // 2 and px < ridge else ROCK[5] if k < ln - 1 else ROCK[4])
    # A turf cap on the flatter tops, with sea-pink thrift and a dark overhang.
    for x in sorted(top):
        t = top[x]
        if abs(top.get(x - 1, t) - t) > 2 or abs(top.get(x + 1, t) - t) > 2:
            continue
        lit = x < ridge
        depth = 2 + (rng.randrange(3) == 0)
        for dy in range(depth):
            rock.put(x, t + dy, KELP[3 if lit else 2] if dy == 0 else KELP[2 if lit else 1])
        rock.put(x, t + depth, KELP[0])
        if rng.randrange(5) == 0:
            rock.put(x, t + depth + 1, KELP[1])
        if rng.randrange(4) == 0:
            rock.put(x, t - 1, KELP[2 if lit else 1])
        elif rng.randrange(9) == 0:
            rock.put(x, t, CORAL[2])
    # A ragged algae line marks the high tide.
    for x, y in sorted(mask):
        if y == TIDE - 1 or (y == TIDE and (x * 7) % 5 < 2) or (y == TIDE + 1 and (x * 7) % 9 == 0):
            rock.put(x, y, ALGAE[2] if x < ridge else ALGAE[1])
    composite(canvas, rock, OUT)


def kelp(L, x, top, length, phase=0.0):
    """A hanging strand of kelp, lit on its left edge, ending in a leaf."""
    dx = 0
    for n in range(length):
        dx = int(round(math.sin(phase + n / 5.0) * 1.4))
        L.put(x + dx, top + n, KELP[3 if n % 6 == 2 else 2])
        if n < length * 2 // 3:
            L.put(x + dx + 1, top + n, KELP[1])
    stamp(L, x + dx - 1, top + length, ['kKk', '.k.'], {'k': KELP[1], 'K': KELP[2]})


def barnacles(L, x, y, pattern):
    """Pale barnacle cones, each with a shadow pixel beneath it."""
    stamp(L, x, y, pattern, {'b': STONE[5], 'o': STONE[1]})


def wave(L, x, y, ln):
    """A lapping crest: a pale curl over a lit swell line."""
    for dx in range(ln):
        L.put(x + dx, y, SEA[3])
    for dx in range(1, ln - 2):
        L.put(x + dx, y - 1, SEA[4])
    for dx in range(2, 5):
        L.put(x + dx, y - 2, FOAM[1] if dx == 3 else FOAM[0])


def build():
    canvas = Image.new('RGBA', SIZE)
    # Open water seen through a natural sea arch in the eastern stack.
    hole = [(134, 112), (134, 98), (137, 90), (142, 85), (149, 85), (154, 91), (156, 100), (156, 112)]
    back = Layer(*SIZE)
    for x, y in sorted(poly_mask(SIZE, hole)):
        back.put(x, y, SEA[1] if y < 95 else SEA[2] if y < 103 else SEA[3])
        if y in (98, 105) and (x // 3) % 3:
            back.put(x, y, SEA[3] if y < 100 else SEA[4])
    composite(canvas, back, None)
    # Stacks and headland: blunt, stratified cousins of the mountain peaks.
    cliff(canvas, [(6, 112), (8, 84), (11, 64), (14, 53), (17, 46), (22, 43), (33, 43), (36, 47), (38, 54),
                   (42, 114)], 26, 11, bias=-1, guano=2)
    cliff(canvas, [(119, 114), (121, 58), (127, 46), (134, 39), (142, 34), (153, 34), (157, 38), (159, 44),
                   (161, 62), (162, 84), (162, 112)], 149, 12, holes=[hole], bias=-1, guano=2)
    cliff(canvas, [(26, 114), (27, 66), (31, 54), (40, 48), (46, 36), (54, 26), (60, 16), (66, 11), (74, 9),
                   (88, 9), (92, 10), (104, 10), (110, 12), (116, 18), (122, 30), (128, 46), (135, 62), (139, 114)],
          94, 13, guano=5)
    # Teal water laps the foot of every stack.
    sea = Layer(*SIZE)
    mask = poly_mask(SIZE, [(8, 104), (20, 102), (50, 101), (84, 100), (120, 101), (150, 102), (160, 104),
                            (160, 132), (8, 132)])
    for x, y in sorted(mask):
        shade = 1 if y < 109 else 2 if y < 121 else 3
        sea.put(x, y, SEA[shade])
        if (x, y - 1) not in mask:
            sea.put(x, y, FOAM[0])
        elif (x, y - 2) not in mask and (x // 2) % 3 == 0:
            sea.put(x, y, FOAM[1])
    for x, y, ln in ((22, 111, 11), (30, 120, 12), (114, 112, 10), (132, 118, 12), (124, 126, 8), (40, 127, 8)):
        wave(sea, x, y, ln)
    composite(canvas, sea, OUT)
    # Wet boulders hide the water's ends; two drowned column stumps break the waves.
    rocks = Layer(*SIZE)
    for x, y, w, h in ((3, 100, 14, 13), (6, 112, 12, 11), (150, 100, 14, 13), (151, 112, 11, 11)):
        m = poly_mask(SIZE, [(x, y + h), (x + 1, y + 4), (x + 4, y), (x + w - 5, y + 1), (x + w, y + 5), (x + w, y + h)])
        for px, py in sorted(m):
            shade = 4 if py < y + 3 else 3 if px < x + w * 2 // 3 else 2
            if py > y + h - 4:
                shade = 1
            rocks.put(px, py, ROCK[shade])
    for x0, top, bottom in ((40, 106, 121), (122, 109, 122)):
        for y in range(top, bottom):
            for w in range(7):
                shade = (4, 4, 3, 3, 2, 1, 1)[w]
                if y > bottom - 5:
                    rocks.put(x0 + w, y, ALGAE[max(0, shade - 2)])
                else:
                    rocks.put(x0 + w, y, STONE[shade])
        for x, dy in zip(range(x0, x0 + 7), (1, 0, -1, -2, 0, 1, 2)):
            rocks.put(x, top + dy, STONE[5] if x < x0 + 4 else STONE[3])
            for y in range(top + dy + 1, top + 3):
                rocks.put(x, y, STONE[4] if x < x0 + 4 else STONE[2])
    composite(canvas, rocks, OUT)
    foam = Layer(*SIZE)
    for x0, x1, y in ((18, 23, 112), (19, 24, 122), (144, 150, 113), (145, 151, 123), (39, 48, 121),
                      (121, 130, 122)):
        for x in range(x0, x1):
            foam.put(x, y, FOAM[1] if (x + y) % 5 else FOAM[0])
            if (x * 3) % 7 < 3:
                foam.put(x, y + 1, FOAM[0])
    composite(canvas, foam, None)
    # The half-drowned shrine: a pediment, two columns (one snapped) and a masonry front.
    shrine = Layer(*SIZE)
    front = poly_mask(SIZE, [(54, 139), (54, 72), (84, 52), (106, 66), (108, 72), (114, 76), (114, 139)])
    flat_wall(shrine, 54, 114, 50, 139, STONE, row_h=7, base=3, seed=5, bw=(7, 12),
              shade=lambda x, y: (1 if x < 66 else 0) - (1 if x > 104 else 0))
    for y in range(SIZE[1]):
        for x in range(SIZE[0]):
            if shrine.on(x, y) and (x, y) not in front:
                shrine.erase(x, y)
    # Raking cornice along the pediment, broken off on the east slope.
    for x, y in line_points(52, 73, 84, 51) + line_points(84, 51, 104, 64):
        shrine.put(x, y, STONE[5] if x <= 84 else STONE[4])
        shrine.put(x, y + 1, STONE[4] if x <= 84 else STONE[3])
        shrine.put(x, y + 2, STONE[1])
    for x in range(52, 117):
        if x > 106 and x % 4 == 0:
            continue
        shrine.put(x, 73, STONE[5] if x < 84 else STONE[4])
        shrine.put(x, 74, STONE[3])
        shrine.put(x, 75, STONE[1])
    # Columns with capitals; the eastern one is snapped off below its capital.
    for x0, top in ((55, 76), (107, 88)):
        for y in range(top, 140):
            for w in range(7):
                shrine.put(x0 + w, y, STONE[(5, 4, 4, 3, 2, 1, 1)[w]])
                if w in (2, 4) and y > top + 1:
                    shrine.put(x0 + w, y, STONE[(5, 4, 4, 3, 2, 1, 1)[w] - 1])
        if top == 76:
            for y in range(76, 80):
                for x in range(x0 - 1, x0 + 8):
                    shrine.put(x, y, STONE[4 if y == 76 else 1 if y == 79 else 3])
        else:
            for x, dy in zip(range(x0, x0 + 7), (0, -2, -3, -1, 1, 2, 0)):
                for y in range(top + dy, top + 2):
                    shrine.put(x, y, STONE[5] if x < x0 + 3 else STONE[3])
    for x, y in sorted(front):
        if y >= 118 and shrine.on(x, y) and shrine.get(x, y) in STONE:
            shrine.put(x, y, ALGAE[min(3, max(0, STONE.index(shrine.get(x, y)) - 1))])
    composite(canvas, shrine, OUT)
    # A round grotto arch ringed by pale voussoirs, with a keystone.
    arch = Layer(*SIZE)
    cx, spring, r = 84, 103, 11
    for y in range(spring - r - 5, 144):
        for x in range(cx - r - 5, cx + r + 6):
            d = math.hypot(x - cx + 0.5, y - spring) if y < spring else abs(x - cx + 0.5)
            if r < d <= r + 4.5:
                wet = y >= 118
                if y < spring:
                    seg = int((math.atan2(y - spring, x - cx + 0.5) + math.pi) * 3.6) % 2
                    c = STONE[5] if seg else STONE[4]
                    if d > r + 3.5:
                        c = STONE[2]
                else:
                    c = STONE[4] if (y - spring) // 5 % 2 else STONE[3]
                    if (y - spring) % 5 == 4 or abs(x - cx + 0.5) > r + 3.5:
                        c = STONE[2]
                if wet:
                    c = ALGAE[min(3, STONE.index(c) - 1)]
                arch.put(x, y, c)
    for y in range(spring - r - 6, spring - r + 1):
        for x in range(cx - 4, cx + 4):
            arch.put(x, y, STONE[5] if x < cx else STONE[4])
    composite(canvas, arch, OUT)
    door = Layer(*SIZE)
    for y in range(spring - r, 144):
        for x in range(cx - r, cx + r):
            d = math.hypot(x - cx + 0.5, y - spring) if y < spring else 0
            if d <= r:
                door.put(x, y, VOID if y < 122 else SEA[0])
    for x, y in ((77, 124), (87, 125), (81, 127)):
        for dx in range(4):
            door.put(x + dx, y, SEA[2])
    for y in (130, 135, 140):
        for x in range(cx - r, cx + r):
            door.put(x, y - 1, STONE[3] if (x + y) % 9 else ALGAE[2])
            door.put(x, y, STONE[2])
            door.put(x, y + 1, STONE[0])
    composite(canvas, door, None)
    # A pale sand strip in front, with a sunlit lip and two small tide pools.
    sand = Layer(*SIZE)
    smask = poly_mask(SIZE, [(12, 144), (6, 140), (4, 135), (7, 130), (16, 127), (28, 126), (42, 129), (54, 133),
                             (54, 144)])
    smask |= poly_mask(SIZE, [(114, 144), (114, 133), (126, 129), (140, 126), (152, 127), (161, 130), (164, 135),
                              (162, 140), (156, 144)])
    smask |= poly_mask(SIZE, [(52, 139), (66, 140), (66, 144), (52, 144)])
    smask |= poly_mask(SIZE, [(101, 140), (116, 139), (116, 144), (101, 144)])
    # The shore's front edge wanders, so the outline breaks the bottom row away from the door.
    for x0, x1, y0 in ((14, 21, 142), (31, 38, 143), (45, 49, 142), (119, 124, 142), (131, 139, 143),
                       (146, 152, 142)):
        smask -= {(x, y) for x in range(x0, x1) for y in range(y0, SIZE[1])}
    rng = random.Random(14)
    for x, y in sorted(smask):
        shade = 2
        if (x, y - 1) not in smask:
            shade = 3
        elif (x, y - 2) not in smask and x % 3 == 0:
            shade = 3
        elif (x, y + 1) not in smask or ((x - 1, y) not in smask and x < 52) or ((x + 1, y) not in smask and x > 116):
            shade = 1
        elif rng.randrange(19) == 0:
            shade = 1
        sand.put(x, y, SAND[shade])
    for px, py, rx, ry in ((26, 138, 9, 3), (142, 139, 8, 3)):
        for y in range(py - ry - 1, py + ry + 2):
            for x in range(px - rx - 2, px + rx + 3):
                e = ((x - px) / rx) ** 2 + ((y - py) / ry) ** 2
                if e <= 1:
                    sand.put(x, y, SEA[1] if y < py - 1 else SEA[2])
                    if y == py and abs(x - px + 2) < 3:
                        sand.put(x, y, SEA[4])
                elif e <= 1.7:
                    sand.put(x, y, SAND[1] if y < py else SAND[3])
    composite(canvas, sand, OUT)
    # Barnacle crusts along the tide line, the arch foot and the drowned stumps.
    crust = Layer(*SIZE)
    p1 = ['b.b..', 'obob.', '.o.o.']
    p2 = ['.b.', 'bob', 'o.o']
    p3 = ['b.', 'ob', '.o']
    for x, y, pat in ((56, 116, p1), (64, 123, p2), (100, 119, p2), (110, 104, p3), (68, 111, p3), (98, 129, p1),
                      (42, 115, p2), (124, 116, p3), (5, 108, p1), (156, 109, p2)):
        barnacles(crust, x, y, pat)
    composite(canvas, crust, None)
    detail = Layer(*SIZE)
    # Kelp hangs from the cornice, the capital and the arch.
    for x, top, length, ph in ((63, 76, 22, 0.0), (68, 76, 12, 1.5), (100, 76, 16, 3.0), (104, 76, 9, 2.0),
                               (75, 95, 11, 1.0), (92, 95, 8, 2.5), (55, 80, 14, 0.5)):
        kelp(detail, x, top, length, ph)
    # Branching coral, a starfish and shells where sand meets the shrine foot.
    coral = ['c...c..', 'C.c.C.c', 'C.C.CC.', '.CCcC..', '..CCC..', '...C...', '..CCC..']
    for x, y in ((60, 132), (101, 132), (30, 123), (133, 123)):
        stamp(detail, x, y, coral, {'c': CORAL[2], 'C': CORAL[1]})
    stamp(detail, 10, 132, ['...s...', '..sSs..', 'sSSSSSs', '.sSSSs.', '.sS.Ss.', 's.....s'],
          {'s': CORAL[1], 'S': CORAL[2]})
    for x, y in ((44, 137), (122, 136), (157, 139)):
        stamp(detail, x, y, ['..H..', '.WHW.', 'WWHWW', 'wwwww'], {'w': SAND[0], 'W': SAND[3], 'H': FOAM[1]})
    # The Drowned Queen's crown crests the pediment over a trident medallion.
    stamp(detail, 79, 46, ['H...HH...H', 'G..GHHG..G', 'GG.GGGG.GG', 'GGGGGGGGGG', 'HGgGHHGgGH', 'gggggggggg'],
          {'g': GOLD[1], 'G': GOLD[2], 'H': GOLD[3]})
    for y in range(57, 72):
        for x in range(76, 93):
            d = math.hypot(x - 83.5, y - 64)
            if d <= 7:
                detail.put(x, y, SEA[0] if d < 6 else GOLD[1])
    stamp(detail, 80, 58, ['H..HH..H', 'G..GG..G', 'G..GG..G', 'GG.GG.GG', '.GGGGGG.', '...GG...', '...GG...',
                           '...GG...', '...GG...', '...gg...'],
          {'g': GOLD[1], 'G': GOLD[2], 'H': GOLD[3]})
    for x, y in ((58, 98), (108, 99)):
        stamp(detail, x, y, ['.w.', 'wWw', '.w.'], {'w': SEA[3], 'W': FOAM[1]})
    # Marram grass tufts at the dune ends tie the shore to the grass around it.
    for x, y in ((5, 124), (156, 125), (47, 127)):
        stamp(detail, x, y, ['k...k..', '.K.k.K.', '.kKkK..', 'kKKKKk.'], {'k': KELP[1], 'K': KELP[3]})
    composite(canvas, detail, OUT)
    # Gulls wheel off the headland.
    gulls = Layer(*SIZE)
    for x, y in ((98, 24), (110, 33), (62, 38)):
        stamp(gulls, x, y, ['WW.WW', '..W..'], {'W': FOAM[1]})
    composite(canvas, gulls, None)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/coast-entrance.bmp')
