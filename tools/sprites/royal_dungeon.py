#!/usr/bin/env python3
"""Royal Dungeon blockhouse in Castle Grounds: 7 x 5 tiles (168 x 120).

Squat crenellated towers and a gatehouse share the Castle of No Return's stone.
The gate opens on stairs down to the cells; barred grates at the foot of the
walls light the cells below. The door tile is JAIL_DOOR_X/Y in src/game/jail.h.
"""
import math
import random
from pathlib import Path

from PIL import Image

from pixelkit import (Layer, composite, cyl_wall, darken_in, flat_wall, gothic_half, line_points,
                      merlons, save_sprite, stamp)

W, H = 168, 120
CX = 84
OUT = (12, 9, 15)
ST = [(20, 18, 25), (30, 27, 36), (42, 38, 50), (55, 50, 63), (70, 64, 79), (88, 81, 96), (106, 99, 112)]
GL = [(80, 12, 18), (138, 22, 26), (196, 42, 32), (240, 88, 44), (255, 164, 92), (255, 222, 150)]
IRON = [(22, 22, 28), (40, 40, 48), (66, 66, 76), (100, 100, 110), (138, 138, 146)]
RF = [(38, 11, 20), (58, 17, 27), (82, 25, 35), (110, 37, 45), (140, 52, 56)]
GOLD = [(96, 70, 30), (160, 120, 48), (214, 172, 72), (246, 214, 120)]
SLIME = [(20, 32, 25), (30, 45, 32), (42, 60, 40)]
VOID = [(6, 5, 9), (12, 10, 16), (20, 17, 25), (30, 26, 36)]


