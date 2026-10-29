#!/usr/bin/env python3
"""Stonewake Quarry, native 24px/tile art built with the shared pixelkit pipeline.

No concept images are sampled. Entrance 168x144, enemies five 48x48 cells,
crane three 72x72 poses, machinery/supply props four 24x24 cells.
"""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, poly_mask, beam, save_sprite, roof_tiles, flat_wall

OUT = (30, 29, 25)
STONE = [(65, 61, 52), (100, 93, 76), (137, 128, 105), (174, 165, 139), (207, 199, 171), (230, 222, 194)]
WOOD = [(49, 34, 25), (81, 53, 32), (116, 79, 43), (153, 111, 63), (191, 148, 88)]
IRON = [(29, 33, 32), (52, 61, 59), (85, 99, 91), (130, 146, 131)]
GREEN = [(37, 67, 45), (65, 108, 65), (107, 162, 94), (178, 220, 133)]
WATER = [(21, 80, 91), (29, 112, 119), (49, 149, 148), (123, 194, 179)]
ROOT = Path(__file__).resolve().parents[2]
OUTDIR = ROOT / 'assets/images'


def polygon(layer, pts, color):
    for x, y in sorted(poly_mask((layer.w, layer.h), pts)):
        layer.put(x, y, color)


def box(layer, x, y, w, h, color):
    for ty in range(y, y + h):
        for tx in range(x, x + w):
            layer.put(tx, ty, color)


def line(layer, x, y, xx, yy, pal=WOOD, width=2):
    ramp = [pal[min(i, len(pal) - 1)] for i in range(4)]
    beam(layer, x, y, xx, yy, ramp, width=width)


def slab(layer, x, y, w, h, pal=STONE):
    polygon(layer, [(x+2,y),(x+w-2,y),(x+w,y+2),(x+w-1,y+h-1),(x+2,y+h),(x,y+h-3),(x,y+2)], pal[2])
    line(layer, x+2, y, x+w-3, y, pal[3:], 1)
    line(layer, x+1, y+1, x+1, y+h-3, pal[3:], 1)
    line(layer, x+2, y+h-1, x+w-2, y+h-1, pal[:2], 1)


def crane(canvas, ox, oy, mode=0):
    """Timber derrick, counterweight, rope and hand winch; grounded at oy."""
    layer = Layer(*canvas.size)
    x, y = ox, oy
    # A trestle with braced legs, rather than a tower silhouette.
    line(layer, x-12, y-2, x-5, y-28, WOOD, 3)
    line(layer, x+10, y-2, x+4, y-28, WOOD, 3)
    line(layer, x-12, y-3, x+10, y-3, WOOD, 3)
    line(layer, x-9, y-6, x+4, y-24, WOOD, 2)
    line(layer, x-5, y-28, x+28, y-45, WOOD, 4)
    line(layer, x+4, y-28, x+28, y-45, WOOD, 2)
    line(layer, x-5, y-28, x-19, y-34, WOOD, 4)
    # Iron plates and pegs at joints.
    for xx, yy in [(x-5,y-28),(x+4,y-28),(x-10,y-3),(x+8,y-3),(x+25,y-44)]:
        box(layer, xx-1, yy-1, 3, 3, IRON[1])
        layer.put(xx-1, yy-1, IRON[3])
    load_y = y - (20 if mode == 0 else 11 if mode == 1 else 32)
    line(layer, x+28, y-44, x+28, load_y, [WOOD[4]], 1)
    line(layer, x+26, y-42, x+26, load_y, [WOOD[2]], 1)
    slab(layer, x+21, load_y, 12, 10)
    box(layer, x+23, load_y+1, 2, 10, IRON[0])
    box(layer, x+29, load_y+1, 2, 10, IRON[1])
    slab(layer, x-23, y-37, 9, 10)
    box(layer, x-22, y-33, 8, 2, IRON[0])
    # Winch, spokes and a visible handle at player height.
    slab(layer, x-8, y-17, 13, 8, WOOD)
    for xx in range(x-6,x+5,3):
        box(layer, xx, y-16, 1, 6, WOOD[4])
    line(layer, x+6, y-17, x+6, y-8, IRON, 1)
    line(layer, x+6, y-8, x+11, y-8, IRON, 1)
    composite(canvas, layer, OUT)


