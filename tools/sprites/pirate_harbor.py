"""Walkable pirate ships' fittings, at native 24-pixel terrain scale.

Uses the same pixelkit layers, outlines, timber and sail palette as the harbor.
Six 96x120 cells: Gull mast, Nightjar mast, sailcloth cargo, strongbox, winch
and lowered winch. Small fittings occupy each cell's bottom-center 24x24 crop.
"""
from PIL import Image
from pixelkit import Layer, beam, composite, line_points, poly_mask, flat_wall, merlons, stone_arch, stamp, LANTERN
from harbor import TIMBER, SAIL, ROPE, IRON

OUT = (22, 20, 17)
SIZE = (96, 120)


def mast(dark=False):
    canvas = Image.new('RGBA', SIZE)
    rigging = Layer(*SIZE)
    for a, b in [((48, 12), (14, 112)), ((49, 12), (84, 112)),
                 ((14, 32), (23, 80)), ((83, 32), (75, 80))]:
        for x, y in line_points(*a, *b):
            rigging.put(x, y, ROPE[1])
    composite(canvas, rigging, None)
    timber = Layer(*SIZE)
    beam(timber, 48, 10, 48, 114, TIMBER, width=4)
    beam(timber, 11, 31, 85, 31, TIMBER, width=3)
    beam(timber, 23, 80, 75, 80, TIMBER, width=2)
    composite(canvas, timber, OUT)
    cloth = Layer(*SIZE)
    colors = [(40, 46, 47), (57, 62, 58), (78, 80, 70), (100, 101, 86), (126, 125, 106)] if dark else SAIL
    for x, y in sorted(poly_mask(SIZE, [(14, 34), (82, 34), (79, 51), (73, 77), (55, 82), (26, 77), (19, 58)])):
        shade = 3 if x < 43 else 2 if x < 65 else 1
        if x % 13 in (0, 1):
            shade = max(0, shade - 1)
        if (x + y * 3) % 41 == 0:
            shade = max(0, shade - 1)
        cloth.put(x, y, colors[shade])
    for x, y in line_points(25, 77, 56, 81) + line_points(56, 81, 73, 77):
        cloth.put(x, y, colors[0])
    for x, y in sorted(poly_mask(SIZE, [(51, 11), (73, 15), (68, 20), (51, 21)])):
        cloth.put(x, y, (138, 56, 40) if not dark else (37, 40, 38))
    composite(canvas, cloth, OUT)
    cap = Layer(*SIZE)
    for y in range(100, 117):
        for x in range(45, 53):
            cap.put(x, y, TIMBER[3] if x < 48 else TIMBER[1])
    for x in range(43, 55):
        cap.put(x, 111, IRON[1])
        cap.put(x, 114, IRON[1])
    composite(canvas, cap, OUT)
    return canvas


def fitting(index):
    canvas = Image.new('RGBA', SIZE)
    layer = Layer(*SIZE)
    if index == 2:
        mask = poly_mask(SIZE, [(39, 102), (52, 100), (57, 105), (56, 116), (40, 117), (38, 111)])
        for x, y in sorted(mask):
            layer.put(x, y, SAIL[2 if y < 109 else 1])
        for a, b in [((43, 101), (44, 117)), ((52, 102), (52, 116)), ((39, 109), (57, 109))]:
            for x, y in line_points(*a, *b):
                layer.put(x, y, ROPE[0])
        for y in range(104, 108):
            for x in range(46, 50):
                layer.put(x, y, (61, 86, 101))
    elif index == 3:
        for y in range(103, 118):
            for x in range(38, 58):
                layer.put(x, y, TIMBER[2 if y < 109 else 1])
        for x in (39, 43, 53, 56):
            for y in range(103, 117):
                layer.put(x, y, IRON[2 if x < 48 else 1])
        for x in range(38, 58):
            layer.put(x, 108, IRON[0])
        for y in range(107, 113):
            for x in range(47, 51):
                layer.put(x, y, (212, 174, 83) if x < 49 else (150, 113, 51))
        layer.put(49, 110, OUT)
    else:
        # Eight-spoked capstan, distinct from loot and sword silhouettes.
        for a, b in [((39, 103), (56, 116)), ((39, 116), (56, 103)),
                     ((37, 109), (58, 109)), ((48, 100), (48, 118))]:
            for x, y in line_points(*a, *b):
                layer.put(x, y, TIMBER[3])
        for x, y in sorted(poly_mask(SIZE, [(44, 104), (51, 104), (54, 108), (53, 114), (44, 116), (41, 111)])):
            layer.put(x, y, TIMBER[3 if y < 109 else 1])
        for x in range(43, 53):
            layer.put(x, 110, ROPE[2])
            layer.put(x, 112, ROPE[1])
        for y in range(105, 108):
            layer.put(47, y, (134, 167, 104) if index == 5 else (220, 182, 86))
    composite(canvas, layer, OUT)
    return canvas


