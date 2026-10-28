# Dawnmere Alehouse

Run `python3 tools/sprites/alehouse.py` to reproduce the runtime BMPs and source
PNGs. Artwork uses the established pixelkit stonework, shaded timber, textured
slate, plaster, windows and doorway helpers at 24 pixels per tile.

The 168×144 exterior follows the supplied timbered alehouse reference: two steep
gables, muted green plaster, a stone chimney, an arched oak door and barrel
signage. The single doorway remains at the ordinary human scale. The 48×24
interior atlas contains a tapped cask and a foaming mug in separate 24×24 cells.

- [Comparison beside the Tavern and Blacksmith](scale-comparison.png).
- [Actual Dawnmere renderer view](dawnmere.png).
- [Interior with cask shelves, tables and bar](interior.png).
- [Dain's quest offer](dain-dialogue.png).

Dain assigns and completes Recover the Treasure Map inside the Alehouse.
Its door opens onto the northeast edge of Dawnmere's plaza, and its interior
exit returns to the same doorstep. The existing Spice Merchant remains
independently accessible. NPC ground comes from the location's floor rules;
changing a character no longer substitutes grass for Dawnmere's paving.

Save version 215 repairs older Dawnmere maps in place, removes Dain's former
plaza tile and adds the Alehouse. Players and stacked dropped items inside the
new footprint move to the doorstep. Quest progress, rewards and exploration
remain intact; existing non-town expeditions do not need new state fields.

Validation covers real door travel, connected walkable interiors, Dain's offer
and one-time reward, interior save/load, older town migrations with stacked
loot, and native SDL rendering. Renderer checks place six NPC types on the
plaza and confirm that their transparent corners retain its cobblestones.