def quarry_bench(canvas, top, face, seed):
    """A broad sawn limestone bench with a lit shelf and fractured vertical face."""
    wall = Layer(*canvas.size)
    mask = poly_mask(canvas.size, face)
    left = min(x for x, y in mask)
    right = max(x for x, y in mask)
    upper = min(y for x, y in mask)
    lower = max(y for x, y in mask)
    flat_wall(wall, left, right + 1, upper, lower + 1, STONE, row_h=12, base=3, seed=seed, bw=(14, 24))
    for y in range(upper, lower + 1):
        for x in range(left, right + 1):
            if (x, y) not in mask:
                wall.erase(x, y)
    rng = random.Random(seed + 7)
    for x, y in sorted(mask):
        if (x - 1, y) not in mask:
            wall.put(x, y, STONE[4])
        elif (x + 1, y) not in mask or (x, y + 1) not in mask:
            wall.put(x, y, STONE[1])
        elif rng.random() < .08:
            wall.put(x, y, STONE[2 if rng.random() < .5 else 4])
    # Short chisel scars and fractured edges keep the cut face distinct from a brick building.
    for x in range(left + 4, right - 2, 9):
        y = rng.randint(upper, lower)
        for dy in range(2 + rng.randrange(4)):
            if (x, y + dy) in mask:
                wall.put(x, y + dy, STONE[2])
                if (x - 1, y + dy) in mask:
                    wall.put(x - 1, y + dy, STONE[4])
    composite(canvas, wall, OUT)
    shelf = Layer(*canvas.size)
    rng = random.Random(seed)
    for x, y in sorted(poly_mask(canvas.size, top)):
        shelf.put(x, y, STONE[3 if rng.random() < .06 else 5 if rng.random() < .12 else 4])
    composite(canvas, shelf, STONE[1])


def entrance():
    canvas = Image.new('RGBA', (168, 144))
    # An open terraced cutting, following the atlas quarry rather than a rounded cave mound.
    bed = Layer(*canvas.size)
    polygon(bed, [(23, 86), (68, 69), (124, 78), (146, 109), (126, 132), (51, 135), (19, 117)], STONE[1])
    composite(canvas, bed, OUT)
    quarry_bench(canvas,
        [(28, 65), (49, 48), (111, 45), (144, 60), (127, 70), (72, 64)],
        [(28, 65), (72, 64), (127, 70), (144, 60), (144, 84), (124, 95), (69, 86), (28, 88)], 173)
    quarry_bench(canvas,
        [(13, 90), (29, 77), (66, 78), (68, 88), (48, 99), (13, 103)],
        [(13, 103), (48, 99), (68, 88), (68, 109), (48, 121), (13, 124)], 181)
    quarry_bench(canvas,
        [(102, 90), (125, 79), (153, 88), (151, 100), (125, 110), (101, 105)],
        [(101, 105), (125, 110), (151, 100), (151, 120), (132, 132), (101, 124)], 193)
    quarry_bench(canvas,
        [(47, 111), (67, 96), (102, 97), (113, 108), (100, 116), (68, 115)],
        [(47, 111), (68, 115), (100, 116), (113, 108), (113, 132), (100, 140), (48, 136)], 197)
    layer = Layer(*canvas.size)
    # A single rail portal: 18px clear width and 25px height, beside a 24px player.
    polygon(layer, [(75, 136), (75, 113), (78, 110), (91, 110), (94, 113), (94, 136)], (22, 25, 23))
    line(layer, 73, 136, 73, 110, WOOD, 3)
    line(layer, 96, 136, 96, 110, WOOD, 3)
    line(layer, 71, 109, 98, 109, WOOD, 4)
    line(layer, 74, 121, 82, 111, WOOD, 1)
    line(layer, 95, 121, 88, 111, WOOD, 1)
    for x in (73, 96):
        for y in (113, 131):
            box(layer, x - 1, y, 3, 2, IRON[1])
            layer.put(x - 1, y, IRON[3])
    # Short rails lead into the dark portal, with no opaque rectangular ground plate.
    for y in range(132, 144, 4):
        line(layer, 75, y, 95, y, WOOD, 1)
    line(layer, 79, 129, 77, 143, IRON, 1)
    line(layer, 90, 129, 93, 143, IRON, 1)
    # A small flooded excavation at the low front edge of the cutting.
    polygon(layer, [(18, 126), (35, 121), (50, 126), (59, 134), (48, 140), (29, 139), (16, 133)], WATER[0])
    line(layer, 21, 127, 36, 127, WATER[2:], 1)
    line(layer, 31, 135, 48, 135, WATER[2:], 1)
    line(layer, 17, 134, 27, 139, STONE[3:], 1)
    # A loaded cart and loose cut blocks are small enough to belong to the same workers.
    for x in (113, 124):
        slab(layer, x, 136, 4, 4, IRON)
    polygon(layer, [(109, 125), (131, 125), (128, 137), (112, 137)], WOOD[1])
    for y in (126, 131, 135):
        line(layer, 112, y, 129, y, WOOD, 1)
    for x, y in ((112, 123), (119, 120), (124, 123)):
        slab(layer, x, y, 7, 5)
    for x, y in ((49, 91), (115, 70), (145, 119), (58, 135)):
        slab(layer, x, y, 6, 4)
    for x, y in ((27, 76), (54, 52), (133, 81), (19, 117), (143, 126), (59, 96)):
        line(layer, x, y, x + 5, y, GREEN[:2], 1)
        layer.put(x + 2, y - 1, GREEN[2])
    composite(canvas, layer, OUT)
    crane(canvas, 36, 102)
    return canvas


