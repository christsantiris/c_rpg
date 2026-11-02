"""Soulstone Shrine: native arcane stonework, matching pixelkit's town art."""
from pathlib import Path
import math
import random

from PIL import Image
from pixelkit import Layer, beam, composite, line_points, poly_mask, save_sprite

W, H = 168, 120
OUT = (23, 27, 35)
STONE = [(39, 42, 52), (58, 62, 73), (81, 88, 96), (113, 121, 123), (148, 153, 143)]
RUNE = [(53, 69, 83), (79, 135, 145), (124, 205, 194), (197, 240, 214)]
CRYSTAL = [(46, 33, 77), (81, 48, 127), (128, 78, 181), (184, 131, 222), (227, 192, 245)]
rng = random.Random(234)
image = Image.new("RGBA", (W, H))


def ellipse(cx, cy, rx, ry, count=48):
    return [(round(cx + math.cos(i * 2 * math.pi / count) * rx),
             round(cy + math.sin(i * 2 * math.pi / count) * ry)) for i in range(count)]


# Low, irregular cobbled platform with a dark beveled rim.
base = Layer(W, H)
for x, y in sorted(poly_mask((W, H), ellipse(84, 80, 62, 29))):
    tone = 1 if y > 91 else 2
    if (x + y // 8 * 11) % 17 < 2 or y % 8 == 0:
        tone = 0
    elif rng.random() < 0.18:
        tone = 3
    base.put(x, y, STONE[tone])
composite(image, base, OUT)

# A carved ritual ring. Breaks and moss keep the stone weathered.
ring = Layer(W, H)
points = ellipse(84, 79, 46, 20)
for a, b in zip(points, points[1:] + points[:1]):
    beam(ring, *a, *b, STONE, 2)
for angle in range(0, 360, 45):
    a = math.radians(angle)
    cx = round(84 + math.cos(a) * 46)
    cy = round(79 + math.sin(a) * 20)
    beam(ring, cx - 2, cy - 2, cx + 2, cy + 2, RUNE, 1)
for a, b in [((75, 84), (84, 78)), ((84, 78), (93, 84)),
             ((93, 84), (84, 90)), ((84, 90), (75, 84)),
             ((71, 82), (71, 88)), ((97, 82), (97, 88))]:
    beam(ring, *a, *b, RUNE, 1)
composite(image, ring, None)

# Rune-cut standing stones with chipped tops and upper-left illumination.
for cx, bottom, height, width in [(39, 59, 31, 11), (126, 61, 36, 12),
                                  (30, 94, 27, 12), (139, 96, 30, 12)]:
    pillar = Layer(W, H)
    shape = [(cx - width // 2, bottom), (cx - width // 2 - 1, bottom - height + 5),
             (cx - 3, bottom - height), (cx + 2, bottom - height + 2),
             (cx + width // 2, bottom - height), (cx + width // 2 + 1, bottom)]
    for x, y in sorted(poly_mask((W, H), shape)):
        tone = 3 if x < cx - 1 else 2 if x < cx + 2 else 1
        if rng.random() < 0.12:
            tone = max(0, tone - 1)
        pillar.put(x, y, STONE[tone])
    beam(pillar, cx - 2, bottom - height + 10, cx + 2, bottom - height + 13, RUNE, 1)
    beam(pillar, cx + 2, bottom - height + 13, cx - 2, bottom - height + 17, RUNE, 1)
    composite(image, pillar, OUT)

# A faceted soulstone levitates above an old stone pedestal.
pedestal = Layer(W, H)
for x, y in sorted(poly_mask((W, H), [(68, 57), (83, 52), (100, 58), (100, 65), (84, 71), (68, 65)])):
    pedestal.put(x, y, STONE[3 if y < 59 else 2 if x < 84 else 1])
beam(pedestal, 69, 58, 84, 63, STONE, 2)
composite(image, pedestal, OUT)
crystal = Layer(W, H)
for points, tone in [([(83, 16), (73, 31), (76, 48), (84, 57), (94, 43), (96, 29)], 1),
                     ([(83, 16), (78, 32), (84, 38), (96, 29)], 3),
                     ([(78, 32), (76, 48), (84, 57), (84, 38)], 2),
                     ([(84, 38), (84, 57), (94, 43), (96, 29)], 0)]:
    for x, y in sorted(poly_mask((W, H), points)):
        crystal.put(x, y, CRYSTAL[tone])
beam(crystal, 83, 18, 79, 31, CRYSTAL, 1)
beam(crystal, 79, 32, 84, 37, CRYSTAL, 1)
composite(image, crystal, OUT)

# Loose stones, sparse moss and arcane motes; no religious insignia.
detail = Layer(W, H)
for x, y in [(51, 99), (110, 104), (62, 62), (116, 74), (47, 71), (102, 96)]:
    beam(detail, x, y, x + 3, y, [(39, 54, 46), (55, 72, 58), (66, 83, 63), (78, 95, 68)], 1)
for x, y in [(71, 19), (102, 35), (69, 43), (97, 16)]:
    detail.put(x, y, RUNE[2])
    detail.put(x, y - 1, RUNE[3])
composite(image, detail, None)
path = Path(__file__).resolve().parents[2] / "assets/images/soulstone-shrine.bmp"
save_sprite(image, path)
print(path)
