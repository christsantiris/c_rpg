#!/usr/bin/env python3
"""Stormwatch runtime and review art at the established native pixel scale.

Two poses per creature in padded 48px cells. Ordinary visible artwork stays
within 24x24; the Matriarch's spread wings deliberately occupy a wider frame.
Run directly to rebuild runtime BMPs and the review atlas under docs/art-sources.
"""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, poly_mask, line_points, stamp, save_sprite, flat_wall

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / 'docs/art-sources/stormwatch'
OUT = (19, 26, 32)
FEATHERS = [(43, 49, 56), (65, 75, 82), (91, 107, 113), (130, 148, 148), (186, 199, 186)]
PALE = [(67, 86, 99), (99, 123, 137), (139, 164, 172), (182, 201, 196), (222, 229, 205)]
STONE = [(41, 46, 49), (66, 73, 75), (99, 107, 105), (139, 146, 134), (177, 182, 163)]
SKIN = [(91, 63, 48), (137, 96, 64), (184, 141, 94), (216, 179, 125)]
CLOTH = [(31, 41, 55), (43, 62, 77), (66, 91, 105), (108, 138, 145)]
GOLD = [(100, 76, 37), (166, 124, 52), (222, 183, 88)]
STEEL = [(58, 69, 75), (104, 124, 133), (191, 208, 205)]


