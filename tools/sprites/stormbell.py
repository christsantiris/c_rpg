#!/usr/bin/env python3
"""Native 24-pixel Stormwatch bell and clapper props, with transparent terrain."""
from pathlib import Path
from PIL import Image
from pixelkit import Layer, composite, poly_mask, save_sprite

ROOT = Path(__file__).resolve().parents[2]


def bell(fitted):
    canvas = Image.new('RGBA', (24, 40))
    frame = Layer(24, 40)
    # Weathered uprights, braced crossbeam, iron straps and stone footings.
    for x in (3, 19):
        for y in range(4, 36):
            for dx in range(3):
                frame.put(x + dx, y, (152, 119, 76) if dx == 0 else (98, 77, 52))
            if y % 8 == 1:
                frame.put(x + 1, y, (67, 57, 42))
        for y in range(35, 39):
            for dx in range(-1, 5):
                frame.put(x + dx, y, (113, 123, 115) if y == 35 else (69, 81, 81))
    for y in range(3, 7):
        for x in range(2, 23):
            frame.put(x, y, (173, 139, 91) if y == 3 else (106, 82, 52))
    for x in (4, 20):
        for y in (8, 28):
            frame.put(x, y, (174, 183, 166))
            frame.put(x, y + 1, (58, 73, 77))
    for y in range(7, 12):
        frame.put(12, y, (130, 142, 137))
    for x, y in poly_mask(canvas.size, [(11, 11), (14, 11), (17, 15), (17, 22), (20, 25), (20, 27), (6, 27), (6, 25), (9, 22), (9, 15)]):
        frame.put(x, y, (224, 185, 93) if x < 12 else (161, 117, 48) if x < 16 else (103, 77, 41))
        if y in (16, 24):
            frame.put(x, y, (239, 207, 124))
    for x in range(7, 20):
        frame.put(x, 27, (60, 56, 39))
    if fitted:
        for y in range(26, 31):
            frame.put(13, y, (191, 155, 76))
        frame.put(12, 30, (231, 202, 112))
        frame.put(14, 30, (125, 93, 43))
    composite(canvas, frame, (30, 39, 39))
    return canvas


def clapper():
    canvas = Image.new('RGBA', (24, 40))
    metal = Layer(24, 40)
    for x, y in poly_mask(canvas.size, [(7, 23), (9, 22), (16, 31), (16, 34), (13, 35), (11, 33), (12, 31)]):
        metal.put(x, y, (222, 185, 95) if x % 3 else (131, 95, 43))
    composite(canvas, metal, (37, 43, 35))
    return canvas


def build():
    atlas = Image.new('RGBA', (72, 40))
    for i, sprite in enumerate((bell(False), bell(True), clapper())):
        atlas.alpha_composite(sprite, (i * 24, 0))
    return atlas


if __name__ == '__main__':
    save_sprite(build(), ROOT / 'assets/images/stormbell-props.bmp')
