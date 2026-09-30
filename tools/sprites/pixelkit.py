"""Shared drawing helpers for the procedural building sprites.

Building scripts in this folder draw at native game resolution (24 px per
tile) and call save_sprite() to write the runtime BMP. See README.md.
"""
import math
import random
import struct

from PIL import Image, ImageDraw


def clamp(v, lo, hi):
    return lo if v < lo else hi if v > hi else v


def poly_mask(size, points):
    """Pixels inside a polygon, as a set of (x, y)."""
    m = Image.new("L", size, 0)
    ImageDraw.Draw(m).polygon(points, fill=255)
    px = m.load()
    w, h = size
    return {(x, y) for y in range(h) for x in range(w) if px[x, y]}


def line_points(x0, y0, x1, y1):
    """Bresenham line from (x0, y0) to (x1, y1), both ends included."""
    pts = []
    dx = abs(x1 - x0)
    dy = -abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    err = dx + dy
    while True:
        pts.append((x0, y0))
        if x0 == x1 and y0 == y1:
            return pts
        e2 = 2 * err
        if e2 >= dy:
            err += dy
            x0 += sx
        if e2 <= dx:
            err += dx
            y0 += sy


class Layer:
    """A transparent RGBA surface that one sprite part is drawn onto."""

    def __init__(self, w, h):
        self.w = w
        self.h = h
        self.im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        self.p = self.im.load()

    def put(self, x, y, c):
        x = int(x)
        y = int(y)
        if 0 <= x < self.w and 0 <= y < self.h:
            self.p[x, y] = (c[0], c[1], c[2], 255)

    def on(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h and self.p[x, y][3] > 0

    def get(self, x, y):
        return self.p[x, y][:3]

    def erase(self, x, y):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.p[x, y] = (0, 0, 0, 0)


def composite(canvas, layer, outline):
    """Paste a layer onto the canvas, ringing it with a 1 px outline first."""
    cp = canvas.load()
    lp = layer.p
    w, h = canvas.size
    if outline is not None:
        for y in range(h):
            for x in range(w):
                if lp[x, y][3]:
                    continue
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    xx = x + dx
                    yy = y + dy
                    if 0 <= xx < w and 0 <= yy < h and lp[xx, yy][3]:
                        cp[x, y] = outline + (255,)
                        break
    canvas.alpha_composite(layer.im)


def stamp(L, x0, y0, rows, pal):
    """Draw a small ASCII pixel template; characters missing from pal are skipped."""
    for dy, row in enumerate(rows):
        for dx, ch in enumerate(row):
            if ch in pal:
                L.put(x0 + dx, y0 + dy, pal[ch])


def darken_in(L, x, y, pal, steps=1):
    if not L.on(x, y):
        return
    c = L.get(x, y)
    if c in pal:
        L.put(x, y, pal[max(0, pal.index(c) - steps)])


def light_spill(canvas, lights, surfaces, warm):
    """Tint surface colours within each (x, y, radius) light towards warm."""
    cp = canvas.load()
    w, h = canvas.size
    for (lx, ly, r) in lights:
        for y in range(ly - r, ly + r + 1):
            for x in range(lx - r, lx + r + 1):
                if not (0 <= x < w and 0 <= y < h):
                    continue
                d = math.hypot(x - lx, y - ly)
                if d > r:
                    continue
                c = cp[x, y][:3]
                if c not in surfaces:
                    continue
                f = 1 - d / r
                k = 0.12 if f < 0.5 else 0.22
                cp[x, y] = tuple(int(c[i] + (warm[i] - c[i]) * k) for i in range(3)) + (255,)


def wobble(n, seed, lo, hi):
    """A random walk between lo and hi, used for irregular edges."""
    r = random.Random(seed)
    vals = []
    v = (lo + hi) // 2
    for _ in range(n):
        if r.random() < 0.3:
            v = clamp(v + r.choice((-1, 1)), lo, hi)
        vals.append(v)
    return vals


# ---------------------------------------------------------------- surfaces
def lambert(t, light=-0.6):
    th = math.asin(clamp(t, -1.0, 1.0))
    return th, math.cos(th - light)


def cyl_index(b):
    if b < 0.0:
        return 0
    if b < 0.3:
        return 1
    if b < 0.55:
        return 2
    if b < 0.8:
        return 3
    if b < 0.95:
        return 4
    return 5


def cyl_wall(L, x0, x1, y0, y1, pal, row_h=7, cols=7, seed=0, bias=0):
    """Round tower masonry, lit from the left, with blocks foreshortened at the edges."""
    rnd = random.Random(seed)
    tones = {}
    r = (x1 - x0 + 1) / 2.0
    cx = x0 + r
    for y in range(y0, y1 + 1):
        ry = (y - y0) // row_h
        yy = (y - y0) % row_h
        last_col = None
        for x in range(x0, x1 + 1):
            t = (x + 0.5 - cx) / r
            th, b = lambert(t)
            idx = cyl_index(b) + bias
            phase = (th / math.pi + 0.5) * cols + (0.5 if ry % 2 else 0.0)
            col = math.floor(phase)
            key = (ry, col)
            if key not in tones:
                tones[key] = rnd.choice((-1, 0, 0, 0, 0, 1))
            if yy == row_h - 1:
                idx = idx - 2
            elif last_col is not None and col != last_col:
                idx = idx - 2
            else:
                idx += tones[key]
                if yy == 0:
                    idx += 1
            last_col = col
            L.put(x, y, pal[clamp(idx, 0, len(pal) - 1)])


def flat_wall(L, x0, x1, y0, y1, pal, row_h=7, base=3, seed=0, shade=None, bw=(8, 14)):
    """Flat masonry of staggered blocks; shade(x, y) nudges the palette index."""
    rnd = random.Random(seed)
    for ry0 in range(y0, y1 + 1, row_h):
        x = x0 - rnd.randint(0, 7)
        while x <= x1:
            w = rnd.randint(bw[0], bw[1])
            off = rnd.choice((-1, 0, 0, 0, 0, 1))
            for y in range(ry0, min(ry0 + row_h, y1 + 1)):
                yy = y - ry0
                for xx in range(max(x, x0), min(x + w, x1 + 1)):
                    idx = base + off
                    if yy == row_h - 1 or xx == x:
                        idx = base - 2
                    elif yy == 0:
                        idx += 1
                    if shade:
                        idx += shade(xx, y)
                    L.put(xx, y, pal[clamp(idx, 0, len(pal) - 1)])
            x += w


# ---------------------------------------------------------------- parts
def merlons(L, x0, x1, y_top, h, pal, mw=7, gap=4, base=3, point=3, start=0, broken=()):
    """Pointed battlements; indices in broken get chipped, lower tops."""
    x = x0 + start
    n = 0
    while x + mw - 1 <= x1:
        cx = x + (mw - 1) / 2.0
        top = y_top + (point + 1 if n in broken else 0)
        for y in range(top, y_top + h):
            k = y - y_top
            if k < point:
                half = (k + 1) * (mw / 2.0) / (point + 1)
            else:
                half = mw / 2.0
            for xx in range(x, x + mw):
                if n in broken and y == top and (xx - x) % 3 == 1:
                    continue
                if abs(xx - cx) <= half - 0.01 or (k >= point):
                    idx = base
                    if xx == x or (k < point and xx < cx):
                        idx = base + 1
                    if xx == x + mw - 1 or (k < point and xx > cx):
                        idx = base - 1
                    if k == point:
                        idx += 1
                    if y == y_top + h - 1:
                        idx = base - 2
                    L.put(xx, y, pal[clamp(idx, 0, len(pal) - 1)])
        x += mw + gap
        n += 1


def spire(L, cx, tip, base, bhw, pal, p=1.55, row_h=3, cols=9, seed=0, hooks=True):
    """Shingled cone roof; p > 1 gives concave, needle-like sides."""
    rnd = random.Random(seed)
    tones = {}
    for y in range(tip, base + 1):
        s = (y - tip) / float(base - tip)
        hw = max(0.5, bhw * s ** p)
        if base - y < 3:
            hw += (3 - (base - y)) * 0.9
        xl = math.ceil(cx - hw - 0.5)
        xr = math.floor(cx + hw - 0.5)
        ry = (y - tip) // row_h
        yy = (y - tip) % row_h
        last_col = None
        for x in range(xl, xr + 1):
            t = (x + 0.5 - cx) / (hw + 0.5)
            th, b = lambert(t, -0.7)
            idx = 0 if b < 0.05 else 1 if b < 0.35 else 2 if b < 0.7 else 3 if b < 0.93 else 4
            phase = (th / math.pi + 0.5) * cols + (0.5 if ry % 2 else 0.0)
            col = math.floor(phase)
            key = (ry, col)
            if key not in tones:
                tones[key] = rnd.choice((-1, 0, 0, 0, 1))
            if yy == row_h - 1:
                idx -= 2
            elif last_col is not None and col != last_col and hw > 3:
                idx -= 1
            else:
                idx += tones[key]
            last_col = col
            L.put(x, y, pal[clamp(idx, 0, len(pal) - 1)])
    if hooks:
        hw = bhw + 2.7
        xl = math.ceil(cx - hw - 0.5)
        xr = math.floor(cx + hw - 0.5)
        for dx, dy in ((0, -1), (-1, -2), (-1, -3)):
            L.put(xl + dx, base + dy, pal[2])
            L.put(xr - dx, base + dy, pal[1])


# ---------------------------------------------------------------- timber buildings
def roof_tiles(L, mask, pal, seed, shade=None, tile_w=4, row_h=4, rounded=True):
    """Staggered rows of roof tiles; rounded adds a shadow side for clay tiles."""
    rnd = random.Random(seed)
    tones = {}
    for (x, y) in sorted(mask):
        row = y // row_h
        yy = y % row_h
        shift = tile_w // 2 if row % 2 else 0
        col = (x + shift) // tile_w
        xx = (x + shift) % tile_w
        key = (row, col)
        if key not in tones:
            tones[key] = rnd.choice((-1, 0, 0, 0, 1))
        idx = 3 + tones[key]
        if xx == 0:
            idx += 1
        elif xx == tile_w - 1 and rounded:
            idx -= 1
        if yy == row_h - 1:
            idx = 1
        elif yy == 0:
            idx += 1
        if shade:
            idx += shade(x, y)
        L.put(x, y, pal[clamp(idx, 0, len(pal) - 1)])


def plaster(L, mask, pal, seed):
    """Lightly speckled plaster infill for timber framing."""
    rnd = random.Random(seed)
    for (x, y) in sorted(mask):
        c = pal[2]
        r = rnd.random()
        if r < 0.05:
            c = pal[1]
        elif r < 0.09:
            c = pal[3]
        L.put(x, y, c)


def beam(L, x0, y0, x1, y1, pal, width=2):
    """Timber beam, lit on its left or top edge."""
    pts = line_points(x0, y0, x1, y1)
    steep = abs(y1 - y0) > abs(x1 - x0)
    for (x, y) in pts:
        for k in range(width):
            px, py = (x + k, y) if steep else (x, y + k)
            c = pal[3] if k == 0 else (pal[1] if k == width - 1 else pal[2])
            L.put(px, py, c)


def shadow_under(L, mask, plaster_pal, timber_pal, c):
    """Darken plaster pixels directly below beams for a little depth."""
    for (x, y) in sorted(mask):
        if L.on(x, y) and L.get(x, y) in plaster_pal and L.get(x, y - 1) in timber_pal:
            L.put(x, y, c)


def lit_window(L, x0, y0, w, h, glow, timber, shutters=True):
    """Warm four-pane window with a sill and optional plank shutters."""
    for y in range(y0 - 1, y0 + h + 1):
        for x in range(x0 - 1, x0 + w + 1):
            if x in (x0 - 1, x0 + w) or y in (y0 - 1, y0 + h):
                L.put(x, y, timber[0])
            else:
                dx = abs((x - x0) - (w - 1) / 2.0) / (w / 2.0)
                dy = (y - y0) / float(h)
                c = glow[3] if dx < 0.5 and dy > 0.35 else glow[2]
                if dy < 0.15:
                    c = glow[1]
                L.put(x, y, c)
    mx = x0 + w // 2
    my = y0 + h // 2
    for y in range(y0, y0 + h):
        L.put(mx, y, timber[1])
    for x in range(x0, x0 + w):
        L.put(x, my, timber[1])
    for x in range(x0 - 2, x0 + w + 2):
        L.put(x, y0 + h + 1, timber[3])
        L.put(x, y0 + h + 2, timber[1])
    if shutters:
        for sx in (x0 - 5, x0 + w + 1):
            for y in range(y0 - 1, y0 + h + 1):
                for x in range(sx, sx + 4):
                    k = x - sx
                    c = timber[3] if k == 0 else (timber[1] if k == 3 else timber[2])
                    if y in (y0 + 1, y0 + h - 2):
                        c = timber[1]
                    L.put(x, y, c)


def arched_door(L, cx, y0, y1, half, timber, iron):
    """Round-topped plank door with iron bands and a ring knocker."""
    for y in range(y0, y1 + 1):
        for x in range(cx - half, cx + half + 1):
            dx = x - cx
            if y < y0 + half and (dx * dx + (y - (y0 + half)) ** 2) > half * half + 1:
                continue
            k = (x - (cx - half)) % 3
            c = timber[2] if k else timber[1]
            if k == 1:
                c = timber[3]
            if y in (y0 + half + 3, y1 - 5):
                c = iron[1] if x != cx - half else iron[2]
            L.put(x, y, c)
    for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
        L.put(cx + dx, y0 + half + 8 + dy, iron[3] if dy < 0 or dx < 0 else iron[2])
    L.put(cx, y0 + half + 8, timber[0])


def stone_arch(L, cx, y0, y1, half, pal, thick=3):
    """Stone surround for arched_door: voussoirs over the top, quoins down the sides."""
    for y in range(y0 - thick, y1 + 1):
        for x in range(cx - half - thick, cx + half + thick + 1):
            dx = x - cx
            cy = y0 + half
            if y < cy:
                d = math.hypot(dx, y - cy)
                if half + 0.5 < d <= half + thick + 0.5:
                    seg = int((math.atan2(y - cy, dx) + math.pi) * 3.2) % 2
                    L.put(x, y, pal[5] if seg else pal[4])
            elif half < abs(dx) <= half + thick:
                blk = (y - cy) // 5
                L.put(x, y, pal[4] if blk % 2 else pal[3])
                if (y - cy) % 5 == 4:
                    L.put(x, y, pal[1])


# Props stamped with {"t", "T", "W"} wood and {"i", "I"} iron colours.
LANTERN = [
    "..i..",
    ".iIi.",
    "iIIIi",
    "iyYyi",
    "iYWYi",
    "iyYyi",
    "iIIIi",
    "..i..",
]

BARREL = [
    "..ttttttt..",
    ".tTTWTTTTt.",
    "tTTTWTTTTTt",
    "iIIIIIIIIIi",
    "tTTTWTTTTTt",
    "tTTTWTTTTTt",
    "tTTTWTTTTTt",
    "tTTTWTTTTTt",
    "iIIIIIIIIIi",
    "tTTTWTTTTTt",
    "tTTTWTTTTTt",
    ".tTTWTTTTt.",
    "..ttttttt..",
]

CRATE = [
    "ttttttttttt",
    "tTWWWWWWWTt",
    "tWtTTTTTtWt",
    "tWTtTTTtTWt",
    "tWTTtTtTTWt",
    "tWTTTtTTTWt",
    "tWTTtTtTTWt",
    "tWTtTTTtTWt",
    "tWtTTTTTtWt",
    "tTWWWWWWWTt",
    "ttttttttttt",
]


def gothic_half(y, ya, ys, hw):
    """Half width of a pointed arch opening at row y (apex ya, springline ys)."""
    if y >= ys:
        return hw
    R = 1.35 * hw
    d = ys - y
    val = R * R - d * d
    if val <= 0:
        return -1
    return hw - R + math.sqrt(val)


# ---------------------------------------------------------------- output
def save_sprite(image, bmp_path):
    """Write the runtime BMP (32-bit, top-down, with alpha)."""
    w, h = image.size
    pixels = image.tobytes("raw", "BGRA")
    info = struct.pack("<IiiHHIIiiII", 124, w, -h, 1, 32, 3, len(pixels), 0, 0, 0, 0)
    info += struct.pack("<IIII", 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000)
    info += b"BGRs" + bytes(64)
    with open(bmp_path, "wb") as f:
        f.write(b"BM" + struct.pack("<IHHI", 14 + len(info) + len(pixels), 0, 0, 14 + len(info)))
        f.write(info)
        f.write(pixels)
