#!/usr/bin/env python3
"""Ironfang's mine entrance and native 24px foreman, using the existing pixelkit."""
from pathlib import Path
import random
import re
from PIL import Image
from pixelkit import Layer, composite, poly_mask, line_points, stamp, save_sprite

ROOT = Path(__file__).resolve().parents[2]
OUT = (20, 23, 26)
STONE = [(28, 33, 37), (43, 49, 53), (64, 70, 73), (88, 94, 95), (122, 127, 123), (158, 162, 147)]
WOOD = [(38, 29, 23), (70, 47, 30), (106, 75, 42), (148, 111, 64), (182, 148, 92)]


def stroke(layer, points, color):
    for a, b in zip(points, points[1:]):
        for x, y in line_points(*a, *b):
            layer.put(x, y, color)


def facet(canvas, points, shades, seed):
    layer = Layer(*canvas.size)
    rng = random.Random(seed)
    for x, y in sorted(poly_mask(canvas.size, points)):
        index = 2 + (x % 7 == 0) - (y % 9 == 0) + (rng.randrange(17) == 0)
        layer.put(x, y, shades[min(len(shades) - 1, index)])
    composite(canvas, layer, OUT)


def entrance():
    canvas = Image.new('RGBA', (168, 144))
    # Jagged mountainside with connected, individually shaded slate faces.
    for seed, points in enumerate([
        [(10,140),(6,101),(23,63),(38,72),(44,24),(64,39),(79,13),(96,30),(114,20),(128,56),(145,66),(159,111),(156,141)],
        [(13,136),(25,72),(45,89),(45,31),(66,49),(60,134)],
        [(58,135),(66,49),(80,19),(98,43),(90,134)],
        [(93,137),(98,43),(114,25),(128,75),(146,92),(151,140)]
    ]):
        facet(canvas, points, STONE, seed + 17)
    detail = Layer(168, 144)
    for x0, y0 in [(32,91),(51,63),(110,80),(136,110),(100,46)]:
        stroke(detail, [(x0,y0),(x0+7,y0-4),(x0+13,y0-2)], (133,87,47))
        stroke(detail, [(x0+3,y0),(x0+9,y0-3)], (190,145,75))
    # Recessed human-scale portal under a timber gantry; no painted ground patch.
    for y in range(119,144):
        for x in range(75,94):
            detail.put(x,y,(12,17,20))
    for x in (71,94):
        for xx in range(x,x+4):
            for y in range(113,144):
                detail.put(xx,y,WOOD[2 if xx % 3 else 3])
    for y in range(110,116):
        for x in range(69,100):
            detail.put(x,y,WOOD[3 if y == 110 else 2])
    stroke(detail, [(70,111),(79,123)],WOOD[4])
    stroke(detail, [(99,111),(90,123)],WOOD[4])
    # A larger working gantry and ore chute identify the place at atlas scale.
    for x in (38,121):
        for xx in range(x,x+4):
            for y in range(73,133):
                detail.put(xx,y,WOOD[2 if xx % 2 else 3])
    for y in range(70,76):
        for x in range(35,128):
            detail.put(x,y,WOOD[3 if y == 70 else 2])
    stroke(detail, [(41,78),(55,91)],WOOD[4])
    stroke(detail, [(121,78),(107,91)],WOOD[4])
    stroke(detail, [(56,75),(56,110)],(172,159,120))
    for x in range(76,93):
        for y in (138,143):
            detail.put(x,y,STONE[4])
    for x in (78,89):
        stroke(detail, [(x,143),(x,133)],(181,187,170))
    stamp(detail, 48,108,['.iiiiii.','iSSSSSSi','iSSSSSSi','.iiiiii.','..o..o..'],{'i':(70,67,57),'S':(145,115,65),'o':OUT})
    for x in (67,100):
        stamp(detail,x,120,['.mm.','mGGm','mGPm','.mm.'],{'m':WOOD[0],'G':(225,166,59),'P':(255,229,148)})
    composite(canvas,detail,None)
    return canvas


def boss():
    # Reuse the established Hobgoblin silhouette and palette at exactly 24px.
    text = (ROOT / 'src/renderer/mountain_enemy_art.h').read_text()
    block = text.split('{ // HOBGOBLIN_GUARD',1)[1].split('},',1)[0]
    rows = re.findall(r'"([^"]+)"',block)
    canvas = Image.new('RGBA',(24,24))
    body = (91,121,45)
    palette = {'o':OUT,'s':(60,80,30),'S':body,'H':(145,165,115),'e':(18,18,29),'y':(244,67,20),
             'c':(62,50,29),'C':(94,77,44),'v':(134,121,85),'m':(68,72,82),'M':(133,141,150),'N':(208,220,223),
             'w':(86,58,32),'W':(155,116,64),'l':(69,43,32),'L':(115,73,40),'K':(159,112,66),'k':(43,31,27),
             'g':(133,89,35),'G':(212,168,71),'f':(225,219,186)}
    layer = Layer(24,24)
    stamp(layer,0,0,rows,palette)
    for y in range(24):
        for x in range(20,24):
            layer.erase(x,y)
    stamp(layer,8,0,['.mmmmmm.','mMMMMMMm','mMMGGMMm','mmmmmmmm'],palette)
    layer.put(12,2,(255,225,128))
    stroke(layer,[(22,8),(22,22)],WOOD[3])
    stroke(layer,[(18,8),(20,6),(23,7)],(208,220,223))
    stroke(layer,[(18,9),(20,7),(23,8)],(68,72,82))
    composite(canvas,layer,None)
    assert canvas.getbbox()[2] - canvas.getbbox()[0] <= 24
    assert canvas.getbbox()[3] == 24
    return canvas


def build():
    output = ROOT / 'docs/art-sources/ironfang'
    output.mkdir(parents=True,exist_ok=True)
    for name, sprite in [('ironfang-entrance',entrance()),('ironfang-boss',boss())]:
        save_sprite(sprite,ROOT/'assets/images'/f'{name}.bmp')
        sprite.save(output/f'{name}.png')


if __name__ == '__main__':
    build()
