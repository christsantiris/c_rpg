#!/usr/bin/env python3
"""Ironfang quest props at native 24px scale, using its existing pixelkit palette."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, poly_mask, beam, line_points, save_sprite
from ironfang import WOOD, STONE, OUT

ROOT = Path(__file__).resolve().parents[2]
IRON = [(39, 46, 47), (80, 92, 88), (147, 156, 142), (211, 216, 189)]


def polygon(layer, points, color):
    for x, y in poly_mask((layer.w, layer.h), points):
        layer.put(x, y, color)


def crate():
    image = Image.new('RGBA', (24, 24))
    layer = Layer(24, 24)
    for y in range(8, 22):
        for x in range(3, 22):
            layer.put(x, y, WOOD[2] if x % 5 else WOOD[0])
    for x in (4, 19):
        for y in range(8, 22):
            layer.put(x, y, IRON[1])
            layer.put(x + 1, y, IRON[2])
    # Visible sword blades, spearhead and a shield distinguish military cargo.
    for x, y in line_points(9, 12, 6, 2):
        layer.put(x, y, IRON[3])
        layer.put(x + 1, y, IRON[1])
    for x, y in line_points(8, 10, 13, 8):
        layer.put(x, y, (201, 153, 64))
    beam(layer, 16, 2, 16, 14, WOOD, width=1)
    polygon(layer, [(14, 3), (16, 0), (18, 3), (16, 5)], IRON[3])
    polygon(layer, [(10, 10), (14, 8), (19, 10), (18, 17), (14, 20), (10, 17)], IRON[0])
    polygon(layer, [(11, 11), (14, 9), (18, 11), (17, 16), (14, 18), (11, 16)], (114, 54, 37))
    layer.put(14, 13, (224, 181, 86))
    composite(image, layer, OUT)
    return image


def gantry(loaded):
    image = Image.new('RGBA', (48, 48))
    layer = Layer(48, 48)
    beam(layer, 6, 5, 6, 43, WOOD, width=3)
    beam(layer, 6, 5, 40, 5, WOOD, width=3)
    beam(layer, 8, 18, 25, 6, WOOD, width=2)
    for x in range(2, 13):
        for y in range(43, 47):
            layer.put(x, y, STONE[4] if y == 43 else STONE[2])
    for y in range(8, 27):
        layer.put(33, y, IRON[2] if y % 3 else IRON[0])
        layer.put(34, y, IRON[0] if y % 3 else IRON[2])
    for x in (6, 37):
        layer.put(x, 5, IRON[3])
    polygon(layer, [(30, 26), (34, 26), (36, 29), (33, 32), (30, 29)], IRON[1])
    composite(image, layer, OUT)
    if loaded:
        image.alpha_composite(crate(), (21, 23))
    return image


def winch():
    image = Image.new('RGBA', (24, 24))
    layer = Layer(24, 24)
    beam(layer, 3, 20, 20, 20, WOOD, width=2)
    beam(layer, 7, 8, 7, 19, WOOD, width=2)
    beam(layer, 17, 8, 17, 19, WOOD, width=2)
    for y in range(10, 16):
        for x in range(5, 20):
            layer.put(x, y, (180, 151, 97) if x % 3 else (88, 69, 44))
    beam(layer, 19, 6, 19, 17, IRON, width=1)
    beam(layer, 19, 6, 22, 6, WOOD, width=1)
    composite(image, layer, OUT)
    return image


def orders():
    image = Image.new('RGBA', (24, 24))
    layer = Layer(24, 24)
    for y in range(10, 22):
        for x in (4, 19):
            layer.put(x, y, WOOD[1])
    for y in range(5, 17):
        for x in range(2, 23):
            layer.put(x, y, WOOD[3] if y % 4 else WOOD[1])
    polygon(layer, [(6, 6), (18, 6), (19, 14), (7, 14)], (219, 203, 150))
    for y in (8, 10, 12):
        for x in range(9, 16):
            layer.put(x, y, (87, 71, 43))
    layer.put(17, 12, (154, 52, 37))
    composite(image, layer, OUT)
    return image


def coffer(empty):
    image = Image.new('RGBA', (24, 24))
    layer = Layer(24, 24)
    polygon(layer, [(3, 11), (7, 8), (19, 8), (22, 11), (21, 21), (3, 21)], WOOD[2])
    for x in (5, 18):
        for y in range(9, 21):
            layer.put(x, y, IRON[2])
    polygon(layer, [(4, 10), (7, 5), (18, 5), (21, 10)], WOOD[3])
    if empty:
        polygon(layer, [(4, 12), (20, 12), (19, 16), (5, 16)], OUT)
    else:
        polygon(layer, [(9, 11), (13, 9), (17, 11), (15, 17), (11, 17)], (191, 148, 62))
        layer.put(13, 12, (69, 89, 38))
        layer.put(14, 12, (69, 89, 38))
        layer.put(13, 14, (69, 89, 38))
    composite(image, layer, OUT)
    return image


def build():
    atlas = Image.new('RGBA', (168, 48))
    for sprite, xy in ((orders(), (0, 0)), (gantry(True), (24, 0)), (gantry(False), (72, 0)),
                       (winch(), (120, 0)), (crate(), (120, 24)), (coffer(False), (144, 0)), (coffer(True), (144, 24))):
        atlas.alpha_composite(sprite, xy)
    return atlas


if __name__ == '__main__':
    save_sprite(build(), ROOT / 'assets/images/war-machine-props.bmp')
