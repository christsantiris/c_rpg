#!/usr/bin/env python3
"""Ridgeshire Town Hall, drawn at the same scale as the other town buildings."""
from pathlib import Path

from PIL import Image

from pixelkit import (Layer, arched_door, beam, composite, flat_wall,
                      lit_window, poly_mask, roof_tiles, save_sprite, stone_arch)

W, H = 168, 120
OUT = (18, 18, 23)
STONE = [(37, 39, 43), (60, 64, 66), (86, 92, 91), (116, 123, 117),
         (146, 150, 134), (181, 181, 153)]
ROOF = [(27, 30, 40), (41, 48, 61), (62, 75, 89), (91, 109, 120), (132, 145, 147)]
WOOD = [(34, 24, 18), (60, 43, 29), (95, 68, 43), (128, 96, 60), (162, 130, 86)]
IRON = [(29, 31, 37), (57, 61, 65), (96, 101, 101), (153, 154, 141)]
GLOW = [(71, 65, 43), (147, 111, 46), (220, 167, 64), (252, 217, 131)]


def rectangle(layer, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            layer.put(x, y, color)


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    facade = Layer(W, H)
    flat_wall(facade, 19, 149, 52, 109, STONE, row_h=6, base=3, seed=71, bw=(8, 12))
    beam(facade, 19, 76, 149, 76, WOOD, width=3)
    for x in (20, 59, 109, 148):
        beam(facade, x, 56, x, 107, WOOD, width=3)
    composite(canvas, facade, OUT)

    roof = Layer(W, H)
    roof_tiles(roof, poly_mask((W, H), [(10, 57), (33, 29), (135, 29), (158, 57)]),
               ROOF, 73, tile_w=6, row_h=4, rounded=False)
    beam(roof, 13, 58, 155, 58, WOOD, width=3)
    composite(canvas, roof, OUT)

    tower = Layer(W, H)
    flat_wall(tower, 69, 99, 18, 66, STONE, row_h=5, base=3, seed=74, bw=(5, 8))
    roof_tiles(tower, poly_mask((W, H), [(64, 22), (84, 5), (104, 22)]),
               ROOF, 75, tile_w=5, row_h=3, rounded=False)
    rectangle(tower, 78, 29, 90, 45, OUT)
    rectangle(tower, 80, 31, 88, 39, IRON[1])
    rectangle(tower, 79, 39, 89, 42, GLOW[2])
    rectangle(tower, 83, 43, 85, 44, GLOW[3])
    composite(canvas, tower, OUT)

    windows = Layer(W, H)
    for x in (30, 121):
        lit_window(windows, x, 61, 16, 12, GLOW, WOOD, shutters=False)
        lit_window(windows, x, 85, 16, 17, GLOW, WOOD)
    # Blue municipal banners with a gold mountain crest.
    for x in (62, 99):
        rectangle(windows, x, 80, x + 6, 100, ROOF[1])
        rectangle(windows, x + 1, 81, x + 5, 98, ROOF[3])
        for y in range(4):
            rectangle(windows, x + 3 - y // 2, 88 + y, x + 3 + y // 2, 88 + y, GLOW[2])
    composite(canvas, windows, OUT)

    entrance = Layer(W, H)
    # Match the workshop and shops: a 13 x 19 pixel doorway.
    stone_arch(entrance, 84, 90, 108, 6, STONE, thick=3)
    arched_door(entrance, 84, 90, 108, 6, WOOD, IRON)
    rectangle(entrance, 75, 109, 93, 112, STONE[4])
    rectangle(entrance, 72, 113, 96, 116, STONE[2])
    composite(canvas, entrance, OUT)
    return canvas


if __name__ == "__main__":
    save_sprite(build(), Path(__file__).resolve().parents[2] / "assets/images/town-hall.bmp")
