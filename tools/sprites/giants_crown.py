#!/usr/bin/env python3
"""Giant's Peaks ruined hall, built at native 24px scale with pixelkit."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, flat_wall, poly_mask, line_points, save_sprite

ROOT = Path(__file__).resolve().parents[2]
SIZE = (168, 144)
OUT = (23, 29, 26)
STONE = [(37, 44, 39), (57, 64, 53), (79, 87, 69), (107, 112, 90), (138, 143, 116), (175, 178, 149)]


def stroke(layer, points, color):
    for a, b in zip(points, points[1:]):
        for x, y in line_points(*a, *b):
            layer.put(x, y, color)


def entrance():
    canvas = Image.new('RGBA', SIZE)
    # Eroded shoulders support a monumental broken lintel and fluted pillars.
    rock = Layer(*SIZE)
    for x, y in poly_mask(SIZE, [(8,140),(13,110),(28,97),(35,64),(47,79),(52,124),(64,140)]):
        rock.put(x, y, STONE[1 + (x % 11 < 5) + (x < 30)])
    for x, y in poly_mask(SIZE, [(114,140),(122,99),(128,72),(140,84),(149,111),(160,140)]):
        rock.put(x, y, STONE[1 + (x % 13 < 5)])
    composite(canvas, rock, OUT)
    wall = Layer(*SIZE)
    flat_wall(wall, 42, 125, 51, 137, STONE, row_h=11, seed=42, bw=(17, 28))
    # The hall's large entrance remains transparent to the ground at its foot.
    for y in range(89, 144):
        width = 19 if y >= 101 else max(4, y - 81)
        for x in range(84 - width, 84 + width + 1):
            wall.put(x, y, (16, 25, 22))
    for x in (44, 113):
        flat_wall(wall, x, x + 12, 63, 136, STONE, row_h=17, seed=x, bw=(12, 12))
        for y in range(70, 130):
            wall.put(x + 3, y, STONE[4])
            wall.put(x + 8, y, STONE[1])
        flat_wall(wall, x - 4, x + 16, 58, 66, STONE, row_h=9, seed=x + 1)
        flat_wall(wall, x - 4, x + 16, 133, 142, STONE, row_h=10, seed=x + 2)
    composite(canvas, wall, OUT)
    pediment = Layer(*SIZE)
    mask = poly_mask(SIZE, [(37,57),(44,46),(63,29),(72,37),(81,19),(92,35),(105,31),(132,57)])
    flat_wall(pediment, 37, 132, 19, 58, STONE, row_h=8, seed=63, bw=(16, 24))
    for y in range(SIZE[1]):
        for x in range(SIZE[0]):
            if (x, y) not in mask:
                pediment.erase(x, y)
    composite(canvas, pediment, OUT)
    detail = Layer(*SIZE)
    # A weathered crown cut into the lintel, fractured masonry and climbing moss.
    stroke(detail, [(70,43),(70,34),(77,40),(84,30),(91,40),(98,34),(98,43),(70,43)], STONE[5])
    stroke(detail, [(102,54),(95,64),(99,76),(92,84)], STONE[0])
    for x, y in [(43,65),(51,107),(111,74),(122,130),(75,53),(128,57)]:
        for dx, dy in [(0,0),(1,0),(2,0),(3,1),(1,1),(1,2),(1,3),(2,4)]:
            detail.put(x + dx, y + dy, (62, 91, 47))
    composite(canvas, detail, None)
    return canvas


if __name__ == '__main__':
    art = entrance()
    save_sprite(art, ROOT / 'assets/images/giants-crown-entrance.bmp')
    output = ROOT / 'docs/art-sources/giants-crown'
    output.mkdir(parents=True, exist_ok=True)
    art.save(output / 'entrance.png')
    # Review at native size beside an established entrance and town building.
    sheet = Image.new('RGB', (600, 190), (17, 35, 23))
    sheet.paste(art, (8, 34), art)
    for path, at in [('assets/images/ashen-entrance.bmp',(200,34)), ('assets/images/blacksmith.bmp',(380,34))]:
        reference = Image.open(ROOT / path).convert('RGBA')
        sheet.paste(reference, at, reference)
    sheet.save(output / 'native-scale-review.png')