def body_segment(canvas, pts, pal, seed):
    layer=Layer(*canvas.size)
    mask=poly_mask(canvas.size,pts)
    left=min(x for x,y in mask); right=max(x for x,y in mask)
    top=min(y for x,y in mask)
    rng=random.Random(seed)
    for x,y in sorted(mask):
        shade=3 if x<left+(right-left)*.4 else 2
        if y==top:
            shade=min(4,len(pal)-1)
        if rng.random()<.055:
            shade=max(1,shade-1)
        layer.put(x,y,pal[min(shade,len(pal)-1)])
    composite(canvas,layer,OUT)


def humanoid(kind):
    # Author bodies at their gameplay size; the atlas cell is padding, never a scale target.
    size = 48 if kind == 'colossus' else 32 if kind == 'troll' else 24
    canvas=Image.new('RGBA',(size,size))
    boss=kind=='colossus'; troll=kind=='troll'
    base=size-2
    cx=size//2; scale=1.2 if boss else .80 if troll else .60
    def p(pts):
        return [(int(cx+x*scale),int(base+y*scale)) for x,y in pts]
    skin=STONE
    # Feet, separated knees, broad angular torso, heavy articulated forearms.
    parts=[([(-9,-1),(-9,-6),(-4,-12),(-2,-9),(-3,-1)],skin),
           ([(3,-1),(2,-9),(5,-12),(9,-6),(10,-1)],skin),
           ([(-8,-12),(-10,-23),(-4,-27),(6,-26),(10,-22),(7,-11),(0,-9)],skin),
           ([(-9,-23),(-13,-21),(-14,-12),(-10,-9),(-7,-13)],skin),
           ([(9,-23),(14,-20),(15,-11),(11,-9),(8,-14)],skin),
           ([(-5,-26),(-5,-31),(0,-34),(5,-31),(5,-26),(0,-23)],skin)]
    for i,(pts,pal) in enumerate(parts):
        body_segment(canvas,p(pts),pal,97+i)
    layer=Layer(size,size)
    # Eyes, carved brow, jaw, iron joint bindings and a restrained green rune.
    head_y=int(base-29*scale)
    eye = 2 if boss or troll else 1
    box(layer,cx-int(4*scale),head_y,eye,1,GREEN[3]);box(layer,cx+int(2*scale),head_y,eye,1,GREEN[3])
    for yy in [-20,-12]:
        line(layer,int(cx-8*scale),int(base+yy*scale),int(cx+8*scale),int(base+yy*scale),WOOD if troll else IRON,2)
    for xx in [-9,9]:
        box(layer,int(cx+xx*scale),int(base-18*scale),1,2,IRON[3])
    rune_y=int(base-19*scale)
    line(layer,cx,rune_y-3,cx,rune_y+2,GREEN[2:],1)
    line(layer,cx-2,rune_y,cx+2,rune_y,GREEN[2:],1)
    if troll:
        # Tusked stone-skinned troll with a crude pick, moss and leather bindings.
        line(layer,cx-3,head_y+4,cx-3,head_y+1,STONE[4:],1)
        line(layer,cx+3,head_y+4,cx+3,head_y+1,STONE[4:],1)
        line(layer,4,29,5,13,WOOD,1)
        line(layer,2,14,11,11,STONE[1:4],1)
        for xx,yy in [(11,8),(23,15),(9,24)]:
            box(layer,xx,yy,3,2,GREEN[1])
    elif boss:
        # Giant squared stone hammer; one green fault line across the head.
        line(layer,5,44,10,16,WOOD,3)
        slab(layer,1,13,15,9)
        line(layer,3,14,4,20,IRON,2)
        line(layer,13,14,14,20,IRON,2)
        line(layer,8,15,7,19,GREEN[2:],1)
    else:
        # Construct's riveted work apron and socketed wrists.
        polygon(layer,[(8,13),(15,13),(16,19),(8,19)],WOOD[1])
        line(layer,9,14,14,14,WOOD[3:],1)
        for xx in [8,15]:
            layer.put(xx,17,IRON[3])
    composite(canvas,layer,OUT)
    return canvas



