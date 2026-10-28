#!/usr/bin/env python3
"""Arrange the pixelkit sprite and actual SDL captures for design review."""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
FONT = ROOT / "assets/PressStart2P-Regular.ttf"
sheet = Image.new("RGB", (1280, 800), (20, 23, 29))
draw = ImageDraw.Draw(sheet)


def text(x, y, label, size=12, color=(209, 201, 177)):
    draw.text((x, y), label, font=ImageFont.truetype(str(FONT), size), fill=color)


text(32, 28, "UNIVERSAL TOWN GATE", 24, (235, 216, 165))
text(32, 70, "STONE GATEHOUSE / APPROVAL CONCEPT", 12)
text(32, 106, "4X DETAIL", 12, (147, 171, 179))
text(672, 106, "NATIVE GAME SCALE", 12, (147, 171, 179))
scene = Image.open(HERE / "open-scene.png").convert("RGB")
crop = scene.crop((228, 60, 376, 192)).resize((592, 528), Image.Resampling.NEAREST)
sheet.paste(crop, (32, 132))
sheet.paste(scene, (672, 132))
text(32, 686, "120 X 96 PX / 24 PX ROAD", 12)
text(32, 716, "TRANSPARENT PASSAGE + WALL JOINS", 11)
text(672, 496, "OPEN", 12)
text(960, 496, "CLOSED STATE", 12)
for x, name in ((672, "open"), (960, "closed")):
    panel = scene.crop((240, 72, 360, 168))
    if name == "closed":
        panel = Image.open(HERE / "closed-scene.png").crop((240, 72, 360, 168))
    sheet.paste(panel.resize((240, 192), Image.Resampling.NEAREST), (x, 528))
text(32, 770, "PIXELKIT ART + SDL RENDERER PREVIEW", 11, (147, 171, 179))
sheet.save(HERE / "proposal.png")
print("Wrote town gate approval sheet.")
