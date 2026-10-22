#!/usr/bin/env python3
"""Goblin mountain entrance, using the town sprites' 24px tile pipeline."""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, flat_wall, line_points, poly_mask, save_sprite, stamp

SIZE = (168, 144)
OUT = (17, 21, 22)
ROCK = [(37, 42, 43), (51, 57, 57), (68, 75, 73), (89, 96, 90), (113, 119, 108), (141, 144, 124)]
WOOD = [(38, 27, 22), (67, 44, 29), (101, 66, 36), (143, 99, 53)]
IRON = [(28, 31, 30), (67, 74, 69), (119, 125, 108)]


def build():
    canvas = Image.new('RGBA', SIZE)
    rng = random.Random(73)
    # Overlapping, jagged peaks, with broad lit faces and cold shadow facets.
    for points, ridge in [([(4, 122), (18, 70), (35, 46), (42, 55), (53, 32), (89, 126)], 53),
                          ([(76, 129), (103, 37), (118, 49), (129, 24), (144, 61), (164, 133)], 129),
                          ([(29, 135), (49, 76), (64, 48), (79, 8), (87, 17), (95, 4), (113, 60), (143, 137)], 95)]:
        rock = Layer(*SIZE)
        mask = poly_mask(SIZE, points)
        for x, y in sorted(mask):
            shade = 3 if x < ridge else 1
            if (y + x // 7) % 17 < 2:
                shade -= 1
            if rng.randrange(29) == 0:
                shade += rng.choice((-1, 1))
            rock.put(x, y, ROCK[max(0, min(5, shade))])
        # Angular ledges interrupt the regular strata and give each peak volume.
        for n in range(14):
            x = rng.randrange(8, 158)
            y = rng.randrange(25, 137)
            for px, py in line_points(x, y, x + rng.randrange(4, 15), y - rng.randrange(0, 4)):
                if (px, py) in mask:
                    rock.put(px, py, ROCK[0])
                    if (px, py - 1) in mask:
                        rock.put(px, py - 1, ROCK[4])
        composite(canvas, rock, OUT)
    # Broken scree, moss and small boulders soften the foot of the mountain.
    scree = Layer(*SIZE)
    for i in range(48):
        x, y = rng.randrange(8, 162), rng.randrange(119, 142)
        for px, py in poly_mask(SIZE, [(x, y), (x + 3, y - 2), (x + 7, y + 1), (x + 5, y + 3), (x, y + 2)]):
            scree.put(px, py, ROCK[3] if py <= y else ROCK[1])
    for x, y in [(16, 122), (33, 115), (130, 131), (151, 124), (53, 91), (117, 84)]:
        for px, py in line_points(x, y, x + 6, y + 1):
            scree.put(px, py, (61, 78, 48))
    composite(canvas, scree, OUT)
    # A deep cave mouth, reinforced by rough goblin masonry and timber.
    arch = Layer(*SIZE)
    flat_wall(arch, 57, 109, 97, 137, ROCK, row_h=7, base=2, seed=71, bw=(7, 12))
    for y in range(91, 138):
        half = min(21, 4 + (y - 91))
        for x in range(84 - half, 85 + half):
            arch.put(x, y, ROCK[4] if x == 84 - half else ROCK[1])
    composite(canvas, arch, OUT)
    cave = Layer(*SIZE)
    for y in range(100, 140):
        half = min(12, 2 + (y - 100))
        for x in range(84 - half, 85 + half):
            cave.put(x, y, (9, 13, 15) if y < 130 else (24, 28, 28))
    for y in (132, 136, 140):
        for x in range(73, 96):
            cave.put(x, y, ROCK[2])
            cave.put(x, y + 1, ROCK[0])
    composite(canvas, cave, None)
    timber = Layer(*SIZE)
    for x in (62, 102):
        for y in range(93, 138):
            for w in range(5):
                timber.put(x + w, y, WOOD[3 if w == 0 else 1 if w > 2 else 2])
        for y in (101, 121, 134):
            for w in range(5):
                timber.put(x + w, y, IRON[1])
            timber.put(x + 1, y, IRON[2])
    for x, y in line_points(62, 94, 106, 92):
        for h in range(5):
            timber.put(x, y + h, WOOD[3 if h == 0 else 1 if h > 2 else 2])
    # Iron spikes, a goblin face sigil, and two ragged red war banners.
    for x in (63, 73, 94, 104):
        for y in range(86, 94):
            timber.put(x, y, IRON[2] if y > 88 else IRON[1])
    stamp(timber, 78, 83, ['..GGGGG..', '.GGGGGGG.', 'G.GGGGG.G', 'GGooGooGG', '.GGGGGGG.', '..GgGgG..', '...GGG...'],
          {'G': (114, 131, 77), 'g': (188, 177, 124), 'o': (24, 30, 23)})
    for x0 in (45, 117):
        for y in range(99, 127):
            for x in range(x0, x0 + 10):
                if y < 122 or (x + y) % 6 < 4:
                    timber.put(x, y, (125, 47, 35) if x < x0 + 5 else (74, 30, 28))
        for x, y in line_points(x0 - 2, 98, x0 + 12, 98):
            timber.put(x, y, WOOD[3])
        stamp(timber, x0 + 2, 104, ['..B..', '.BBB.', 'B.B.B', '.BBB.', '..B..'], {'B': (183, 155, 89)})
    composite(canvas, timber, OUT)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/mountain-entrance.bmp')
