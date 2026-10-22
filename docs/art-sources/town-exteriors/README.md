# Medieval town exterior sprites

Generate with `python3 tools/sprites/towns.py` or `make sprites`. Native
pixelkit stonework, textured plaster, timber beams, tiled roofs, windows and
doors match the existing building pipeline. No individual shop image is scaled
or pasted into a four-building square.

The user's photograph of a compact walled medieval hill town supplies the
rounded curtain wall, projecting towers, dense roof clusters and narrow streets.
Terracotta roofs predominate; Stillbury adds plum roofs, Ridgeshire uses slate,
stone facades and snow on the parapets, and each town has its own flag colors.

Runtime assets are `assets/images/<town>-town.bmp`, 408×360 pixels (17×15 native
24px tiles). These are entrance landmarks. Walk into a connected cardinal side
to visit; there are no exterior gate targets or portcullises. Gates inside the
actual town are for leaving. Existing regional connections remain unchanged.

`scale-comparison.png` places all five towns beside the existing Inn at native
scale. Each `<town>-preview.png` comes from the actual SDL game renderer.
`forest-road-preview.png` shows an open road continuation without the former
misleading town gate. Previews are reviewed for roof density, silhouette,
transparency, labels, entrance contact and bypass visibility.

See [town travel](../../town-roads.md) for navigation and save compatibility.
