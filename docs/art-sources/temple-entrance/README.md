# Ruined Temple landmark

Generate `assets/images/temple-entrance.bmp` with
`python3 tools/sprites/temple.py`, or `make sprites`. The 168×144 image uses
the established pixelkit stonework, outlines, ledges and native pixel scale.
Five stepped terraces, a roofless sun sanctuary, moon carving, hanging vines
and fallen masonry represent the whole temple. The former enlarged doorway
crop from `island-sprites.bmp` is no longer used for the temple.

The sprite occupies the central island clearing at tile (17, 9), spanning
7×6 tiles. Contact with any cardinal side enters the existing first temple
floor. Retreat returns to (20, 15), on the path beside Nahla. The camp,
island landmarks, docks, ship and Nahla's quest retain their positions and roles.

Save version 181 migrates active island maps in place, retires the two old
northern entrance tiles and preserves exploration, quests and temple caches.
Players or dropped items covered by the new footprint move to its southern
approach; unrelated island positions and loot remain in place. Saved temple
interiors retain their existing floors and use the new island exit on retreat.

`scale-comparison.png` shows the temple beside the Haunted Forest entrance and
existing Inn at native resolution. `game-preview.png` comes from the actual
SDL renderer. Both were inspected for scale, transparency, placement, the
whole-monument silhouette and the surrounding island paths.