def shape(canvas, points, colors):
    part = Layer(*canvas.size)
    for x, y in sorted(poly_mask(canvas.size, points)):
        shade = 2 + ((x + y // 4) % 7 < 2) - ((x + y) % 9 > 6)
        part.put(x, y, colors[max(0, min(len(colors) - 1, shade))])
    composite(canvas, part, OUT)


def stroke(part, points, color):
    for a, b in zip(points, points[1:]):
        for x, y in line_points(*a, *b):
            part.put(x, y, color)


def harpy(caller, pose):
    canvas = Image.new('RGBA', (48, 48))
    feather = PALE if caller else FEATHERS
    sway = pose
    # Folded flight feathers, proportioned to the existing 24px player body.
    shape(canvas, [(19, 32), (16, 34), (15, 41), (18, 40), (17, 43),
                   (22, 40), (23, 34)], feather)
    shape(canvas, [(28, 32), (31, 33), (32, 40), (29, 42), (27, 38)], feather)
    shape(canvas, [(22, 32), (27, 32), (29, 40), (26, 42), (21, 40)], CLOTH)
    shape(canvas, [(23, 26), (27, 26), (28, 30), (26, 33), (22, 30)], SKIN)
    details = Layer(48, 48)
    stamp(details, 21, 25, ['.pp.pp.', 'ppPPPp.', '.pPpp..'],
          {'p': feather[1], 'P': feather[4]})
    details.put(26, 29, (236, 219, 148))
    details.put(27, 30, SKIN[0])
    for x in range(22, 29):
        details.put(x, 39, GOLD[1])
    for x in (22, 27):
        stroke(details, [(x, 42), (x, 45), (x - 2, 46)], GOLD[1])
        details.put(x + 1, 46, GOLD[2])
    for x, y in ((17, 37), (18, 36), (19, 35), (29, 36)):
        stroke(details, [(x, y), (x, y + 3)], feather[3])
    if caller:
        # Short rune staff, pale feather mantle and a blue storm sash.
        stroke(details, [(28, 34), (32, 36), (34, 34)], SKIN[2])
        stroke(details, [(34, 29), (34, 44)], GOLD[0])
        stroke(details, [(35, 29), (35, 43)], GOLD[1])
        stamp(details, 32 + sway, 26, ['.a.', 'aAa', '.a.'],
              {'a': (92, 163, 185), 'A': (223, 241, 225)})
        stroke(details, [(23, 33), (27, 35), (23, 38)], PALE[3])
    else:
        stroke(details, [(28, 34), (31, 36), (33, 35)], SKIN[2])
        stroke(details, [(33, 33), (34, 31 - sway), (34, 37)], STEEL[2])
        stroke(details, [(33, 34), (33, 39)], STEEL[0])
        stroke(details, [(31, 39), (35, 39)], GOLD[1])
        details.put(33, 40, SKIN[0])
    composite(canvas, details, OUT)
    return canvas


def gargoyle(pose):
    canvas = Image.new('RGBA', (48, 48))
    # Compact folded stone wings and planted claws; no miniature giant scale.
    shape(canvas, [(21, 33), (15, 29 + pose), (14, 38), (17, 36), (19, 41), (23, 40)], STONE)
    shape(canvas, [(29, 33), (33, 29 + pose), (34, 39), (31, 37), (29, 42), (27, 39)], STONE)
    shape(canvas, [(21, 32), (28, 32), (30, 38), (28, 43), (21, 43), (19, 38)], STONE)
    shape(canvas, [(22, 26), (27, 26), (29, 29), (27, 33), (22, 32), (20, 29)], STONE)
    details = Layer(48, 48)
    stamp(details, 21, 25, ['s....s', 'S....S'], {'s': STONE[1], 'S': STONE[3]})
    stamp(details, 22, 29, ['A..A', '.dd.', 's..s'], {'A': GOLD[2], 'd': STONE[0], 's': STONE[3]})
    for x in (20, 27):
        stamp(details, x, 42, ['.ss.', '.SS.', 'sSSs', 's.ss', 'S.SS'], {'s': STONE[1], 'S': STONE[3]})
    stroke(details, [(15, 32), (18, 35), (20, 39)], STONE[3])
    stroke(details, [(32, 32), (30, 36), (29, 39)], STONE[3])
    stroke(details, [(25, 34), (24, 37), (26, 40)], STONE[0])
    composite(canvas, details, OUT)
    return canvas


def elemental(pose):
    canvas = Image.new('RGBA', (48, 48))
    wind = [(32, 62, 76), (48, 95, 117), (79, 134, 152), (132, 186, 194), (214, 232, 218)]
    shape(canvas, [(20, 28), (26, 25), (31, 27), (33, 31), (29, 34),
                   (31, 38), (27, 43), (26, 46), (22, 46), (21, 42),
                   (17, 39), (15, 34), (17, 30)], wind)
    mist = Layer(48, 48)
    for x, y, width in ((18 + pose, 29, 12), (16, 34 + pose, 15), (19 - pose, 39, 11), (22, 44, 5)):
        stroke(mist, [(x, y + 1), (x + 2, y), (x + width - 3, y), (x + width, y - 1)], wind[3])
    stamp(mist, 23, 31, ['A..A', '.aa.'], {'A': wind[4], 'a': wind[0]})
    stroke(mist, [(26, 35), (23, 38), (26, 39), (23, 42)], wind[4])
    composite(canvas, mist, None)
    return canvas


def matriarch(pose):
    canvas = Image.new('RGBA', (48, 48))
    lift = pose * 2
    shape(canvas, [(20, 32), (12, 25 - lift), (6, 18 - lift), (4, 22), (7, 31),
                   (11, 29), (12, 35), (16, 33), (18, 39), (22, 37)], PALE)
    shape(canvas, [(28, 32), (35, 25 - lift), (41, 18 - lift), (43, 22), (40, 31),
                   (36, 29), (35, 35), (31, 33), (29, 39), (25, 37)], PALE)
    shape(canvas, [(21, 29), (27, 29), (30, 39), (26, 43), (20, 40)], CLOTH)
    shape(canvas, [(22, 23), (27, 23), (28, 28), (25, 31), (21, 27)], SKIN)
    details = Layer(48, 48)
    stamp(details, 21, 20, ['g...g.g', 'Gg.GGg.', '.GGGG..', '..GG...'], {'g': GOLD[0], 'G': GOLD[2]})
    stamp(details, 23, 26, ['a..a'], {'a': PALE[4]})
    for x in range(21, 29):
        details.put(x, 36, GOLD[1])
    stroke(details, [(22, 39), (22, 44), (19, 46), (24, 46)], GOLD[2])
    stroke(details, [(27, 39), (27, 44), (30, 46), (26, 46)], GOLD[2])
    for a, b in [((7, 22), (17, 33)), ((11, 23), (20, 34)), ((39, 22), (30, 33)), ((35, 23), (27, 34))]:
        stroke(details, [a, b], PALE[4])
    stroke(details, [(29, 32), (33, 35), (36, 34)], GOLD[1])
    stroke(details, [(36, 30), (35, 39), (33, 43)], STEEL[2])
    composite(canvas, details, OUT)
    return canvas


def entrance():
    """Wind-weathered curtain wall and watchtower, built at native tile scale."""
    canvas = Image.new('RGBA', (168, 144))
    masonry = Layer(168, 144)
    stone = [(32, 42, 47), (50, 63, 68), (72, 86, 91), (97, 112, 115), (129, 143, 140), (163, 174, 157)]
    for x0, x1, top, bottom in ((19, 150, 100, 142), (57, 112, 37, 141), (22, 50, 65, 141), (122, 148, 73, 141)):
        flat_wall(masonry, x0, x1, top, bottom, stone, row_h=6, base=3, seed=x0, bw=(8, 14))
        for x in range(x0, x1):
            masonry.put(x, top, stone[5])
        for x in range(x0, x1, 9):
            for xx in range(x, min(x + 5, x1)):
                for yy in range(top - 6, top):
                    masonry.put(xx, yy, stone[4 if yy == top - 6 else 2])
    # Projecting courses, recessed arrow slits and damaged blue-gray masonry.
    for x0, x1, y in ((57,112,44),(57,112,93),(22,50,88),(122,148,96),(19,150,139)):
        for x in range(x0, x1):
            masonry.put(x, y, stone[5])
            masonry.put(x, y + 1, stone[0])
    for x0, y0 in ((32,77),(132,85),(69,56),(96,56),(80,80)):
        stamp(masonry, x0, y0, ['.ss.', 'sdds', 'sdds', 'sdds', 'sdds', 'sdds', '.ss.'], {'s': stone[1], 'd': (13,23,29)})
    # Human-sized arched entrance: 13px wide, 19px tall, aligned to the road.
    for y in range(119, 143):
        half = min(10, y - 118)
        for x in range(84 - half, 85 + half):
            masonry.put(x, y, stone[5 if x < 84 else 2])
        if y >= 123:
            inner = min(6, y - 122)
            for x in range(84 - inner, 85 + inner):
                masonry.put(x, y, (15, 23, 27))
    composite(canvas, masonry, OUT)
    detail = Layer(168, 144)
    # Ochre wind pennants, timber shutter, sea-green moss and narrow stairs.
    stroke(detail, [(84, 10), (84, 31)], GOLD[0])
    for x,y in poly_mask(canvas.size, [(85,11),(104,15),(98,19),(85,17)]):
        detail.put(x,y, GOLD[2 if y % 3 else 1])
    for x in range(78, 91):
        detail.put(x, 125, GOLD[0])
    for x in (79, 82, 86, 89):
        stroke(detail, [(x,126),(x,129)], STONE[0])
    for x0,y0 in ((24,108),(145,119),(58,96),(109,111)):
        for n in range(15):
            detail.put(x0 + n % 3 - 1, y0 + n, (52 + n % 3 * 9, 83 + n % 3 * 6, 66))
    for y in (140, 143):
        for x in range(76, 93):
            detail.put(x,y,stone[4])
    composite(canvas, detail, None)
    return canvas


def build():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    atlas = Image.new('RGBA', (240, 96))
    names = ['Harpy Raider', 'Cliff Gargoyle', 'Harpy Stormcaller', 'Storm Elemental', 'Tempest Matriarch']
    for pose in range(2):
        frames = [harpy(False, pose), gargoyle(pose), harpy(True, pose), elemental(pose), matriarch(pose)]
        for i, frame in enumerate(frames):
            bounds = frame.getbbox()
            width, height = bounds[2] - bounds[0], bounds[3] - bounds[1]
            assert bounds[3] == 48, (names[i], bounds)
            assert i == 4 or (width <= 24 and height <= 24), (names[i], bounds)
            atlas.alpha_composite(frame, (i * 48, pose * 48))
            print(f'{names[i]}, pose {pose}: {width}x{height}, feet at 47')
    save_sprite(atlas, OUTPUT / 'enemies.bmp')
    save_sprite(atlas, ROOT / 'assets/images/stormwatch-enemies.bmp')
    save_sprite(entrance(), ROOT / 'assets/images/stormwatch-entrance.bmp')
    atlas.save(OUTPUT / 'enemies.png')


if __name__ == '__main__':
    build()