def rectangle(layer, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            layer.put(x, y, color)


def weather(layer, x0, x1, y0, y1, seed, streaks=6, cracks=5):
    # Grime streaks and cracks, then moss creeping up from the ground.
    rng = random.Random(seed)
    for n in range(streaks):
        x = rng.randint(x0, x1)
        y = rng.randint(y0, y1 - 12)
        for k in range(rng.randint(6, 14)):
            darken_in(layer, x, y + k, ST)
    for n in range(cracks):
        x = rng.randint(x0, x1)
        y = rng.randint(y0, y1 - 6)
        for k in range(rng.randint(3, 7)):
            if layer.on(x, y) and layer.get(x, y) in ST:
                layer.put(x, y, ST[0])
            x += rng.choice((-1, 0, 1))
            y += 1
    reach = y1 - 6
    for x in range(x0, x1 + 1):
        if rng.random() < 0.4:
            reach = max(y1 - 10, min(y1 - 3, reach + rng.choice((-1, 1))))
        for y in range(reach, y1 + 1):
            if layer.on(x, y) and layer.get(x, y) in ST and (y >= reach + 2 or (x + y) % 2 == 0):
                layer.put(x, y, SLIME[min(2, max(0, ST.index(layer.get(x, y)) - 2))])


def tower(canvas, x0, x1, top, seed):
    # A squat drum tower with square merlons and a dark arrow slit.
    layer = Layer(W, H)
    cyl_wall(layer, x0, x1, top + 8, 117, ST, row_h=6, cols=6, seed=seed)
    merlons(layer, x0 - 1, x1 + 1, top, 9, ST, mw=6, gap=3, base=4, point=0, start=1)
    for x in range(x0 - 1, x1 + 2):
        layer.put(x, top + 8, ST[1])
        layer.put(x, top + 9, ST[5] if x < (x0 + x1) // 2 else ST[3])
    cx = (x0 + x1) // 2
    for y in range(top + 22, top + 38):
        layer.put(cx, y, VOID[0])
        layer.put(cx + 1, y, VOID[1])
    for y in range(top + 27, top + 31):
        for x in range(cx - 2, cx + 4):
            layer.put(x, y, VOID[0] if cx <= x <= cx + 1 else layer.get(x, y))
    rectangle(layer, cx - 2, top + 38, cx + 3, top + 39, ST[5])
    weather(layer, x0, x1, top + 12, 117, seed + 100, streaks=4, cracks=3)
    composite(canvas, layer, OUT)


def curtain(canvas):
    # The low curtain wall between the towers, with a sloped battered plinth.
    layer = Layer(W, H)
    flat_wall(layer, 30, 137, 36, 117, ST, row_h=7, base=3, seed=41, bw=(8, 14))
    merlons(layer, 30, 137, 27, 10, ST, mw=7, gap=4, base=4, point=0, start=2)
    for x in range(30, 138):
        layer.put(x, 36, ST[1])
        layer.put(x, 37, ST[5])
    for y in range(106, 118):
        spread = (y - 106) // 3
        for x in range(30 - spread, 138 + spread):
            idx = 4 if y == 106 else 3 if (x + (y // 4) * 3) % 11 else 1
            layer.put(x, y, ST[idx])
    weather(layer, 30, 137, 40, 117, 43)
    composite(canvas, layer, OUT)


def gatehouse(canvas):
    # The taller gatehouse carries the royal crest above a pointed arch.
    layer = Layer(W, H)
    flat_wall(layer, 58, 109, 18, 117, ST, row_h=7, base=4, seed=52, bw=(7, 12),
              shade=lambda x, y: 1 if x < 61 else -1 if x > 106 else 0)
    merlons(layer, 57, 110, 9, 10, ST, mw=6, gap=3, base=5, point=0, start=1)
    for x in range(57, 111):
        layer.put(x, 18, ST[1])
        layer.put(x, 19, ST[6])
    # Corbels under the parapet.
    for x in range(60, 108, 6):
        rectangle(layer, x, 20, x + 2, 22, ST[5])
        layer.put(x + 1, 23, ST[3])
    weather(layer, 58, 109, 26, 117, 53, streaks=5, cracks=3)
    composite(canvas, layer, OUT)


def arch(canvas):
    # Voussoirs around a pointed arch; inside, worn stairs drop into darkness.
    layer = Layer(W, H)
    apex = 62
    spring = 80
    half = 11
    for y in range(apex - 5, 118):
        rim = gothic_half(y, apex - 5, spring, half + 5)
        hole = gothic_half(y, apex, spring, half)
        if rim < 0:
            continue
        for x in range(CX - math.ceil(rim), CX + math.ceil(rim) + 1):
            dx = abs(x + 0.5 - CX - 0.5)
            if hole >= 0 and dx <= hole:
                continue
            if y < spring:
                seg = int(math.atan2(y - spring, x - CX + 0.01) * 4.0) % 2
                layer.put(x, y, ST[6] if seg else ST[4])
            else:
                block = (y - spring) // 6
                layer.put(x, y, ST[5] if (block + (x < CX)) % 2 else ST[3])
                if (y - spring) % 6 == 5:
                    layer.put(x, y, ST[1])
    # Keystone.
    rectangle(layer, CX - 2, apex - 7, CX + 2, apex - 1, ST[6])
    rectangle(layer, CX - 1, apex - 6, CX + 1, apex - 2, ST[5])
    composite(canvas, layer, OUT)

    opening = Layer(W, H)
    for y in range(apex, 118):
        hole = gothic_half(y, apex, spring, half)
        if hole < 0:
            continue
        for x in range(CX - int(hole), CX + int(hole) + 1):
            depth = (y - apex) / float(118 - apex)
            opening.put(x, y, VOID[0] if depth < 0.55 else VOID[1])
    # Steps descending away from the threshold, fading into the dark.
    for n, y in enumerate((116, 112, 108, 104, 101)):
        edge = ST[4 - min(3, n)]
        hole = gothic_half(y, apex, spring, half)
        for x in range(CX - int(hole) + 1 + n, CX + int(hole) - n):
            opening.put(x, y, edge)
            opening.put(x, y + 1, VOID[2] if n < 2 else VOID[1])
    # The portcullis hangs half raised: a grid of bars ending in iron teeth.
    for x in range(CX - half + 1, CX + half, 4):
        top = apex + 2 if abs(x - CX) < 6 else spring - 8 + abs(x - CX) // 3
        for y in range(top, 92):
            opening.put(x, y, IRON[3] if (x - CX) % 8 == 1 else IRON[2])
        opening.put(x, 92, IRON[4])
        opening.put(x, 93, IRON[2])
    for y in (70, 78, 86):
        hole = gothic_half(y, apex, spring, half)
        for x in range(CX - int(hole), CX + int(hole) + 1):
            opening.put(x, y, IRON[2] if x % 4 else IRON[3])
    composite(canvas, opening, None)


def crest(canvas):
    # A gold crown on a crimson shield above the gate, as on the castle front.
    layer = Layer(W, H)
    stamp(layer, CX - 7, 30, [
        "rrrrrrrrrrrrrrr",
        "rRRRRRRRRRRRRRr",
        "rRRgRRRgRRRgRRr",
        "rRRgGRgGgRgGRRr",
        "rRRgGGGGGGGGRRr",
        "rRRgGyGyGyGgRRr",
        "rRRgggggggggRRr",
        "rRRRRRRRRRRRRRr",
        ".rRRRRRRRRRRRr.",
        "..rRRRRRRRRRr..",
        "...rRRRRRRRr...",
        ".....rrrrr.....",
    ], {'r': RF[0], 'R': RF[2], 'g': GOLD[1], 'G': GOLD[2], 'y': GOLD[3]})
    composite(canvas, layer, OUT)


def banners(canvas):
    # Crimson royal banners with a gold crown hang from the curtain wall.
    layer = Layer(W, H)
    for x0 in (36, 119):
        for x in range(x0 - 1, x0 + 14):
            layer.put(x, 40, IRON[3])
        for y in range(41, 74):
            for x in range(x0, x0 + 13):
                if y > 66 and (abs(x - (x0 + 6)) < (y - 66) * 1.4 or (x - x0) % 4 == (y + x0) % 3 and y > 64):
                    continue
                k = x - x0
                c = RF[2] if k < 2 else RF[0] if k > 10 else RF[1]
                if k in (1, 11):
                    c = GOLD[1]
                layer.put(x, y, c)
        stamp(layer, x0 + 3, 47, ["g.g.g", "gGgGg", "GGGGG", "ggggg"], {'g': GOLD[1], 'G': GOLD[2]})
        for x in range(x0 + 2, x0 + 11):
            layer.put(x, 55, GOLD[1])
    composite(canvas, layer, OUT)


def grates(canvas):
    # Barred cell grates at the foot of the wall glow with the cells' fires.
    layer = Layer(W, H)
    for x0 in (36, 115):
        rectangle(layer, x0 - 2, 91, x0 + 18, 104, ST[5])
        rectangle(layer, x0 - 1, 92, x0 + 17, 103, ST[2])
        for y in range(93, 103):
            for x in range(x0, x0 + 17):
                ember = y - 93 + (3 - abs(x - x0 - 8) // 3)
                layer.put(x, y, GL[1] if ember > 9 else GL[0] if ember > 6 else VOID[1] if ember > 3 else VOID[0])
        for x in range(x0 + 1, x0 + 17, 3):
            for y in range(92, 104):
                layer.put(x, y, IRON[3] if y < 95 else IRON[2])
        for x in range(x0, x0 + 17):
            layer.put(x, 97, IRON[2])
        # Dark drips stain the wall under each grate.
        for x in range(x0 + 2, x0 + 16, 5):
            for y in range(105, 109 + x % 3):
                darken_in(layer, x, y, ST)
    # A prisoner's pale hands grip the bars of the eastern grate.
    for x in (118, 124):
        stamp(layer, x, 93, [".HH.", "HhhH", "HhhH", "hhhh", ".hh."], {'h': (150, 120, 100), 'H': (214, 184, 156)})
    composite(canvas, layer, OUT)


def chains(canvas):
    # Shackles hang on chains beside the gate.
    layer = Layer(W, H)
    for x in (54, 113):
        for y in range(44, 64):
            layer.put(x, y, IRON[4] if y % 3 == 0 else IRON[2])
        stamp(layer, x - 2, 64, [".iii.", "i...i", "i...i", ".iii."], {'i': IRON[3]})
    composite(canvas, layer, OUT)


def torches(canvas):
    # Iron sconces with burning torches flank the arch.
    layer = Layer(W, H)
    for x in (66, 101):
        stamp(layer, x - 2, 66, [
            "..y..",
            ".yYy.",
            ".oYo.",
            "oYWYo",
            ".oyo.",
            "iIIIi",
            ".iIi.",
            "..t..",
            "..t..",
            ".iii.",
        ], {'y': GL[3], 'Y': GL[4], 'W': GL[5], 'o': GL[2], 'i': IRON[1], 'I': IRON[3], 't': (77, 51, 34)})
    composite(canvas, layer, OUT)


def torchlight(canvas):
    # Warm light catches only the stone right beside each flame.
    pixels = canvas.load()
    for fx, fy in ((66, 69), (101, 69)):
        for y in range(fy - 7, fy + 8):
            for x in range(fx - 7, fx + 8):
                d = math.hypot(x - fx, y - fy)
                c = pixels[x, y]
                if 2.5 < d <= 7 and c[3] and c[:3] in ST:
                    k = 0.34 if d < 5 else 0.18
                    pixels[x, y] = tuple(int(c[i] + (GL[3][i] - c[i]) * k) for i in range(3)) + (255,)


def gibbet(canvas):
    # An iron gibbet cage swings from a timber beam on the western tower.
    layer = Layer(W, H)
    wood = [(33, 22, 18), (54, 36, 26), (77, 51, 34), (101, 69, 44)]
    for x in range(14, 34):
        layer.put(x, 33, wood[3])
        layer.put(x, 34, wood[2])
        layer.put(x, 35, wood[1])
    for x, y in line_points(28, 36, 33, 46):
        layer.put(x, y, wood[2])
    for y in range(36, 44):
        layer.put(17, y, IRON[3] if y % 2 else IRON[2])
    stamp(layer, 12, 44, [
        "...III...",
        "..IvvvI..",
        ".IvIvIvI.",
        "IvvIvIvvI",
        "IvvIvIvvI",
        "IIIIIIIII",
        "IvvIvIvvI",
        "IvvIvIvvI",
        "IvvIbIvvI",
        "IvbIBIbvI",
        "IIIIIIIII",
        ".IIIIIII.",
        "....I....",
    ], {'I': IRON[4], 'v': VOID[0], 'b': (134, 124, 104), 'B': (182, 172, 148)})
    composite(canvas, layer, OUT)


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    curtain(canvas)
    tower(canvas, 3, 35, 14, 61)
    tower(canvas, 132, 164, 14, 62)
    gatehouse(canvas)
    arch(canvas)
    crest(canvas)
    banners(canvas)
    grates(canvas)
    chains(canvas)
    torches(canvas)
    torchlight(canvas)
    gibbet(canvas)
    return canvas


if __name__ == "__main__":
    save_sprite(build(), Path(__file__).resolve().parents[2] / "assets/images/royal-dungeon.bmp")
