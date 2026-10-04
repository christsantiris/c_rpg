#!/usr/bin/env python3
"""Rosemoor Adventurer's Guild sprite.

Draws a compact facade centered in a 7 x 5 tile (168 x 120) canvas for the
Guild at TOWN_GUILD_X/Y. Run `make sprites` to rewrite the BMP asset.
"""
from pathlib import Path

from PIL import Image

from pixelkit import (Layer, arched_door, beam, composite, flat_wall, lit_window,
                      poly_mask, roof_tiles, save_sprite, stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 168, 120

OUT = (15, 14, 18)
ROOF = [(22, 28, 42), (34, 44, 62), (52, 68, 90), (72, 91, 116), (98, 118, 142)]
STONE = [(31, 35, 40), (43, 49, 55), (65, 73, 80), (91, 101, 108),
         (119, 129, 134), (151, 159, 161)]
PLASTER = [(119, 112, 91), (157, 149, 122), (190, 181, 148), (220, 211, 174)]
WOOD = [(25, 19, 17), (48, 35, 27), (73, 51, 36), (101, 71, 46), (132, 97, 61)]
IRON = [(29, 31, 37), (55, 61, 68), (89, 98, 106), (132, 142, 145)]
GOLD = [(116, 73, 24), (171, 119, 42), (218, 172, 74), (249, 218, 137)]
GLASS = [(35, 54, 69), (81, 117, 136), (157, 192, 196), (236, 211, 141)]

GLYPHS = {
    "G": ("111", "100", "101", "101", "111"),
    "U": ("101", "101", "101", "101", "111"),
    "I": ("111", "010", "010", "010", "111"),
    "L": ("100", "100", "100", "100", "111"),
    "D": ("110", "101", "101", "101", "110"),
}


def rectangle(layer, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            layer.put(x, y, color)


def guild_sign(layer):
    rectangle(layer, 54, 46, 113, 64, OUT)
    rectangle(layer, 57, 48, 110, 61, WOOD[2])
    rectangle(layer, 59, 50, 108, 51, WOOD[4])
    text = "GUILD"
    text_width = len(text) * 3 + (len(text) - 1) * 2
    x0 = (W - text_width) // 2
    for index, letter in enumerate(text):
        for y, row in enumerate(GLYPHS[letter]):
            for x, pixel in enumerate(row):
                if pixel == "1":
                    layer.put(x0 + index * 5 + x, 53 + y, GOLD[3])
    rectangle(layer, 82, 43, 85, 46, IRON[2])


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))

    roof = Layer(W, H)
    roof_shape = poly_mask((W, H), [
        (17, 52), (67, 13), (81, 8), (87, 8), (101, 13), (151, 52),
        (142, 58), (26, 58),
    ])
    roof_tiles(roof, roof_shape, ROOF, 33, tile_w=6, row_h=4, rounded=False)
    for x in range(20, 149, 12):
        roof.put(x, 51, ROOF[4])
        roof.put(x + 1, 52, ROOF[1])
    composite(canvas, roof, OUT)

    walls = Layer(W, H)
    for x in range(29, 140):
        walls.put(x, 49, WOOD[2])
        walls.put(x, 50, WOOD[1])
    wall = {(x, y) for x in range(29, 140) for y in range(51, 104)}
    flat_wall(walls, 30, 138, 51, 103, STONE, row_h=8, base=3, seed=17,
              bw=(8, 14))
    beam(walls, 29, 50, 139, 50, WOOD, width=3)
    beam(walls, 30, 51, 30, 103, WOOD, width=3)
    beam(walls, 136, 51, 136, 103, WOOD, width=3)
    for x in range(33, 137, 12):
        beam(walls, x, 92, x, 102, WOOD, width=2)
    rectangle(walls, 28, 102, 140, 109, STONE[1])
    for x in range(30, 139, 11):
        walls.put(x, 104, STONE[4] if x % 2 else STONE[2])
    composite(canvas, walls, OUT)

    front = Layer(W, H)
    lit_window(front, 42, 67, 17, 16, GLASS, WOOD)
    lit_window(front, 109, 67, 17, 16, GLASS, WOOD)
    for cx in (50, 117):
        beam(front, cx, 63, cx, 88, WOOD, width=2)
        beam(front, cx - 12, 76, cx + 12, 76, WOOD, width=2)
    composite(canvas, front, OUT)

    entrance = Layer(W, H)
    # 17 x 25 source pixels become a shop-scale door after the 75% reduction.
    stone_arch(entrance, 84, 84, 108, 8, STONE, thick=4)
    arched_door(entrance, 84, 84, 108, 8, WOOD, IRON)
    rectangle(entrance, 74, 108, 94, 110, STONE[3])
    rectangle(entrance, 71, 111, 97, 113, STONE[2])
    composite(canvas, entrance, OUT)

    sign = Layer(W, H)
    guild_sign(sign)
    composite(canvas, sign, OUT)

    crest = Layer(W, H)
    rectangle(crest, 80, 21, 87, 39, WOOD[1])
    rectangle(crest, 77, 23, 90, 34, GOLD[2])
    rectangle(crest, 79, 25, 88, 32, ROOF[1])
    for offset in range(5):
        crest.put(81 + offset, 31 - offset, GOLD[3])
        crest.put(86 - offset, 31 - offset, GOLD[3])
    crest.put(83, 23, GOLD[3])
    crest.put(84, 22, GOLD[3])
    composite(canvas, crest, OUT)

    scaled = canvas.resize((W * 3 // 4, H * 3 // 4), Image.Resampling.NEAREST)
    result = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    result.alpha_composite(scaled, ((W - scaled.width) // 2, H - scaled.height))
    return result


def main():
    save_sprite(build(), ASSETS / "adventurers-guild.bmp")


if __name__ == "__main__":
    main()
