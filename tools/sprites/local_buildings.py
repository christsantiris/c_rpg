#!/usr/bin/env python3
"""Local town buildings, using the same native pixel art helpers as the shops."""
from pathlib import Path

from PIL import Image

from pixelkit import (BARREL, CRATE, LANTERN, Layer, arched_door, beam, composite,
                      flat_wall, light_spill, lit_window, plaster, poly_mask,
                      roof_tiles, save_sprite, shadow_under, stamp, stone_arch)

ASSETS = Path(__file__).resolve().parents[2] / "assets/images"
OUT = (20, 14, 12)
WOOD = [(26, 15, 9), (44, 26, 15), (64, 39, 22), (86, 54, 31), (110, 71, 41)]
STONE = [(30, 30, 38), (38, 38, 48), (80, 80, 94), (100, 100, 114), (124, 124, 136), (148, 148, 158)]
PLASTER = [(136, 110, 78), (170, 144, 106), (200, 176, 134), (222, 202, 162)]
IRON = [(28, 28, 32), (52, 52, 58), (84, 84, 92), (122, 122, 130)]
GLOW = [(150, 66, 10), (214, 114, 16), (246, 156, 24), (255, 202, 70), (255, 236, 150)]
ROOFS = [
    [(45, 23, 19), (76, 34, 24), (118, 50, 29), (156, 74, 43), (192, 103, 61), (221, 139, 91)],
    [(30, 23, 28), (52, 32, 40), (82, 43, 49), (116, 59, 62), (147, 82, 78), (175, 110, 96)],
    [(24, 24, 36), (38, 35, 55), (64, 51, 83), (91, 71, 112), (120, 97, 137), (154, 129, 166)],
]

LOAF = ["..bbbb..", ".bBBWWb.", "bBWBBWBb", "bBBBBBBb", ".bbbbbb."]
BREAD = {"b": (132, 77, 33), "B": (210, 145, 60), "W": (250, 210, 125)}
PROPS = {"t": WOOD[1], "T": WOOD[2], "W": WOOD[3], "i": IRON[1], "I": IRON[2]}


