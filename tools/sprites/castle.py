#!/usr/bin/env python3
"""Castle of No Return sprite with its moat and drawbridge (Town 3).

Draws 17 x 10 tiles (408 x 240): the 15 x 8 castle plus the one-tile moat ring
from TOWN_MOAT_* in src/game/map.h. The drawbridge covers tile (20, 11). Run
`make sprites` to rewrite assets/images/castle-of-no-return.bmp.
"""
import math
import random
from pathlib import Path

from PIL import Image

from pixelkit import (Layer, clamp, composite, cyl_wall, darken_in, flat_wall, gothic_half,
                      light_spill, merlons, save_sprite, spire, stamp, wobble)

ASSETS = Path(__file__).resolve().parents[2] / "assets" / "images"
W, H = 408, 240
CX = 204

OUT = (12, 9, 15)
ST = [(24, 21, 29), (36, 32, 43), (50, 45, 58), (65, 59, 73), (82, 75, 89), (101, 93, 107)]
RF = [(22, 8, 14), (38, 11, 20), (58, 17, 27), (82, 25, 35), (110, 37, 45)]
GL = [(80, 12, 18), (138, 22, 26), (196, 42, 32), (240, 88, 44), (255, 164, 92)]
BONE = [(80, 72, 62), (134, 124, 104), (182, 172, 148), (222, 214, 192)]
IRON = [(22, 22, 28), (40, 40, 48), (66, 66, 76), (100, 100, 110)]
WOOD = [(33, 22, 18), (54, 36, 26), (77, 51, 34), (101, 69, 44)]
CLOTH = [(19, 10, 23), (32, 16, 38), (49, 25, 55), (66, 37, 72)]
BLOOD = [(84, 13, 21), (132, 22, 29), (176, 38, 38)]
WATER = [(6, 17, 21), (11, 27, 31), (16, 37, 41), (26, 55, 57), (42, 80, 78), (70, 112, 100)]
CURB = [(43, 39, 36), (58, 57, 54), (76, 79, 75), (91, 91, 82), (105, 101, 88), (126, 121, 103)]
SLIME = [(20, 32, 25), (30, 45, 32), (42, 60, 40)]
SOIL = [(26, 21, 15), (38, 31, 21), (51, 43, 28), (66, 55, 36)]
REED = [(44, 38, 26), (70, 60, 40), (96, 84, 56)]


def tint_slime(L, x, y):
    if not L.on(x, y):
        return
    c = L.get(x, y)
    if c in ST:
        i = ST.index(c)
        L.put(x, y, SLIME[min(2, max(0, i - 2))])


def weather(L, x0, x1, y0, y1, seed, streaks=6, crack_n=5, slime_from=None):
    """Grime streaks, cracks, and a slime line rising from slime_from."""
    rnd = random.Random(seed)
    for _ in range(streaks):
        x = rnd.randint(x0, x1)
        y = rnd.randint(y0, y1 - 10)
        for k in range(rnd.randint(6, 16)):
            darken_in(L, x, y + k, ST)
    for _ in range(crack_n):
        x = rnd.randint(x0, x1)
        y = rnd.randint(y0, y1)
        for _ in range(rnd.randint(3, 7)):
            if L.on(x, y) and L.get(x, y) in ST:
                L.put(x, y, ST[0])
            x += rnd.choice((-1, 0, 1))
            y += 1
    if slime_from is not None:
        reach = slime_from
        for x in range(x0, x1 + 1):
            if rnd.random() < 0.4:
                reach = clamp(reach + rnd.choice((-2, -1, 1, 2)), slime_from - 5, slime_from + 3)
            top = reach - (rnd.randint(4, 9) if rnd.random() < 0.08 else 0)
            for y in range(top, y1 + 1):
                if y >= reach or (y - top) % 2 == 0:
                    tint_slime(L, x, y)


# ---------------------------------------------------------------- parts
def finial(L, x, tip, orb=True):
    for y in range(tip - 9, tip):
        L.put(x, y, IRON[3] if y < tip - 5 else IRON[2])
    L.put(x - 1, tip - 7, IRON[2])
    L.put(x + 1, tip - 7, IRON[1])
    L.put(x - 1, tip - 8, IRON[3])
    L.put(x + 1, tip - 8, IRON[2])
    if orb:
        L.put(x - 1, tip - 4, GL[1])
        L.put(x, tip - 4, GL[3])
        L.put(x + 1, tip - 4, GL[1])
        L.put(x, tip - 5, GL[2])
        L.put(x, tip - 3, GL[1])


