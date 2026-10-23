#!/usr/bin/env python3
"""Pirate Coast's harbor fort and six enemies, using the established pixelkit pipeline."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, poly_mask, line_points, stamp, save_sprite, flat_wall, roof_tiles, beam, stone_arch, CRATE, BARREL
import harbor

HARBOR_SAIL = harbor.SAIL

ROOT = Path(__file__).resolve().parents[2]
OUT = (20, 21, 23)
WOOD = [(34, 26, 22), (57, 41, 29), (85, 60, 37), (119, 83, 47), (153, 111, 64)]
STONE = [(30, 35, 39), (47, 54, 58), (72, 83, 85), (99, 111, 111), (132, 142, 137), (166, 169, 152)]
RED = [(44, 22, 26), (75, 30, 33), (117, 42, 41), (161, 57, 47), (194, 84, 59)]
SKIN = [(65, 39, 30), (108, 65, 42), (153, 100, 62), (196, 139, 88), (229, 178, 118)]
NAVY = [(20, 28, 39), (28, 43, 61), (43, 64, 82), (65, 87, 103), (96, 116, 126)]
GOLD = (232, 186, 78)
BONE = (222, 211, 166)


def shape(canvas, points, pal, seed=0):
    layer = Layer(*canvas.size)
    for x, y in sorted(poly_mask(canvas.size, points)):
        shade = 2 + (x % 7 < 3) - (y % 9 > 6)
        if (x * 13 + y * 7 + seed) % 31 == 0:
            shade += 1
        layer.put(x, y, pal[max(0, min(len(pal) - 1, shade))])
    composite(canvas, layer, OUT)


def skull(layer, x, y):
    stamp(layer, x, y, ["..bbb..", ".bbbbb.", "bb.b.bb", "bbbbbbb", ".b.b.b.", "..bbb..", "b..b..b", ".bb.bb.", "b.....b"], {'b': BONE})


def gateway():
    canvas = Image.new('RGBA', (168, 144))
    # A black-sailed ship rises behind the coastal fort, tied into the existing harbor style.
    harbor.SAIL = [(13, 17, 23), (22, 28, 35), (35, 42, 48), (49, 57, 63), (72, 78, 81)]
    boat = Layer(120, 96)
    harbor.boat(boat)
    canvas.alpha_composite(boat.im, (15, 1))
    for left in (11, 116):
        tower = Layer(168, 144)
        flat_wall(tower, left, left + 36, 66, 123, STONE, row_h=6, seed=left, bw=(7, 11))
        composite(canvas, tower, OUT)
        roof = Layer(168, 144)
        roof_tiles(roof, poly_mask(canvas.size, [(left - 5, 68), (left + 17, 38), (left + 40, 68)]), RED, left)
        composite(canvas, roof, OUT)
        details = Layer(168, 144)
        for y in range(82, 99):
            details.put(left + 16, y, OUT)
            details.put(left + 17, y, OUT)
            details.put(left + 18, y, STONE[0])
        # Narrow arrow slits defend each bastion.
        for yy in range(109, 122):
            details.put(left + 16, yy, OUT)
            details.put(left + 17, yy, OUT)
            details.put(left + 18, yy, STONE[0])
        composite(canvas, details, OUT)
    timber = Layer(168, 144)
    for x in list(range(3, 61, 6)) + list(range(109, 165, 6)):
        beam(timber, x, 96, x, 133, WOOD, width=4)
        for yy in range(93, 97):
            timber.put(x, yy, WOOD[3])
        for yy in (109, 125):
            timber.put(x - 1, yy, (34, 39, 42))
            timber.put(x + 1, yy, (34, 39, 42))
    beam(timber, 3, 108, 60, 108, WOOD, width=3)
    beam(timber, 109, 108, 164, 108, WOOD, width=3)
    composite(canvas, timber, OUT)
    arch = Layer(168, 144)
    flat_wall(arch, 61, 107, 90, 135, STONE, row_h=6, seed=12, bw=(7, 11))
    composite(canvas, arch, OUT)
    entry = Layer(168, 144)
    for y in range(105, 140):
        half = min(12, 5 + (y - 105) // 2)
        for x in range(84 - half, 85 + half):
            entry.put(x, y, (12 + (y - 105) // 4, 20 + (y - 105) // 3, 23))
    stone_arch(entry, 84, 103, 139, 12, STONE, thick=3)
    for y in range(136, 144):
        for x in range(69, 100):
            entry.put(x, y, WOOD[2 if y % 3 else 0])
    composite(canvas, entry, None)
    flag = Layer(168, 144)
    beam(flag, 65, 42, 65, 90, WOOD, width=2)
    for x, y in sorted(poly_mask(canvas.size, [(67, 43), (98, 46), (95, 56), (101, 63), (67, 60)])):
        flag.put(x, y, NAVY[0 if y % 5 else 1])
    skull(flag, 77, 48)
    stamp(flag, 47, 125, BARREL, {'o': OUT, 'd': WOOD[0], 'm': WOOD[2], 'l': WOOD[4], 'i': STONE[2]})
    composite(canvas, flag, OUT)
    return canvas


def creature(index):
    canvas = Image.new('RGBA', (64, 64))
    brute = index == 3
    captain = index == 5
    coat = WOOD if index == 2 else RED if index in (0, 5) else NAVY
    left = 21 if brute else 25
    right = 44 if brute else 40
    shape(canvas, [(left, 34), (left - 3, 40), (left, 45), (left + 4, 44), (left + 3, 52),
                   (right - 1, 52), (right - 3, 44), (right + 2, 43), (right + 4, 37), (right - 1, 34)], coat)
    shape(canvas, [(27, 25), (36, 24), (40, 29), (38, 35), (29, 37), (25, 31)], SKIN)
    detail = Layer(64, 64)
    for x in (28, 36):
        for y in range(47, 55):
            detail.put(x, y, WOOD[1]); detail.put(x + 1, y, WOOD[0]); detail.put(x + 2, y, WOOD[2])
        for xx in range(x - 2, x + 4):
            detail.put(xx, 55, OUT); detail.put(xx, 54, WOOD[1])
    for x in range(left + 1, right):
        detail.put(x, 43, WOOD[0])
    detail.put(33, 43, GOLD); detail.put(34, 43, GOLD)
    detail.put(29, 29, OUT); detail.put(36, 29, OUT)
    detail.put(38, 32, GOLD)
    for x, y in line_points(29, 35, 36, 39):
        detail.put(x, y, (203, 184, 141))
    if captain or index == 1:
        shape(canvas, [(21, 25), (26, 21), (29, 23), (33, 17 if captain else 20), (38, 23), (43, 21), (42, 27), (24, 28)], NAVY)
        for x in range(24, 42):
            detail.put(x, 26, GOLD if captain else WOOD[4])
        if captain:
            skull(detail, 30, 18)
    else:
        for y in range(23, 27):
            for x in range(25, 39):
                detail.put(x, y, RED[3 if y == 24 else 1])
        for x, y in line_points(26, 26, 21, 31):
            detail.put(x, y, RED[3])
    if index in (0, 3, 5):
        for x, y in line_points(43, 45, 51, 31 if brute else 24):
            detail.put(x, y, STONE[4]); detail.put(x + 1, y, STONE[2])
        for x, y in line_points(39, 42, 48, 45):
            detail.put(x, y, GOLD)
        if brute:
            for x, y in poly_mask(canvas.size, [(47, 32), (46, 23), (54, 25), (56, 32)]):
                detail.put(x, y, STONE[3])
    elif index == 1:
        for a, b in [((39, 38), (52, 44)), ((45, 35), (45, 48)), ((45, 35), (53, 43)), ((53, 43), (45, 48))]:
            for x, y in line_points(*a, *b):
                detail.put(x, y, WOOD[4])
    elif index in (2, 4):
        # A curved cutlass has the same grip and blade scale as the Raider's.
        shape(canvas, [(43, 42), (46, 31), (53, 25), (52, 31), (46, 43)], STONE)
        for x, y in line_points(45, 40, 51, 28):
            detail.put(x, y, STONE[5])
        for x, y in line_points(40, 40, 48, 44):
            detail.put(x, y, GOLD)
        for x, y in line_points(43, 43, 41, 47):
            detail.put(x, y, WOOD[1])
        if index == 4:
            # The Swashbuckler carries an offhand sword as well.
            shape(canvas, [(23, 43), (15, 29), (14, 23), (19, 27), (26, 41)], STONE)
            for x, y in line_points(23, 40, 16, 27):
                detail.put(x, y, STONE[5])
            for x, y in line_points(19, 44, 27, 39):
                detail.put(x, y, GOLD)
    composite(canvas, detail, OUT)
    return canvas


if __name__ == '__main__':
    dest = ROOT / 'assets/images'
    entrance = gateway()
    save_sprite(entrance, dest / 'pirate-entrance.bmp')
    atlas = Image.new('RGBA', (384, 64))
    for index in range(6):
        atlas.alpha_composite(creature(index), (index * 64, 0))
    save_sprite(atlas, dest / 'pirate-enemies.bmp')
    preview = Image.new('RGBA', (336, 160), (25, 44, 35, 255))
    preview.alpha_composite(entrance, (0, 8))
    harbor.SAIL = HARBOR_SAIL
    preview.alpha_composite(harbor.build(), (190, 48))
    preview.convert('RGB').save('/tmp/pirate-building-comparison.png')
    atlas.resize((768, 128), Image.Resampling.NEAREST).save('/tmp/pirate-enemies.png')