def rectangle(layer, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            layer.put(x, y, color)


def sack(layer, x, y, spice=None):
    for row in range(12):
        half = 3 if row < 3 else 5 if row < 10 else 4
        for col in range(-half, half + 1):
            color = (173, 145, 101) if col < 0 else (126, 99, 66)
            if col == -half:
                color = (205, 178, 125)
            if row == 11:
                color = WOOD[1]
            layer.put(x + col, y + row, color)
    rectangle(layer, x - 3, y + 2, x + 3, y + 3, WOOD[2])
    if spice:
        rectangle(layer, x - 2, y, x + 2, y + 1, spice)
    else:
        stamp(layer, x - 2, y + 5, ["..w..", ".www.", "..w..", ".www.", "..w.."], {"w": (225, 202, 154)})


def hanging_sign(layer, kind):
    beam(layer, 96, 55, 117, 55, IRON, width=2)
    for x in (103, 114):
        rectangle(layer, x, 57, x, 61, IRON[2])
    rectangle(layer, 100, 62, 117, 74, WOOD[0])
    rectangle(layer, 101, 63, 116, 73, WOOD[2])
    beam(layer, 102, 63, 115, 63, WOOD, width=1)
    if kind == 0:
        stamp(layer, 105, 66, LOAF, BREAD)
    elif kind == 1:
        stamp(layer, 103, 65, ["tttiiiiiiii", "tWWiIIIIIIi", "...iIIIIIWi", "....iiiiii.", "...........", "..G..G..G.."],
              {"t": WOOD[0], "W": (224, 216, 186), "i": IRON[1], "I": IRON[3], "G": BREAD["B"]})
    else:
        rectangle(layer, 106, 65, 111, 66, (180, 117, 65))
        rectangle(layer, 105, 67, 112, 71, (116, 58, 38))
        rectangle(layer, 106, 67, 108, 70, (213, 158, 74))


def stall(layer, kind):
    # A shaded lean-to, with goods that distinguish each trade.
    rectangle(layer, 5, 64, 33, 91, WOOD[0])
    for x in (5, 32):
        beam(layer, x, 63, x, 92, WOOD, width=2)
    beam(layer, 6, 87, 32, 87, WOOD, width=3)
    if kind == 0:
        flat_wall(layer, 8, 29, 69, 85, STONE, row_h=4, base=3, seed=82, bw=(4, 6))
        stone_arch(layer, 19, 73, 85, 5, STONE, thick=2)
        arched_door(layer, 19, 73, 85, 5, WOOD, IRON)
        rectangle(layer, 15, 80, 23, 84, GLOW[0])
        rectangle(layer, 17, 82, 21, 84, GLOW[2])
        stamp(layer, 11, 85, LOAF, BREAD)
        stamp(layer, 21, 85, LOAF, BREAD)
    elif kind == 1:
        for y in range(67, 85):
            beam(layer, 8, y, 30, y, WOOD, width=1)
            if y % 5 == 0:
                rectangle(layer, 8, y, 30, y, WOOD[0])
        beam(layer, 8, 66, 30, 66, IRON, width=1)
        for x in (12, 22, 29):
            rectangle(layer, x, 67, x, 72, IRON[3])
        meat = {"r": (97, 37, 37), "R": (172, 78, 65), "H": (218, 137, 111), "B": (233, 212, 174)}
        stamp(layer, 9, 72, ["..B..", ".rBr.", "rRRRr", "rHHRr", "rRHRr", ".RRr.", "..r.."], meat)
        stamp(layer, 19, 73, ["rRRRr", "rBHB.", "rBRBr", "rBHB.", "rBRBr", "rBHB.", ".rrr."], meat)
        stamp(layer, 28, 72, [".r.", "rHr", "rRr", ".r.", "rHr", "rRr", ".r.", "rHr", "rRr", ".r."], meat)
        # A thick chopping counter, marbled cuts and a steel knife.
        beam(layer, 7, 85, 31, 85, WOOD, width=3)
        rectangle(layer, 8, 85, 30, 85, (166, 119, 72))
        for x in (10, 27):
            beam(layer, x, 88, x, 93, WOOD, width=2)
        stamp(layer, 10, 82, [".rHRr.", "rRBBRr", ".rrrr."], meat)
        beam(layer, 22, 84, 26, 83, IRON, width=1)
        beam(layer, 27, 83, 29, 83, WOOD, width=1)
    else:
        for x, color in ((11, (192, 72, 35)), (20, (221, 167, 46)), (29, (102, 139, 61))):
            sack(layer, x, 77, color)
        beam(layer, 8, 73, 30, 73, WOOD, width=2)
        for x in (12, 22):
            rectangle(layer, x, 68, x + 4, 72, (150, 87, 51))
            rectangle(layer, x + 1, 66, x + 3, 67, (206, 158, 86))


def build_shop(kind):
    size = (120, 96)
    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    roof_pal = ROOFS[kind]
    chimney = Layer(*size)
    flat_wall(chimney, 84, 94, 7, 40, STONE, row_h=4, base=3, seed=80 + kind, bw=(4, 6))
    rectangle(chimney, 83, 4, 95, 6, STONE[3])
    rectangle(chimney, 86, 4, 92, 4, OUT)
    composite(canvas, chimney, OUT)

    wing = Layer(*size)
    stall(wing, kind)
    composite(canvas, wing, OUT)
    wing_roof = Layer(*size)
    mask = poly_mask(size, [(3, 59), (39, 45), (40, 55), (3, 68)])
    roof_tiles(wing_roof, mask, roof_pal, 88 + kind, tile_w=5, row_h=3, rounded=kind == 0)
    beam(wing_roof, 3, 68, 40, 55, WOOD, width=2)
    composite(canvas, wing_roof, OUT)
    if kind == 1:
        dormer = Layer(*size)
        plaster(dormer, poly_mask(size, [(9, 59), (17, 48), (25, 59)]), PLASTER, 89)
        roof_tiles(dormer, poly_mask(size, [(6, 58), (17, 44), (28, 58)]) -
                   poly_mask(size, [(11, 58), (17, 51), (23, 58)]), roof_pal, 90,
                   tile_w=4, row_h=3, rounded=False)
        beam(dormer, 9, 59, 25, 59, WOOD, width=2)
        lit_window(dormer, 15, 53, 5, 6, GLOW, WOOD, shutters=False)
        composite(canvas, dormer, OUT)

    # The gable has the same density of shingles and bracing as the blacksmith.
    outer = poly_mask(size, [(25, 63), (64, 14), (65, 14), (104, 63)])
    inner = poly_mask(size, [(37, 64), (64, 28), (65, 28), (92, 64)])
    roof = Layer(*size)
    roof_tiles(roof, outer - inner, roof_pal, 93 + kind, shade=lambda x, y: -1 if x > 64 else 0,
               tile_w=5, row_h=3, rounded=kind == 0)
    composite(canvas, roof, OUT)
    gable = Layer(*size)
    plaster(gable, inner, PLASTER, 97)
    beam(gable, 64, 11, 64, 39, WOOD)
    beam(gable, 37, 64, 64, 28, WOOD)
    beam(gable, 65, 28, 92, 64, WOOD)
    beam(gable, 44, 58, 85, 58, WOOD, width=3)
    for x in (49, 78):
        beam(gable, x, 48, x, 63, WOOD)
    beam(gable, 49, 51, 41, 61, WOOD)
    beam(gable, 78, 51, 87, 61, WOOD)
    shadow_under(gable, inner, PLASTER, WOOD, PLASTER[1])
    lit_window(gable, 59, 43, 11, 10, GLOW, WOOD)
    composite(canvas, gable, OUT)

    facade = Layer(*size)
    flat_wall(facade, 36, 94, 67, 91, STONE, row_h=5, base=3, seed=102,
              bw=(6, 10), shade=lambda x, y: -1 if y < 71 else 0)
    beam(facade, 34, 64, 96, 64, WOOD, width=3)
    beam(facade, 35, 67, 35, 91, WOOD, width=3)
    beam(facade, 93, 67, 93, 91, WOOD, width=3)
    if kind == 1:
        lit_window(facade, 78, 74, 12, 12, GLOW, WOOD, shutters=False)
        meat = {"r": (98, 39, 36), "R": (174, 83, 68), "B": (244, 207, 162)}
        stamp(facade, 79, 79, ["rRr.rRr.rRr", "RBR.RBR.RBR", ".r...r...r."], meat)
        beam(facade, 78, 84, 89, 84, WOOD, width=1)
    else:
        lit_window(facade, 78, 74, 9, 9, GLOW, WOOD, shutters=False)
    composite(canvas, facade, OUT)
    if kind == 1:
        awning = Layer(*size)
        for x0, x1, top in ((4, 33, 69), (76, 92, 70)):
            for y in range(top, top + 4):
                for x in range(x0, x1 + 1):
                    stripe = (x - x0) // 4 % 2
                    colors = ((215, 196, 151), (145, 57, 51)) if y < top + 2 else ((153, 129, 96), (90, 34, 33))
                    awning.put(x, y, colors[stripe])
            beam(awning, x0, top - 1, x1, top - 1, WOOD, width=1)
        composite(canvas, awning, OUT)

    entrance = Layer(*size)
    stone_arch(entrance, 60, 75, 91, 6, STONE)
    arched_door(entrance, 60, 75, 91, 6, WOOD, IRON)
    rectangle(entrance, 52, 92, 68, 93, STONE[4])
    rectangle(entrance, 50, 94, 70, 95, STONE[2])
    composite(canvas, entrance, OUT)

    props = Layer(*size)
    hanging_sign(props, kind)
    stamp(props, 95, 76, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3], "W": GLOW[4]})
    if kind == 0:
        sack(props, 104, 82)
        sack(props, 113, 83)
        stamp(props, 37, 83, BARREL, PROPS)
    elif kind == 1:
        stamp(props, 102, 83, BARREL, PROPS)
        stamp(props, 40, 85, CRATE, PROPS)
        stamp(props, 114, 86, ["..I..", ".III.", "iIIWi", "iIIIi", ".iii.", ".....", ".TTT.", "TTTTT", ".ttt."], PROPS)
        stamp(props, 39, 74, [".l.", "lLl", "LLl", ".Ll", "..l"],
              {"l": (33, 62, 30), "L": (83, 123, 52)})
    else:
        sack(props, 104, 82, (221, 167, 46))
        sack(props, 113, 83, (192, 72, 35))
        stamp(props, 38, 85, CRATE, PROPS)
    composite(canvas, props, OUT)
    if kind == 0:
        light_spill(canvas, ((19, 83, 12),), set(STONE) | set(WOOD), (255, 150, 55))
    elif kind == 1:
        light_spill(canvas, ((83, 80, 12), (97, 79, 7)), set(STONE) | set(WOOD), (255, 180, 76))
    return canvas


