#!/usr/bin/env python3
"""Import the refined Temple sheet at the renderer's native display sizes."""
from pathlib import Path

from PIL import Image

from pixelkit import save_sprite

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'docs/art-sources/enemy-batch-15/source.png'
DEST = ROOT / 'assets/images/temple-enemies.bmp'
CELL = 48
# Source rectangles isolate all ten identities and the broken Guardian pose.
CROPS = (
    (25, 70, 350, 374),  # Relic Scarabs
    (350, 10, 715, 370),  # Temple Stalker
    (715, 35, 1045, 374),  # Blowdart Hunter
    (1070, 5, 1448, 380),  # Vinebound Guardian
    (15, 370, 340, 698),  # Sun Priest
    (350, 370, 715, 699),  # Serpent Spirit
    None,  # Reserved atlas slot; shared enemy artwork is drawn separately.
    (1080, 400, 1448, 685),  # Lunar Effigy
    (20, 702, 330, 1086),  # Moonbound Sentinel
    (330, 701, 715, 1086),  # Sun Guardian
    (715, 725, 1155, 1086),  # Broken Sun Guardian
)


def main():
    source = Image.open(SOURCE).convert('RGBA')
    atlas = Image.new('RGBA', (CELL * 4, CELL * 3))
    for index, crop in enumerate(CROPS):
        if crop is None:
            continue
        sprite = source.crop(crop)
        # Ignore faint edge pixels when finding each silhouette's bounds.
        bounds = sprite.getchannel('A').point(lambda a: 255 if a >= 128 else 0).getbbox()
        if bounds is None:
            raise ValueError(f'Empty Temple sprite {index}')
        sprite = sprite.crop(bounds)
        size = 40 if index >= 9 else 28
        sprite.thumbnail((size - 2, size - 2), Image.Resampling.NEAREST)
        alpha = sprite.getchannel('A').point(lambda a: 255 if a >= 128 else 0)
        sprite = sprite.convert('RGB').quantize(colors=32, dither=Image.Dither.NONE).convert('RGBA')
        sprite.putalpha(alpha)
        # Every frame is centered and bottom-aligned with one clear edge pixel.
        frame = Image.new('RGBA', (size, size))
        frame.alpha_composite(sprite, ((size - sprite.width) // 2, size - sprite.height - 1))
        atlas.alpha_composite(frame, (index % 4 * CELL + (CELL - size) // 2, index // 4 * CELL + CELL - size))
    save_sprite(atlas, DEST)
    print(f'Saved {DEST} ({atlas.width} x {atlas.height})')


if __name__ == '__main__':
    main()
