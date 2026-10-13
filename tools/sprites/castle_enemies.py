#!/usr/bin/env python3
"""Import Castle enemies and boss phases at their native display sizes."""
from pathlib import Path

from PIL import Image

from pixelkit import save_sprite

ROOT = Path(__file__).resolve().parents[2]
SOURCES = ROOT / 'docs/art-sources/enemy-batch-16'
REGULAR_CROPS = (
    (30, 45, 480, 525),  # Oathbound Soldier
    (480, 45, 1050, 525),  # Iron Warden
    (1080, 85, 1536, 525),  # Royal Marksman
    (210, 525, 615, 1005),  # Court Hexer
    (630, 525, 1070, 1005),  # Bell Herald
)
BOSS_CROPS = (
    (0, 25, 555, 490),  # Castellan
    (590, 25, 1020, 490),  # Royal Arcanist
    (1060, 25, 1515, 490),  # Lord Veyr: armored
    (15, 520, 545, 980),  # Lord Veyr: damaged
    (550, 510, 1060, 980),  # Lord Veyr: spectral
)


def import_sheet(source, crops, frame_size, cell, destination):
    atlas = Image.new('RGBA', (cell * 3, cell * 2))
    for index, crop in enumerate(crops):
        sprite = source.crop(crop)
        bounds = sprite.getchannel('A').point(lambda a: 255 if a >= 128 else 0).getbbox()
        if bounds is None:
            raise ValueError(f'Empty Castle sprite {index} in {destination}')
        sprite = sprite.crop(bounds)
        width, height = frame_size
        sprite.thumbnail((width - 2, height - 2), Image.Resampling.NEAREST)
        alpha = sprite.getchannel('A').point(lambda a: 255 if a >= 128 else 0)
        sprite = sprite.convert('RGB').quantize(colors=32, dither=Image.Dither.NONE).convert('RGBA')
        sprite.putalpha(alpha)
        frame = Image.new('RGBA', frame_size)
        frame.alpha_composite(sprite, ((width - sprite.width) // 2, height - sprite.height - 1))
        atlas.alpha_composite(frame, (index % 3 * cell + (cell - width) // 2, index // 3 * cell + cell - height))
    save_sprite(atlas, destination)
    print(f'Saved {destination} ({atlas.width} x {atlas.height})')


def main():
    regular = Image.open(SOURCES / 'regular-source.png').convert('RGBA')
    bosses = Image.open(SOURCES / 'boss-source.png').convert('RGBA')
    import_sheet(regular, REGULAR_CROPS, (24, 26), 32, ROOT / 'assets/images/castle-enemies.bmp')
    import_sheet(bosses, BOSS_CROPS, (36, 36), 40, ROOT / 'assets/images/castle-bosses.bmp')


if __name__ == '__main__':
    main()
