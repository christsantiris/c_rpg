#!/usr/bin/env python3
"""Ashen Hollow's volcanic rift, using the established native pixelkit pipeline."""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, poly_mask, line_points, save_sprite

ROOT = Path(__file__).resolve().parents[2]
SIZE = (168, 144)
OUT = (14, 15, 20)
ROCK = [(24, 25, 33), (38, 39, 48), (57, 57, 66), (78, 77, 84), (104, 101, 105), (134, 127, 126)]
LAVA = [(89, 20, 20), (142, 31, 21), (207, 58, 22), (246, 111, 29), (255, 181, 58), (255, 232, 141)]
SNOW = [(99, 116, 128), (151, 168, 173), (207, 218, 213)]


def line(layer, points, color):
    for a, b in zip(points, points[1:]):
        for x, y in line_points(*a, *b):
            layer.put(x, y, color)


def facet(canvas, points, shade, seed):
    layer = Layer(*SIZE)
    rng = random.Random(seed)
    for x, y in sorted(poly_mask(SIZE, points)):
        tone = max(0, min(5, shade + rng.choice((-1, 0, 0, 0, 0, 1))))
        layer.put(x, y, ROCK[tone])
    composite(canvas, layer, OUT)


def spire(canvas, cx, top, base, width, seed, snowy=False):
    # Separate broken faces make a serrated rim, without a volcanic cone.
    half = width // 2
    points = [(cx-half,base),(cx-half+2,top+22),(cx-3,top+11),
              (cx,top),(cx+3,top+10),(cx+half-2,top+17),(cx+half,base)]
    facet(canvas, points, 2, seed)
    facet(canvas, [(cx-half,base),(cx-half+2,top+22),(cx,top),(cx+1,base)], 3, seed+1)
    facet(canvas, [(cx+1,base),(cx,top),(cx+3,top+10),(cx+half-2,top+17),(cx+half,base)], 1, seed+2)
    detail = Layer(*SIZE)
    line(detail, [(cx-1,top+9),(cx-3,top+24),(cx-2,base-3)], ROCK[4])
    for y in range(top+23, base-3, 13):
        line(detail, [(cx-half+3,y),(cx-1,y+2),(cx+half-3,y-1)], ROCK[0])
    if snowy:
        for x, y in sorted(poly_mask(SIZE, [(cx-4,top+13),(cx,top),(cx+4,top+12),(cx+1,top+10),(cx-1,top+15)])):
            detail.put(x, y, SNOW[2 if x <= cx else 1])
    composite(canvas, detail, None)


def fissure(layer, points, width=3):
    hot = set()
    for a, b in zip(points, points[1:]):
        for x, y in line_points(*a, *b):
            for dy in range(-width, width+1):
                for dx in range(-width, width+1):
                    distance = abs(dx) + abs(dy)
                    if distance <= width:
                        layer.put(x+dx, y+dy, LAVA[4 if distance == 0 else 3 if distance == 1 else 2 if distance == 2 else 1])
                        hot.add((x+dx, y+dy))
    return hot