def window(L, cx, y0, w, h, bars=True):
    """Pointed window glowing red, with iron bars and a stone sill."""
    half = w // 2
    for y in range(y0 - 1, y0 + h + 1):
        k = y - y0
        for x in range(cx - half - 1, cx + half + 2):
            dx = abs(x - cx)
            if k < 0:
                inside = False
                edge = dx == 0
            else:
                lim = min(half, k)
                inside = dx <= lim and k < h
                edge = (dx <= lim + 1 and k <= h) and not inside
            if edge:
                L.put(x, y, OUT)
            elif inside:
                if k == 0 or dx == half and k < 3:
                    c = GL[1]
                elif dx == half:
                    c = GL[2]
                elif k > h * 0.55 and dx < max(1, half):
                    c = GL[4] if dx == 0 and k > h * 0.7 else GL[3]
                else:
                    c = GL[3] if dx == 0 else GL[2]
                L.put(x, y, c)
    if bars and w >= 5:
        yb = y0 + h // 2
        for x in range(cx - half, cx + half + 1):
            L.put(x, yb, IRON[1])
        for y in range(y0 + 2, y0 + h):
            L.put(cx, y, IRON[1] if (y - y0) % 2 else IRON[0])
    for x in range(cx - half - 1, cx + half + 2):
        L.put(x, y0 + h + 1, ST[4])
    for x in range(cx - half, cx + half + 2):
        L.put(x, y0 + h + 2, ST[1])


