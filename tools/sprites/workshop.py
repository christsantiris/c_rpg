#!/usr/bin/env python3
"""Ridgeshire workshop sprite.

Draws 5 x 4 tiles (120 x 96) at TOWN4_WORKSHOP_X/Y. Run `make sprites` to
rewrite assets/images/workshop.bmp.
"""
from pathlib import Path

from PIL import Image

from pixelkit import (Layer, arched_door, beam, composite, flat_wall, lit_window,
                      poly_mask, roof_tiles, save_sprite, stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 120, 96

OUT = (15, 13, 12)
ROOF = [(25, 35, 38), (38, 53, 55), (55, 74, 73), (77, 98, 92), (105, 123, 110)]
STONE = [(32, 34, 34), (48, 52, 49), (70, 76, 68), (96, 103, 89),
         (125, 130, 108), (154, 157, 128)]
WOOD = [(25, 18, 14), (49, 34, 24), (76, 52, 34), (105, 74, 45), (139, 103, 62)]
IRON = [(27, 29, 31), (54, 59, 58), (88, 94, 89), (132, 137, 124)]
GLOW = [(139, 53, 13), (199, 88, 17), (239, 142, 35), (255, 197, 78)]
GLASS = [(46, 57, 52), (91, 119, 103), (164, 180, 141), (238, 192, 92)]
SIGN = [(28, 31, 27), (76, 57, 35), (133, 99, 49), (221, 179, 81)]


def rectangle(layer, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            layer.put(x, y, color)


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    chimney = Layer(W, H)
    flat_wall(chimney, 87, 99, 7, 31, STONE, row_h=5, base=3, seed=14, bw=(4, 6))
    for x in range(86, 101):
        chimney.put(x, 5, STONE[4])
        chimney.put(x, 6, STONE[2])
    composite(canvas, chimney, OUT)

    roof = Layer(W, H)
    roof_shape = poly_mask((W, H), [
        (9, 49), (37, 17), (59, 10), (63, 10), (84, 17), (111, 49),
        (103, 54), (17, 54),
    ])
    roof_tiles(roof, roof_shape, ROOF, 29, tile_w=6, row_h=4, rounded=False)
    for x in range(12, 109, 10):
        roof.put(x, 49, ROOF[4])
        roof.put(x + 1, 50, ROOF[1])
    composite(canvas, roof, OUT)

    facade = Layer(W, H)
    flat_wall(facade, 17, 103, 48, 84, STONE, row_h=6, base=3, seed=22, bw=(7, 11))
    beam(facade, 17, 47, 103, 47, WOOD, width=3)
    beam(facade, 18, 49, 18, 83, WOOD, width=3)
    beam(facade, 102, 49, 102, 83, WOOD, width=3)
    beam(facade, 25, 79, 96, 79, WOOD, width=2)
    for x in range(28, 98, 14):
        beam(facade, x, 48, x, 78, WOOD, width=2)
    rectangle(facade, 17, 82, 103, 87, STONE[1])
    composite(canvas, facade, OUT)

    front = Layer(W, H)
    lit_window(front, 27, 58, 15, 14, GLASS, WOOD)
    lit_window(front, 78, 58, 15, 14, GLASS, WOOD)
    for cx in (34, 85):
        beam(front, cx, 56, cx, 75, WOOD, width=2)
        beam(front, cx - 9, 65, cx + 9, 65, WOOD, width=2)
    composite(canvas, front, OUT)

    entrance = Layer(W, H)
    # Shop-scale door: 13 x 19 pixels, keeping the threshold at y=86.
    stone_arch(entrance, 60, 68, 86, 6, STONE, thick=3)
    arched_door(entrance, 60, 68, 86, 6, WOOD, IRON)
    rectangle(entrance, 52, 87, 68, 89, STONE[4])
    rectangle(entrance, 50, 90, 70, 93, STONE[2])
    composite(canvas, entrance, OUT)

    sign = Layer(W, H)
    rectangle(sign, 43, 38, 77, 51, OUT)
    rectangle(sign, 46, 40, 74, 49, SIGN[1])
    rectangle(sign, 48, 41, 72, 42, SIGN[2])
    rectangle(sign, 57, 44, 63, 46, SIGN[3])
    rectangle(sign, 59, 42, 61, 48, SIGN[3])
    rectangle(sign, 53, 47, 67, 48, IRON[2])
    composite(canvas, sign, OUT)

    forge = Layer(W, H)
    rectangle(forge, 20, 73, 25, 81, IRON[1])
    rectangle(forge, 21, 75, 24, 79, GLOW[2])
    rectangle(forge, 22, 76, 23, 78, GLOW[3])
    composite(canvas, forge, OUT)

    return canvas


def main():
    save_sprite(build(), ASSETS / "workshop.bmp")


if __name__ == "__main__":
    main()
