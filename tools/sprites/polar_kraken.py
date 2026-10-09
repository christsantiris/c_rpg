#!/usr/bin/env python3
"""Import the Polar Kraken at native game scale with a limited pixel-art palette."""
from pathlib import Path

from PIL import Image

from pixelkit import save_sprite

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "docs/art-sources/polar-kraken/source.png"
DEST = ROOT / "assets/images/polar-kraken.bmp"
SIZE = 96


def main():
    image = Image.open(SOURCE).convert("RGBA")
    image = image.resize((SIZE, SIZE), Image.Resampling.NEAREST)
    alpha = image.getchannel("A")
    sprite = image.convert("RGB").quantize(colors=32).convert("RGBA")
    sprite.putalpha(alpha)
    save_sprite(sprite, DEST)
    print(f"Saved {DEST} ({SIZE} x {SIZE})")


if __name__ == "__main__":
    main()
