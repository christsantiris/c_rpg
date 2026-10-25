#!/usr/bin/env python3
"""Stonewake Quarry, native 24px/tile art built with the shared pixelkit pipeline.

No concept images are sampled. Entrance 168x144, enemies five 48x48 cells,
crane three 72x72 poses, machinery/supply props four 24x24 cells.
"""
from pathlib import Path
import random
from PIL import Image
from pixelkit import Layer, composite, poly_mask, beam, save_sprite, roof_tiles, flat_wall
import glassdeep as rock

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


def entrance():
    canvas = Image.new('RGBA', (168,144))
    rock.ROCK = STONE
    rock.OUT = OUT
    # Low stepped beds. The highest bench is behind the machinery, not a peak.
    for cx,cy,rx,ry,seed,cell in [(81,92,59,47,87,19),(45,113,38,27,93,16),(129,117,31,25,99,14)]:
        rock.outcrop(canvas,cx,cy,rx,ry,seed,cell)
    layer = Layer(*canvas.size)
    rng = random.Random(173)
    for row, left, right in [(81,32,136),(105,14,145),(126,10,156)]:
        for yy in range(row,row+3):
            for xx in range(left,right):
                layer.put(xx,yy,STONE[3 if yy==row else 1])
        for xx in range(left+2,right-3):
            if rng.random()<.25:
                layer.put(xx,row-1,STONE[5])
    # Reinforced entry with a player-scale dark mouth.
    polygon(layer,[(68,143),(68,99),(72,91),(94,91),(100,99),(100,143)], (22,25,23))
    line(layer,66,143,66,94,WOOD,5)
    line(layer,101,143,101,94,WOOD,5)
    line(layer,64,94,103,94,WOOD,5)
    line(layer,66,111,79,95,WOOD,2)
    line(layer,101,111,89,95,WOOD,2)
    for xx in [65,100]:
        for yy in [100,124,138]:
            box(layer,xx-1,yy,4,3,IRON[1])
            layer.put(xx,yy,IRON[3])
    # Rails end naturally at the edge of the sprite.
    for yy in range(125,144,5):
        line(layer,71,yy,96,yy,WOOD,2)
    line(layer,76,122,73,143,IRON,1)
    line(layer,91,122,94,143,IRON,1)
    # Small flooded cutting and moss in the lower left.
    polygon(layer,[(13,119),(19,114),(43,114),(54,122),(49,131),(26,134),(12,129)],WATER[0])
    line(layer,18,119,38,119,WATER[2:],1)
    line(layer,26,128,45,128,WATER[2:],1)
    line(layer,14,130,27,133,STONE[4:],1)
    # Loaded minecart, wheels below the bed, warm iron/wood at the same scale as a doorway.
    for xx in [113,132]:
        slab(layer,xx,136,5,5,IRON)
    polygon(layer,[(108,123),(139,123),(135,137),(112,137)],WOOD[1])
    for yy in [124,130,135]:
        line(layer,111,yy,136,yy,WOOD,1)
    for xx in [113,132]:
        box(layer,xx,124,2,12,IRON[1])
    for xx,yy in [(114,121),(121,117),(129,120)]:
        slab(layer,xx,yy,9,6)
    for xx, yy in [(27,99),(38,136),(115,88),(146,126),(61,116),(140,109)]:
        line(layer,xx,yy,xx+7,yy,GREEN[:2],2)
        layer.put(xx+2,yy-1,GREEN[2])
    composite(canvas,layer,OUT)
    crane(canvas,110,94)
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
    canvas=Image.new('RGBA',(48,48))
    boss=kind=='colossus'; troll=kind=='troll'; sentinel=kind=='sentinel'
    base=45; top=2 if boss else 10 if troll else 17
    cx=24; scale=1.5 if boss else 1.15 if troll else .85
    def p(pts):
        return [(int(cx+x*scale),int(base+y*scale)) for x,y in pts]
    skin=IRON if sentinel else STONE
    # Feet, separated knees, broad angular torso, heavy articulated forearms.
    parts=[([(-9,-1),(-9,-6),(-4,-12),(-2,-9),(-3,-1)],skin),
           ([(3,-1),(2,-9),(5,-12),(9,-6),(10,-1)],skin),
           ([(-8,-12),(-10,-23),(-4,-27),(6,-26),(10,-22),(7,-11),(0,-9)],skin),
           ([(-9,-23),(-13,-21),(-14,-12),(-10,-9),(-7,-13)],skin),
           ([(9,-23),(14,-20),(15,-11),(11,-9),(8,-14)],skin),
           ([(-5,-26),(-5,-31),(0,-34),(5,-31),(5,-26),(0,-23)],skin)]
    for i,(pts,pal) in enumerate(parts):
        body_segment(canvas,p(pts),pal,97+i)
    layer=Layer(48,48)
    # Eyes, carved brow, jaw, iron joint bindings and a restrained green rune.
    head_y=int(base-29*scale)
    box(layer,cx-4,head_y,3,1,GREEN[3]);box(layer,cx+2,head_y,3,1,GREEN[3])
    for yy in [-20,-12]:
        line(layer,int(cx-8*scale),int(base+yy*scale),int(cx+8*scale),int(base+yy*scale),WOOD if troll else IRON,2)
    for xx in [-9,9]:
        box(layer,int(cx+xx*scale),int(base-18*scale),2,2,IRON[3])
    rune_y=int(base-19*scale)
    line(layer,cx,rune_y-3,cx,rune_y+2,GREEN[2:],1)
    line(layer,cx-2,rune_y,cx+2,rune_y,GREEN[2:],1)
    if troll:
        # Tusked stone-skinned troll with a crude pick, moss and leather bindings.
        line(layer,cx-3,head_y+4,cx-3,head_y+1,STONE[4:],1)
        line(layer,cx+3,head_y+4,cx+3,head_y+1,STONE[4:],1)
        line(layer,7,43,9,20,WOOD,2)
        line(layer,4,21,16,18,STONE[1:4],2)
        for xx,yy in [(18,13),(32,24),(15,34)]:
            box(layer,xx,yy,3,2,GREEN[1])
    elif sentinel:
        # Closed visor and broad shield; a sword, never a firearm.
        box(layer,20,head_y-2,9,2,IRON[3])
        box(layer,19,head_y+1,11,2,IRON[0])
        box(layer,23,head_y,2,6,IRON[2])
        polygon(layer,[(32,29),(40,31),(39,41),(35,44),(31,39)],IRON[1])
        line(layer,32,30,38,32,IRON[3:],1)
        line(layer,7,41,9,23,IRON,2)
        line(layer,5,36,12,36,WOOD[3:],1)
    elif boss:
        # Giant squared stone hammer; one green fault line across the head.
        line(layer,5,42,10,13,WOOD,3)
        slab(layer,0,10,17,10)
        line(layer,2,11,3,18,IRON,2)
        line(layer,13,11,14,18,IRON,2)
        line(layer,8,12,7,16,GREEN[2:],1)
    else:
        # Construct's riveted work apron and socketed wrists.
        polygon(layer,[(18,31),(29,31),(30,39),(18,39)],WOOD[1])
        line(layer,19,32,27,32,WOOD[3:],1)
        for xx in [18,28]:
            layer.put(xx,35,IRON[3])
    composite(canvas,layer,OUT)
    return canvas


def rockworm():
    canvas=Image.new('RGBA',(48,48))
    for i,(x,y) in enumerate([(33,41),(30,36),(26,33),(21,33),(16,36),(13,40)]):
        body_segment(canvas,[(x-4,y-3),(x+2,y-4),(x+5,y),(x+3,y+4),(x-3,y+3),(x-5,y)],STONE,23+i)
    layer=Layer(48,48)
    polygon(layer,[(8,38),(17,38),(18,44),(15,46),(8,45)],STONE[1])
    box(layer,9,40,7,3,OUT)
    for xx in [9,12,15]:
        layer.put(xx,40,STONE[5]);layer.put(xx,43,STONE[4])
    box(layer,9,37,2,1,GREEN[3]);box(layer,15,37,2,1,GREEN[3])
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
    for i,im in enumerate([rockworm(),humanoid('construct'),humanoid('troll'),humanoid('sentinel'),humanoid('colossus')]):
        enemies.alpha_composite(im,(i*48,0))
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
