#!/usr/bin/env python3
"""Ironfang's mine entrance and native 24px foreman, using the existing pixelkit."""
from pathlib import Path
import random
import re
from PIL import Image
from pixelkit import Layer, composite, poly_mask, line_points, stamp, save_sprite

ROOT = Path(__file__).resolve().parents[2]
OUT = (20, 23, 26)
STONE = [(28, 33, 37), (43, 49, 53), (64, 70, 73), (88, 94, 95), (122, 127, 123), (158, 162, 147)]
WOOD = [(38, 29, 23), (70, 47, 30), (106, 75, 42), (148, 111, 64), (182, 148, 92)]


def stroke(layer, points, color):
    for a, b in zip(points, points[1:]):
        for x, y in line_points(*a, *b):
            layer.put(x, y, color)


def facet(canvas, points, tone, seed):
    """Broken slate planes with sparse chips, never a repeating tiled surface."""
    layer = Layer(*canvas.size)
    mask = poly_mask(canvas.size, points)
    pixels = sorted(mask)
    rng = random.Random(seed)
    for x, y in pixels:
        layer.put(x, y, STONE[tone])
    # Short irregular shelves follow the rock slope instead of a masonry grid.
    for _ in range(len(mask) // 70):
        x, y = rng.choice(pixels)
        length = rng.randrange(2, 10)
        rise = rng.choice((-3, -2, -1, 1))
        for px, py in line_points(x, y, x + length, y + rise):
            if (px, py) in mask:
                layer.put(px, py, STONE[max(0, tone - 1)])
            if (px, py - 1) in mask:
                layer.put(px, py - 1, STONE[min(5, tone + 1)])
    for _ in range(len(mask) // 24):
        x, y = rng.choice(pixels)
        layer.put(x, y, STONE[min(5, tone + rng.choice((-1, 1)))])
    composite(canvas, layer, OUT)


def timber(layer, a, b, width):
    """Solid beams with a lit edge, grain, iron straps and inset bolt heads."""
    horizontal = abs(b[0] - a[0]) > abs(b[1] - a[1])
    for x, y in line_points(*a, *b):
        for n in range(width):
            layer.put(x if horizontal else x + n, y + n if horizontal else y,
                      WOOD[3 if n == 0 else 0 if n == width - 1 else 2])
    for x, y in line_points(*a, *b)[3:-3]:
        if (x + y * 3) % 13 < 4:
            layer.put(x if horizontal else x + 2, y + 2 if horizontal else y, WOOD[1])


def entrance():
    canvas = Image.new('RGBA', (168, 144))
    # An overlapping ridge, naturally fractured and lit from the upper left.
    for seed, (tone, points) in enumerate([
        (1, [(11, 137), (7, 119), (15, 94), (23, 90), (30, 67), (44, 73), (50, 44),
             (63, 48), (76, 17), (86, 24), (94, 44), (110, 34), (120, 54), (126, 57),
             (136, 88), (148, 96), (159, 126), (153, 139), (123, 140), (105, 136)]),
        (3, [(40, 106), (51, 48), (62, 53), (77, 23), (80, 45), (71, 64), (76, 82), (65, 105)]),
        (2, [(77, 23), (88, 30), (94, 47), (85, 58), (84, 79), (73, 90), (69, 67), (80, 45)]),
        (4, [(49, 50), (61, 54), (54, 72), (57, 85), (43, 111), (28, 119), (33, 89), (42, 77)]),
        (2, [(61, 54), (68, 48), (69, 67), (64, 89), (57, 105), (43, 111), (57, 85), (54, 72)]),
        (3, [(86, 60), (95, 48), (109, 39), (111, 57), (103, 73), (109, 91), (94, 111), (78, 110)]),
        (1, [(109, 39), (117, 57), (124, 61), (134, 90), (125, 99), (109, 91), (103, 73), (111, 57)]),
        (3, [(13, 119), (25, 96), (32, 77), (40, 82), (32, 104), (33, 119), (22, 132), (13, 133)]),
        (2, [(36, 107), (46, 102), (61, 109), (57, 125), (44, 132), (27, 135), (33, 119)]),
        (2, [(123, 104), (137, 96), (145, 101), (151, 116), (143, 128), (121, 131), (116, 119)]),
        (1, [(145, 101), (152, 113), (157, 128), (151, 137), (137, 138), (143, 128), (151, 116)]),
        (3, [(51, 113), (66, 102), (82, 98), (99, 103), (112, 116), (108, 140), (56, 140)])
    ]):
        facet(canvas, points, tone, seed + 17)
    detail = Layer(168, 144)
    for points in [[(64, 69), (59, 77), (61, 83), (54, 91)], [(97, 59), (93, 70), (96, 75)],
                   [(32, 109), (26, 117), (27, 123)], [(130, 110), (134, 117), (129, 126)]]:
        stroke(detail, points, STONE[0])
    # Rust-coloured seams and green ledges are sparse accents, not a texture grid.
    for x, y in [(51, 83), (96, 80), (126, 109)]:
        stroke(detail, [(x, y), (x + 5, y - 3), (x + 8, y - 2), (x + 12, y - 5)], (110, 75, 42))
        stroke(detail, [(x + 1, y - 1), (x + 5, y - 4), (x + 8, y - 3)], (167, 123, 61))
    for x, y in [(20, 128), (40, 99), (67, 62), (114, 90), (144, 131)]:
        stamp(detail, x, y, ['..m..', '.mMmm', 'mmm..'],
              {'m':(43, 58, 37), 'M':(68, 82, 47)})
    composite(canvas, detail, None)
    # Deep, irregular tunnel mouth; the existing road remains visible below it.
    cave = Layer(168, 144)
    for x, y in sorted(poly_mask(canvas.size, [(61, 140), (63, 114), (69, 102), (78, 97),
                                               (91, 99), (102, 110), (105, 140)])):
        cave.put(x, y, (10, 14, 16) if x < 96 else (20, 25, 27))
    # A second set of supports disappears into the shaft, giving it depth.
    timber(cave, (77, 120), (77, 137), 2)
    timber(cave, (90, 120), (90, 137), 2)
    timber(cave, (77, 119), (91, 119), 2)
    for y in range(128, 144):
        for x in range(74, 95):
            cave.put(x, y, (30, 31, 28) if (x * 7 + y * 3) % 17 < 3 else (18, 22, 23))
    for y in (133, 138, 143):
        stroke(cave, [(75, y), (93, y)], WOOD[1])
        stroke(cave, [(75, y + 1), (93, y + 1)], WOOD[0])
    for a, b in [((81, 127), (77, 143)), ((87, 127), (91, 143))]:
        stroke(cave, [a, b], (147, 153, 139))
        stroke(cave, [(a[0] + 1, a[1]), (b[0] + 1, b[1])], STONE[1])
    composite(canvas, cave, None)
    frame = Layer(168, 144)
    timber(frame, (65, 109), (65, 141), 6)
    timber(frame, (97, 109), (97, 141), 6)
    timber(frame, (62, 106), (105, 106), 7)
    timber(frame, (71, 113), (78, 121), 3)
    timber(frame, (96, 113), (90, 121), 3)
    for x in (65, 97):
        for y in (118, 135):
            for n in range(6):
                frame.put(x + n, y, (62, 69, 67))
                frame.put(x + n, y + 1, STONE[0])
            frame.put(x + 1, y, (143, 148, 130))
    # Iron-capped crossbeam and paired lanterns frame the usable door tile.
    for x in (64, 100):
        for y in range(107, 112):
            frame.put(x, y, STONE[1])
        frame.put(x, 109, STONE[4])
    for x in (60, 104):
        stamp(frame, x, 122, ['..i..', '.iii.', 'ioGoi', 'iGPoi', 'ioGoi', '.iii.'],
              {'i':(37, 35, 29), 'o':(134, 88, 37), 'G':(231, 166, 60), 'P':(255, 227, 139)})
    composite(canvas, frame, OUT)
    # A side-mounted ore hoist, with wheel, rope, brace and a hanging bucket.
    hoist = Layer(168, 144)
    timber(hoist, (125, 76), (125, 132), 5)
    timber(hoist, (105, 75), (135, 75), 5)
    timber(hoist, (111, 81), (124, 96), 3)
    stamp(hoist, 106, 70, ['..iii..', '.iMMMi.', 'iMoioMi', 'iMiMiMi', 'iMoioMi', '.iMMMi.', '..iii..'],
          {'i':OUT, 'M':(135, 125, 92), 'o':WOOD[1]})
    stroke(hoist, [(109, 79), (109, 96)], (157, 141, 98))
    stamp(hoist, 104, 97, ['..iiiii..', '.iMMSMMi.', 'iMMMMMMMi', '.iMMSMMi.', '..iMMMi..', '...iii...'],
          {'i':OUT, 'M':(96, 100, 91), 'S':(157, 161, 143)})
    for y in (85, 119, 128):
        stroke(hoist, [(125, y), (129, y)], STONE[1])
        hoist.put(126, y, STONE[4])
    composite(canvas, hoist, OUT)
    # An ore-filled cart and broken scree make this a working goblin mine.
    props = Layer(168, 144)
    stamp(props, 30, 119, [
        '....ss...ss............', '...sSSs.sSSSs..ss......',
        '..sSHSSsSSSHSsSSSs.....', '.sSSSSSSSSSSSSSSSSs....',
        'iiiiiiiiiiiiiiiiiiiiii.', 'iWWWWwWWWWwWWWWwWWWWWi.',
        '.iwwwWwwwwWwwwwWwwwwi..', '.iWWWwWWWWwWWWWwWWWWi..',
        '..iiiiiiiiiiiiiiiiii...', '...iMi........iMi......',
        '..iMoMi......iMoMi.....', '...iii........iii......'],
          {'i':OUT, 's':STONE[1], 'S':STONE[3], 'H':STONE[5],
           'W':WOOD[3], 'w':WOOD[1], 'M':STONE[3], 'o':STONE[0]})
    for x, y in [(15, 136), (24, 140), (52, 138), (114, 137), (135, 140), (147, 134)]:
        stamp(props, x, y, ['..ss..', '.sSSs.', 'sSHSSs', '.ssss.'],
              {'s':STONE[1], 'S':STONE[3], 'H':STONE[4]})
    composite(canvas, props, OUT)
    return canvas


def boss():
    # Reuse the established Hobgoblin silhouette and palette at exactly 24px.
    text = (ROOT / 'src/renderer/mountain_enemy_art.h').read_text()
    block = text.split('{ // HOBGOBLIN_GUARD',1)[1].split('},',1)[0]
    rows = re.findall(r'"([^"]+)"',block)
    canvas = Image.new('RGBA',(24,24))
    body = (91,121,45)
    palette = {'o':OUT,'s':(60,80,30),'S':body,'H':(145,165,115),'e':(18,18,29),'y':(244,67,20),
             'c':(62,50,29),'C':(94,77,44),'v':(134,121,85),'m':(68,72,82),'M':(133,141,150),'N':(208,220,223),
             'w':(86,58,32),'W':(155,116,64),'l':(69,43,32),'L':(115,73,40),'K':(159,112,66),'k':(43,31,27),
             'g':(133,89,35),'G':(212,168,71),'f':(225,219,186)}
    layer = Layer(24,24)
    stamp(layer,0,0,rows,palette)
    for y in range(24):
        for x in range(20,24):
            layer.erase(x,y)
    stamp(layer,8,0,['.mmmmmm.','mMMMMMMm','mMMGGMMm','mmmmmmmm'],palette)
    layer.put(12,2,(255,225,128))
    stroke(layer,[(22,8),(22,22)],WOOD[3])
    stroke(layer,[(18,8),(20,6),(23,7)],(208,220,223))
    stroke(layer,[(18,9),(20,7),(23,8)],(68,72,82))
    composite(canvas,layer,None)
    assert canvas.getbbox()[2] - canvas.getbbox()[0] <= 24
    assert canvas.getbbox()[3] == 24
    return canvas


def build():
    output = ROOT / 'docs/art-sources/ironfang'
    output.mkdir(parents=True,exist_ok=True)
    for name, sprite in [('ironfang-entrance',entrance()),('ironfang-boss',boss())]:
        save_sprite(sprite,ROOT/'assets/images'/f'{name}.bmp')
        sprite.save(output/f'{name}.png')


if __name__ == '__main__':
    build()