def build():
    atlas = Image.new('RGBA', (96 * 6, 120))
    atlas.alpha_composite(mast(), (0, 0))
    atlas.alpha_composite(mast(True), (96, 0))
    for i in range(2, 6):
        atlas.alpha_composite(fitting(i), (96 * i, 0))
    return atlas


def town_gate():
    """Dawnmere's sea gate: coastal stone, anchor relief, rope and lanterns."""
    canvas = Image.new('RGBA', (72, 72))
    stone = [(32, 37, 38), (48, 55, 55), (76, 84, 81), (103, 111, 103), (137, 142, 127), (174, 174, 149)]
    wall = Layer(72, 72)
    flat_wall(wall, 18, 53, 22, 71, stone, row_h=5, base=3, seed=81, bw=(6, 9))
    # A transparent one-tile arch keeps the cobblestone route visible underneath.
    for y in range(43, 72):
        half = 11 if y >= 54 else int(max(0, 121 - (y - 54) ** 2) ** 0.5)
        for x in range(36 - half, 37 + half):
            wall.erase(x, y)
    stone_arch(wall, 36, 43, 71, 11, stone, thick=3)
    for left, right, seed in [(2, 20, 82), (52, 69, 83)]:
        flat_wall(wall, left, right, 13, 71, stone, row_h=5, base=3, seed=seed, bw=(5, 8),
                  shade=lambda x, y: -1 if x % 24 > 13 else 0)
        merlons(wall, left - 1, right + 1, 5, 8, stone, mw=4, gap=3, base=4, point=0)
        beam(wall, left - 1, 15, right + 1, 15, stone)
        beam(wall, left - 1, 66, right + 1, 66, stone)
    composite(canvas, wall, OUT)
    detail = Layer(72, 72)
    brass = (205, 170, 94)
    # An anchor belongs to the town's sea route, rather than a goblin/mountain crest.
    for a, b in [((36, 29), (36, 39)), ((32, 33), (40, 33)), ((30, 36), (32, 39)),
                 ((32, 39), (36, 40)), ((36, 40), (40, 39)), ((40, 39), (42, 36))]:
        for x, y in line_points(*a, *b):
            detail.put(x, y, brass)
    for x, y in [(35, 27), (36, 26), (37, 27), (36, 28)]:
        detail.put(x, y, brass)
    for x0 in (8, 58):
        for y in range(24, 47):
            for x in range(x0, x0 + 6):
                if y < 43 or x not in (x0, x0 + 5):
                    detail.put(x, y, (48, 83, 88) if x % 3 else (68, 105, 103))
        for y in (22, 48, 50):
            for x in range(x0 - 1, x0 + 7):
                detail.put(x, y, ROPE[1 if y != 48 else 2])
    lamp = {'i': IRON[1], 'I': IRON[2], 'y': (202, 135, 53), 'Y': (242, 191, 93), 'W': (255, 227, 151)}
    stamp(detail, 19, 49, LANTERN, lamp)
    stamp(detail, 47, 49, LANTERN, lamp)
    composite(canvas, detail, OUT)
    return canvas