def machinery(repaired=False):
    canvas=Image.new('RGBA',(24,24));layer=Layer(24,24)
    for xx in [4,17]:
        line(layer,xx,20,xx,5,WOOD,3)
    line(layer,4,5,18,5,WOOD,3)
    slab(layer,6,10,11,7,IRON)
    for xx in [7,10,13,16]:
        box(layer,xx,11,1,5,WOOD[4])
    line(layer,18,10,20,15,IRON,1)
    line(layer,20,15,22,15,WOOD[3:],1)
    layer.put(10,7,GREEN[3] if repaired else (178,95,54))
    if not repaired:
        line(layer,12,16,14,21,WOOD[:3],1)
    composite(canvas,layer,OUT);return canvas


def supplies(empty=False):
    canvas=Image.new('RGBA',(24,24));layer=Layer(24,24)
    for xx,yy in [(2,12),(10,8)]:
        slab(layer,xx,yy,11,11,WOOD)
        line(layer,xx+1,yy+1,xx+9,yy+9,WOOD,1)
        line(layer,xx+9,yy+1,xx+1,yy+9,WOOD,1)
    if empty:
        box(layer,3,13,8,7,WOOD[0]);box(layer,11,9,8,7,WOOD[0])
    else:
        box(layer,12,11,2,5,IRON[3]);box(layer,17,11,2,5,IRON[2])
    composite(canvas,layer,OUT);return canvas


def workshop():
    canvas=Image.new('RGBA',(96,72));layer=Layer(96,72)
    flat_wall(layer,15,79,40,64,STONE,row_h=5,base=2,seed=118,bw=(9,16))
    for xx in [15,75]:
        line(layer,xx,64,xx,30,WOOD,3)
    # Crooked tile roof and a timber-framed open work bay.
    mask=poly_mask(canvas.size,[(8,35),(29,9),(60,12),(85,38),(78,42),(13,40)])
    roof_tiles(layer,mask,WOOD,117,tile_w=5,row_h=4)
    line(layer,9,36,82,39,WOOD,3)
    line(layer,29,9,59,12,WOOD,2)
    polygon(layer,[(29,41),(66,42),(67,63),(28,62)],(36,32,26))
    line(layer,28,42,28,64,WOOD,3);line(layer,68,43,68,65,WOOD,3)
    line(layer,28,43,38,51,WOOD,2);line(layer,66,44,57,51,WOOD,2)
    # Bench, laid-out tools, stacked dressed stone and sacks.
    line(layer,31,56,63,56,WOOD,3)
    for xx in [33,59]:
        line(layer,xx,56,xx,65,WOOD,2)
    line(layer,40,54,48,54,IRON,1);line(layer,45,51,45,56,WOOD,1)
    line(layer,55,53,61,51,IRON,1)
    for xx,yy in [(7,59),(15,65),(77,58),(81,66)]:
        slab(layer,xx,yy,10,5)
    # Hooks and a pickaxe on the wall, visibly medieval workshop equipment.
    line(layer,73,46,76,56,WOOD,1);line(layer,71,47,79,44,IRON,1)
    line(layer,17,49,22,49,IRON,1)
    composite(canvas,layer,OUT)
    return canvas


def build():
    OUTDIR.mkdir(exist_ok=True)
    save_sprite(entrance(),OUTDIR/'stonewake-entrance.bmp')
    save_sprite(workshop(),OUTDIR/'stonewake-workshop.bmp')
    enemies=Image.new('RGBA',(240,48))
    for i,im in [(1,humanoid('construct')),(2,humanoid('troll')),(4,humanoid('colossus'))]:
        enemies.alpha_composite(im,(i*48+(48-im.width)//2,48-im.getbbox()[3]))
    save_sprite(enemies,OUTDIR/'stonewake-enemies.bmp')
    derricks=Image.new('RGBA',(216,72))
    for mode in range(3):
        derrick=Image.new('RGBA',(72,72));crane(derrick,30,68,mode)
        derricks.alpha_composite(derrick,(mode*72,0))
    save_sprite(derricks,OUTDIR/'stonewake-crane.bmp')
    props=Image.new('RGBA',(96,24))
    for i,im in enumerate([machinery(),machinery(True),supplies(),supplies(True)]):
        props.alpha_composite(im,(i*24,0))
    save_sprite(props,OUTDIR/'stonewake-props.bmp')


if __name__=='__main__':
    build()
