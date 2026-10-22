#!/usr/bin/env python3
"""Royal Catacombs mausoleum at the established 24px pixelkit scale."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, flat_wall, line_points, poly_mask, save_sprite, stamp

SIZE = (168, 144)
OUT = (16, 18, 25)
STONE = [(31, 34, 45), (47, 51, 64), (67, 72, 85), (91, 97, 109), (118, 125, 134), (151, 159, 163)]
GOLD = [(81, 62, 37), (137, 107, 54), (195, 158, 79), (230, 204, 131)]
MOSS = [(26, 40, 32), (44, 64, 43), (66, 87, 53)]


def build():
    canvas = Image.new('RGBA', SIZE)
    stones = Layer(*SIZE)
    # Broken burial pillars flank a royal crypt, with a lower wing on each side.
    for x0, x1, top in ((17, 54, 76), (114, 151, 76), (44, 124, 47)):
        flat_wall(stones, x0, x1, top, 135, STONE, row_h=7, base=3, seed=x0, bw=(9, 16))
    pediment = poly_mask(SIZE, [(38, 49), (84, 17), (130, 49)])
    for x, y in sorted(pediment):
        stones.put(x, y, STONE[4 if x < 84 else 2] if y % 7 else STONE[1])
    for x0, y0, x1, y1 in ((38, 49, 84, 17), (84, 17, 130, 49)):
        for x, y in line_points(x0, y0, x1, y1):
            stones.put(x, y, STONE[5 if x < 84 else 4])
            stones.put(x, y + 1, STONE[1])
    for y in (49, 50, 54, 76, 80, 132):
        for x in range(43 if y < 70 else 16, 125 if y < 70 else 152):
            stones.put(x, y, STONE[5 if y in (49, 76, 132) else 1])
    for x0 in (19, 40, 120, 142):
        for y in range(77, 132):
            for dx in range(7):
                stones.put(x0 + dx, y, STONE[(4, 5, 4, 3, 3, 2, 1)[dx]])
    # Deep doorway and carved voussoirs, sized to the existing forest entrance.
    for y in range(69, 138):
        half = min(23, 2 + y - 69)
        for x in range(84 - half, 85 + half):
            stones.put(x, y, STONE[4 if x < 84 else 2])
        half = min(14, max(0, y - 76))
        if y >= 77:
            for x in range(84 - half, 85 + half):
                stones.put(x, y, (9, 13, 20) if y < 123 else STONE[1])
        if y > 94 and y % 9 == 0:
            for x in list(range(61, 69)) + list(range(100, 108)):
                stones.put(x, y, STONE[0])
    composite(canvas, stones, OUT)
    detail = Layer(*SIZE)
    # Tarnished crown over the royal burial entrance.
    stamp(detail, 74, 40, ['g...g...g...g...g', 'gg.ggg.ggg.ggg.gg', '.ggggggggggggggg.', '..GGGGGGGGGGGGG..', '..ggggggggggggg..'], {'g': GOLD[1], 'G': GOLD[3]})
    for x in (55, 112):
        for y in range(62, 115):
            detail.put(x, y, GOLD[1] if y % 4 else GOLD[0])
        stamp(detail, x - 4, 68, ['rrrrrrrrr', '.rrrrrrr.', '.rrrgrrr.', '.rrrrrrr.', '..rrrrr..', '...rrr...', '....r....'], {'r': (86, 35, 47), 'g': GOLD[2]})
    for x0, y0, length in ((21, 89, 19), (47, 55, 15), (117, 90, 27), (143, 80, 21)):
        for y in range(y0, y0 + length):
            x = x0 + (y // 5) % 2
            detail.put(x, y, MOSS[1])
            if y % 3 == 0:
                detail.put(x - 1, y, MOSS[2])
    for y in range(131, 144):
        inset = max(0, (139 - y) // 3)
        for x in range(65 + inset, 104 - inset):
            detail.put(x, y, STONE[4] if y % 4 == 3 else STONE[2])
    composite(canvas, detail, OUT)
    return canvas


if __name__ == '__main__':
    save_sprite(build(), Path(__file__).resolve().parents[2] / 'assets/images/catacombs-entrance.bmp')
