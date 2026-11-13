#!/usr/bin/env python3
"""Ridgeshire's arcane lodge, built with the established pixelkit masonry pipeline."""
from local_buildings import ASSETS, build_monastery
from pixelkit import save_sprite

if __name__ == "__main__":
    save_sprite(build_monastery(runewright=True), ASSETS / "runewright-lodge.bmp")