def build():
    canvas = Image.new('RGBA', SIZE)
    # The back rim is a broken range surrounding a hollow, not one central peak.
    for args in [(22,60,132,25,10,False),(43,37,119,29,20,True),
                 (66,21,108,25,30,True),(91,33,107,23,40,False),
                 (114,25,116,29,50,True),(139,48,132,29,60,False),
                 (156,77,139,19,70,False)]:
        spire(canvas, *args)
    # A tilted, jagged basin opens downward into a black underworld chasm.
    rim = [(18,101),(26,85),(43,79),(52,68),(71,72),(85,61),(101,73),
           (119,70),(130,83),(145,90),(153,111),(145,131),(126,139),
           (106,135),(94,142),(71,141),(57,134),(39,138),(22,124)]
    facet(canvas, rim, 2, 80)
    hole = [(32,101),(43,92),(58,91),(67,80),(83,86),(95,77),(106,89),
            (123,88),(137,101),(139,116),(123,127),(108,124),(96,135),
            (73,136),(59,125),(44,128),(31,116)]
    pit = Layer(*SIZE)
    for x, y in sorted(poly_mask(SIZE, hole)):
        pit.put(x, y, (10, 9, 17))
    composite(canvas, pit, None)
    lava = Layer(*SIZE)
    # The molten cleft cuts inward and branches below the fractured rim.
    fissure(lava, [(36,106),(51,109),(61,100),(76,107),(88,100),(104,111),(121,104),(137,109)], 3)
    fissure(lava, [(76,107),(72,116),(82,121),(84,133)], 2)
    fissure(lava, [(105,110),(112,119),(107,125)], 2)
    fissure(lava, [(41,94),(48,86),(46,75)], 1)
    fissure(lava, [(126,94),(133,84),(131,72)], 1)
    composite(canvas, lava, None)
    # Broken inner walls remain visible above the glow, giving the rift depth.
    front = Layer(*SIZE)
    for x, y in sorted(poly_mask(SIZE, [(23,115),(38,122),(46,119),(58,128),(70,134),(68,142),(40,139),(25,129)])):
        front.put(x, y, ROCK[2 if x % 7 < 3 else 1])
    for x, y in sorted(poly_mask(SIZE, [(99,133),(111,121),(122,125),(144,114),(148,130),(127,142),(101,142)])):
        front.put(x, y, ROCK[2 if x % 9 < 4 else 1])
    line(front, [(24,115),(38,121),(46,118),(58,127),(70,133)], ROCK[4])
    line(front, [(100,132),(111,120),(123,124),(145,114)], ROCK[4])
    for x, top in [(27,117),(40,125),(54,131),(115,127),(130,128),(142,120)]:
        line(front, [(x,top),(x-1,top+8)], ROCK[0])
    composite(canvas, front, OUT)
    # A human-scale flight of basalt steps descends through the southern lip.
    steps = Layer(*SIZE)
    for n, y in enumerate((125,129,133,137,141)):
        half = 5 + n
        for yy in range(y, min(y+4, SIZE[1])):
            for x in range(84-half, 84+half+1):
                steps.put(x, yy, ROCK[4 if yy == y else 2 if yy == y+1 else 1])
    composite(canvas, steps, None)
    # Small vent wisps leave the rift readable and do not obscure the rocky rim.
    smoke = Layer(*SIZE)
    rng = random.Random(113)
    for cx, cy, rx, ry in [(50,70,4,5),(52,61,5,6),(57,51,4,5),(60,43,3,3),
                           (116,66,3,4),(112,57,4,5),(115,48,3,4)]:
        for y in range(cy-ry, cy+ry+1):
            for x in range(cx-rx, cx+rx+1):
                if ((x-cx)/rx)**2 + ((y-cy)/ry)**2 <= 1:
                    smoke.put(x, y, ROCK[3 if x < cx-1 else 2 if x <= cx+1 else 1])
    composite(canvas, smoke, None)
    embers = Layer(*SIZE)
    for x, y in [(44,63),(62,56),(76,72),(91,65),(105,79),(121,60),(132,95),(67,95)]:
        embers.put(x, y, LAVA[4])
        if rng.randrange(2):
            embers.put(x, y+1, LAVA[2])
    composite(canvas, embers, None)
    return canvas


if __name__ == '__main__':
    art = build()
    save_sprite(art, ROOT / 'assets/images/ashen-entrance.bmp')
    output = ROOT / 'docs/art-sources/ashen-rift'
    output.mkdir(parents=True, exist_ok=True)
    art.save(output / 'entrance.png')
    sheet = Image.new('RGB', (600, 190), (17,35,23))
    sheet.paste(art, (8,34), art)
    for name, pos in [('assets/images/blacksmith.bmp',(205,34)),('assets/images/giants-crown-entrance.bmp',(380,34))]:
        image = Image.open(ROOT / name).convert('RGBA')
        sheet.paste(image, pos, image)
    sheet.save(output / 'native-scale-review.png')
