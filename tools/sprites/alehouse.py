#!/usr/bin/env python3
"""Dawnmere's timbered Alehouse and native beer cask, using pixelkit."""
from pathlib import Path

from PIL import Image

from pixelkit import (BARREL, LANTERN, Layer, arched_door, beam, composite,
                      flat_wall, light_spill, lit_window, plaster, poly_mask,
                      roof_tiles, save_sprite, shadow_under, stamp, stone_arch)

ROOT = Path(__file__).resolve().parents[2]
SIZE = (168, 144)
OUT = (22, 18, 13)
WOOD = [(26, 20, 13), (45, 35, 22), (67, 53, 32), (92, 74, 44), (126, 102, 61)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
GREEN = [(62, 72, 51), (85, 99, 69), (111, 123, 88), (143, 149, 105)]
SLATE = [(25, 29, 30), (39, 45, 45), (54, 63, 61), (72, 83, 75), (92, 103, 89), (117, 125, 103)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
BARREL_PAL = {'t': WOOD[1], 'T': WOOD[2], 'W': WOOD[4], 'i': IRON[1], 'I': IRON[3]}


def gable(canvas, cx, apex, bottom, half, seed):
    roof = Layer(*SIZE)
    outer = poly_mask(SIZE, [(cx - half, bottom), (cx, apex), (cx + half, bottom)])
    face = poly_mask(SIZE, [(cx - half + 6, bottom), (cx, apex + 9), (cx + half - 6, bottom)])
    roof_tiles(roof, outer - face, SLATE, seed, shade=lambda x, y: -1 if x > cx else 0)
    composite(canvas, roof, OUT)
    wall = Layer(*SIZE)
    plaster(wall, face, GREEN, seed + 1)
    beam(wall, cx, apex + 9, cx, bottom, WOOD, width=2)
    beam(wall, cx - half + 6, bottom, cx, apex + 9, WOOD, width=2)
    beam(wall, cx, apex + 9, cx + half - 6, bottom, WOOD, width=2)
    for y in range(apex + 30, bottom + 1, 18):
        span = (y - apex - 9) * (half - 6) // (bottom - apex - 9)
        beam(wall, cx - span, y, cx + span, y, WOOD, width=2)
        if span > 15:
            beam(wall, cx - span + 4, y, cx - 5, y - 13, WOOD, width=2)
            beam(wall, cx + 5, y - 13, cx + span - 4, y, WOOD, width=2)
    shadow_under(wall, face, GREEN, WOOD, GREEN[0])
    composite(canvas, wall, OUT)


def building():
    canvas = Image.new('RGBA', SIZE)
    chimney = Layer(*SIZE)
    flat_wall(chimney, 36, 47, 10, 53, STONE, row_h=4, base=2, seed=225, bw=(4, 6))
    beam(chimney, 35, 8, 48, 8, STONE, width=3)
    beam(chimney, 38, 8, 45, 8, WOOD, width=1)
    composite(canvas, chimney, OUT)
    roof = Layer(*SIZE)
    roof_tiles(roof, poly_mask(SIZE, [(9, 95), (24, 55), (119, 42), (159, 95)]), SLATE, 226)
    composite(canvas, roof, OUT)
    lower = Layer(*SIZE)
    flat_wall(lower, 19, 151, 104, 139, STONE, row_h=5, base=3, seed=227, bw=(6, 11),
              shade=lambda x, y: -1 if x > 126 or y < 108 else 0)
    for x in (19, 56, 108, 149):
        beam(lower, x, 104, x, 139, WOOD, width=3)
    beam(lower, 20, 138, 150, 138, WOOD, width=2)
    composite(canvas, lower, OUT)
    upper = Layer(*SIZE)
    mask = poly_mask(SIZE, [(16, 75), (153, 75), (153, 102), (16, 102)])
    plaster(upper, mask, GREEN, 228)
    for x in (17, 43, 67, 103, 126, 151):
        beam(upper, x, 76, x, 101, WOOD, width=2)
    for x in (19, 105):
        beam(upper, x, 99, x + 19, 78, WOOD, width=2)
    shadow_under(upper, mask, GREEN, WOOD, GREEN[0])
    composite(canvas, upper, OUT)
    gable(canvas, 128, 34, 94, 34, 229)
    gable(canvas, 67, 14, 94, 57, 231)
    details = Layer(*SIZE)
    # Overhanging timber floors and closely spaced carved supports.
    beam(details, 14, 97, 156, 97, WOOD, width=4)
    beam(details, 16, 103, 154, 103, WOOD, width=2)
    for x in range(21, 151, 9):
        beam(details, x, 101, x + 2, 106, WOOD, width=2)
    lit_window(details, 61, 66, 11, 13, GLOW, WOOD)
    lit_window(details, 123, 72, 9, 11, GLOW, WOOD)
    lit_window(details, 33, 113, 13, 12, GLOW, WOOD)
    lit_window(details, 119, 113, 12, 12, GLOW, WOOD)
    # A standard single doorway, aligned with the existing town door tile.
    stone_arch(details, 84, 119, 138, 7, STONE, thick=2)
    arched_door(details, 84, 119, 138, 7, WOOD, IRON)
    beam(details, 75, 139, 93, 139, STONE, width=2)
    beam(details, 73, 142, 95, 142, STONE, width=2)
    for x in (64, 102):
        stamp(details, x, 119, LANTERN,
              {'i': IRON[1], 'I': IRON[2], 'y': GLOW[2], 'Y': GLOW[3], 'W': GLOW[4]})
    # A barrel sign and casks identify the ale trade without changing the ground.
    beam(details, 144, 99, 165, 99, IRON, width=2)
    beam(details, 162, 100, 162, 108, IRON, width=1)
    stamp(details, 153, 109, BARREL, BARREL_PAL)
    stamp(details, 23, 127, BARREL, BARREL_PAL)
    stamp(details, 38, 128, BARREL, BARREL_PAL)
    composite(canvas, details, OUT)
    light_spill(canvas, [(66, 123, 12), (104, 123, 12)], STONE + WOOD, GLOW[3])
    return canvas


def cask():
    canvas = Image.new('RGBA', (48, 24))
    layer = Layer(48, 24)
    # Rounded staves, iron hoops, an elliptical lid and a brass serving tap.
    for y in range(4, 22):
        half = 7 if y in (4, 21) else 9 if 8 <= y <= 17 else 8
        for x in range(11 - half, 12 + half):
            tone = 4 if x < 8 else 3 if x < 14 else 2
            if x % 4 == 1:
                tone = max(0, tone - 2)
            if y in (8, 9, 17, 18):
                layer.put(x, y, IRON[3 if y in (8, 17) and x < 14 else 1])
            else:
                layer.put(x, y, WOOD[tone])
    for y, half in [(2, 5), (3, 7), (4, 8), (5, 7), (6, 5)]:
        for x in range(11 - half, 12 + half):
            layer.put(x, y, WOOD[3 if y < 5 else 1])
    for x in (6, 10, 14):
        beam(layer, x, 3, x + 1, 5, WOOD, width=1)
    stamp(layer, 13, 12, ['..GG.', '.GgG.', '..G..', '..GGG', '....G'],
          {'G': (199, 153, 66), 'g': (100, 74, 32)})
    stamp(layer, 31, 10, ['.fffffff..', 'fFFFFfFFf.', '.pAAAAAp..', '.pABBBApPP',
                         '.pABBBAp.P', '.pABBBAp.P', '.pAAAAApPP', '.ppppppp..', '..PPPPP...'],
          {'f': (216, 204, 162), 'F': (249, 239, 202), 'p': (65, 66, 62),
           'P': (144, 146, 130), 'A': (155, 87, 27), 'B': (209, 139, 45)})
    composite(canvas, layer, OUT)
    return canvas


if __name__ == '__main__':
    output = ROOT / 'docs/art-sources/alehouse'
    output.mkdir(parents=True, exist_ok=True)
    for name, sprite in [('alehouse', building()), ('alehouse-cask', cask())]:
        save_sprite(sprite, ROOT / 'assets/images' / f'{name}.bmp')
        sprite.save(output / f'{name}.png')