def build_monastery():
    size = (168, 120)
    canvas = Image.new("RGBA", size, (0, 0, 0, 0))
    slate = [(22, 26, 35), (33, 40, 52), (54, 65, 80), (75, 88, 103), (105, 119, 131), (143, 154, 159)]
    pale = [(38, 36, 34), (60, 58, 52), (100, 97, 84), (132, 128, 111), (165, 158, 134), (201, 188, 155)]
    gold = [(81, 53, 22), (130, 86, 32), (189, 134, 49), (234, 190, 89), (255, 225, 152)]
    glass = [(39, 74, 97), (67, 130, 159), (128, 192, 202), (133, 53, 57), (203, 106, 86)]
    leaf = [(27, 52, 30), (45, 79, 39), (70, 110, 49), (108, 138, 66)]

    def stained_window(layer, cx, top, bottom, half):
        stone_arch(layer, cx, top, bottom, half, pale, thick=2)
        for y in range(top, bottom + 1):
            for x in range(cx - half, cx + half + 1):
                dx = x - cx
                if y < top + half and dx * dx + (y - top - half) ** 2 > half * half:
                    continue
                color = glass[1 if dx < 0 else 3]
                if (abs(dx) + y - top) % 7 < 2:
                    color = glass[2 if dx < 0 else 4]
                if x == cx or (y - top + abs(dx)) % 7 == 0:
                    color = IRON[0]
                layer.put(x, y, color)
        rectangle(layer, cx - half - 2, bottom + 1, cx + half + 2, bottom + 2, pale[4])

    # A low, shaded cloister makes the silhouette asymmetric.
    cloister = Layer(*size)
    flat_wall(cloister, 122, 161, 81, 110, pale, row_h=5, base=2, seed=117, bw=(6, 9))
    for cx in (131, 143, 155):
        stone_arch(cloister, cx, 88, 106, 4, pale, thick=2)
        for y in range(88, 107):
            for x in range(cx - 4, cx + 5):
                if y < 92 and (x - cx) ** 2 + (y - 92) ** 2 > 16:
                    continue
                cloister.put(x, y, (31, 30, 27) if x > cx else (51, 47, 37))
        rectangle(cloister, cx - 3, 102, cx + 3, 103, WOOD[3])
        rectangle(cloister, cx - 3, 104, cx + 3, 106, WOOD[1])
    roof_tiles(cloister, poly_mask(size, [(120, 73), (145, 71), (165, 83), (120, 85)]),
               slate, 118, tile_w=5, row_h=3, rounded=False)
    beam(cloister, 121, 85, 164, 83, WOOD, width=2)
    rectangle(cloister, 121, 109, 162, 113, pale[2])
    composite(canvas, cloister, OUT)

    facade = Layer(*size)
    flat_wall(facade, 48, 128, 77, 111, pale, row_h=5, base=3, seed=119, bw=(6, 10),
              shade=lambda x, y: -1 if y < 83 or x > 118 else 0)
    rectangle(facade, 46, 109, 131, 114, pale[2])
    rectangle(facade, 47, 109, 129, 109, pale[4])
    composite(canvas, facade, OUT)

    # Deep roof slopes surround a stone gable with a carved rose window.
    outer = poly_mask(size, [(41, 81), (88, 22), (90, 22), (137, 81)])
    inner = poly_mask(size, [(55, 81), (89, 39), (123, 81)])
    roof = Layer(*size)
    roof_tiles(roof, outer - inner, slate, 121,
               shade=lambda x, y: -1 if x > 89 else 0, tile_w=5, row_h=3, rounded=False)
    beam(roof, 41, 81, 88, 22, WOOD, width=2)
    beam(roof, 90, 22, 137, 81, WOOD, width=2)
    composite(canvas, roof, OUT)

    gable = Layer(*size)
    flat_wall(gable, 55, 123, 39, 81, pale, row_h=5, base=3, seed=123, bw=(6, 9))
    for y in range(39, 82):
        for x in range(55, 124):
            if (x, y) not in inner:
                gable.erase(x, y)
    beam(gable, 55, 81, 89, 39, pale, width=2)
    beam(gable, 89, 39, 123, 81, pale, width=2)
    beam(gable, 54, 81, 124, 81, pale, width=3)
    for y in range(51, 76):
        for x in range(77, 102):
            dx, dy = x - 89, y - 63
            distance = dx * dx + dy * dy
            if distance > 144:
                continue
            if distance > 100:
                color = pale[5] if dx + dy < 0 else pale[2]
                if (abs(dx) + abs(dy)) % 6 == 0:
                    color = pale[1]
            elif distance > 81:
                color = gold[2] if dx < 0 else gold[0]
            else:
                color = glass[1 if abs(dx) > abs(dy) else 3]
                if (dx + dy) % 5 < 2:
                    color = glass[2 if abs(dx) > abs(dy) else 4]
                if dx == 0 or dy == 0 or abs(dx) == abs(dy):
                    color = IRON[0]
                if distance < 6:
                    color = gold[3]
            gable.put(x, y, color)
    stamp(gable, 87, 45, ["..G..", "GGGGG", "..G..", "..G.."], {"G": gold[3]})
    composite(canvas, gable, OUT)

    # A tall bell tower sits beside the chapel rather than on its roof.
    tower = Layer(*size)
    flat_wall(tower, 19, 48, 29, 110, pale, row_h=5, base=3, seed=127, bw=(5, 8),
              shade=lambda x, y: -1 if x > 40 else 0)
    for x in (19, 45):
        flat_wall(tower, x, x + 3, 30, 110, pale, row_h=5, base=4, seed=x, bw=(4, 4))
    roof_tiles(tower, poly_mask(size, [(12, 30), (33, 8), (35, 8), (55, 30)]),
               slate, 129, tile_w=4, row_h=3, rounded=False)
    beam(tower, 12, 31, 55, 31, pale, width=3)
    beam(tower, 34, 2, 34, 12, IRON, width=1)
    beam(tower, 31, 5, 37, 5, gold, width=1)
    stone_arch(tower, 34, 40, 62, 7, pale, thick=2)
    for y in range(40, 63):
        for x in range(27, 42):
            if y < 47 and (x - 34) ** 2 + (y - 47) ** 2 > 49:
                continue
            tower.put(x, y, OUT)
    beam(tower, 28, 44, 40, 44, WOOD, width=2)
    rectangle(tower, 34, 44, 34, 50, WOOD[3])
    for row in range(12):
        half = 2 + max(0, row - 3) // 3
        for x in range(34 - half, 35 + half):
            color = gold[3] if x < 34 else gold[1]
            if x == 34 - half:
                color = gold[4]
            tower.put(x, 48 + row, color)
    rectangle(tower, 28, 58, 40, 59, gold[2])
    rectangle(tower, 28, 58, 39, 58, gold[4])
    rectangle(tower, 33, 61, 35, 62, gold[3])
    for y in (65, 74):
        beam(tower, 17, y, 50, y, pale, width=3)
    stained_window(tower, 34, 81, 100, 4)
    rectangle(tower, 16, 109, 51, 114, pale[2])
    rectangle(tower, 17, 108, 50, 109, pale[4])
    composite(canvas, tower, OUT)

    detail = Layer(*size)
    for x in (51, 124):
        flat_wall(detail, x, x + 4, 81, 110, pale, row_h=5, base=4, seed=x, bw=(5, 5))
        roof_tiles(detail, poly_mask(size, [(x - 2, 84), (x + 2, 77), (x + 7, 84)]),
                   slate, x, tile_w=4, row_h=3, rounded=False)
        rectangle(detail, x - 2, 106, x + 6, 112, pale[2])
        rectangle(detail, x - 2, 106, x + 6, 106, pale[4])
    for cx in (63, 113):
        stained_window(detail, cx, 87, 103, 4)
    # Narrow blue altar banners and a layered stone portal frame the oak door.
    for x in (72, 100):
        beam(detail, x - 1, 87, x + 5, 87, IRON, width=1)
        rectangle(detail, x, 88, x + 4, 103, slate[1])
        rectangle(detail, x, 88, x, 102, gold[2])
        stamp(detail, x + 1, 91, [".g.", "ggg", ".g.", ".g."], {"g": gold[3]})
        for col in range(5):
            detail.put(x + col, 104 + min(col, 4 - col), slate[1])
    stone_arch(detail, 84, 92, 110, 6, pale, thick=4)
    arched_door(detail, 84, 92, 110, 6, WOOD, IRON)
    rectangle(detail, 83, 88, 85, 91, pale[5])
    rectangle(detail, 75, 111, 93, 113, pale[4])
    rectangle(detail, 72, 114, 96, 117, pale[2])
    for x in (68, 96):
        stamp(detail, x, 101, LANTERN, {"i": IRON[1], "I": IRON[2], "y": GLOW[2], "Y": GLOW[3], "W": GLOW[4]})
    # Ivy grows on the outer masonry; the entrance stays easy to read.
    for x, top, length in ((19, 77, 31), (48, 70, 18), (127, 82, 26), (159, 90, 22)):
        for k in range(length):
            vine_x = x + (k // 6) % 2
            detail.put(vine_x, top + k, leaf[1])
            if k % 3 == 0:
                stamp(detail, vine_x - 2, top + k, [".Ll.", "LLlL", ".lL."],
                      {"l": leaf[0], "L": leaf[2] if k % 2 else leaf[3]})
    for x in (8, 151):
        stamp(detail, x, 108, ["...L..L..", ".LLlLLlL.", "LllLLLlLL", ".cCCCCCc.", ".cCCcCCc.", "..ccccc.."],
              {"L": leaf[2], "l": leaf[0], "c": pale[1], "C": pale[3]})
    stamp(detail, 135, 107, ["WWWWWWWW", "TTTTTTTT", ".t....t.", ".t....t."], PROPS)
    composite(canvas, detail, OUT)
    light_spill(canvas, ((70, 106, 10), (98, 106, 10)), set(pale) | set(WOOD), (255, 179, 85))
    return canvas


def main():
    for kind, name in enumerate(("bakery", "butcher", "spice-merchant")):
        save_sprite(build_shop(kind), ASSETS / f"{name}.bmp")
    save_sprite(build_monastery(), ASSETS / "monastery.bmp")


if __name__ == "__main__":
    main()
