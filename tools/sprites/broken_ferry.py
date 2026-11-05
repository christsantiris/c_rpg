"""Oakhaven's ruined south-bank ferry, at the native 24px tile scale."""
from pathlib import Path
import random

from PIL import Image
from pixelkit import Layer, beam, composite, line_points, poly_mask, save_sprite


W, H = 168, 144
OUT = (22, 29, 29)
WOOD = [(45, 40, 33), (65, 57, 43), (92, 80, 57), (126, 109, 76)]
ROPE = [(75, 64, 43), (110, 94, 61), (147, 128, 85), (185, 161, 109)]
STONE = [(44, 57, 56), (64, 79, 74), (89, 103, 92), (121, 131, 111)]
rng = random.Random(216)
image = Image.new("RGBA", (W, H))


def planks(layer, points, vertical=False):
    mask = poly_mask((W, H), points)
    for x, y in sorted(mask):
        cross = x if vertical else y
        grain = y if vertical else x
        tone = 2
        if cross % 5 == 0:
            tone = 0
        elif cross % 5 == 1:
            tone = 3
        elif rng.random() < 0.12 or (grain % 19 == 0 and cross % 5 == 3):
            tone = 1
        layer.put(x, y, WOOD[tone])


# Shore stones, with the same dark outlines and upper-left light as buildings.
rocks = Layer(W, H)
for cx, cy, radius in ((35, 109, 10), (48, 119, 7), (112, 135, 8), (146, 133, 9)):
    shape = [(cx - radius, cy), (cx - radius + 3, cy - 5),
             (cx + 3, cy - 7), (cx + radius, cy - 1),
             (cx + radius - 2, cy + 5), (cx - 5, cy + 6)]
    for x, y in sorted(poly_mask((W, H), shape)):
        rocks.put(x, y, STONE[3 if y < cy - 3 else 2 if x < cx else 1])
composite(image, rocks, OUT)

# The landing stops in splintered boards, barely reaching into the south shore.
dock = Layer(W, H)
beam(dock, 65, 103, 97, 103, WOOD, 4)
beam(dock, 65, 124, 97, 124, WOOD, 4)
planks(dock, [(68, 132), (68, 93), (72, 97), (75, 88), (79, 96),
              (83, 91), (87, 98), (92, 91), (96, 96), (96, 132)], True)
for x in (70, 95):
    for y in (105, 125):
        dock.put(x, y, (44, 49, 46))
        dock.put(x, y - 1, (152, 149, 118))
composite(image, dock, OUT)

# Two rope-wrapped pilings. Neither rope reaches the opposite riverbank.
posts = Layer(W, H)
for x, top, bottom in ((61, 81, 122), (98, 86, 131)):
    beam(posts, x, top, x, bottom, WOOD, 6)
    beam(posts, x - 1, top, x + 6, top, WOOD, 3)
    for y in (top + 10, top + 13, top + 16):
        beam(posts, x - 1, y, x + 6, y, ROPE, 2)
composite(image, posts, OUT)
rope = Layer(W, H)
for a, b in (((64, 95), (55, 79)), ((55, 79), (48, 59)),
             ((48, 59), (45, 43)), ((101, 99), (112, 104)),
             ((112, 104), (125, 103))):
    beam(rope, *a, *b, ROPE, 2)
for a, b in (((45, 43), (41, 38)), ((45, 43), (47, 37)),
             ((125, 103), (130, 100)), ((125, 103), (132, 105))):
    beam(rope, *a, *b, ROPE, 1)
composite(image, rope, OUT)

# A small stranded ferry hull, torn open at the stern, with exposed ribs.
boat = Layer(W, H)
planks(boat, [(106, 111), (119, 104), (142, 106), (158, 116),
              (149, 128), (123, 133), (109, 126), (113, 123),
              (105, 121), (112, 117)])
inner = poly_mask((W, H), [(115, 113), (123, 109), (141, 111),
                         (150, 117), (143, 123), (121, 126), (113, 121)])
for x, y in sorted(inner):
    boat.put(x, y, WOOD[0 if y < 118 else 1])
for x in (123, 135, 144):
    beam(boat, x, 112, x + 2, 123, WOOD, 2)
beam(boat, 122, 109, 141, 111, WOOD, 2)
beam(boat, 141, 111, 154, 117, WOOD, 2)
beam(boat, 153, 118, 144, 127, WOOD, 2)
beam(boat, 143, 127, 122, 131, WOOD, 2)
beam(boat, 111, 128, 105, 132, WOOD, 2)
beam(boat, 110, 121, 102, 120, WOOD, 2)
composite(image, boat, OUT)

# A broken oar on the bank, and a few foam flecks around the ruined dock.
detail = Layer(W, H)
beam(detail, 114, 137, 147, 136, WOOD, 2)
beam(detail, 147, 135, 156, 135, WOOD, 4)
for x, y in ((60, 73), (79, 83), (89, 87), (104, 79), (119, 83)):
    for dx, dy in line_points(0, 0, 6, 0):
        detail.put(x + dx, y + dy, (181, 217, 211))
composite(image, detail, None)

path = Path(__file__).resolve().parents[2] / "assets/images/broken-ferry.bmp"
save_sprite(image, path)
print(path)
