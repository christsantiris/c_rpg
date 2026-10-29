#!/usr/bin/env python3
"""Living Elderwood gateway and creatures, at the game's native pixel scale."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, poly_mask, line_points, stamp, save_sprite
import forest

ROOT = Path(__file__).resolve().parents[2]
OUT = (13, 24, 19)
BARK = [(35, 37, 25), (58, 55, 31), (83, 72, 40), (112, 94, 50), (145, 121, 65)]
LEAF = [(19, 44, 27), (29, 64, 32), (43, 83, 37), (65, 105, 44), (93, 128, 55), (137, 155, 72)]


def shape(canvas, points, palette, seed=1):
    part = Layer(*canvas.size)
    for x, y in sorted(poly_mask(canvas.size, points)):
        shade = 2 + ((x + y // 7) % 9 < 3) - ((x + y // 11) % 13 > 8)
        if (x * 17 + y * 11 + seed) % 19 == 0:
            shade += 1
        part.put(x, y, palette[max(0, min(len(palette) - 1, shade))])
    composite(canvas, part, OUT)


def gateway():
    forest.BARK = BARK
    forest.LEAF = LEAF
    canvas = Image.new('RGBA', (168, 144))
    # A mossy ruined watchtower recedes behind monumental living trunks.
    stone = [(44, 52, 39), (62, 72, 50), (83, 93, 66), (112, 119, 84), (137, 142, 101)]
    shape(canvas, [(18, 124), (20, 46), (28, 46), (28, 35), (34, 35), (34, 48),
                   (42, 48), (42, 39), (49, 39), (49, 58), (55, 58), (52, 127)], stone)
    masonry = Layer(168, 144)
    for y in range(58, 124, 9):
        for x in range(21, 52):
            masonry.put(x, y, stone[0])
        for yy in range(y + 1, y + 9):
            masonry.put(30 if y % 2 else 42, yy, stone[0])
    for y in range(66, 83):
        for x in range(32, 38):
            masonry.put(x, y, OUT)
    composite(canvas, masonry, None)
    path = Layer(168, 144)
    for y in range(68, 144):
        half = 3 + (y - 68) // 4
        for x in range(84 - half, 85 + half):
            path.put(x, y, (25 + (y - 68) // 5, 43 + (y - 68) // 5, 26))
            if (x * 11 + y * 7) % 29 == 0:
                path.put(x, y, (123, 117, 62))
    composite(canvas, path, None)
    forest.woody(canvas, [(32, 139), (41, 118), (40, 72), (30, 51), (40, 39), (54, 62),
                          (58, 82), (76, 74), (98, 68), (99, 78), (65, 96), (57, 101),
                          (57, 120), (69, 139), (50, 134), (43, 143)], 402)
    forest.woody(canvas, [(108, 139), (121, 116), (119, 93), (107, 80), (83, 84), (72, 75),
                          (99, 70), (118, 78), (125, 57), (137, 42), (144, 52), (135, 83),
                          (141, 117), (156, 139), (135, 133), (125, 143)], 403)
    for args in [(17, 68, 15, 27, 21), (150, 67, 15, 30, 22), (39, 35, 33, 30, 23),
                 (129, 34, 33, 30, 24), (83, 25, 37, 24, 25), (60, 59, 29, 22, 26),
                 (113, 57, 29, 22, 27), (20, 99, 17, 18, 28), (149, 101, 15, 19, 29)]:
        forest.crown(canvas, *args)
    details = Layer(168, 144)
    for x, y in [(53, 88), (64, 84), (105, 83), (120, 98), (41, 124), (141, 125)]:
        for n in range(13):
            details.put(x + n // 6, y + n, LEAF[3 + n % 2])
    for x, y in [(17, 135), (65, 137), (106, 138), (151, 136)]:
        stamp(details, x, y, ['..g..', '.ggg.', 'ggAgg', '..b..'],
              {'g': LEAF[3], 'A': (211, 169, 65), 'b': BARK[2]})
    composite(canvas, details, OUT)
    return canvas


def creature(index):
    canvas = Image.new('RGBA', (64, 64))
    if index == 2:
        # Dryad: a leaf-crowned humanoid with a thorn bow and root skirt.
        shape(canvas, [(29, 34), (35, 34), (37, 40), (35, 44), (39, 54), (31, 51),
                       (26, 55), (28, 43), (25, 41)], BARK)
        shape(canvas, [(25, 32), (28, 28), (33, 30), (37, 27), (39, 33), (35, 36), (28, 35)], LEAF)
        details = Layer(64, 64)
        for a, b in [((36, 40), (42, 36)), ((42, 36), (45, 43)), ((45, 43), (42, 50)), ((42, 36), (42, 50)), ((30, 43), (24, 48))]:
            for x, y in line_points(*a, *b):
                details.put(x, y, BARK[4])
        details.put(31, 37, (216, 183, 85)); details.put(34, 37, (216, 183, 85))
        composite(canvas, details, None)
    elif index == 3:
        # Treefolk: broad trunk, roots, branch arms and a heavy leaf canopy.
        shape(canvas, [(19, 51), (23, 43), (23, 30), (17, 35), (14, 44), (10, 43),
                       (13, 30), (21, 25), (27, 23), (39, 25), (46, 30), (51, 43),
                       (46, 44), (41, 34), (40, 45), (47, 54), (37, 52), (33, 48), (29, 54)], BARK)
        shape(canvas, [(17, 25), (18, 18), (25, 17), (28, 12), (36, 15), (42, 14),
                       (47, 22), (43, 29), (35, 27), (28, 30)], LEAF)
        details = Layer(64, 64)
        stamp(details, 27, 31, ['AA..AA', '.a..a.', '..bb..', '.bbbb.'],
              {'A': (239, 192, 77), 'a': (146, 115, 40), 'b': BARK[0]})
        for x in [26, 33, 38]:
            for y in range(39, 48):
                details.put(x + y % 2, y, BARK[0])
        composite(canvas, details, None)
    else:
        # Elderthorn: large bark-armored stag; luminous heart and branch antlers.
        shape(canvas, [(13, 47), (16, 32), (22, 27), (34, 27), (42, 23), (42, 15),
                       (48, 15), (52, 22), (51, 30), (45, 34), (42, 46), (45, 57),
                       (40, 58), (35, 44), (28, 43), (24, 56), (19, 57), (20, 44),
                       (15, 41), (9, 34), (8, 28)], BARK)
        branches = Layer(64, 64)
        for a, b in [((44, 19), (32, 9)), ((32, 9), (31, 2)), ((37, 13), (26, 12)),
                     ((49, 18), (56, 9)), ((56, 9), (55, 2)), ((54, 12), (61, 9)),
                     ((34, 10), (39, 4)), ((51, 14), (48, 6))]:
            for x, y in line_points(*a, *b):
                branches.put(x, y, BARK[4]); branches.put(x, y + 1, BARK[2])
        stamp(branches, 26, 32, ['..aa..', '.aAAa.', 'aAHHAa', '.aAAa.', '..aa..'],
              {'a': (148, 100, 33), 'A': (227, 164, 46), 'H': (255, 229, 128)})
        branches.put(48, 22, (251, 221, 111))
        composite(canvas, branches, OUT)
        shape(canvas, [(15, 28), (18, 23), (25, 25), (30, 23), (37, 26), (34, 31), (23, 30)], LEAF)
    return canvas


def main():
    save_sprite(gateway(), ROOT / 'assets/images/elderwood-entrance.bmp')
    atlas = Image.new('RGBA', (256, 64))
    for i in range(2, 5):
        atlas.alpha_composite(creature(i), ((i - 1) * 64, 0))
    save_sprite(atlas, ROOT / 'assets/images/elderwood-enemies.bmp')


if __name__ == '__main__':
    main()
