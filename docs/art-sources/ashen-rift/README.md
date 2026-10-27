# Ashen Hollow rift entrance

The native 168 × 144 entrance uses `tools/sprites/ashen.py` and the established
pixelkit pipeline. Jagged basalt spires surround a deep fissure, with branching
lava below the rim and a southern stairway aligned with the existing entrance.

- `entrance.png`: transparent native sprite; runtime asset is `assets/images/ashen-entrance.bmp`.
- `native-scale-review.png`: the new entrance beside the established blacksmith and giant hall, without resizing.
- `approach.png`: capture from the actual SDL renderer.
- `previous-entrance.png`: the former cinder-cone sprite for comparison.

The atlas label was edited with the built-in imagegen tool and saved as
`docs/images/world-map-giants-peaks.png`. Its final prompt was:

> Use case: text-localization. Edit the supplied finished fantasy world atlas by changing EXACTLY ONE text label. The western region immediately WEST of Rosemoor currently says "Giant's Crown". Replace that text with exactly "Giant's Peaks" in the same black medieval serif lettering, at the same location and size, on its existing pale parchment label. Change ONLY the letters in this one label. Preserve the entire surrounding picture, all geography, the ruins above/below the label, every road, border, title, compass, all towns and all other text. Especially preserve Ashen Hollow as the jagged glowing volcanic rift in the distant north and the clean dark menacing Castle of No Return below it with clear separation between them. Preserve Sunken Coast cove, branching Docks, Stonewake cliffs/beaches and every other feature exactly. Do not redraw or reinterpret the map. No extra text. Same 1536x1024 landscape image.