def slit(L, cx, y0, h):
    for y in range(y0 - 1, y0 + h + 1):
        for x in range(cx - 1, cx + 2):
            L.put(x, y, OUT)
    for y in range(y0, y0 + h):
        L.put(cx, y, GL[1] if y < y0 + h // 2 else GL[2])


def banner(L, x0, y0, w, h, seed=0):
    """Tattered black banner hanging from an iron rod."""
    rnd = random.Random(seed)
    for x in range(x0 - 2, x0 + w + 2):
        L.put(x, y0 - 2, IRON[3] if x < x0 + w // 2 else IRON[2])
        L.put(x, y0 - 1, IRON[1])
    L.put(x0 - 3, y0 - 2, IRON[2])
    L.put(x0 + w + 2, y0 - 2, IRON[1])
    for i in range(w):
        center = abs(i - (w - 1) / 2.0) / ((w - 1) / 2.0)
        length = h - int((1 - center) * 5) - rnd.choice((0, 0, 1, 3, 6))
        x = x0 + i
        for y in range(y0, y0 + length):
            if i == 0 or i == w - 1:
                c = BLOOD[0]
            elif i in (1, w - 3):
                c = CLOTH[3]
            elif i in (2, w - 2):
                c = CLOTH[1]
            else:
                c = CLOTH[2]
            if y == y0:
                c = CLOTH[3]
            L.put(x, y, c)
        if rnd.random() < 0.3:
            L.erase(x, y0 + length - 3)


SIGIL = [
    "r.....r",
    "rr...rr",
    ".rRRRr.",
    ".RkRkR.",
    ".RRRRR.",
    "..RkR..",
    "..R.R..",
]


def sigil(L, cx, y0):
    stamp(L, cx - 3, y0, SIGIL, {"r": BLOOD[1], "R": BLOOD[2], "k": CLOTH[0]})


def shield_crest(L, cx, y0):
    """Black and crimson heater shield with a downward sword, above the gate."""
    h = 25
    rows = []
    for r in range(h):
        if r <= 11:
            rows.append(10)
        else:
            s = (r - 11) / 13.0
            rows.append(int(round(10 * (1 - s ** 1.8))))
    mask = set()
    for r, half in enumerate(rows):
        for x in range(cx - half, cx + half + 1):
            mask.add((x, y0 + r))

    def ring(x, y, d):
        return any((x + dx, y + dy) not in mask
                   for dx in range(-d, d + 1) for dy in range(-d, d + 1)
                   if abs(dx) + abs(dy) <= d)

    for (x, y) in mask:
        dx = x - cx
        r = y - y0
        if ring(x, y, 1):
            c = IRON[3] if dx < 0 or r == 0 else IRON[2]
        elif ring(x, y, 2):
            c = IRON[0]
        elif dx < 0:
            c = CLOTH[2] if dx == -8 or r == 2 else CLOTH[1]
        else:
            c = BLOOD[1] if dx < 6 and r < 18 else BLOOD[0]
            if dx == 1 and r > 2:
                c = BLOOD[2]
        L.put(x, y, c)
    for rx in (cx - 8, cx - 3, cx + 3, cx + 8):
        L.put(rx, y0 + 1, IRON[1])
    # sword, point down
    L.put(cx, y0 + 3, GL[2])
    L.put(cx - 1, y0 + 4, GL[1])
    L.put(cx, y0 + 4, GL[3])
    L.put(cx + 1, y0 + 4, GL[1])
    L.put(cx, y0 + 5, GL[2])
    for y in (y0 + 6, y0 + 7):
        L.put(cx, y, WOOD[1])
    for x in range(cx - 5, cx + 6):
        L.put(x, y0 + 8, IRON[3] if x <= cx else IRON[2])
    for x in (cx - 5, cx + 5):
        L.put(x, y0 + 9, IRON[2])
        L.put(x, y0 + 10, IRON[1])
    for y in range(y0 + 9, y0 + 19):
        L.put(cx - 1, y, IRON[3])
        L.put(cx, y, BONE[2])
        L.put(cx + 1, y, IRON[2])
    L.put(cx - 1, y0 + 19, IRON[3])
    L.put(cx, y0 + 19, BONE[1])
    L.put(cx + 1, y0 + 19, IRON[1])
    L.put(cx, y0 + 20, IRON[2])
    L.put(cx, y0 + 21, IRON[1])


FLAME = [
    "...y...",
    "..yY...",
    "..yYy..",
    ".oyYyo.",
    ".oYYYo.",
    "oryYyro",
    ".rooor.",
]

BRAZIER = [
    "iIIIIIi",
    ".iIIIi.",
    "..iIi..",
    "...i...",
    "..iii..",
]


def brazier(L, cx, y0):
    stamp(L, cx - 3, y0, FLAME, {"y": GL[4], "Y": (255, 236, 170), "o": GL[3], "r": GL[2]})
    stamp(L, cx - 3, y0 + 7, BRAZIER, {"i": IRON[1], "I": IRON[3]})


PIKE_SKULL = [
    ".bBb.",
    "bBBBs",
    "BkBks",
    ".BtB.",
    ".sbs.",
]


# ---------------------------------------------------------------- build
def moat_and_island():
    """Murky moat ring with grass banks, dead reeds and a stone quay at the plaza."""
    L = Layer(W, H)
    rnd = random.Random(7)
    top_out = wobble(W, 1, 3, 5)
    left_out = wobble(H, 3, 1, 3)
    right_out = wobble(H, 5, 1, 3)
    top_in = wobble(W, 2, 22, 24)
    left_in = wobble(H, 4, 23, 25)
    right_in = wobble(H, 6, 23, 25)
    water = {}
    for y in range(H):
        for x in range(W):
            if y >= 236:
                continue
            if y < top_out[x] or x < left_out[y] or x > W - 1 - right_out[y]:
                continue
            # rounded outer corners
            if x < 8 and y < 8 and (8 - x) ** 2 + (8 - y) ** 2 > 64:
                continue
            if x > W - 9 and y < 8 and (x - (W - 9)) ** 2 + (8 - y) ** 2 > 64:
                continue
            inside = (left_in[y] <= x <= W - 1 - right_in[y] and top_in[x] <= y < 222)
            if inside:
                # rounded island corners (back only)
                cx = 29 if x < 29 else (W - 30 if x > W - 30 else None)
                if cx is not None and y < 29 and (x - cx) ** 2 + (y - 29) ** 2 > 36:
                    inside = False
            if inside:
                continue
            water[(x, y)] = True
    for (x, y) in water:
        c = WATER[2] if (x * 3 + y * 5) % 17 else WATER[1]
        L.put(x, y, c)
    # shading near banks
    for (x, y) in water:
        up = (x, y - 1) in water
        if not up:
            L.put(x, y, WATER[0])
            if (x, y + 1) in water:
                L.put(x, y + 1, WATER[1])
        elif not (x - 1, y) in water or not (x + 1, y) in water:
            L.put(x, y, WATER[1])
    # ripples + scum
    keys = list(water.keys())
    for _ in range(95):
        x, y = rnd.choice(keys)
        ln = rnd.randint(3, 7)
        if all((x + i, y) in water and (x + i, y - 1) in water and (x + i, y + 1) in water
               for i in range(ln)):
            for i in range(ln):
                L.put(x + i, y, WATER[3])
            if ln > 4:
                for i in range(1, ln - 1):
                    L.put(x + i, y - 1, WATER[4] if i == 2 else WATER[3])
    for _ in range(14):
        x, y = rnd.choice(keys)
        if all((x + dx, y + dy) in water for dx in range(-2, 4) for dy in range(-1, 3)):
            for dx, dy in ((0, 0), (1, 0), (2, 0), (-1, 1), (0, 1), (1, 1), (2, 1), (3, 1), (1, 2)):
                L.put(x + dx, y + dy, SLIME[0] if dy else SLIME[1])
    # far (north) bank face visible above the back moat
    for x in range(W):
        y0 = top_out[x]
        for k in range(1, 4):
            y = y0 - k
            if y < 0:
                continue
            if x < 8 and y < 8 and (8 - x) ** 2 + (8 - y) ** 2 > 81:
                continue
            if x > W - 9 and y < 8 and (x - (W - 9)) ** 2 + (8 - y) ** 2 > 81:
                continue
            L.put(x, y, SOIL[1] if k == 1 else (SOIL[2] if k == 2 else SOIL[3]))
        if rnd.random() < 0.12 and y0 >= 3:
            L.put(x, y0 - 2, CURB[3])
            L.put(x + 1, y0 - 2, CURB[2])
    # side banks and island rim: thin earth line on dry pixels touching water
    for y in range(H):
        for x in range(W):
            if (x, y) in water or L.on(x, y):
                continue
            touch = any((x + dx, y + dy) in water for dx, dy in ((1, 0), (-1, 0), (0, -1)))
            if touch and y < 236:
                L.put(x, y, SOIL[1] if rnd.random() < 0.7 else SOIL[2])
    # island rim below the back moat: grass lip with earth
    for x in range(W):
        y = top_in[x]
        if (x, y - 1) in water and not (x, y) in water and y < 60:
            L.put(x, y, SOIL[2])
            L.put(x, y + 1, SOIL[0])
    # front quay along the plaza
    for x in range(W):
        for y in range(236, 240):
            k = (x + (4 if y >= 238 else 0)) % 10
            c = CURB[0] if k == 0 else (CURB[5] if y == 236 else CURB[3])
            if y == 237 and k:
                c = CURB[4]
            L.put(x, y, c)
    for y in range(226, 236):
        for x in (0, 1, W - 2, W - 1):
            L.put(x, y, CURB[3] if (y + x) % 5 else CURB[0])
        L.put(2, y, CURB[1])
        L.put(W - 3, y, CURB[1])
    # dead reeds along the outer banks
    for (rx, ry) in ((10, 40), (12, 44), (395, 70), (397, 74), (60, 5), (340, 6), (8, 170),
                     (399, 150), (150, 4), (262, 5)):
        for i, h in enumerate((4, 6, 3)):
            x = rx + i * 2
            y = ry
            for k in range(h):
                if 0 <= y - k < H:
                    L.put(x, y - k, REED[1] if k < h - 1 else REED[2])
            L.put(x, y + 1, REED[0])
    return L


def build():
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    composite(canvas, moat_and_island(), None)

    # --- keep (back, centre) -----------------------------------------
    keep_spire = Layer(W, H)
    spire(keep_spire, CX + 0.5, 7, 42, 27, RF, seed=5, cols=13, p=2.1)
    for y in range(0, 7):
        keep_spire.put(CX, y, IRON[3] if y < 4 else IRON[2])
    keep_spire.put(CX - 1, 4, GL[1])
    keep_spire.put(CX + 1, 4, GL[1])
    keep_spire.put(CX, 4, GL[3])
    keep_spire.put(CX, 3, GL[2])
    keep_spire.put(CX - 1, 1, IRON[2])
    keep_spire.put(CX + 1, 1, IRON[1])
    composite(canvas, keep_spire, OUT)
    keep = Layer(W, H)
    flat_wall(keep, 174, 234, 46, 118, ST, base=2, seed=11,
              shade=lambda x, y: -1 if x > 226 or y < 52 else (1 if x < 180 else 0))
    merlons(keep, 172, 236, 36, 10, ST, mw=7, gap=4, base=3, start=0, broken=(4,))
    for x in range(172, 237):
        keep.put(x, 46, ST[4])
        keep.put(x, 47, ST[1])
    for x in range(175, 234, 5):
        keep.put(x, 48, OUT)
        keep.put(x + 1, 48, OUT)
    window(keep, CX, 58, 9, 24)
    window(keep, 186, 64, 5, 12)
    window(keep, 222, 64, 5, 12)
    weather(keep, 176, 232, 52, 110, 3, streaks=5, crack_n=4)
    composite(canvas, keep, OUT)

    # keep corner turrets
    for tx in (166, 242):
        tsp = Layer(W, H)
        spire(tsp, tx + 0.5, 24, 38, 8, RF, seed=tx, cols=5)
        finial(tsp, tx, 24, orb=False)
        composite(canvas, tsp, OUT)
        tur = Layer(W, H)
        cyl_wall(tur, tx - 7, tx + 7, 38, 80, ST, row_h=6, cols=4, seed=tx, bias=-1)
        for x in range(tx - 8, tx + 9):
            tur.put(x, 38, ST[4])
            tur.put(x, 39, ST[1])
        slit(tur, tx, 50, 6)
        composite(canvas, tur, OUT)

    # --- inner towers behind curtain walls ---------------------------
    for tx in (122, 286):
        tsp = Layer(W, H)
        spire(tsp, tx + 0.5, 34, 60, 17, RF, seed=tx + 1, cols=7)
        finial(tsp, tx, 34, orb=False)
        composite(canvas, tsp, OUT)
        tw = Layer(W, H)
        cyl_wall(tw, tx - 13, tx + 13, 64, 124, ST, row_h=7, cols=6, seed=tx, bias=-1)
        merlons(tw, tx - 15, tx + 15, 56, 9, ST, mw=5, gap=3, base=2, start=1)
        for x in range(tx - 15, tx + 16):
            tw.put(x, 64, ST[3])
            tw.put(x, 65, ST[0])
        window(tw, tx, 78, 5, 13)
        slit(tw, tx, 101, 7)
        composite(canvas, tw, OUT)

    # --- foundation behind the front walls -----------------------------
    fd = Layer(W, H)
    flat_wall(fd, 84, 324, 207, 222, ST, base=2, row_h=8, seed=91, bw=(12, 20),
              shade=lambda x, y: -1 if y > 218 else 0)
    for x in range(84, 325):
        fd.put(x, 206, ST[4])
        fd.put(x, 207, ST[2])
    weather(fd, 84, 324, 207, 222, 17, streaks=0, crack_n=6, slime_from=216)
    composite(canvas, fd, OUT)

    # --- curtain walls -----------------------------------------------
    for (x0, x1, s, sh) in ((84, 162, 21, lambda x, y: -1 if x < 93 or y < 131 else 0),
                            (246, 324, 22, lambda x, y: -1 if x < 257 or y < 131 else 0)):
        cw = Layer(W, H)
        flat_wall(cw, x0, x1, 124, 205, ST, base=3, seed=s, shade=sh)
        merlons(cw, x0, x1, 114, 10, ST, mw=7, gap=4, base=3, start=2,
                broken=((2,) if s == 21 else (4,)))
        for x in range(x0, x1 + 1):
            cw.put(x, 124, ST[5])
            cw.put(x, 125, ST[2])
            if (x - x0) % 6 < 3:
                cw.put(x, 126, ST[1])
                cw.put(x, 127, OUT)
        for wx in (x0 + 20, x1 - 20):
            window(cw, wx, 146, 7, 16)
        slit(cw, (x0 + x1) // 2, 150, 8)
        slit(cw, (x0 + x1) // 2, 182, 7)
        weather(cw, x0 + 2, x1 - 2, 130, 205, s, streaks=8, crack_n=6)
        composite(canvas, cw, OUT)

    # --- side towers -------------------------------------------------
    for tx, s in ((57, 31), (351, 32)):
        tsp = Layer(W, H)
        spire(tsp, tx + 0.5, 28, 70, 26, RF, seed=s, cols=10, p=1.7)
        finial(tsp, tx, 28)
        composite(canvas, tsp, OUT)
        tw = Layer(W, H)
        cyl_wall(tw, tx - 27, tx + 27, 88, 204, ST, row_h=7, cols=9, seed=s)
        # battered base down into the moat
        cyl_wall(tw, tx - 30, tx + 30, 205, 222, ST, row_h=8, cols=9, seed=s + 9, bias=-1)
        for x in range(tx - 30, tx + 31):
            tw.put(x, 205, ST[4] if x < tx + 12 else ST[3])
        # parapet ring
        cyl_wall(tw, tx - 31, tx + 31, 72, 84, ST, row_h=6, cols=10, seed=s + 5)
        for x in range(tx - 31, tx + 32):
            tw.put(x, 72, ST[5] if x < tx + 16 else ST[4])
            tw.put(x, 84, ST[1])
        for x in range(tx - 29, tx + 30):
            k = (x - tx + 29) % 6
            for y in range(85, 89):
                if k in (0, 1):
                    tw.put(x, y, ST[3] if x < tx else ST[2])
                else:
                    tw.put(x, y, OUT if y > 85 else ST[0])
        merlons(tw, tx - 31, tx + 31, 63, 10, ST, mw=6, gap=4, base=3, start=1,
                broken=((3,) if s == 31 else (2,)))
        banner(tw, tx - 5, 98, 11, 46, seed=s)
        sigil(tw, tx, 110)
        window(tw, tx - 17, 104, 5, 13)
        window(tw, tx + 17, 104, 5, 13)
        slit(tw, tx - 16, 160, 8)
        slit(tw, tx + 16, 160, 8)
        window(tw, tx, 166, 7, 16)
        weather(tw, tx - 26, tx + 26, 92, 222, s, streaks=9, crack_n=6, slime_from=212)
        composite(canvas, tw, OUT)

    # --- gatehouse ---------------------------------------------------
    gh = Layer(W, H)
    flat_wall(gh, 172, 236, 108, 222, ST, base=3, seed=41, bw=(7, 12),
              shade=lambda x, y: -1 if y < 116 or x < 180 else 0)
    merlons(gh, 172, 236, 98, 10, ST, mw=7, gap=4, base=4, start=1)
    for x in range(172, 237):
        gh.put(x, 108, ST[5])
        gh.put(x, 109, ST[2])
    ya, ys, hw = 158, 176, 14
    for y in range(ya - 5, 214):
        half = gothic_half(y, ya, ys, hw)
        outer = gothic_half(y, ya - 5, ys, hw + 4)
        for x in range(CX - hw - 5, CX + hw + 6):
            dx = abs(x + 0.5 - (CX + 0.5))
            if half >= 0 and dx <= half + 0.01:
                gh.erase(x, y)
            elif outer >= 0 and dx <= outer + 0.01:
                seg = int(math.atan2(y - ys, x - CX) * 7) % 2
                c = ST[5] if x < CX else ST[4]
                if seg and y < ys:
                    c = ST[3]
                if dx > outer - 1.2 or (half >= 0 and dx <= half + 1.2):
                    c = ST[1]
                gh.put(x, y, c)
    weather(gh, 174, 234, 112, 222, 44, streaks=5, crack_n=3, slime_from=214)
    composite(canvas, gh, OUT)
    sh = Layer(W, H)
    shield_crest(sh, CX, 118)
    composite(canvas, sh, OUT)

    # gate passage (dark, faint fire deep inside) + portcullis
    gate = Layer(W, H)
    for y in range(ya, 214):
        half = gothic_half(y, ya, ys, hw)
        if half < 0:
            continue
        for x in range(CX - hw, CX + hw + 1):
            dx = abs(x + 0.5 - (CX + 0.5))
            if dx > half + 0.01:
                continue
            c = OUT
            inner = gothic_half(y, 199, 204, 5)
            if inner >= 0 and dx <= inner + 0.01:
                c = (62, 15, 19) if dx <= inner - 1.5 else (38, 10, 14)
                if y > 207 and dx < 2.5:
                    c = (104, 24, 22)
            elif y > 209 and (x + y) % 2 == 0:
                c = (26, 8, 12)
            gate.put(x, y, c)
    for y in range(ya, 197):
        half = gothic_half(y, ya, ys, hw)
        for x in range(CX - hw, CX + hw + 1):
            dx = abs(x + 0.5 - (CX + 0.5))
            if half < 0 or dx > half + 0.01:
                continue
            if (x - CX) % 4 == 0:
                gate.put(x, y, IRON[2] if x < CX else IRON[1])
            elif (y - ya) % 7 == 3:
                gate.put(x, y, IRON[1])
    for x in range(CX - hw, CX + hw + 1):
        if (x - CX) % 4 == 0:
            gate.put(x, 197, IRON[2])
            gate.put(x, 198, IRON[3])
            gate.put(x, 199, IRON[1])
    composite(canvas, gate, None)

    # gate turrets flanking gatehouse
    for tx, s in ((166, 51), (242, 52)):
        tsp = Layer(W, H)
        spire(tsp, tx + 0.5, 64, 96, 10, RF, seed=s, cols=6)
        finial(tsp, tx, 64, orb=False)
        composite(canvas, tsp, OUT)
        tw = Layer(W, H)
        cyl_wall(tw, tx - 9, tx + 9, 100, 222, ST, row_h=7, cols=5, seed=s)
        merlons(tw, tx - 10, tx + 10, 92, 9, ST, mw=5, gap=3, base=3, start=1)
        for x in range(tx - 10, tx + 11):
            tw.put(x, 100, ST[5] if x < tx + 5 else ST[4])
            tw.put(x, 101, ST[1])
        slit(tw, tx, 116, 8)
        banner(tw, tx - 4, 134, 9, 36, seed=s)
        sigil(tw, tx, 145)
        weather(tw, tx - 8, tx + 8, 104, 222, s, streaks=3, crack_n=2, slime_from=213)
        composite(canvas, tw, OUT)

    br = Layer(W, H)
    brazier(br, 180, 170)
    brazier(br, 228, 170)
    composite(canvas, br, OUT)

    # waterline sheen
    wl = Layer(W, H)
    for x in range(26, 382):
        if 186 <= x <= 222:
            continue
        if x % 5 != 0:
            wl.put(x, 224, WATER[4] if x % 3 else WATER[3])
    composite(canvas, wl, None)

    light_spill(canvas, ((180, 173, 9), (228, 173, 9)), set(ST) | set(SLIME), (214, 78, 34))

    # --- drawbridge --------------------------------------------------
    db = Layer(W, H)
    bx0, bx1 = 191, 217
    for y in range(213, 240):
        for x in range(bx0, bx1 + 1):
            k = (y - 213) % 4
            if x in (bx0, bx1):
                c = WOOD[1]
            elif k == 3:
                c = WOOD[0]
            elif k == 0:
                c = WOOD[3] if x < bx1 - 3 else WOOD[2]
            else:
                c = WOOD[2] if (x + y // 4) % 9 else WOOD[1]
            if x in (bx0 + 5, bx1 - 5):
                c = IRON[2] if k != 3 else IRON[1]
            if y in (213, 214):
                c = IRON[2] if y == 213 else IRON[1]
            db.put(x, y, c)
    for y in range(217, 240, 6):
        db.put(bx0, y, IRON[3])
        db.put(bx1, y, IRON[3])
    composite(canvas, db, OUT)
    cp = canvas.load()
    for y in range(223, 235):
        for x in (bx0 - 2, bx1 + 2):
            cp[x, y] = WATER[0] + (255,)

    # chains from the gatehouse to the bridge lip
    ch = Layer(W, H)
    for (ax, ay, bx, by) in ((186, 150, bx0 + 2, 233), (222, 150, bx1 - 2, 233)):
        n = by - ay
        for i in range(n + 1):
            x = round(ax + (bx - ax) * i / n)
            y = ay + i
            ch.put(x, y, IRON[3] if i % 3 else IRON[1])
        ch.put(bx - 1, by + 1, IRON[3])
        ch.put(bx + 1, by + 1, IRON[2])
        ch.put(bx, by + 2, IRON[2])
    for ax in (186, 222):
        for dx in (-1, 0, 1):
            ch.put(ax + dx, 149, OUT)
    composite(canvas, ch, None)

    # skulls on pikes flanking the bridge
    pk = Layer(W, H)
    for px in (185, 223):
        for y in range(221, 240):
            pk.put(px, y, WOOD[1] if y % 3 else WOOD[0])
        stamp(pk, px - 2, 216, PIKE_SKULL,
              {"b": BONE[2], "B": BONE[3], "s": BONE[1], "k": OUT, "t": BONE[1]})
    composite(canvas, pk, OUT)
    return canvas


def main():
    save_sprite(build(), ASSETS / "castle-of-no-return.bmp")


if __name__ == "__main__":
    main()
